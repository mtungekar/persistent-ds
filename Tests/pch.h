// ISD Copyright (c) 2021 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/ISD/blob/main/LICENSE

#ifndef PCH_H
#define PCH_H

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

#include "TestHelpers\structure_generation.h"
#include "TestHelpers\random_vals.h"

const size_t global_number_of_passes = 1;

#ifdef _DEBUG
#define ISDExpectSanityCheckDebugFailMacro( statement ) Assert::ExpectException<std::exception>( [&]() { statement } );
#define ISDExpectSanityCheckCoreDebugFailMacro( statement ) Assert::ExpectException<std::exception>( [&]() { statement } );
#else
#define ISDExpectSanityCheckDebugFailMacro( statement ) 
#define ISDExpectSanityCheckCoreDebugFailMacro( statement ) 
#endif

#define STANDARD_TEST_INIT() \
	TEST_METHOD_INITIALIZE( InitMethod )\
		{\
		setup_random_seed();\
		}

#endif //PCH_H
