// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <assert.h>
#include <exception>
#include <vector>
#include <sstream>
#include <fstream>
#include <Psapi.h>
#include <atlstr.h>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace boost::archive;
using namespace std;