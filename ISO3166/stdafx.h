// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// TODO: reference additional headers your program requires here

#include <atlconv.h>
#include <stdio.h>
#include <tchar.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <iostream>
#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <assert.h>
#include <boost\serialization\access.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\lexical_cast.hpp>

using namespace std;
using namespace boost;

#ifdef ISO3166_EXPORTS
#define ISO3166_API __declspec(dllexport)
#else
#define ISO3166_API __declspec(dllimport)
#endif