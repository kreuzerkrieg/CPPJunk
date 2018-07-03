#pragma once

#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
#include <algorithm>

#include <iostream>
#include <numeric>
#include <cmath>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <fstream>


template<typename Functor>
class Throughput
{
public:
    Throughput(Functor&& func, size_t iters, size_t thrds, size_t sec) : Throughput(std::move(func), iters, thrds, sec,
                                                                                    false)
    {
    }

    Throughput(Functor&& func, size_t sec) : Throughput(std::move(func), 10,
                                                        std::max(1u, std::thread::hardware_concurrency() / 2), sec,
                                                        true)
    {
    }

    void Start()
    {
        shouldStart = true;
        if (!selfTuning) {
            shouldCollect = true;
        }
        while (!shouldStop) {
            std::this_thread::yield();
        }
    }

    void Stop()
    {
        statFile.close();
        StopExecutionThreads();
        shouldStop = true;
        statThread.join();
        if (selfTuning && tuneThread.joinable()) {
            tuneThread.join();
        }
    }

private:
    Throughput(Functor&& func, size_t iters, size_t thrds, size_t sec, bool tune) : rollingMean(
            boost::accumulators::tag::rolling_window::window_size = 10), functor(std::move(func)), iterations(iters),
                                                                                    seconds(sec), selfTuning(tune)
    {
        CreateExecutionThreads(thrds);
        statThread = std::thread([this]() {
            try {
                while (!shouldCollect) {
                    std::this_thread::yield();
                }
                while (!shouldStop) {
                    Collect();
                }
            } catch (const std::exception& ex) {

                std::cout << "Stats thread has thrown exception. Reason: " << ex.what() << std::endl;
//            } catch (const Exception& ex) {
//
//                std::cout << "Stats thread has thrown exception. Reason: " << ex.GetMessage() << std::endl;
            } catch (...) {
                std::cout << "Stats thread has thrown unknown exception." << std::endl;
            }
        });
        if (selfTuning) {
            tuneThread = std::thread([this]() {
                while (!shouldStart) {
                    std::this_thread::yield();
                }
                SelfTune(true);
            });
        }
        statFile.open(GetISODT() + ".csv");
    }

    void Execute()
    {
        uint64_t iters = iterations;
        for (size_t i = 0; i < iters; ++i) {
            auto then = std::chrono::high_resolution_clock::now();
            ReportIOSize(functor());
            auto now = std::chrono::high_resolution_clock::now();
            ReportTime(std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count());
        }
    }

    void CreateExecutionThreads(size_t thrds)
    {
        try {
            shouldStart = false;
            shouldStopExecution = false;
            for (size_t i = 0; i < thrds; ++i) {
                executionThreads.emplace_back(std::thread([this]() {
                    while (!shouldStart) {
                        std::this_thread::yield();
                    }
                    while (!shouldStopExecution) {
                        Execute();
                    }
                }));
            }
            shouldStart = true;
        } catch (const std::exception& ex) {

            std::cout << "Stats thread has thrown exception. Reason: " << ex.what() << std::endl;
//        } catch (const Exception& ex) {
//
//            std::cout << "Stats thread has thrown exception. Reason: " << ex.GetMessage() << std::endl;
        } catch (...) {
            std::cout << "Stats thread has thrown unknown exception." << std::endl;
        }
    }

    void StopExecutionThreads()
    {
        shouldStopExecution = true;
        for (auto& thread:executionThreads) {
            thread.join();
        }
        executionThreads.clear();
    }

