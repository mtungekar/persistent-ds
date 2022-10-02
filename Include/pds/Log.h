// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"

#include <map>
#include <mutex>
#include <iostream>

namespace pds
	{
	class Log
		{
		public:
			static std::ostream &Error( const char *funcsig, const char *filename, int fileline ) 
				{ 
				filename;
				fileline;
				std::cout << "Error: " << funcsig << "\n" << "\t";
				return std::cout; 
				}

		};

	};