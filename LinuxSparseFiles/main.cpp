#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    int file;
    int mode;

    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    file = open("sparsefile", O_WRONLY | O_CREAT, mode);
    if(file == -1)
    {
        return -1;
    }
    ftruncate(file, 0x100000);
    close(file);

    return 0;
}