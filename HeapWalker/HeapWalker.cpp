// HeapWalker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "heap_walker.h"

#define KB_SIZE 1024

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 2)
	{
		try
		{
			std::string process_id(argv[1]);
			DWORD proc_id = boost::lexical_cast<DWORD>(process_id);
			heap_walker_t walker(proc_id);
			//ofstream f("f:/output.csv");
			//walker.dump_csv_mem_data(f);
			walker.dump_mem_data(cout);
			//f.close();
		}
		catch (std::exception &ex)
		{
			cout << "Error: " << ex.what() << "." <<endl;
		}
		catch (...)
		{
			cout << "Error: Unknown error." << endl;
		}
	}
}
