// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <gtest/gtest.h>

#include <pds/pds.h>

// silence warnings which we can't control
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4201 )
#pragma warning( disable : 4127 )
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

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

#include "TestHelpers/random_vals.h"

// set this to a higher number to run more passes where the values are randomized
const size_t global_number_of_passes = 1;
