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


template<typename ThreadModel, typename Functor>
class Throughput {
public:
    Throughput(Functor &&func, size_t iters, size_t minThreadsArg, size_t maxThreadsArg, size_t sec) : Throughput(
            std::move(func), iters, minThreadsArg, maxThreadsArg, sec, false) {
    }

    Throughput(Functor &&func, size_t iters, size_t thrds, size_t sec) : Throughput(std::move(func), iters, thrds,
                                                                                    thrds, sec, false) {
    }

    Throughput(Functor &&func, size_t sec) : Throughput(std::move(func), 10, 1,
                                                        std::max(1u, std::thread::hardware_concurrency() / 2), sec,
                                                        true) {
    }

    void Start() {
        config.shouldStart = true;
        if (!config.selfTuning) {
            config.shouldCollect = true;
        }
        while (!config.shouldStop) {
            std::this_thread::yield();
        }
    }

    void Stop() {
        statFile.close();
        StopExecutionThreads();
        config.shouldStop = true;
        statThread.join();
        if (config.selfTuning && tuneThread.joinable()) {
            tuneThread.join();
        }
    }

private:
    Throughput(Functor &&func, size_t iters, size_t minThreadsArg, size_t maxThreadsArg, size_t sec, bool tune)
            : iterations(iters), functor(std::move(func)) {
        config.seconds = sec;
        config.minThreads = static_cast<uint32_t>(minThreadsArg);
        config.maxThreads = static_cast<uint32_t>(maxThreadsArg);
        config.selfTuning = tune;
        CreateExecutionThreads(config.minThreads);
        statThread = std::thread([this]() {
            try {
                while (!config.shouldCollect) {
                    std::this_thread::yield();
                }
                while (!config.shouldStop) {
                    Collect();
                }
            } catch (const std::exception &ex) {
                std::cout << "Stats thread has thrown exception. Reason: " << ex.what() << std::endl;
            } catch (...) {
                std::cout << "Stats thread has thrown unknown exception." << std::endl;
            }
        });
        if (config.selfTuning) {
            tuneThread = std::thread([this]() {
                while (!config.shouldStart) {
                    std::this_thread::yield();
                }
                SelfTune(true);
            });
        }
        statFile.open(GetISODT() + ".csv");
    }

