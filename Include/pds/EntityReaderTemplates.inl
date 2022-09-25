// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <pds/DataValuePointers.h>

// value_type: the ValueType enum to read the block as
// object_type: the C++ object that stores the data (can be a basic type), such as u32, or glm::vec3
// item_type: the actual basic type that stores the data, int the case of glm::vec3, it is a float
// item_count: the count of the the basic type, again in the case of glm::vec3, the count is 3

namespace pds
	{
	enum class reader_status
		{
		fail, // general fail, (or value is empty, but empty is not allowed for this value)
		success_empty, // success, value is empty
		success // success, has value
		};

	// read the header of a large block
	// returns the stream position of the expected end of the block, to validate the read position
	// a stream position of 0 is not possible, and indicates error
	inline u64 begin_read_large_block( MemoryReadStream &sstream, ValueType VT, const char *key, const u8 key_size_in_bytes )
		{
		const u64 start_pos = sstream.GetPosition();
		pdsSanityCheckDebugMacro( key_size_in_bytes <= EntityMaxKeyLength ); // max key length

		// read and make sure we have the correct value type
		const u8 value_type = sstream.Read<u8>();
		if( value_type != (u8)VT )
			{
			// not the expected type
			pdsErrorLog << "The type in the input stream:" << (u32)value_type << " does not match expected type: " << (u32)VT << pdsErrorLogEnd;
			return 0;
			}

		// check the size, and calculate expected end position
		const u64 block_size = sstream.Read<u64>();
		const u64 expected_end_pos = sstream.GetPosition() + block_size;
		if( expected_end_pos > sstream.GetSize() )
			{
			// not the expected type
			pdsErrorLog << "The block size:" << block_size << " points beyond the end of the stream size" << pdsErrorLogEnd;
			return 0;
			}

		// read in the key length
		const u8 read_key_size_in_bytes = sstream.Read<u8>();
		if( read_key_size_in_bytes != key_size_in_bytes )
			{
			// not the expected type
			std::string expected_key_name( key, key_size_in_bytes );
			pdsErrorLog << "The size of the input key:" << (u32)read_key_size_in_bytes << " does not match expected size: " << (u32)key_size_in_bytes << " for key: \"" << expected_key_name << "\"" << pdsErrorLogEnd;
			return 0;
			}

		// read in the key data
		char read_key[EntityMaxKeyLength];
		sstream.Read( (i8 *)read_key, (u64)key_size_in_bytes );
		if( memcmp( key, read_key, (u64)key_size_in_bytes ) != 0 )
			{
			std::string expected_key_name( key, key_size_in_bytes );
			std::string read_key_name( read_key, key_size_in_bytes );
			pdsErrorLog << "Unexpected key name in the stream. Expected name: " << expected_key_name << " read name: " << read_key_name << pdsErrorLogEnd;
			return 0;
			}

		return expected_end_pos;
		}

	// ends the block, write the size of the block
	inline bool end_read_large_block( MemoryReadStream &sstream, u64 expected_end_pos )
		{
		const u64 end_pos = sstream.GetPosition();
		return (end_pos == expected_end_pos); // make sure we have read in the full block
		}

	// template method that Reads a small block of a specific ValueType VT to the stream. Since most value types 
	// can have different bit depths, the second parameter I is the actual type of the data stored. The data can have more than one values of type I, the count is stored in IC.
	template<ValueType VT, class T> inline reader_status read_single_item( MemoryReadStream &sstream, const char *key, const u8 key_size_in_bytes, const bool empty_value_is_allowed, T *dest_data )
		{
		static_assert((VT >= ValueType::VT_Bool) && (VT <= ValueType::VT_Hash), "Invalid type for generic template of read_single_item");

		const u64 value_size = sizeof( data_type_information<T>::value_type );
		const u64 value_count = data_type_information<T>::value_count;

		// record start position, for validation
		const u64 start_pos = sstream.GetPosition();

		// read u8 value_type, check the value
		const u8 value_type = sstream.Read<u8>();
		if( value_type != (u8)VT )
			{
			// not the expected type
			pdsErrorLog << "The type in the input stream:" << (u32)value_type << " does not match expected type: " << (u32)VT << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// calc the expected possible sizes. if empty value, the data size must be 0, else it is the expected size based on the item type (I) and count (IC)
		const u64 dest_data_size_in_bytes = value_size * value_count;
		const u64 expected_block_size_if_empty = key_size_in_bytes;
		const u64 expected_block_size = dest_data_size_in_bytes + expected_block_size_if_empty;

		pdsSanityCheckCoreDebugMacro( key_size_in_bytes <= EntityMaxKeyLength );
		pdsSanityCheckCoreDebugMacro( expected_block_size < 256 ); // must fit in a byte

		// read in size of the small block, if the size does not match the expected block size, check if empty value is ok (is_optional_value == true), and if not raise error
		// any size other than expected_block_size is regarded as empty, and we will check that size if empty is actually allowed
		const u64 block_size = sstream.Read<u8>();
		const bool is_empty_value = (block_size != expected_block_size) ? true : false;
		if( is_empty_value )
			{
			if( empty_value_is_allowed )
				{
				// if empty is allowed, make sure that we have the block size of an empty block
				if( block_size != expected_block_size_if_empty )
					{
					pdsErrorLog << "The size of the block in the input stream:" << block_size << " does not match expected possible sizes: " << expected_block_size_if_empty << " (if empty value) or " << expected_block_size << " (if non-empty) " << pdsErrorLogEnd;
					return reader_status::fail;
					}
				}
			else
				{
				// empty is not allowed, so regardless of the size, it is invalid, error out
				pdsErrorLog << "The size of the block in the input stream:" << block_size << " does not match expected size (empty is not allowed): " << expected_block_size << pdsErrorLogEnd;
				return reader_status::fail;
				}
			}

		// read in the value(s)
		if( !is_empty_value )
			{
			const u64 read_count = sstream.Read( value_ptr( *dest_data ), value_count );
			if( read_count != value_count )
				{
				pdsErrorLog << "Could not read all expected values from the input stream. Expected count: " << value_count << " read count: " << read_count << pdsErrorLogEnd;
				return reader_status::fail;
				}
			}

		// read in the key data
		char read_key[EntityMaxKeyLength];
		const u64 read_key_length = sstream.Read( (u8 *)read_key, (u64)key_size_in_bytes );
		if( read_key_length != (u64)key_size_in_bytes
			|| memcmp( key, read_key, (u64)key_size_in_bytes ) != 0 )
			{
			std::string expected_key_name( key, key_size_in_bytes );
			std::string read_key_name( read_key, key_size_in_bytes ); // cap string at lenght of expected data
			pdsErrorLog << "Unexpected key name in the stream. Expected name: " << expected_key_name << " read name: " << read_key_name << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// get the position beyond the end of the block, and validate position
		const u64 expected_end_pos = (is_empty_value) ? (start_pos + 2 + expected_block_size_if_empty) : (start_pos + 2 + expected_block_size);
		const u64 end_pos = sstream.GetPosition();
		pdsSanityCheckCoreDebugMacro( end_pos == expected_end_pos );
		if( end_pos != expected_end_pos )
			{
			pdsErrorLog << "Invaild position in the read stream. Expected position: " << expected_end_pos << " current position: " << end_pos << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// return success, either with value or empty
		if( is_empty_value )
			return reader_status::success_empty;
		return reader_status::success;
		};

	// special implementation of read_small_block for bool values, which reads a u8 and converts to bool
	template<> inline reader_status read_single_item<ValueType::VT_Bool, bool>( MemoryReadStream &sstream, const char *key, const u8 key_size_in_bytes, const bool empty_value_is_allowed, bool *dest_data )
		{
		u8 u8val;
		reader_status status = read_single_item<ValueType::VT_Bool, u8>( sstream, key, key_size_in_bytes, empty_value_is_allowed, &u8val );
		if( status != reader_status::fail )
			{
			(*dest_data) = (bool)u8val;
			}
		return status;
		};

	// template method that Reads a small block of a specific ValueType VT to the stream. Since most value types 
	// can have different bit depths, the second parameter I is the actual type of the data stored. The data can have more than one values of type I, the count is stored in IC.
	template<> inline reader_status read_single_item<ValueType::VT_String, string>( MemoryReadStream &sstream, const char *key, const u8 key_size_in_bytes, const bool empty_value_is_allowed, string *dest_data )
		{
		static_assert(sizeof( u64 ) == sizeof( size_t ), "Unsupported size_t, current code requires it to be 8 bytes in size, equal to u64");

		pdsSanityCheckCoreDebugMacro( dest_data );

		// read block header
		const u64 expected_end_position = begin_read_large_block( sstream, ValueType::VT_String, key, key_size_in_bytes );
		if( expected_end_position == 0 )
			{
			pdsErrorLog << "begin_read_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// if we are at the end of the block, we have an empty string
		if( sstream.GetPosition() == expected_end_position )
			{
			// marked as empty, check that this is allowed
			if( empty_value_is_allowed )
				{
				// empty value is allowed, early out if the block end checks out
				if( !end_read_large_block( sstream, expected_end_position ) )
					{
					pdsErrorLog << "End position of data " << sstream.GetPosition() << " does not equal the expected end position which is " << expected_end_position << pdsErrorLogEnd;
					return reader_status::fail;
					}

				// all good
				return reader_status::success_empty;
				}
			else
				{
				// empty is not allowed
				pdsErrorLog << "The read stream value is empty, which is not allowed for value \"" << key << "\"" << pdsErrorLogEnd;
				return reader_status::fail;
				}
			}

		// non-empty, read in the string size
		const u64 string_size = sstream.Read<u64>();

		// make sure the item count is plausible before allocating the vector
		const u64 expected_string_size = (expected_end_position - sstream.GetPosition());
		if( string_size > expected_string_size )
			{
			pdsErrorLog << "The string size in the stream is invalid, it is beyond the size of the value block" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// resize the destination string
		dest_data->resize( string_size );

		// read in the string data
		if( string_size > 0 )
			{
			i8 *p_data = (i8 *)&(dest_data->front());
			const u64 read_item_count = sstream.Read( p_data, string_size );
			if( read_item_count != string_size )
				{
				pdsErrorLog << "The stream could not read the whole string" << pdsErrorLogEnd;
				return reader_status::fail;
				}
			}

		// make sure we are at the expected end pos
		if( !end_read_large_block( sstream, expected_end_position ) )
			{
			pdsErrorLog << "End position of data " << sstream.GetPosition() << " does not equal the expected end position which is " << expected_end_position << pdsErrorLogEnd;
			return reader_status::fail;
			}

		return reader_status::success;
		}

	inline reader_status end_read_empty_large_block( MemoryReadStream &sstream, const char *key, const bool empty_value_is_allowed, const u64 expected_end_position )
		{
		// check that empty value this is allowed
		if( empty_value_is_allowed )
			{
			// empty value is allowed, early out if the block end checks out
			if( !end_read_large_block( sstream, expected_end_position ) )
				{
				pdsErrorLog << "End position of data " << sstream.GetPosition() << " does not equal the expected end position which is " << expected_end_position << pdsErrorLogEnd;
				return reader_status::fail;
				}

			// all good
			return reader_status::success_empty;
			}
		else
			{
			// empty is not allowed
			pdsErrorLog << "The read stream value is empty, which is not allowed for value \"" << key << "\"" << pdsErrorLogEnd;
			return reader_status::fail;
			}
		}

	// reads an array header and value size from the stream, and decodes into flags, then reads the index if one exists. 
	inline bool read_array_metadata_and_index( MemoryReadStream &sstream, size_t &out_per_item_size, size_t &out_item_count, const u64 block_end_position , std::vector<i32> *dest_index )
		{
		static_assert(sizeof( u64 ) <= sizeof( size_t ), "Unsupported size_t, current code requires it to be at least 8 bytes in size, equal to u64");

		const u64 start_position = sstream.GetPosition();
		u64 expected_end_position = start_position + sizeof( u16 ) + sizeof( u64 );

		const u16 array_flags = sstream.Read<u16>();
		out_per_item_size = (size_t)(array_flags & 0xff);
		const bool has_index = (array_flags & 0x100) != 0;
		const bool index_is_64bit = (array_flags & 0x200) != 0;

		// we don't support 64 bit index (yet)
		if( index_is_64bit )
			{
			pdsErrorLog << "The block has a 64 bit index, which is not supported" << pdsErrorLogEnd;
			return false;
			}

		// read in the item count
		out_item_count = (size_t)sstream.Read<u64>();

		// if we have an index, read it
		if( has_index )
			{
			// make sure we DO expect an index
			if( !dest_index )
				{
				pdsErrorLog << "Invalid array type: The stream type has an index, but the destination object does not." << pdsErrorLogEnd;
				return false;
				}

			// read in the size of the index
			pdsSanityCheckCoreDebugMacro( block_end_position >= sstream.GetPosition() );
			const u64 index_count = sstream.Read<u64>();
			const u64 maximum_possible_index_count = (block_end_position - sstream.GetPosition()) / sizeof( u32 );
			if( index_count > maximum_possible_index_count )
				{
				pdsErrorLog << "The index item count in the stream is invalid, it is beyond the size of the block" << pdsErrorLogEnd;
				return false;
				}

			// resize the dest vector
			dest_index->resize( index_count );

			// read in the data
			i32 *p_index_data = dest_index->data();
			sstream.Read( p_index_data, index_count );

			// modify the expected end position
			expected_end_position += sizeof( u64 ) + (index_count * sizeof( i32 ));
			}
		else
			{
			// make sure we do NOT expect an index
			if( dest_index )
				{
				pdsErrorLog << "Invalid array type: The stream type has does not have an index, but the destination object does." << pdsErrorLogEnd;
				return false;
				}
			}

		if( expected_end_position != sstream.GetPosition() )
			{
			pdsErrorLog << "Failed to read full array header from block." << pdsErrorLogEnd;
			return false;
			}

		return true;
		}

	template<ValueType VT, class T> inline reader_status read_array( MemoryReadStream &sstream, const char *key, const u8 key_size_in_bytes, const bool empty_value_is_allowed, std::vector<T> *dest_items, std::vector<i32> *dest_index )
		{
		static_assert((VT >= ValueType::VT_Array_Bool) && (VT <= ValueType::VT_Array_Hash), "Invalid type for generic read_array template");
		static_assert(sizeof( u64 ) >= sizeof( size_t ), "Unsupported size_t, current code requires it to be at max 8 bytes in size, equal to u64");
		const size_t value_size = sizeof( data_type_information<T>::value_type );

		pdsSanityCheckCoreDebugMacro( dest_items );

		// read block header. if we are already at the end, the block is empty, end the block and make sure empty is allowed
		const u64 block_end_position = begin_read_large_block( sstream, VT, key, key_size_in_bytes );
		if( block_end_position == 0 )
			{
			pdsErrorLog << "begin_read_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return reader_status::fail;
			}
		else if( block_end_position == sstream.GetPosition() )
			{
			return end_read_empty_large_block( sstream, key, empty_value_is_allowed, block_end_position );
			}

		// read item size & count and index if it exists, or make sure we do not expect an index
		size_t per_item_size = 0;
		size_t item_count = 0;
		if( !read_array_metadata_and_index( sstream, per_item_size, item_count, block_end_position, dest_index ) )
			{
			return reader_status::fail;
			}

		// make sure we have the right item size
		if( value_size != per_item_size )
			{
			pdsErrorLog << "The size of the items in the stream does not match the expected size" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// make sure the item count is plausible before allocating the vector
		const u64 maximum_possible_item_count = (block_end_position - sstream.GetPosition()) / value_size;
		if( item_count > maximum_possible_item_count )
			{
			pdsErrorLog << "The array item count in the stream is invalid, it is beyond the size of the block" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// resize the destination vector
		const u64 type_count = item_count / data_type_information<T>::value_count;
		dest_items->resize( type_count );

		// read in the data
		T *p_data = dest_items->data();
		const u64 read_item_count = sstream.Read( value_ptr( *p_data ), item_count );
		if( read_item_count != item_count )
			{
			pdsErrorLog << "The stream could not read all the items for the array" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// make sure we are at the expected end pos
		if( !end_read_large_block( sstream, block_end_position ) )
			{
			pdsErrorLog << "End position of data " << sstream.GetPosition() << " does not equal the expected end position which is " << block_end_position << pdsErrorLogEnd;
			return reader_status::fail;
			}

		return reader_status::success;
		}

	// read_array implementation for bool arrays (which need specific packing)
	template <> inline reader_status read_array<ValueType::VT_Array_Bool, bool>( MemoryReadStream &sstream, const char *key, const u8 key_size_in_bytes, const bool empty_value_is_allowed, std::vector<bool> *dest_items, std::vector<i32> *dest_index )
		{
		pdsSanityCheckCoreDebugMacro( dest_items );

		// read block header. if we are already at the end, the block is empty, end the block and make sure empty is allowed
		const u64 block_end_position = begin_read_large_block( sstream, ValueType::VT_Array_Bool, key, key_size_in_bytes );
		if( block_end_position == 0 )
			{
			pdsErrorLog << "begin_read_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return reader_status::fail;
			}
		else if( block_end_position == sstream.GetPosition() )
			{
			return end_read_empty_large_block( sstream, key, empty_value_is_allowed, block_end_position );
			}

		// read item size & count and index if it exists, or make sure we do not expect an index
		size_t per_item_size = 0;
		size_t bool_count = 0;
		if( !read_array_metadata_and_index( sstream, per_item_size, bool_count, block_end_position, dest_index ) )
			{
			return reader_status::fail;
			}

		// calculate the number of packed items.
		// round up, should the last u8 be not fully filled
		const u64 number_of_packed_u8s = (bool_count+7) / 8; 

		// make sure the item count is plausible before allocating the vector
		const u64 maximum_possible_item_count = (block_end_position - sstream.GetPosition());
		if( number_of_packed_u8s > maximum_possible_item_count )
			{
			pdsErrorLog << "The array item count in the stream is invalid, it is beyond the size of the block" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// resize the destination vector
		dest_items->resize( bool_count );

		// read in a u8 vector with the packed values
		std::vector<u8> packed_vec( number_of_packed_u8s );
		sstream.Read( packed_vec.data(), number_of_packed_u8s );

		// unpack the bool vector from the u8 vector
		for( u64 bool_index = 0; bool_index < bool_count; ++bool_index )
			{
			const size_t packed_index = bool_index >> 3; // bool_index / 8
			const size_t packed_subindex = bool_index & 0x7; // bool_index % 8
			(*dest_items)[bool_index] = ((packed_vec[packed_index]) & (1 << packed_subindex)) != 0;
			}

		// make sure we are at the expected end pos
		if( !end_read_large_block( sstream, block_end_position ) )
			{
			pdsErrorLog << "End position of data " << sstream.GetPosition() << " does not equal the expected end position which is " << block_end_position << pdsErrorLogEnd;
			return reader_status::fail;
			}

		return reader_status::success;
		}

	template<> inline reader_status read_array<ValueType::VT_Array_String, string>( MemoryReadStream &sstream, const char *key, const u8 key_size_in_bytes, const bool empty_value_is_allowed, std::vector<string> *dest_items, std::vector<i32> *dest_index )
		{
		static_assert(sizeof( u64 ) == sizeof( size_t ), "Unsupported size_t, current code requires it to be 8 bytes in size, equal to u64");

		pdsSanityCheckCoreDebugMacro( dest_items );

		// read block header. if we are already at the end, the block is empty, end the block and make sure empty is allowed
		const u64 block_end_position = begin_read_large_block( sstream, ValueType::VT_Array_String, key, key_size_in_bytes );
		if( block_end_position == 0 )
			{
			pdsErrorLog << "begin_read_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return reader_status::fail;
			}
		else if( block_end_position == sstream.GetPosition() )
			{
			return end_read_empty_large_block( sstream, key, empty_value_is_allowed, block_end_position );
			}

		// read item size & count and index if it exists, or make sure we do not expect an index
		size_t per_item_size = 0;
		size_t string_count = 0;
		if( !read_array_metadata_and_index( sstream, per_item_size, string_count, block_end_position, dest_index ) )
			{
			return reader_status::fail;
			}

		// make sure the item count is plausible before allocating the vector
		// (the size is assuming only empty strings, so only the size of the string size (sizeof(u64)) per string)
		const u64 maximum_possible_item_count = (block_end_position - sstream.GetPosition()) / sizeof(u64); 
		if( string_count > maximum_possible_item_count )
			{
			pdsErrorLog << "The array string count in the stream is invalid, it is beyond the size of the block" << pdsErrorLogEnd;
			return reader_status::fail;
			}

		// resize the destination vector
		dest_items->resize( string_count );

		// read in each string separately
		for( u64 string_index = 0; string_index < string_count; ++string_index )
			{
			string &dest_string = (*dest_items)[string_index];

			const u64 string_size = sstream.Read<u64>();

			// make sure the string is not outsize of possible size
			const u64 maximum_possible_string_size = (block_end_position - sstream.GetPosition()); 
			if( string_size > maximum_possible_string_size )
				{
				pdsErrorLog << "A string size in a string array in the stream is invalid, it is beyond the size of the block" << pdsErrorLogEnd;
				return reader_status::fail;
				}

			// setup the destination string, and read in the data
			dest_string.resize( string_size );
			if( string_size > 0 )
				{
				i8 *p_data = (i8 *)&(dest_string.front());
				const u64 read_item_count = sstream.Read( p_data, string_size );
				if( read_item_count != string_size )
					{
					pdsErrorLog << "The stream could not read one of the strings" << pdsErrorLogEnd;
					return reader_status::fail;
					}
				}
			}

		// make sure we are at the expected end pos
		if( !end_read_large_block( sstream, block_end_position ) )
			{
			pdsErrorLog << "End position of data " << sstream.GetPosition() << " does not equal the expected end position which is " << block_end_position << pdsErrorLogEnd;
			return reader_status::fail;
			}

		return reader_status::success;
		}

#ifdef PDS_MAIN_BUILD_FILE
	EntityReader::EntityReader( MemoryReadStream &_sstream ) : sstream( _sstream ) , end_position( _sstream.GetSize() )
		{
		}

	EntityReader::EntityReader( MemoryReadStream &_sstream , const u64 _end_position ) : sstream( _sstream ) , end_position( _end_position )
		{
		}

	// Read a section. 
	// If the section is null, the section is directly closed, nullptr+success is returned 
	// from BeginReadSection, and EndReadSection shall not be called.
	std::tuple<EntityReader *, bool> EntityReader::BeginReadSection( const char *key, const u8 key_length, const bool null_section_is_allowed )
		{
		if( this->active_subsection )
			{
			pdsErrorLog << "There is already an active subsection." << pdsErrorLogEnd;
			return std::tuple<EntityReader *, bool>( nullptr, false );
			}

		// read block header
		const u64 end_of_section = begin_read_large_block( sstream, ValueType::VT_Subsection, key, key_length );
		if( end_of_section == 0 )
			{
			pdsErrorLog << "begin_read_large_block() failed unexpectedly, stream is probably corrupted" << pdsErrorLogEnd;
			return std::tuple<EntityReader *, bool>( nullptr, false );
			}
		else if( end_of_section == sstream.GetPosition() )
			{
			if( end_read_empty_large_block( sstream, key, null_section_is_allowed, end_of_section ) == reader_status::fail )
				{
				return std::pair<EntityReader *, bool>( nullptr, false );
				}
			return std::tuple<EntityReader *, bool>( nullptr, true );
			}

		// allocate the subsection and return it to the caller to be used to read items in the subsection
		this->active_subsection = std::unique_ptr<EntityReader>( new EntityReader( this->sstream , end_of_section ) );
		return std::tuple<EntityReader *, bool>( this->active_subsection.get(), true );
		}

	bool EntityReader::EndReadSection( const EntityReader *section_reader )
		{
		if( section_reader != this->active_subsection.get() )
			{
			pdsErrorLog << "Invalid parameter section_reader, it does not match the internal expected value." << pdsErrorLogEnd;
			return false;
			}

		if( !end_read_large_block( this->sstream, this->active_subsection->end_position ) )
			{
			pdsErrorLog << "end_read_large_block failed unexpectedly, the stream is probably corrupted." << pdsErrorLogEnd;
			return false;
			}

		this->active_subsection.reset();
		this->active_subsection_end_pos = 0;
		return true;
		}

	// Build a sections array. 
	// If the section is null, the section array is directly closed, nullptr+success is returned 
	// from BeginReadSectionsArray, and EndReadSectionsArray shall not be called.
	std::tuple<EntityReader *, size_t, bool> EntityReader::BeginReadSectionsArray( const char *key, const u8 key_length, const bool null_section_array_is_allowed, std::vector<i32> *dest_index )
		{
		if( this->active_subsection )
			{
			pdsErrorLog << "There is already an active subsection." << pdsErrorLogEnd;
			return std::tuple<EntityReader *, size_t, bool>( nullptr, 0, false );
			}

		// read block header. if we are already at the end, the block is empty, end the block and make sure empty is allowed
		const u64 end_of_section = begin_read_large_block( sstream, ValueType::VT_Array_Subsection, key, key_length );
		if( end_of_section == 0 )
			{
			pdsErrorLog << "begin_read_large_block() failed unexpectedly, stream is probably corrupted" << pdsErrorLogEnd;
			return std::tuple<EntityReader *, size_t, bool>( nullptr, 0, false );
			}
		else if( end_of_section == sstream.GetPosition() )
			{
			if( end_read_empty_large_block( sstream, key, null_section_array_is_allowed, end_of_section ) == reader_status::fail )
				{
				return std::tuple<EntityReader *, size_t, bool>( nullptr, 0, false );
				}
			return std::tuple<EntityReader *, size_t, bool>( nullptr, 0, true );
			}

		// read item size & count and index if it exists, or make sure we do not expect an index
		size_t per_item_size = 0;
		if( !read_array_metadata_and_index( sstream, per_item_size, this->active_subsection_array_size, end_of_section, dest_index ) )
			{
			return std::tuple<EntityReader *, size_t, bool>( nullptr, 0, false );
			}
		this->active_subsection_index = ~0;

		// allocate the subsection and return it to the caller to be used to read items in the subsection
		this->active_subsection = std::unique_ptr<EntityReader>( new EntityReader( this->sstream , end_of_section ) );
		return std::tuple<EntityReader *, size_t, bool>( this->active_subsection.get(), this->active_subsection_array_size, true );
		}

	bool EntityReader::BeginReadSectionInArray( const EntityReader *sections_array_reader , const size_t section_index, bool *dest_section_has_data )
		{
		if( this->active_subsection.get() != sections_array_reader )
			{
			pdsErrorLog << "Synch error, currently not writing a subsection array" << pdsErrorLogEnd;
			return false;
			}
		if( (this->active_subsection_index+1) != section_index )
			{
			pdsErrorLog << "Synch error, incorrect subsection index" << pdsErrorLogEnd;
			return false;
			}
		if( section_index >= this->active_subsection_array_size )
			{
			pdsErrorLog << "Incorrect subsection index, out of array bounds" << pdsErrorLogEnd;
			return false;
			}

		this->active_subsection_index = section_index;
		const u64 section_size = sstream.Read<u64>();
		this->active_subsection_end_pos = sstream.GetPosition() + section_size;

		if( dest_section_has_data == nullptr )
			{
			// make sure that the section size is not empty
			if( section_size == 0 )
				{
				pdsErrorLog << "Section in array in stream is marked as null, but this is not allowed for the array it is read into" << pdsErrorLogEnd;
				return false;
				}
			}
		else
			{
			*dest_section_has_data = (section_size != 0);
			}

		return true;
		}

	bool EntityReader::EndReadSectionInArray( const EntityReader *sections_array_reader, const size_t section_index )
		{
		if( this->active_subsection.get() != sections_array_reader || this->active_subsection_index != section_index )
			{
			pdsErrorLog << "Synch error, currently not reading a subsection array, or incorrect section index" << pdsErrorLogEnd;
			return false;
			}

		const u64 end_pos = sstream.GetPosition();

		if( end_pos != this->active_subsection_end_pos )
			{
			pdsErrorLog << "The current subsection did not end where expected" << pdsErrorLogEnd;
			return false;
			}

		return true;
		}

	bool EntityReader::EndReadSectionsArray( const EntityReader *sections_array_reader )
		{
		if( this->active_subsection.get() != sections_array_reader )
			{
			pdsErrorLog << "Invalid parameter section_reader, it does not match the internal expected value." << pdsErrorLogEnd;
			return false;
			}
		if( (this->active_subsection_index+1) != this->active_subsection_array_size )
			{
			pdsErrorLog << "Synch error, the subsection index does not equal the end of the array" << pdsErrorLogEnd;
			return false;
			}

		if( !end_read_large_block( this->sstream, this->active_subsection->end_position ) )
			{
			pdsErrorLog << "end_read_large_block failed unexpectedly, the stream is probably corrupted." << pdsErrorLogEnd;
			return false;
			}

		this->active_subsection.reset();
		this->active_subsection_array_size = 0;
		this->active_subsection_index = ~0;
		this->active_subsection_end_pos = 0;
		return true;
		}

#endif//PDS_MAIN_BUILD_FILE

	};
