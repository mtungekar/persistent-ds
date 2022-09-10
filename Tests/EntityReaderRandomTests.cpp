// ISD Copyright (c) 2021 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/ISD/blob/main/LICENSE


#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MemoryStreamTests
	{
	TEST_CLASS( RandomFileDataReadTest )
		{
		STANDARD_TEST_INIT()

		TEST_METHOD( Test_EntityReader_with_random_file_fuzzing_expect_no_exceptions )
			{
			for( uint pass_index = 0; pass_index < global_number_of_passes; ++pass_index )
				{
				std::vector<u8> random_file_data;
				random_vector( random_file_data, 10000, 50000 );

				MemoryReadStream rs( random_file_data.data(), random_file_data.size(), (pass_index & 0x1) != 0 );
				EntityReader er( rs );

				// try reading some values. calls are allowed to fail with error, but not crash
				for( size_t i = 0; i < 1000; ++i )
					{
					i32 v;
					er.Read( "i", 1, v );
					std::vector<i8> dv;
					er.Read( "dv", 2, dv );
					std::string ds;
					er.Read( "ds", 2, ds );
					}
				}
			}

		};
	}
