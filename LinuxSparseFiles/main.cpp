#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>

int main()
{
    int file;

    file = open("sparsefile", O_WRONLY | O_CREAT | O_TRUNC);
    if(file == -1)
    {
        return -1;
    }
    ftruncate(file, 1024 * 1024);
    close(file);
    file = open("sparsefile", O_WRONLY);
    if(file == -1)
    {
        return -1;
    }
    std::cout << "Seek hole at 0: " << lseek(file, 0, SEEK_HOLE) << std::endl;
    std::cout << "Seek hole at 5 * 1024: " << lseek(file, 5 * 1024, SEEK_HOLE) << std::endl;
    std::cout << "Seek data at 0: " << lseek(file, 0, SEEK_DATA) << std::endl;
    std::cout << "Seek data at 5 * 1024: " << lseek(file, 5 * 1024, SEEK_DATA) << std::endl;

    lseek(file, 5 * 1024, SEEK_SET);
    std::cout << "File offset set to 5 * 1024: " << std::endl;
    std::vector<char> buff(1024, 'A');
    write(file, buff.data(), buff.size());
    std::cout << buff.size() << " bytes written to the file" << std::endl;

    std::cout << "Seek hole at 0: " << lseek(file, 0, SEEK_HOLE) << std::endl;
    std::cout << "Seek hole at 5 * 1024: " << lseek(file, 5 * 1024, SEEK_HOLE) << std::endl;
    std::cout << "Seek data at 0: " << lseek(file, 0, SEEK_DATA) << std::endl;
    std::cout << "Seek data at 5 * 1024: " << lseek(file, 5 * 1024, SEEK_DATA) << std::endl;

    std::cout << "Seek hole at 5 * 1024 + 10: " << lseek(file, 5 * 1024 + 10, SEEK_HOLE) << std::endl;
    std::cout << "Seek data at 5 * 1024 + 10: " << lseek(file, 5 * 1024 + 10, SEEK_DATA) << std::endl;
    close(file);

    return 0;
}