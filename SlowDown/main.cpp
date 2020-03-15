#include "FolderUploader.h"
#include "Utils.h"
#include <algorithm>
#include <atomic>
#include <aws/core/Aws.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <unistd.h>
#include <vector>

class PrefixGenerator
{
public:
    PrefixGenerator()
    {
        if constexpr(bucketize)
        {
            std::generate(virtual_buckets.begin(), virtual_buckets.end(), boost::uuids::random_generator());
        }
    }

    std::string generate() const
    {
        if constexpr(bucketize)
        {
            return boost::uuids::to_string(virtual_buckets[++counter % virtual_bucket_size]);
        }
        return boost::uuids::to_string(boost::uuids::random_generator()());
    }

private:
    static constexpr bool bucketize = false;
    mutable std::atomic_size_t counter{0};
    static constexpr size_t virtual_bucket_size = 32;
    std::vector<boost::uuids::uuid> virtual_buckets{virtual_bucket_size};
};

static void createFile(const std::string& name, size_t size)
{
    std::ofstream file(name, std::ios::binary | std::ios::trunc);
    std::vector<char> buffer(std::min(size, 1_MiB));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis;
    std::generate(buffer.begin(), buffer.end(), [&]() { return dis(gen); });

    size_t written = 0;
    while(written < size)
    {
        file.write(buffer.data(), buffer.size());
        written += buffer.size();
    }
}

static PrefixGenerator prefix_generator;
static fs::path createFolder()
{
    fs::path folder_path = prefix_generator.generate();
    folder_path /= "foo/bar/baz";
    folder_path /= prefix_generator.generate();
    fs::create_directories(folder_path);
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<size_t> small(10_KiB, 100_KiB);
    for(auto i = 0; i < 5; ++i)
    {
        fs::path file_name = folder_path;
        file_name /= std::to_string(std::hash<uint64_t>()(i)) + ".bin";
        createFile(file_name.c_str(), small(gen));
    }

    std::uniform_int_distribution<size_t> big(50_MiB, 200_MiB);
    for(auto i = 5; i < 10; ++i)
    {
        fs::path file_name = folder_path;
        file_name /= std::to_string(std::hash<uint64_t>()(i)) + ".bin";
        createFile(file_name.c_str(), big(gen));
    }
    return folder_path;
}

int main()
{
    std::set_terminate([]() {
        std::cout << "Unhandled exception was thrown!" << std::endl;
        std::abort();
    });
    try
    {
        Aws::SDKOptions options;
        options.loggingOptions.logger_create_fn = []() {
            return std::make_shared<Aws::Utils::Logging::DefaultLogSystem>(Aws::Utils::Logging::LogLevel::Error, "test_");
        };
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Error;
        Aws::InitAPI(options);
        const size_t num_of_threads = std::thread::hardware_concurrency() * 2;
        std::vector<std::thread> threads;
        for(auto i = 0ul; i < num_of_threads; ++i)
        {
            threads.emplace_back([]() {
                try
                {
                    while(true)
                    {
                        auto local_folder = createFolder();
                        try
                        {
                            FolderUploader uploader(local_folder, std::string("s3://ewzs3test/") + local_folder.c_str());
                            uploader.Upload();
                            fs::remove_all(local_folder);
                        }
                        catch(const std::exception& ex)
                        {
                            fs::remove_all(local_folder);
                            throw;
                        }
                    }
                }
                catch(const std::exception& ex)
                {
                    std::cout << "Exception thrown. Reason: " << ex.what() << std::endl;
                }
            });
        }
        for(auto& thread : threads)
        {
            thread.join();
        }
    }
    catch(const std::exception& ex)
    {
        std::cout << "Exception thrown. Reason: " << ex.what() << std::endl;
        fs::remove_all("foo/");
    }
    std::cout << "Somehow we managed to finish and preperly exit the program" << std::endl;
    return 0;
}