// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <gtest/gtest.h>

// allow constant conditionals in all tests
#pragma warning( disable : 4127 )

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <pds/pds.h>

// include glm,  silence warnings we can't control
#pragma warning( push )
#pragma warning( disable : 4201 )
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#pragma warning( pop )

using namespace pds;

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "TestHelpers/random_vals.h"

const size_t global_number_of_passes = 1;

//#ifdef _DEBUG
//#define pdsExpectSanityCheckDebugFailMacro( statement ) Assert::ExpectException<std::exception>( [&]() { statement } );
//#define pdsExpectSanityCheckCoreDebugFailMacro( statement ) Assert::ExpectException<std::exception>( [&]() { statement } );
//#else
//#define pdsExpectSanityCheckDebugFailMacro( statement ) 
//#define pdsExpectSanityCheckCoreDebugFailMacro( statement ) 
//#endif

//#define STANDARD_TEST_INIT() \
//	TEST_METHOD_INITIALIZE( InitMethod )\
//		{\
//		setup_random_seed();\
//		}
