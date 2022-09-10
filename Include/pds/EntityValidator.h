// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <pds/pds.h>

namespace pds
	{
	struct ValidationError 
		{
		static const u64 NoError		= 0x00; 
		static const u64 InvalidCount	= 0x01;	// an invalid size of lists etc
		static const u64 NullNotAllowed	= 0x02;	// an object is empty/null, and this is not allowed in the type
		static const u64 MissingObject	= 0x04;	// a required object is missing
		static const u64 InvalidObject	= 0x08;	// an object is invalid or used in an invalid way
		static const u64 InvalidSetup	= 0x10;	// the set up of an object or system is invalid 
		static const u64 InvalidValue	= 0x20;	// a value or index is out of bounds or not allowed
		};

	class EntityValidator
		{
		uint ErrorCount = 0;
		u64 ErrorIds = 0;

		public:
			std::ostream &ReportError( u64 errorid , const char *funcsig, const char *filename, int fileline ) 
				{ 
				std::cout << "Validation error: errorid=" << errorid << "\n" << "\t";
				++this->ErrorCount;
				this->ErrorIds |= errorid;
				return std::cout; 
				}

			void ClearErrorCount() { this->ErrorCount = 0; }
			uint GetErrorCount() const { return this->ErrorCount; }
			u64 GetErrorIds() const { return this->ErrorIds; }
		};
	};