// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BasicEntitiesTests
	{
	TEST_CLASS( ItemTableTests )
		{
		template<class _Kty> void ItemTableBasicTests_Validation()
			{
			// check with no validation
			if( true )
				{
				typedef ItemTable<_Kty, TestEntity, ItemTableFlags::NullEntities | ItemTableFlags::ZeroKeys> Dict;
				Dict dict;

				EntityValidator validator;
				const _Kty zero = data_type_information<_Kty>::zero;

				// add a zero key  with a null value entry
				dict.Entries()[zero] = std::unique_ptr<TestEntity>();
				Assert::IsTrue( dict.Entries().size() == 1 );

				// check validation, this should fail
				Assert::IsTrue( Dict::MF::Validate( dict, validator ) );
				Assert::IsTrue( validator.GetErrorCount() == 0 );
				}

			// check zero key validation
			if( true )
				{
				typedef ItemTable<_Kty, TestEntity, ~ItemTableFlags::ZeroKeys> Dict;
				Dict dict;

				EntityValidator validator;
				const _Kty zero = data_type_information<_Kty>::zero;

				// add a zero key entry (invalid)
				dict.Entries()[zero] = std::make_unique<TestEntity>();
				Assert::IsTrue( dict.Entries().size() == 1 );

				// check validation, this should fail
				Assert::IsTrue( Dict::MF::Validate( dict, validator ) );
				Assert::IsTrue( validator.GetErrorCount() == 1 );
				}

			// check null value validation
			if( true )
				{
				typedef ItemTable<_Kty, TestEntity, ~ItemTableFlags::NullEntities> Dict;
				Dict dict;

				EntityValidator validator;
				const _Kty inf_val = data_type_information<_Kty>::inf;

				// add a null ptr value entry (invalid)
				dict.Entries()[inf_val] = std::unique_ptr<TestEntity>();
				Assert::IsTrue( dict.Entries().size() == 1 );

				// check validation, this should fail
				Assert::IsTrue( Dict::MF::Validate( dict, validator ) );
				Assert::IsTrue( validator.GetErrorCount() == 1 );
				}

			// check all validations
			if( true )
				{
				typedef ItemTable < _Kty, TestEntity, ~(ItemTableFlags::NullEntities | ItemTableFlags::ZeroKeys) > Dict;
				Dict dict;

				EntityValidator validator;

				// add a number of values, but skip zero values
				size_t cnt = capped_rand( 100, 300 );
				for( size_t i = 0; i < cnt; ++i )
					{
					const _Kty rand_val = random_value<_Kty>();
					if( rand_val != data_type_information<_Kty>::zero )
						{
						dict.Entries()[rand_val] = std::make_unique<TestEntity>();
						}
					}

				// check validation, this should fail
				Assert::IsTrue( Dict::MF::Validate( dict, validator ) );
				Assert::IsTrue( validator.GetErrorCount() == 0 );
				}

			// test copying and moving contents of one dictionary to another 
			if( true )
				{
				typedef ItemTable<_Kty, TestEntity> Dict;
				Dict dict;

				// add a number of random values, record the pointers
				size_t cnt = capped_rand( 3, 10 );
				std::set<TestEntity *> ptrs;
				for( size_t i = 0; i < cnt; ++i )
					{
					const _Kty rand_val = random_value<_Kty>();
					dict.Entries()[rand_val] = std::make_unique<TestEntity>();
					dict.Entries()[rand_val]->Name() = random_value<string>();
					ptrs.insert( dict.Entries()[rand_val].get() );
					}
				size_t dict_size = dict.Size();

				// make a deep copy
				Dict dict_copy( dict );
				Assert::IsTrue( dict.Size() == dict_size );
				Assert::IsTrue( dict_copy.Size() == dict_size );
				Assert::IsTrue( dict_copy == dict );

				// make sure the pointers are different (so the copy was an actual copy, and not a move)
				for( const auto &p : dict_copy.Entries() )
					{
					Assert::IsTrue( ptrs.find( p.second.get() ) == ptrs.end() );
					}

				// move dictionary to another
				Dict dict_move = std::move( dict );
				Assert::IsTrue( dict.Size() == 0 );
				Assert::IsTrue( dict_move.Size() == dict_size );

				// make sure the pointers are still the same (so the move was an actual move, and not a copy)
				for( const auto &p : dict_move.Entries() )
					{
					Assert::IsTrue( ptrs.find( p.second.get() ) != ptrs.end() );
					}
				}

			}

		TEST_METHOD( ItemTableBasicTests )
			{
			ItemTableBasicTests_Validation<i8>();
			ItemTableBasicTests_Validation<i16>();
			ItemTableBasicTests_Validation<i32>();
			ItemTableBasicTests_Validation<i64>();
			
			ItemTableBasicTests_Validation<u8>();
			ItemTableBasicTests_Validation<u16>();
			ItemTableBasicTests_Validation<u32>();
			ItemTableBasicTests_Validation<u64>();
			
			ItemTableBasicTests_Validation<float>();
			ItemTableBasicTests_Validation<double>();
			
			ItemTableBasicTests_Validation<uuid>();
			ItemTableBasicTests_Validation<item_ref>();
			ItemTableBasicTests_Validation<hash>();
			ItemTableBasicTests_Validation<entity_ref>();
			ItemTableBasicTests_Validation<string>();
			}

		template<class T> void ItemTableReadWriteTests_TestKeyType( const MemoryWriteStream &ws, EntityWriter &ew )
			{
			typedef ItemTable<T, TestEntity> Dict;

			Dict random_dict;

			// create random dictionary with random entries (half of them null)
			GenerateRandomItemTable<Dict>( random_dict );

			EntityValidator validator;

			// validate
			Assert::IsTrue( Dict::MF::Validate( random_dict , validator ) );

			// write dictionary to stream
			u64 start_pos = ws.GetPosition();
			Assert::IsTrue( Dict::MF::Write( random_dict , ew ) );

			// set up a temporary entity reader 
			MemoryReadStream rs( ws.GetData(), ws.GetSize(), ws.GetFlipByteOrder() );
			EntityReader er( rs );
			rs.SetPosition( start_pos );
			
			// read back the dictionary 
			Dict readback_dict;
			Assert::IsTrue( Dict::MF::Read( readback_dict , er ) );
			
			// validate
			Assert::IsTrue( Dict::MF::Validate( readback_dict , validator ) );
			Assert::IsTrue( validator.GetErrorCount() == 0 );

			// compare the values in the registries
			Assert::IsTrue( random_dict.Entries().size() == readback_dict.Entries().size() );
			Dict::iterator it1 = random_dict.Entries().begin();
			while( it1 != random_dict.Entries().end() )
				{
				Dict::iterator it2 = readback_dict.Entries().find( it1->first );
				Assert::IsTrue( it2 != readback_dict.Entries().end() );

				bool has_1 = it1->second != nullptr;
				bool has_2 = it2->second != nullptr;
			
				Assert::IsTrue( has_1 == has_2 );
				if( has_1 )
					{
					Assert::IsTrue( it1->second->Name() == it2->second->Name() );
					}
			
				++it1;
				}
			}

		TEST_METHOD( ItemTableReadWriteTests )
			{
			setup_random_seed();

			for( uint pass_index=0; pass_index<(2*global_number_of_passes); ++pass_index )
				{
				MemoryWriteStream ws;
				EntityWriter ew( ws );

				ws.SetFlipByteOrder( (pass_index & 0x1) != 0 );

				// log the pass
				std::stringstream ss;
				ss << "Pass #" << (pass_index / 2)+1 << " ";
				if( ws.GetFlipByteOrder() )
					ss << "Testing flipped byte order\n";
				else
					ss << "Testing native byte order\n";
				Logger::WriteMessage(ss.str().c_str());

				ItemTableReadWriteTests_TestKeyType<i8>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<i16>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<i32>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<i64>( ws, ew );
				
				ItemTableReadWriteTests_TestKeyType<u8>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<u16>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<u32>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<u64>( ws, ew );
				
				ItemTableReadWriteTests_TestKeyType<float>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<double>( ws, ew );
								
				ItemTableReadWriteTests_TestKeyType<uuid>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<item_ref>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<hash>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<entity_ref>( ws, ew );
				ItemTableReadWriteTests_TestKeyType<string>( ws, ew );
				}
			}

		};
	}
