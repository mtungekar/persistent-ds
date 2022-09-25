// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include <pds/DataValuePointers.h>

namespace pds
	{
	// called to begin a large block
	// returns the stream position of the start of the block, to be used when writing the size when ending the block
	inline bool begin_write_large_block( MemoryWriteStream &dstream, ValueType VT, const char *key, const u8 key_size_in_bytes )
		{
		const u8 value_type = (u8)VT;
		const u64 start_pos = dstream.GetPosition();
		pdsSanityCheckDebugMacro( key_size_in_bytes <= EntityMaxKeyLength ); 

		// sizeof(value_type)=1 + sizeof(block_size)=8 + sizeof(key_size_in_bytes)=1 + key_size_in_bytes;
		const u64 expected_end_pos = start_pos + key_size_in_bytes + 10; 

		// write block header 
		// write empty stand in value for now (MAXi64 on purpose), which is definitely 
		// wrong, as to trigger any test if the value is not overwritten with the correct value
		dstream.Write( value_type );
		dstream.Write( (u64)MAXINT64 );
		dstream.Write( key_size_in_bytes );
		dstream.Write( (i8*)key, key_size_in_bytes );

		const u64 end_pos = dstream.GetPosition();
		pdsSanityCheckCoreDebugMacro( end_pos == expected_end_pos );
		return (end_pos == expected_end_pos);
		}

	// ends the block
	// writes the size of the block in the header of the block
	inline bool end_write_large_block( MemoryWriteStream &dstream, u64 start_pos )
		{
		const u64 end_pos = dstream.GetPosition();
		const u64 block_size = end_pos - start_pos - 9; // total block size - ( sizeof( valuetype )=1 + sizeof( block_size_variable )=8 )
		dstream.SetPosition( start_pos + 1 ); // skip over the valuetype
		dstream.Write( block_size );
		dstream.SetPosition( end_pos ); // move back the where we were
		return (end_pos > start_pos); // only thing we really can check
		}

	// template method that writes a small block of a specific ValueType VT to the stream. Since most value types 
	// can have different bit depths, the second parameter I is the actual type of the data stored. The data can have more than one values of type I, the count is stored in IC.
	template<ValueType VT, class T> inline bool write_single_value( MemoryWriteStream &dstream, const char *key, const u8 key_length, const T *data )
		{
		static_assert((VT >= ValueType::VT_Bool) && (VT <= ValueType::VT_Hash), "Invalid type for general write_single_value template");

		const u8 value_type = (u8)VT;
		const size_t value_size = sizeof( data_type_information<T>::value_type );
		const size_t value_count = data_type_information<T>::value_count;
		const size_t data_size_in_bytes = (data != nullptr) ? (value_size * value_count) : 0; // if data == nullptr, the block is empty
		const size_t block_size = data_size_in_bytes + key_length;
		pdsSanityCheckDebugMacro( key_length <= EntityMaxKeyLength ); // max key length
		pdsSanityCheckCoreDebugMacro( block_size < 256 ); // must fit in a byte
		const u8 u8_block_size = (u8)(block_size);
		const u64 start_pos = dstream.GetPosition();
		const u64 expected_end_pos = start_pos + 2 + block_size;

		// write data block 
		dstream.Write( value_type );
		dstream.Write( u8_block_size );
		if( data_size_in_bytes > 0 )
			{
			const data_type_information<T>::value_type *pvalue = value_ptr( (*data) );
			dstream.Write( pvalue, value_count );
			}
		dstream.Write( (i8*)key, key_length );

		const u64 end_pos = dstream.GetPosition();
		pdsSanityCheckCoreDebugMacro( end_pos == expected_end_pos );
		return (dstream.GetPosition() == expected_end_pos);
		};

	// specialization of write_single_value for bool values, which need conversion to u8
	template<> inline bool write_single_value<ValueType::VT_Bool, bool>( MemoryWriteStream &dstream, const char *key, const u8 key_length, const bool *data )
		{
		// if data is set, convert to an u8 value, and point at it
		const u8 u8val = ( data ) ? ((u8)(*data)) : 0;
		const u8 *p_data = ( data ) ? (&u8val) : nullptr;
		return write_single_value<ValueType::VT_Bool, u8>( dstream, key, key_length, p_data );
		}

	// specialization of write_single_value for strings
	template<> inline bool write_single_value<ValueType::VT_String, std::string>( MemoryWriteStream &dstream, const char *key, const u8 key_size_in_bytes, const std::string *string_value )
		{
		// record start position, we need this in the end block
		const u64 start_pos = dstream.GetPosition();

		// begin a large block
		if( !begin_write_large_block( dstream, ValueType::VT_String, key, key_size_in_bytes ) )
			{
			pdsErrorLog << "begin_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// record start of the array data, for error check
		const u64 string_data_start_pos = dstream.GetPosition();

		// if empty value (not same as empty string, which just has size 0)
		if( !string_value )
			{
			// empty string, early out 
			if( !end_write_large_block( dstream, start_pos ) )
				{
				pdsErrorLog << "end_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
				return false;
				}
			return true;
			}

		// write the size of the string, and the actual string values
		const u64 character_count = u64( string_value->size() );
		const u64 values_size = character_count + sizeof( u64 );
		dstream.Write( character_count );
		if( character_count > 0 )
			{
			const i8 *data = (const i8*)string_value->data();
			dstream.Write( data, character_count );
			}

		// make sure all data was written
		const u64 expected_end_pos = string_data_start_pos + values_size;

		// end the block by going back to the start and writing the start position offset
		if( !end_write_large_block( dstream, start_pos ) )
			{
			pdsErrorLog << "end_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// make sure we are at the expected end pos
		const u64 end_pos = dstream.GetPosition();
		if( end_pos != expected_end_pos )
			{
			pdsErrorLog << "End position of data " << end_pos << " does not equal the expected end position which is " << expected_end_pos << pdsErrorLogEnd;
			return false;
			}

		// succeeded
		return true;
		}

	// reads an array header and value size from the stream, and decodes into flags, then reads the index if one exists. 
	inline bool write_array_metadata_and_index( MemoryWriteStream &dstream, size_t per_item_size, size_t item_count, const std::vector<i32> *index )
		{
		static_assert(sizeof( u64 ) <= sizeof( size_t ), "Unsupported size_t, current code requires it to be at least 8 bytes in size, equal to u64");
		pdsSanityCheckDebugMacro( per_item_size <= 0xff );

		const u64 start_pos = dstream.GetPosition();

		// indexed array flags: size of each item (if need to decode array outside regular decoding) and bit set if index is used 
		const u16 has_index = (index) ? (0x100) : (0);
		const u16 index_is_64bit = 0; // we do not support 64 bit indices yet
		const u16 array_flags = has_index | index_is_64bit | u16(per_item_size);
		dstream.Write( array_flags );

		// write the number of items
		dstream.Write( u64(item_count) );

		// if we have an index, write it 
		u64 index_size = 0;
		if( index != nullptr )
			{
			const u64 index_count = index->size();
			dstream.Write( index_count );
			dstream.Write( index->data(), index_count );

			index_size = (index_count * sizeof( i32 )) + sizeof( u64 ); // the index values and the value count
			}

		// make sure all data was written
		const u64 expected_end_pos = 
			start_pos
			+ sizeof( u16 ) // the flags
			+ sizeof( u64 ) // the item count
			+ index_size; // the (optional) index

		const u64 end_pos = dstream.GetPosition();
		if( end_pos != expected_end_pos )
			{
			pdsErrorLog << "End position of data " << end_pos << " does not equal the expected end position which is " << expected_end_pos << pdsErrorLogEnd;
			return false;
			}

		return true;
		}

	// write indexed array to stream
	template<ValueType VT, class T> inline bool write_array( MemoryWriteStream &dstream, const char *key, const u8 key_size_in_bytes, const std::vector<T> *items, const std::vector<i32> *index )
		{
		static_assert((VT >= ValueType::VT_Array_Bool) && (VT <= ValueType::VT_Array_Hash), "Invalid type for write_array");
		static_assert(sizeof( data_type_information<T>::value_type ) <= 0xff, "Invalid value size, cannot exceed 255 bytes");
		static_assert(sizeof( u64 ) >= sizeof( size_t ), "Unsupported size_t, current code requires it to be at most 8 bytes in size, equal to an u64"); // assuming sizeof(u64) >= sizeof(size_t)
		const size_t value_size = sizeof( data_type_information<T>::value_type );
		const size_t values_per_type = data_type_information<T>::value_count;

		// record start position, we need this in the end block
		const u64 start_pos = dstream.GetPosition();

		// begin a large block
		if( !begin_write_large_block( dstream, VT, key, key_size_in_bytes ) )
			{
			pdsErrorLog << "begin_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}
		
		// write data if we have it
		if( items )
			{
			const u64 values_count = items->size() * values_per_type;
			if( !write_array_metadata_and_index( dstream, value_size, values_count, index ) )
				{
				return false;
				}
			
			// write the values
			if( values_count > 0 )
				{
				const data_type_information<T>::value_type *p_values = value_ptr( *(items->data()) );

				const u64 values_expected_end_pos = dstream.GetPosition() + (values_count * value_size);
				dstream.Write( p_values , values_count );
				const u64 values_end_pos = dstream.GetPosition();

				// make sure all were written
				if( values_end_pos != values_expected_end_pos )
					{
					pdsErrorLog << "End position of data " << values_end_pos << " does not equal the expected end position which is " << values_expected_end_pos << pdsErrorLogEnd;
					return false;
					}
				}
			}

		// end the block by going back to the start and writing the size of the payload
		if( !end_write_large_block( dstream, start_pos ) )
			{
			pdsErrorLog << "end_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// succeeded
		return true;
		}

	// specialization of write_array for bool arrays
	template<> inline bool write_array<ValueType::VT_Array_Bool, bool>( MemoryWriteStream &dstream, const char *key, const u8 key_size_in_bytes, const std::vector<bool> *items, const std::vector<i32> *index )
		{
		// record start position, we need this in the end block
		const u64 start_pos = dstream.GetPosition();

		// begin a large block
		if( !begin_write_large_block( dstream, ValueType::VT_Array_Bool, key, key_size_in_bytes ) )
			{
			pdsErrorLog << "begin_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// write data if we have it
		if( items )
			{
			// write the item count and items
			if( !write_array_metadata_and_index( dstream, 0, items->size(), index ) )
				{
				return false;
				}

			if( items->size() > 0 )
				{
				const u64 number_of_packed_u8s = (items->size()+7) / 8; 

				// pack the bool vector to a temporary u8 vector
				// round up, should the last u8 be not fully filled
				std::vector<u8> packed_vec( number_of_packed_u8s );
				for( size_t bool_index = 0; bool_index < items->size(); ++bool_index )
					{
					if( (*items)[bool_index] )
						{
						const size_t packed_index = bool_index >> 3; // bool_index / 8
						const size_t packed_subindex = bool_index & 0x7; // bool_index % 8
						packed_vec[packed_index] |= 1 << packed_subindex;
						}
					}

				// write u8 vector to stream
				const u64 values_expected_end_pos = dstream.GetPosition() + number_of_packed_u8s;
				dstream.Write( packed_vec.data(), number_of_packed_u8s );
				const u64 values_end_pos = dstream.GetPosition();

				// make sure all were written
				if( values_end_pos != values_expected_end_pos )
					{
					pdsErrorLog << "End position of data " << values_end_pos << " does not equal the expected end position which is " << values_expected_end_pos << pdsErrorLogEnd;
					return false;
					}
				}
			}

		// end the block by going back to the start and writing the start position offset
		if( !end_write_large_block( dstream, start_pos ) )
			{
			pdsErrorLog << "end_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// succeeded
		return true;
		}

	// specialization of write_array for string arrays
	template<> inline bool write_array<ValueType::VT_Array_String, std::string>( MemoryWriteStream &dstream, const char *key, const u8 key_size_in_bytes, const std::vector<std::string> *items, const std::vector<i32> *index )
		{
		// record start position, we need this in the end block
		const u64 start_pos = dstream.GetPosition();

		// begin a large block
		if( !begin_write_large_block( dstream, ValueType::VT_Array_String, key, key_size_in_bytes ) )
			{
			pdsErrorLog << "begin_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// write data if we have it
		if( items )
			{
			// write the item count and items
			if( !write_array_metadata_and_index( dstream, 0, items->size(), index ) )
				{
				return false;
				}

			if( items->size() > 0 )
				{
				const u64 values_start_pos = dstream.GetPosition();

				// this is the minimum size, with only empty strings. each string will add to the values_size
				u64 values_size = sizeof( u64 ) * items->size();

				// write each string in the array
				for( size_t string_index = 0; string_index < items->size(); ++string_index )
					{
					u64 string_length = (*items)[string_index].size();
					dstream.Write( string_length );
					if( string_length > 0 )
						{
						i8 *p_data = (i8 *)((*items)[string_index].data());
						dstream.Write( p_data, string_length );

						// update size of values
						values_size += string_length;
						}
					}

				const u64 values_expected_end_pos = values_start_pos + values_size;
				const u64 values_end_pos = dstream.GetPosition();

				// make sure we are at the expected end pos
				if( values_end_pos != values_expected_end_pos )
					{
					pdsErrorLog << "End position of data " << values_end_pos << " does not equal the expected end position which is " << values_expected_end_pos << pdsErrorLogEnd;
					return false;
					}

				}
			}

		// end the block by going back to the start and writing the start position offset
		if( !end_write_large_block( dstream, start_pos ) )
			{
			pdsErrorLog << "end_write_large_block() failed unexpectedly" << pdsErrorLogEnd;
			return false;
			}

		// succeeded
		return true;
		}

#ifdef PDS_MAIN_BUILD_FILE
	EntityWriter::EntityWriter( MemoryWriteStream &_dstream ) : dstream( _dstream ) , start_position( _dstream.GetPosition() ) {}

	// Build a section. 
	EntityWriter *EntityWriter::BeginWriteSection( const char *key, const u8 key_length )
		{
		if( this->active_subsection )
			{
			pdsErrorLog << "There is already an active subsection." << pdsErrorLogEnd;
			return nullptr;
			}

		// create a writer for the array, to store the start position before calling the begin large block 
		this->active_subsection = std::unique_ptr<EntityWriter>(new EntityWriter( this->dstream ));

		if( !begin_write_large_block( this->dstream, ValueType::VT_Subsection, key, key_length ) )
			{
			pdsErrorLog << "begin_write_large_block failed to write header." << pdsErrorLogEnd;
			return nullptr;
			}

		return this->active_subsection.get();
		}

	bool EntityWriter::EndWriteSection( const EntityWriter *section_writer )
		{
		if( this->active_subsection.get() != section_writer )
			{
			pdsErrorLog << "Invalid parameter section_writer, it does not match the internal value." << pdsErrorLogEnd;
			return false;
			}

		if( !end_write_large_block( this->dstream, this->active_subsection->start_position ) )
			{
			pdsErrorLog << "end_write_large_block failed unexpectedly." << pdsErrorLogEnd;
			return false;
			}

		this->active_subsection.reset();
		return true;
		}

	bool EntityWriter::WriteNullSection( const char *key, const u8 key_length )
		{
		EntityWriter *subsection = this->BeginWriteSection( key, key_length );
		if( !subsection )
			{
			return false;
			}
		return this->EndWriteSection( subsection );
		}

	EntityWriter *EntityWriter::BeginWriteSectionsArray( const char *key, const u8 key_length, const size_t array_size , const std::vector<i32> *index )
		{
		if( this->active_subsection )
			{
			pdsErrorLog << "There is already an active subsection" << pdsErrorLogEnd;
			return nullptr;
			}

		// create a writer for the array, to store the start position before calling the begin large block 
		this->active_subsection = std::unique_ptr<EntityWriter>(new EntityWriter( this->dstream ));

		if( !begin_write_large_block( this->dstream, ValueType::VT_Array_Subsection, key, key_length ) )
			{
			pdsErrorLog << "begin_write_large_block failed to write header." << pdsErrorLogEnd;
			return nullptr;
			}

		// if array_size is ~0, the array is null, so end directly
		if( array_size == ~0 )
			{
			this->active_array_size = 0;
			return this->active_subsection.get();
			}

		// write out flags, index and array size
		if( !write_array_metadata_and_index( dstream, 0, array_size, index ) )
			{
			return nullptr;
			}

		// reset the size and write index in the array
		this->active_array_size = array_size;
		this->active_array_index = ~0;
		this->active_array_index_start_position = 0;
		return this->active_subsection.get();
		}

	bool EntityWriter::BeginWriteSectionInArray( const EntityWriter *sections_array_writer, const size_t section_index )
		{
		if( this->active_subsection.get() != sections_array_writer )
			{
			pdsErrorLog << "Synch error, currently not writing a subsection array" << pdsErrorLogEnd;
			return false;
			}
		if( (this->active_array_index+1) != section_index )
			{
			pdsErrorLog << "Synch error, incorrect subsection index" << pdsErrorLogEnd;
			return false;
			}
		if( section_index >= this->active_array_size )
			{
			pdsErrorLog << "Incorrect subsection index, out of array bounds" << pdsErrorLogEnd;
			return false;
			}

		this->active_array_index = section_index;
		this->active_array_index_start_position = this->dstream.GetPosition();

		// write a temporary subsection size
		dstream.Write( (u64)MAXINT64 );

		return dstream.GetPosition() == (this->active_array_index_start_position + sizeof( u64 ));
		}

	bool EntityWriter::EndWriteSectionInArray( const EntityWriter *sections_array_writer, const size_t section_index )
		{
		if( this->active_subsection.get() != sections_array_writer || this->active_array_index != section_index )
			{
			pdsErrorLog << "Synch error, currently not writing a subsection array, or incorrect section index" << pdsErrorLogEnd;
			return false;
			}

		const u64 end_pos = dstream.GetPosition();
		const u64 block_size = end_pos - this->active_array_index_start_position - sizeof(u64); // total block size - ( sizeof( section_size_value )=8 )
		dstream.SetPosition( this->active_array_index_start_position ); 
		dstream.Write( block_size );
		dstream.SetPosition( end_pos ); // move back the where we were
		return (end_pos > this->active_array_index_start_position); // only thing we really can check
		}

	bool EntityWriter::EndWriteSectionsArray( const EntityWriter *sections_array_writer )
		{
		if( this->active_subsection.get() != sections_array_writer )
			{
			pdsErrorLog << "Synch error, currently not writing a subsection array" << pdsErrorLogEnd;
			return false;
			}
		if( (this->active_array_index+1) != this->active_array_size )
			{
			pdsErrorLog << "Synch error, the subsection index does not equal the end of the array" << pdsErrorLogEnd;
			return false;
			}

		if( !end_write_large_block( this->dstream, this->active_subsection->start_position ) )
			{
			pdsErrorLog << "end_write_large_block failed unexpectedly." << pdsErrorLogEnd;
			return false;
			}

		// release active subsection writer
		this->active_subsection.reset();
		this->active_array_size = 0;
		this->active_array_index = ~0;
		this->active_array_index_start_position = 0;
		return true;
		}

	bool EntityWriter::WriteNullSectionsArray( const char *key, const u8 key_length )
		{
		EntityWriter *subsection = this->BeginWriteSectionsArray( key, key_length, ~0, nullptr );
		if( !subsection )
			{
			return false;
			}
		return this->EndWriteSectionsArray( subsection );
		}

#endif//PDS_MAIN_BUILD_FILE

	};