    void Execute() {
        uint64_t iters = iterations;
        for (size_t i = 0; i < iters; ++i) {
            auto then = std::chrono::high_resolution_clock::now();
            ReportIOSize(functor());
            auto now = std::chrono::high_resolution_clock::now();
            ReportTime(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count()));
        }
    }

    void CreateExecutionThreads(size_t thrds) {
        try {
            config.shouldStart = false;
            config.shouldStopExecution = false;
            for (size_t i = 0; i < thrds; ++i) {
                executionThreads.emplace_back(std::make_unique<ThreadModel>([this]() {
                    while (!config.shouldStart) {
                        std::this_thread::yield();
                    }
                    while (!config.shouldStopExecution) {
                        Execute();
                    }
                }));
            }
            config.shouldStart = true;
        } catch (const std::exception &ex) {
            std::cout << "Stats thread has thrown exception. Reason: " << ex.what() << std::endl;
        } catch (...) {
            std::cout << "Stats thread has thrown unknown exception." << std::endl;
        }
    }

    void StopExecutionThreads() {
        config.shouldStopExecution = true;
        for (auto &thread:executionThreads) {
            thread->join();
        }
        executionThreads.clear();
    }

    void SelfTune(bool fastReaction) {
        bool firstRun = true;
        bool shouldStabilize = false;
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
            if (!shouldStabilize) {
                if (stats.completedOps > 0 && elapsed > 0) {
                    firstRun = false;
                    uint64_t completed = stats.completedOps.exchange(0);
                    double newIOPS = completed / elapsed * 1'000'000;
                    TuneIterations(iterationTuningResolution, elapsed, completed);
                    CreateExecutionThreads(1);
                    shouldStabilize = true;
                    ++config.runs;
                    stats.rollingMean(int64_t(newIOPS));
                    auto rm = boost::accumulators::rolling_mean(stats.rollingMean);
                    //std::cout << "Rolling mean: " << rm << " Rolling mean diff: " << std::abs(rm - (newIOPS)) << std::endl;
                    stats.Reset(false);
                    if (std::abs(rm - newIOPS) < (newIOPS * 0.01) && config.runs > 10) {
                        config.minThreads = 1;
                        config.maxThreads = executionThreads.size();
                        stats.Reset(true);
                        std::cout << "Test tuned to run at maximum in " << config.maxThreads << " threads."
                                  << std::endl;
                        StopExecutionThreads();
                        CreateExecutionThreads(1);
                        config.shouldCollect = true;
                        config.runs = 0;
                        break;
                    }
                }
            } else {
                shouldStabilize = false;
                stats.Reset(false);
            }
        }
    }

    void TuneIterations(uint64_t iterationTuningResolution, double elapsed, uint64_t completed) {
        auto iters = iterations.load();
        auto tunedIters = std::max(1., (1. / (stats.averageLat.load() / completed)) * iterationTuningResolution);
        if (std::abs(iters - tunedIters) > iters * 1.1) {
            iterations = tunedIters;
            std::cout << "Number of iterations tuned to " << iterations.load() << std::endl;
        }
    }

    void Collect() {
        auto then = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(config.seconds));
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - then).count();
        if (stats.completedOps > 0 && elapsed > 0) {
            uint64_t thrput = stats.throughput.exchange(0);
            uint64_t completed = stats.completedOps.exchange(0);
            TuneIterations(1'000'000'000, elapsed, completed);

            auto IOPS = completed / elapsed * 1'000'000;
            std::string literal;
            std::cout << "Running in " << executionThreads.size() << " threads." << std::endl;
            std::cout << "Min latency: "
                      << TimeScale(stats.minLat.exchange(std::numeric_limits<uint64_t>::max()), literal) << literal
                      << " Max latency: " << TimeScale(stats.maxLat.exchange(0), literal) << literal
                      << " Average latency: " << TimeScale(stats.averageLat.exchange(0) / completed, literal) << literal
                      << std::endl;
            std::cout << "Combined throughput: " << ByteScale(thrput / elapsed * 1'000'000, literal) << literal;
            std::cout << " Combined IOPS: " << IOPS << " of " << ByteScale(thrput / completed, literal) << literal
                      << " operations" << std::endl << std::endl;
            statFile << IOPS << "," << executionThreads.size() << std::endl;
            ++config.runs;
            stats.Reset(false);
            if (config.selfTuning || config.minThreads != config.maxThreads) {
                if (executionThreads.size() < config.maxThreads) {
                    CreateExecutionThreads(1);
                } else {
                    config.shouldStop = true;
                }
            } else {
                stats.rollingMean(int64_t(IOPS));
                auto rm = boost::accumulators::rolling_mean(stats.rollingMean);
                if (std::abs(rm - IOPS) < (IOPS * 0.01) && config.runs > 10) {
                    config.shouldStop = true;
                }
            }
        }
    }

    void ReportIOSize(uint64_t size) {
        ++stats.completedOps;
        stats.throughput += size;
    }

    void ReportTime(uint64_t duration) {
        stats.minLat = std::min(stats.minLat.load(), duration);
        stats.maxLat = std::max(stats.maxLat.load(), duration);
        stats.averageLat += duration;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    double TimeScale(T ns, std::string &literal) {
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
            default:
                throw std::runtime_error("Failed to scale a time.");
        }
        return retVal;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    double ByteScale(T bytes, std::string &literal) {
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
            default:
                throw std::runtime_error("Failed to scale a size.");
        }
        return retVal;
    }

//    double StdDev(const std::vector<double> &input) {
//
//        double sum = std::accumulate(input.begin(), input.end(), 0.0);
//        double mean = sum / input.size();
//
//        std::vector<double> diff(input.size());
//        std::transform(input.begin(), input.end(), diff.begin(), [mean](double x) { return x - mean; });
//        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
//        return std::sqrt(sq_sum / input.size());
//    }

    std::string GetISODT() {
        time_t rawtime;
        struct tm *timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", timeinfo);
        return std::string(buffer);
    }


    using RollingMean = boost::accumulators::accumulator_set<int64_t, boost::accumulators::stats<boost::accumulators::tag::rolling_mean>>;

    struct TestStats {
        TestStats() : rollingMean(boost::accumulators::tag::rolling_window::window_size = 10) {}

        void Reset(bool resetMean) {
            minLat = std::numeric_limits<uint64_t>::max();
            maxLat = 0;
            averageLat = 0;
            throughput = 0;
            completedOps = 0;
            if (resetMean) {
                rollingMean = RollingMean(boost::accumulators::tag::rolling_window::window_size = 10);
            }
        }

        std::atomic<uint64_t> minLat{std::numeric_limits<uint64_t>::max()};
        std::atomic<uint64_t> maxLat{0};
        std::atomic<uint64_t> averageLat{0};
        std::atomic<uint64_t> throughput{0};
        std::atomic<uint64_t> completedOps{0};
        RollingMean rollingMean;
    };

    struct TestConfig {
        size_t seconds = 1;
        size_t runs = 0;
        size_t minThreads = 1;
        size_t maxThreads = 1;
        bool shouldStart = false;
        bool shouldCollect = false;
        bool shouldStop = false;
        bool selfTuning = false;
        bool shouldStopExecution = false;
    };

    std::ofstream statFile;
    Functor functor;
    std::thread statThread;
    std::thread tuneThread;
    std::vector<std::unique_ptr<ThreadModel>> executionThreads;
    std::atomic<uint64_t> iterations{1};
    TestStats stats;
    TestConfig config;
};