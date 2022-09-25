// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <gtest/gtest.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <pds/pds.h>
#include <pds/MemoryReadStream.h>
#include <pds/MemoryWriteStream.h>
#include <pds/EntityWriter.h>
#include <pds/EntityReader.h>
#include <pds/DirectedGraph.h>
#include <pds/EntityValidator.h>
#include <pds/ItemTable.h>
#include <pds/SHA256.h>
#include <pds/DynamicTypes.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace pds;

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <algorithm>

//#include "TestHelpers\structure_generation.h"
#include "TestHelpers\random_vals.h"

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
