// module_rebase.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#ifdef _M_IX86
ULONG_PTR g_dll_top_address = 0x67000000;
#elif _M_X64
ULONG_PTR g_dll_top_address = 0x0000006700000000;
#endif

BOOL report_error(CPath &tmp_module)
{
	DWORD err = GetLastError();
	if (0 != err)
	{
		LPTSTR pBuffer = NULL;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			0,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&pBuffer, 
			0, 
			0);
		_tprintf(_T("Module '%s' is not rebased. Reason: %s\n"), tmp_module.m_strPath, CString(pBuffer));
		LocalFree(pBuffer);
		return FALSE;
	}
	return TRUE;
}

BOOL module_rebase (CString &filename, ULONG_PTR &new_base_address)
{
	ULONG OldSize = 0, NewSize = 0;
	ULONG_PTR OldBase = 0, NewBase = 0;
	CString ansi_filename(filename);
	CPath tmp_path(ansi_filename);
	tmp_path.Canonicalize();
	tmp_path.RemoveFileSpec();
	BOOL bRetVal = ReBaseImage(
		(LPSTR)CT2CA(filename),
		(LPSTR)CT2CA(tmp_path.m_strPath),
		FALSE,
		FALSE,
		TRUE,
		0,
		&OldSize,
		&OldBase,
		&NewSize,
		&new_base_address,
		0);

	CPath tmp_module(filename);
	tmp_module.MakePretty();
	tmp_module.StripPath();
	// rumors say ReBaseImage may fail returning true, so lets check the GetLastError()
	if (report_error(tmp_module))
	{
		bRetVal = ReBaseImage(
			(LPSTR)CT2CA(filename),
			(LPSTR)CT2CA(tmp_path.m_strPath),
			TRUE,
			FALSE,
			TRUE,
			0,
			&OldSize,
			&OldBase,
			&NewSize,
			&new_base_address,
			0);

		if (report_error(tmp_module))
		{
			_tprintf(_T("Module '%s' is rebased from address 0x%p to address 0x%p\n"), tmp_module.m_strPath, OldBase + NewSize, new_base_address);
		}
	}
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 1)
	{
		_tprintf(_T("Change base address of PE Executables\n\tUsage:\tmodule_rebase <path>\n"));
		return 1;
	}
	_tprintf(_T("Looking for files to rebase."));
	_tfinddata_t fileinfo;
	ULONG_PTR new_base = g_dll_top_address;
	CPath target_file;
	CPath target_path(argv[1]);
	target_path.UnquoteSpaces();
	target_path.Canonicalize();
	target_path.MakePretty();

	CPath target_spec(target_path);

	// DLLs
	target_spec.Append(_T("*.dll"));
	intptr_t handle = _tfindfirst(target_spec.m_strPath, &fileinfo);
	target_file.Combine(target_path, fileinfo.name);
	CString full_file_name = target_file;
	if (handle != -1 && module_rebase(full_file_name, new_base))
	{
		while (-1 != _tfindnext(handle, &fileinfo))
		{
			target_file.Combine(target_path, fileinfo.name);
			full_file_name = CString(target_file);
			module_rebase(full_file_name, new_base);
		}
	}
	_findclose(handle);
	_tprintf(_T("Done rebasing DLLs."));
	return 0;

}

