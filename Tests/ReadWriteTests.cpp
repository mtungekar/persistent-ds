// ISD Copyright (c) 2021 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/ISD/blob/main/LICENSE


#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace MemoryStreamTests
	{
	TEST_CLASS( ReadWriteTests )
		{
		STANDARD_TEST_INIT()

		template<class T> void AsserteReadValueIs( MemoryReadStream *rs, T ref_value )
			{
			T val = rs->Read<T>();
			Assert::IsTrue( val == ref_value );
			}

		TEST_METHOD( MemoryWriteReadStream )
			{
			// run twice, one with flipped, one with non-flipped byte order
			for( uint pass_index = 0; pass_index < 2*global_number_of_passes; ++pass_index )
				{
				// random values
				const u8 u8val = u8_rand();
				const u16 u16val = u16_rand();
				const u32 u32val = u32_rand();
				const u64 u64val = u64_rand();
				const float f32val = float_rand();
				const double f64val = double_rand();
				const uuid id = uuid_rand();
				const hash hs = hash_rand();

				// set up a random order of the values to write and read
				const uint num_values = 1000;
				int order[num_values];

				// write random stuff to write stream
				u64 expected_size = 0;
				MemoryWriteStream *ws = new MemoryWriteStream();
				ws->SetFlipByteOrder( (pass_index & 0x1) != 0 );

				// log the pass
				std::stringstream ss;
				ss << "Pass #" << (pass_index / 2)+1 << " ";
				if( ws->GetFlipByteOrder() )
					ss << "Testing flipped byte order\n";
				else
					ss << "Testing native byte order\n";
				Logger::WriteMessage(ss.str().c_str());


				for( int i = 0; i < num_values; ++i )
					{
					int item_type = rand() % 8;
					order[i] = item_type;

					switch( item_type )
						{
						case 0:
							ws->Write( u8val ); expected_size += 1;
							break;
						case 1:
							ws->Write( u16val ); expected_size += 2;
							break;
						case 2:
							ws->Write( u32val ); expected_size += 4;
							break;
						case 3:
							ws->Write( u64val ); expected_size += 8;
							break;
						case 4:
							ws->Write( f32val ); expected_size += 4;
							break;
						case 5:
							ws->Write( f64val ); expected_size += 8;
							break;
						case 6:
							ws->Write( id ); expected_size += 16;
							break;
						case 7:
							ws->Write( hs ); expected_size += 32;
							break;
						}
					}

				// check the expected size
				Assert::IsTrue( expected_size == ws->GetSize() );

				// get the data, and set up a read stream
				std::vector<u8> memdata;
				memdata.resize( expected_size );
				memcpy( memdata.data(), ws->GetData(), expected_size );
				delete ws;
				ws = nullptr;

				// read back everything in the same order
				MemoryReadStream *rs = new MemoryReadStream( memdata.data(), expected_size );
				rs->SetFlipByteOrder( (pass_index & 0x1) != 0 );
				for( int i = 0; i < num_values; ++i )
					{
					int item_type = order[i];

					switch( item_type )
						{
						case 0:
							Assert::IsTrue( rs->Peek() == u8val ); // also test Peek functionality
							AsserteReadValueIs( rs, u8val );
							break;
						case 1:
							AsserteReadValueIs( rs, u16val );
							break;
						case 2:
							AsserteReadValueIs( rs, u32val );
							break;
						case 3:
							AsserteReadValueIs( rs, u64val );
							break;
						case 4:
							AsserteReadValueIs( rs, f32val );
							break;
						case 5:
							AsserteReadValueIs( rs, f64val );
							break;
						case 6:
							AsserteReadValueIs( rs, id );
							break;
						case 7:
							AsserteReadValueIs( rs, hs );
							break;
						}
					}

				Assert::IsTrue( rs->GetPosition() == expected_size );
				delete rs;
				rs = nullptr;
				}
			}

		};
	}