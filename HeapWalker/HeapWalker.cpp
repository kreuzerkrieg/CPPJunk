// HeapWalker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "heap_walker.h"

#define KB_SIZE 1024

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD proc_id = GetCurrentProcessId();
	HANDLE hPrivate = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 2*KB_SIZE,64*1024);
	if (hPrivate == NULL)
	{
		return 1;
	}
	else
	{
		LPVOID lpFirstBlock = HeapAlloc(hPrivate,HEAP_ZERO_MEMORY,2 * KB_SIZE);
		char *characters = new char[4*KB_SIZE];
		ZeroMemory(characters, 4*KB_SIZE);
		LPVOID lpSecondBlock = HeapAlloc(hPrivate,HEAP_ZERO_MEMORY,16 * KB_SIZE);
		void *mem = malloc(8*KB_SIZE);
		ZeroMemory(mem, 8*KB_SIZE);
		heap_walker_t walker;
		//walker.dump_mem_data(proc_id, L"f:\\dump.mem");
		walker.dump_mem_data(2164, L"f:\\dump.mem");
		delete [] characters;
		free(mem);
		return 0;
	}
}