    void SelfTune(bool fastReaction)
    {
        bool firstRun = true;
        while (true) {
            uint64_t iterationTuningResolution = 1'000'000'000;
            auto then = std::chrono::high_resolution_clock::now();
            if (firstRun) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } else {
                if (iterations > 100) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    iterationTuningResolution = 100'000'000;
                } else {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }

            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - then).count();
            if (completedOps > 0 && elapsed > 0) {
                firstRun = false;
                uint64_t completed = completedOps.exchange(0);
                double newIOPS = completed / elapsed * 1'000'000;
                TuneIterations(iterationTuningResolution, elapsed, completed);
                CreateExecutionThreads(1);

                ++runs;
                rollingMean(int64_t(newIOPS));
                auto rm = boost::accumulators::rolling_mean(rollingMean);
                //std::cout << "Rolling mean: " << rm << " Rolling mean diff: " << std::abs(rm - (newIOPS)) << std::endl;

                if (std::abs(rm - newIOPS) < (newIOPS * 0.01) && runs > 10) {
                    minThreads = 1;
                    maxThreads = executionThreads.size();
                    StopExecutionThreads();
                    CreateExecutionThreads(1);
                    minLat = std::numeric_limits<uint64_t>::max();
                    maxLat = 0;
                    averageLat = 0;
                    throughput = 0;
                    completedOps = 0;
                    lastIOPS = 0;
                    shouldCollect = true;
                    runs = 0;
                    rollingMean = RollingMean(boost::accumulators::tag::rolling_window::window_size = 10);
                    break;
                }
            }
        }
    }

    void TuneIterations(uint64_t iterationTuningResolution, double elapsed, uint64_t completed)
    {
        auto iters = iterations.load();
        auto tunedIters = std::max(1., (1. / (averageLat.load() / completed)) * iterationTuningResolution);
        if (std::abs(iters - tunedIters) > iters * 1.1) {
            iterations = tunedIters;
            std::cout << "Number of iterations tuned to " << iterations.load() << std::endl;
        }
    }

    void Collect()
    {
        auto then = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - then).count();
        if (completedOps > 0 && elapsed > 0) {
            uint64_t thrput = throughput.exchange(0);
            uint64_t completed = completedOps.exchange(0);
            TuneIterations(1'000'000'000, elapsed, completed);

            auto IOPS = completed / elapsed * 1'000'000;
            std::string literal;
            std::cout << "Running in " << executionThreads.size() << " threads." << std::endl;
            std::cout << "Min latency: " << TimeScale(minLat.exchange(std::numeric_limits<uint64_t>::max()), literal)
                      << literal << " Max latency: " << TimeScale(maxLat.exchange(0), literal) << literal
                      << " Average latency: " << TimeScale(averageLat.exchange(0) / completed, literal) << literal
                      << std::endl;
            std::cout << "Combined throughput: " << ByteScale(thrput / elapsed * 1'000'000, literal) << literal;
            std::cout << " Combined IOPS: " << IOPS << " of " << ByteScale(thrput / completed, literal) << literal
                      << " operations" << std::endl << std::endl;
            statFile << IOPS << "," << executionThreads.size() << std::endl;
            ++runs;
            if (selfTuning) {
                if (executionThreads.size() < maxThreads) {
                    CreateExecutionThreads(1);
                } else {
                    shouldStop = true;
                }
            } else {
                rollingMean(int64_t(IOPS));
                auto rm = boost::accumulators::rolling_mean(rollingMean);
                if (std::abs(rm - IOPS) < (IOPS * 0.01) && runs > 10) {
                    shouldStop = true;
                }
            }
        }
    }

    void ReportIOSize(uint64_t size)
    {
        ++completedOps;
        throughput += size;
    }

    void ReportTime(uint64_t duration)
    {
        minLat = std::min(minLat.load(), duration);
        maxLat = std::max(maxLat.load(), duration);
        averageLat += duration;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    double TimeScale(T ns, std::string& literal)
    {
        uint8_t counter = 0;
        double retVal = double(ns);
        while (retVal > 1000 && counter < 3) {
            retVal /= 1000;
            ++counter;
        }
        switch (counter) {
            case 0:
                literal = "ns.";
                break;
            case 1:
                literal = "us.";
                break;
            case 2:
                literal = "ms.";
                break;
            case 3:
                literal = "s.";
                break;
        }
        return retVal;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    double ByteScale(T bytes, std::string& literal)
    {
        uint8_t counter = 0;
        double retVal = double(bytes);
        while (retVal > 1024 && counter < 4) {
            retVal /= 1024;
            ++counter;
        }
        switch (counter) {
            case 0:
                literal = "bytes.";
                break;
            case 1:
                literal = "KiB.";
                break;
            case 2:
                literal = "MiB.";
                break;
            case 3:
                literal = "GiB.";
                break;
            case 4:
                literal = "TiB.";
                break;
        }
        return retVal;
    }

    double StdDev(const std::vector<double>& input)
    {

        double sum = std::accumulate(input.begin(), input.end(), 0.0);
        double mean = sum / input.size();

        std::vector<double> diff(input.size());
        std::transform(input.begin(), input.end(), diff.begin(), [mean](double x) { return x - mean; });
        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        return std::sqrt(sq_sum / input.size());
    }

    std::string GetISODT()
    {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", timeinfo);
        return buffer;
    }

    using RollingMean = boost::accumulators::accumulator_set<int64_t, boost::accumulators::stats<boost::accumulators::tag::rolling_mean>>;
    std::ofstream statFile;
    RollingMean rollingMean;
    Functor functor;
    std::thread statThread;
    std::thread tuneThread;
    std::vector<double> IOPSStats;
    std::vector<std::thread> executionThreads;
    std::atomic<uint64_t> iterations{1};
    std::atomic<uint64_t> minLat{std::numeric_limits<uint64_t>::max()};
    std::atomic<uint64_t> maxLat{0};
    std::atomic<uint64_t> averageLat{0};
    std::atomic<uint64_t> throughput{0};
    std::atomic<uint64_t> completedOps{0};
    uint64_t lastIOPS = 0;
    uint32_t seconds = 1;
    uint32_t runs = 0;
    uint32_t minThreads = 1;
    uint32_t maxThreads = 1;
    bool shouldStart = false;
    bool shouldCollect = false;
    bool shouldStop = false;
    bool selfTuning = false;
    bool shouldStopExecution = false;
};