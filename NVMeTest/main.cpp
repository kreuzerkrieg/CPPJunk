#include "NVMeManager.h"
#include <iostream>

int main()
{
	std::string transport = "trtype:PCIe traddr:0000:00:1e.0";
	NVMeManager manager(transport);
	std::cout << "Size: " << manager.GetSize() * manager.GetBlockSize() / 1024 / 1024 << "MiB" << std::endl;
	return 0;
}