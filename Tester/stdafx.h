// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <atlconv.h>
#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
// TODO: reference additional headers your program requires here
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <assert.h>
#include <boost\archive\text_iarchive.hpp>
#include <boost\archive\text_oarchive.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\timer.hpp>

using namespace std;
using namespace boost;
using namespace boost::archive;

#ifdef ISO3166_EXPORTS
#define ISO3166_API __declspec(dllexport)
#else
#define ISO3166_API __declspec(dllimport)
#endif