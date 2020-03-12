#include "Storage.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <random>
#include <unistd.h>
#include <vector>

int main()
{

    std::string source_file = "./source.bin";
    std::string dest_file = "./destination.bin";
    auto source = open(source_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> source_size(2 * 1024 * 1024, 10 * 1024 * 1024);
    auto size = source_size(gen);

    std::seed_seq seq{static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()), size};
    std::vector<std::uint32_t> seeds(size);
    seq.generate(seeds.begin(), seeds.end());
    write(source, seeds.data(), seeds.size() * sizeof(decltype(seeds)::value_type));
    close(source);

    std::cout << "Sequential read test." << std::endl;
    {
        ReadLeveledStorage storage(Storage{.path = dest_file, .priority = 100}, Storage{.path = source_file, .priority = 50});

        auto fd = open(source_file.c_str(), O_RDONLY);

        std::vector<char> orig_buffer(1024);
        std::vector<char> buff(1024);

        ssize_t res = 1;
        while(res > 0)
        {
            res = storage.read(buff.data(), buff.size());
            read(fd, orig_buffer.data(), orig_buffer.size());
            if(memcmp(buff.data(), orig_buffer.data(), orig_buffer.size()) != 0)
            {
                std::cout << "Buffer comparison failed." << std::endl;
                return 1;
            }
        }
        close(fd);
        storage.printExtents();
        storage.printStats();
    }
    std::filesystem::remove(dest_file);

    std::cout << "Random read test." << std::endl;
    {
        ReadLeveledStorage storage(Storage{.path = dest_file, .priority = 100}, Storage{.path = source_file, .priority = 50});

        auto fd = open(source_file.c_str(), O_RDONLY);

        std::vector<char> orig_buffer(1024);
        std::vector<char> buff(1024);

        std::uniform_int_distribution<size_t> random_position(0, std::filesystem::file_size(source_file) - 1);
        ssize_t res = 1;

        size_t counter = 0;
        for(auto i = 0; i < 1'000'000; ++i)
        {
            ++counter;
            std::uniform_int_distribution<> random_buff_size(0, buff.size());
            auto pos = random_position(gen);
            auto buff_size = random_buff_size(gen);

            storage.lseek(pos, SEEK_SET);
            res = storage.read(buff.data(), buff_size);
            lseek(fd, pos, SEEK_SET);
            read(fd, orig_buffer.data(), buff_size);
            if(memcmp(buff.data(), orig_buffer.data(), buff_size) != 0)
            {
                std::cout << "Buffer comparison failed." << std::endl;
                return 1;
            }
        }
        close(fd);
        storage.printExtents();
        storage.printStats();
        std::cout << counter << " random reads performed." << std::endl;
    }
    std::filesystem::remove(dest_file);

    std::cout << "Extent persistancy test." << std::endl;
    {
        ReadLeveledStorage storage(Storage{.path = dest_file, .priority = 100}, Storage{.path = source_file, .priority = 50});

        std::vector<char> buff(1024);
        storage.lseek(1024, SEEK_SET);
        auto res = storage.read(buff.data(), buff.size());
        storage.lseek(5 * 1024, SEEK_SET);
        res = storage.read(buff.data(), buff.size());
        storage.lseek(8 * 1024, SEEK_SET);
        res = storage.read(buff.data(), buff.size());
        storage.lseek(10 * 1024, SEEK_SET);
        res = storage.read(buff.data(), buff.size());
        storage.printExtents();
        storage.printStats();
    }
    std::cout << "Extens saved. Reloading" << std::endl;
    {
        ReadLeveledStorage storage(Storage{.path = dest_file, .priority = 100}, Storage{.path = source_file, .priority = 50});

        storage.printExtents();
        storage.printStats();
    }
    std::filesystem::remove(dest_file);
    std::filesystem::remove(source_file);

    return 0;
}
