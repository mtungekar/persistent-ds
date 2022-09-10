// ISD Copyright (c) 2021 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/ISD/blob/main/LICENSE

#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EntityManagementTests
	{
	class section_array;

	class section_object
		{
		public:
			optional_value<std::string> object_name;
			mat4 object_transform = {};
			std::unique_ptr<section_object> sub;

			std::vector< std::unique_ptr<section_object> > vec;
			optional_idx_vector< std::unique_ptr<section_object> > opt_arr;

			void SetupRandom( int levels_left );
			uint CountItems() const;
			void Compare( const section_object *other ) const;
			bool Write( const MemoryWriteStream &ws, EntityWriter &ew );
			bool Read( const MemoryReadStream &rs, EntityReader &er );
		};

	void section_object::SetupRandom( int levels_left )
		{
		if( levels_left < 0 )
			return;

		if( random_value<bool>() )
			{
			object_name.set( random_value<std::string>() );
			}

		object_transform = random_value<mat4>();

		sub = std::make_unique<EntityManagementTests::section_object>();
		sub->SetupRandom( levels_left - 1 );

		random_vector<std::unique_ptr<section_object>>( this->vec, 0, 10 );
		for( size_t i = 0; i < vec.size(); ++i )
			{
			if( vec[i] ) { vec[i]->SetupRandom( levels_left - 1 ); }
			}

		random_optional_idx_vector<std::unique_ptr<section_object>>( this->opt_arr, 0, 10 );
		if( opt_arr.has_value() )
			{
			for( size_t i = 0; i < opt_arr.values().size(); ++i )
				{
				if( opt_arr.values()[i] ) { opt_arr.values()[i]->SetupRandom( levels_left - 1 ); }
				}
			}
		}

	uint section_object::CountItems() const
		{
		uint count = 1; // start with this item

		if( sub ) { count += sub->CountItems(); }

		for( size_t i = 0; i < this->vec.size(); ++i )
			{
			if( this->vec[i] ) { count += this->vec[i]->CountItems(); }
			}

		if( this->opt_arr.has_value() )
			{
			const auto &vals = opt_arr.values();
			for( size_t i = 0; i < vals.size(); ++i )
				{
				if( vals[i] ) { count += vals[i]->CountItems(); }
				}
			}

		return count;
		}

	void section_object::Compare( const section_object *other ) const
		{
		Assert::IsTrue( this->object_name.has_value() == other->object_name.has_value() );
		if( this->object_name.has_value() )
			{
			Assert::IsTrue( this->object_name.value() == other->object_name.value() );
			}

		Assert::IsTrue( this->object_transform == other->object_transform );
		Assert::IsTrue( (this->sub != nullptr) == (other->sub != nullptr) );
		if( this->sub != nullptr )
			{
			this->sub->Compare( other->sub.get() );
			}

		Assert::IsTrue( this->vec.size() == other->vec.size() );
		for( size_t i = 0; i < this->vec.size(); ++i )
			{
			Assert::IsTrue( (this->vec[i] != nullptr) == (other->vec[i] != nullptr) );
			if( this->vec[i] != nullptr )
				{
				this->vec[i]->Compare( other->vec[i].get() );
				}
			}

		Assert::IsTrue( this->opt_arr.has_value() == other->opt_arr.has_value() );
		if( this->opt_arr.has_value() )
			{
			Assert::IsTrue( this->opt_arr.values().size() == other->opt_arr.values().size() );
			for( size_t i = 0; i < this->opt_arr.values().size(); ++i )
				{
				Assert::IsTrue( (this->opt_arr.values()[i] != nullptr) == (other->opt_arr.values()[i] != nullptr) );
				if( this->opt_arr.values()[i] != nullptr )
					{
					this->opt_arr.values()[i]->Compare( other->opt_arr.values()[i].get() );
					}
				}
			Assert::IsTrue( this->opt_arr.index() == other->opt_arr.index() );
			}
		}

	bool section_object::Write( const MemoryWriteStream &ws, EntityWriter &ew )
		{
		Assert::IsTrue( ew.Write( "object_name", 11, object_name ) );

		Assert::IsTrue( ew.Write( "object_transform", 16, object_transform ) );

		if( sub )
			{
			EntityWriter *section_writer = ew.BeginWriteSection( "sub", 3 );
			Assert::IsTrue( section_writer != nullptr );
			if( section_writer )
				{
				Assert::IsTrue( sub->Write( ws, *section_writer ) );
				Assert::IsTrue( ew.EndWriteSection( section_writer ) );
				}
			}
		else
			{
			ew.WriteNullSection( "sub", 3 );
			}

		EntityWriter *section_array_writer = ew.BeginWriteSectionsArray( "vec", 3, vec.size() );
		Assert::IsTrue( section_array_writer != nullptr );
		if( section_array_writer )
			{
			for( size_t i = 0; i < vec.size(); ++i )
				{
				Assert::IsTrue( ew.BeginWriteSectionInArray( section_array_writer, i ) );
				if( vec[i] )
					{
					Assert::IsTrue( vec[i]->Write( ws, *section_array_writer ) );
					}
				Assert::IsTrue( ew.EndWriteSectionInArray( section_array_writer, i ) );
				}

			Assert::IsTrue( ew.EndWriteSectionsArray( section_array_writer ) );
			}

		if( opt_arr.has_value() )
			{
			EntityWriter *section_array_writer = ew.BeginWriteSectionsArray( "opt_arr", 7, opt_arr.values().size(), &opt_arr.index() );
			Assert::IsTrue( section_array_writer != nullptr );
			if( section_array_writer )
				{
				for( size_t i = 0; i < opt_arr.values().size(); ++i )
					{
					Assert::IsTrue( ew.BeginWriteSectionInArray( section_array_writer, i ) );
					if( opt_arr.values()[i] )
						{
						Assert::IsTrue( opt_arr.values()[i]->Write( ws, *section_array_writer ) );
						}
					Assert::IsTrue( ew.EndWriteSectionInArray( section_array_writer, i ) );
					}

				Assert::IsTrue( ew.EndWriteSectionsArray( section_array_writer ) );
				}
			}
		else
			{
			Assert::IsTrue( ew.WriteNullSectionsArray( "opt_arr", 7 ) );
			}

		return true;
		}

	bool section_object::Read( const MemoryReadStream &rs, EntityReader &er )
		{
		bool success = false;

		Assert::IsTrue( er.Read( "object_name", 11, object_name ) );

		Assert::IsTrue( er.Read( "object_transform", 16, object_transform ) );

		// read sub section
		EntityReader *section_reader = nullptr;
		std::tie( section_reader , success ) = er.BeginReadSection( "sub", 3, true );
		Assert::IsTrue( success );
		if( section_reader )
			{
			this->sub = std::make_unique<EntityManagementTests::section_object>();
			Assert::IsTrue( sub->Read( rs, *section_reader ) );
			Assert::IsTrue( er.EndReadSection( section_reader ) );
			}
		else
			{
			this->sub.reset();
			}

		// read vector
		size_t array_size = 0;
		EntityReader *section_array_reader = nullptr;
		std::tie( section_array_reader , array_size , success ) = er.BeginReadSectionsArray( "vec", 3, false );
		Assert::IsTrue( success );
		if( section_array_reader )
			{
			this->vec.resize( array_size );
			for( size_t i = 0; i < array_size; ++i )
				{
				bool section_has_data = false;
				Assert::IsTrue( er.BeginReadSectionInArray( section_array_reader, i, &section_has_data ) );
				if( section_has_data )
					{
					vec[i] = std::make_unique<EntityManagementTests::section_object>();
					Assert::IsTrue( vec[i]->Read( rs, *section_array_reader ) );
					}
				Assert::IsTrue( er.EndReadSectionInArray( section_array_reader, i ) );
				}
			Assert::IsTrue( er.EndReadSectionsArray( section_array_reader ) );
			}

		// read optional indexed vector
		opt_arr.set();
		std::tie( section_array_reader , array_size , success ) = er.BeginReadSectionsArray( "opt_arr", 7, true, &opt_arr.index() );
		Assert::IsTrue( success );
		if( section_array_reader )
			{
			this->opt_arr.values().resize( array_size );
			for( size_t i = 0; i < array_size; ++i )
				{
				bool section_has_data = false;
				Assert::IsTrue( er.BeginReadSectionInArray( section_array_reader, i, &section_has_data ) );
				if( section_has_data )
					{
					opt_arr.values()[i] = std::make_unique<EntityManagementTests::section_object>();
					Assert::IsTrue( opt_arr.values()[i]->Read( rs, *section_array_reader ) );
					}
				Assert::IsTrue( er.EndReadSectionInArray( section_array_reader, i ) );
				}

			Assert::IsTrue( er.EndReadSectionsArray( section_array_reader ) );
			}
		else
			{
			opt_arr.reset();
			}

		return true;
		}



	TEST_CLASS( SectionHierarchyReadWriteTests )
		{
		TEST_METHOD( TestEntitySectionWriterAndReadback )
			{
			setup_random_seed();

			for( uint pass_index = 0; pass_index < global_number_of_passes; ++pass_index )
				{
				std::stringstream ss;
				ss << "Pass #" << pass_index << "\n";
				Logger::WriteMessage( ss.str().c_str() );

				section_object my_hierarchy;
				my_hierarchy.SetupRandom( (int)capped_rand( 2, 5 ) );

				ss = std::stringstream();
				ss << "\tCreated hierarchy, size:" << my_hierarchy.CountItems() << "\n";
				Logger::WriteMessage( ss.str().c_str() );

				MemoryWriteStream ws;
				EntityWriter ew( ws );
				ws.SetFlipByteOrder( random_value<bool>() );

				Assert::IsTrue( my_hierarchy.Write( ws, ew ) );

				ss = std::stringstream();
				ss << "\tWrote hierarchy, size of stream: " << ws.GetPosition() << "\n";
				Logger::WriteMessage( ss.str().c_str() );

				MemoryReadStream rs( ws.GetData() , ws.GetSize() , ws.GetFlipByteOrder() );
				EntityReader er( rs );

				section_object readback_hierarchy;
				readback_hierarchy.Read( rs , er );

				uint a = readback_hierarchy.CountItems();
				uint b = my_hierarchy.CountItems();

				Assert::IsTrue( a == b );

				readback_hierarchy.Compare( &my_hierarchy );
				}
			}
		};
	}

	// implement the random value function for std::unique_ptr<EntityManagementTests::section_object>
	template<> std::unique_ptr<EntityManagementTests::section_object> random_value< std::unique_ptr<EntityManagementTests::section_object> >()
		{
		if( random_value<bool>() )
			{
			return std::make_unique<EntityManagementTests::section_object>();
			}
		return std::unique_ptr<EntityManagementTests::section_object>( nullptr );
		}
