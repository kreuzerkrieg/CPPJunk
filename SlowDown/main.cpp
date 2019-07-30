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

static fs::path createFolder()
{
    auto prefix = boost::uuids::random_generator()();
    fs::path folder_path = boost::uuids::to_string(prefix);
    folder_path /= "foo/bar/baz";
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
    try
    {
        Aws::SDKOptions options;
        options.loggingOptions.logger_create_fn = []() {
            return std::make_shared<Aws::Utils::Logging::DefaultLogSystem>(Aws::Utils::Logging::LogLevel::Debug, "test_");
        };
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
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
    return 0;
}