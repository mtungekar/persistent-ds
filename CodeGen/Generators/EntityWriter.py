# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

def EntityWriter_h():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append('#include "ValueTypes.h"')
	lines.append('')
	lines.append('namespace pds')
	lines.append('    {')
	lines.append('    class MemoryWriteStream;')
	lines.append('')
	lines.append('    class EntityWriter')
	lines.append('        {')
	lines.append('        private:')
	lines.append('            MemoryWriteStream &dstream;')
	lines.append('            const u64 start_position;')
	lines.append('')
	lines.append('            std::unique_ptr<EntityWriter> active_subsection;')
	lines.append('')
	lines.append('            size_t active_array_size = 0;')
	lines.append('            size_t active_array_index = size_t(~0);')
	lines.append('            u64 active_array_index_start_position = 0;')
	lines.append('')
	lines.append('        public:')
	lines.append('            EntityWriter( MemoryWriteStream &_dstream );')
	lines.append('')
	lines.append('            // Build a section. ')
	lines.append('            EntityWriter *BeginWriteSection( const char *key, const u8 key_length );')
	lines.append('            bool EndWriteSection( const EntityWriter *section_writer );')
	lines.append('            bool WriteNullSection( const char *key, const u8 key_length );')
	lines.append('')
	lines.append('            // Build a sections array. ')
	lines.append('            EntityWriter *BeginWriteSectionsArray( const char *key, const u8 key_length, const size_t array_size, const std::vector<i32> *index = nullptr );')
	lines.append('            bool BeginWriteSectionInArray( const EntityWriter *sections_array_writer , const size_t section_index );')
	lines.append('            bool EndWriteSectionInArray( const EntityWriter *sections_array_writer , const size_t section_index );')
	lines.append('            bool EndWriteSectionsArray( const EntityWriter *sections_array_writer );')
	lines.append('            bool WriteNullSectionsArray( const char *key, const u8 key_length );')
	lines.append('')
	lines.append('            // The Write function template, specifically implemented below for all supported value types.')
	lines.append('            template <class T> bool Write( const char *key, const u8 key_length, const T &value );')
	lines.append('')
	
	# print the base types
	for basetype in hlp.base_types:
		type_name = 'VT_' + basetype.name
		lines.append('            // ' + type_name )
		for type_impl in basetype.variants:
			type_impl_name = type_impl.implementing_type
			lines.append('            template <> bool Write<' + type_impl_name + '>( const char *key, const u8 key_length, const ' + type_impl_name + ' &value );')
			lines.append('            template <> bool Write<optional_value<' + type_impl_name + '>>( const char *key, const u8 key_length, const optional_value<' + type_impl_name + '> &value );')
		lines.append('')

	# print the array types
	for basetype in hlp.base_types:
		type_name = 'VT_Array_' + basetype.name
		lines.append('            // ' + type_name )
		for type_impl in basetype.variants:
			type_impl_name = type_impl.implementing_type
			lines.append('            template <> bool Write<std::vector<' + type_impl_name + '>>( const char *key, const u8 key_length, const std::vector<' + type_impl_name + '> &value );')
			lines.append('            template <> bool Write<optional_vector<' + type_impl_name + '>>( const char *key, const u8 key_length, const optional_vector<' + type_impl_name + '> &value );')
			lines.append('            template <> bool Write<idx_vector<' + type_impl_name + '>>( const char *key, const u8 key_length, const idx_vector<' + type_impl_name + '> &value );')
			lines.append('            template <> bool Write<optional_idx_vector<' + type_impl_name + '>>( const char *key, const u8 key_length, const optional_idx_vector<' + type_impl_name + '> &value );')
		lines.append('')

	lines.append('		};')
	lines.append('')
	#lines.append('	// Write function. Specialized for all supported value types.')
	#lines.append('	template <class T> bool EntityWriter::Write( const char *key, const u8 key_length, const T &value )')
	#lines.append('		{')
	#lines.append('		static_assert(false, "Error: EntityWriter::Write template: The value type T cannot be serialized.");')
	#lines.append('		}')
	lines.append('	};')
	hlp.write_lines_to_file("../Include/pds/EntityWriter.h",lines)

def EntityWriter_inl():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include "EntityWriter.h"')
	lines.append('#include "MemoryWriteStream.h"')
	lines.append('')
	lines.append('#include "EntityWriterTemplates.inl"')
	lines.append('')
	lines.append('namespace pds')
	lines.append('	{')
	 
	# print the base types
	for basetype in hlp.base_types:
		
		# all variants
		for type_impl in basetype.variants:
			implementing_type = str(type_impl.implementing_type)
			item_type = str(type_impl.item_type)
			num_items_per_object = str(type_impl.num_items_per_object)

			if type_impl.overrides_type:
				lines.append(f'	// {implementing_type}: using {item_type} to store')
				lines.append(f'	template <> inline bool EntityWriter::Write<{implementing_type}>( const char *key, const u8 key_length, const {implementing_type} &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		const {item_type} tmp_variable = src_variable;')
				lines.append(f'		return this->Write<{item_type}>( key, key_length, tmp_variable );')
				lines.append(f'		}}')
				lines.append(f'')
			
				lines.append(f'	// {implementing_type}: using optional_value<{item_type}> to store' )
				lines.append(f'	template <> inline bool EntityWriter::Write<optional_value<{implementing_type}>>( const char *key, const u8 key_length, const optional_value<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		optional_value<{item_type}> tmp_variable;')
				lines.append(f'		if( src_variable.has_value() )')
				lines.append(f'		    tmp_variable.set( src_variable.value() );')
				lines.append(f'		return this->Write<optional_value<{item_type}>>( key, key_length, tmp_variable );')
				lines.append(f'		}}')
				lines.append(f'')
			
				lines.append(f'	// {implementing_type}: using std::vector<{item_type}> to store' )
				lines.append(f'	template <> inline bool EntityWriter::Write<std::vector<{implementing_type}>>( const char *key, const u8 key_length, const std::vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		std::vector<{item_type}> tmp_variable;')
				lines.append(f'		tmp_variable.reserve( src_variable.size() );')
				lines.append(f'		std::copy( src_variable.begin(), src_variable.end(), std::back_inserter(tmp_variable) );')
				lines.append(f'		return this->Write<std::vector<{item_type}>>( key, key_length, tmp_variable );')
				lines.append(f'		}}')
				lines.append(f'')
			
				lines.append(f'	//  {implementing_type}: optional_vector<{item_type}> to store' )
				lines.append(f'	template <> inline bool EntityWriter::Write<optional_vector<{implementing_type}>>( const char *key, const u8 key_length, const optional_vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		optional_vector<{item_type}> tmp_variable;')
				lines.append(f'		if( src_variable.has_value() )')
				lines.append(f'			{{')
				lines.append(f'			tmp_variable.set();')
				lines.append(f'			tmp_variable.values().reserve( src_variable.values().size() );')
				lines.append(f'			std::copy( src_variable.values().begin(), src_variable.values().end(), std::back_inserter(tmp_variable.values()) );')
				lines.append(f'			}}')
				lines.append(f'		return this->Write<optional_vector<{item_type}>>( key, key_length, tmp_variable );')
				lines.append(f'		}}')
				lines.append(f'')
				
				lines.append(f'	// {implementing_type}: using idx_vector<{item_type}> to store' )
				lines.append(f'	template <> inline bool EntityWriter::Write<idx_vector<{implementing_type}>>( const char *key, const u8 key_length, const idx_vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		idx_vector<{item_type}> tmp_variable;')
				lines.append(f'		tmp_variable.index().reserve( src_variable.index().size() );')
				lines.append(f'		tmp_variable.values().reserve( src_variable.values().size() );')
				lines.append(f'		std::copy( src_variable.index().begin(), src_variable.index().end(), std::back_inserter(tmp_variable.index()) );')
				lines.append(f'		std::copy( src_variable.values().begin(), src_variable.values().end(), std::back_inserter(tmp_variable.values()) );')
				lines.append(f'		return this->Write<idx_vector<{item_type}>>( key, key_length, tmp_variable );')
				lines.append(f'		}}')
				lines.append(f'')

				lines.append(f'	//  {implementing_type}: optional_idx_vector<{item_type}> to store' )
				lines.append(f'	template <> inline bool EntityWriter::Write<optional_idx_vector<{implementing_type}>>( const char *key, const u8 key_length, const optional_idx_vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		optional_idx_vector<{item_type}> tmp_variable;')
				lines.append(f'		if( src_variable.has_value() )')
				lines.append(f'			{{')
				lines.append(f'			tmp_variable.set();')
				lines.append(f'			tmp_variable.index().reserve( src_variable.index().size() );')
				lines.append(f'			tmp_variable.values().reserve( src_variable.values().size() );')
				lines.append(f'			std::copy( src_variable.index().begin(), src_variable.index().end(), std::back_inserter(tmp_variable.index()) );')
				lines.append(f'			std::copy( src_variable.values().begin(), src_variable.values().end(), std::back_inserter(tmp_variable.values()) );')
				lines.append(f'			}}')
				lines.append(f'		return this->Write<optional_idx_vector<{item_type}>>( key, key_length, tmp_variable );')
				lines.append(f'		}}')
				lines.append(f'')
				
			else:
				type_name = 'VT_' + basetype.name
				array_type_name = 'VT_Array_' + basetype.name
				
				lines.append(f'	// {type_name}: {implementing_type}')
				lines.append(f'	template <> inline bool EntityWriter::Write<{implementing_type}>( const char *key, const u8 key_length, const {implementing_type} &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		return write_single_value<ValueType::{type_name},{implementing_type}>( this->dstream, key, key_length, &src_variable );')
				lines.append(f'		}}')
				lines.append(f'')
				
				lines.append(f'	// {type_name}: optional_value<{implementing_type}>' )
				lines.append(f'	template <> inline bool EntityWriter::Write<optional_value<{implementing_type}>>( const char *key, const u8 key_length, const optional_value<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		const {implementing_type} *p_src_variable = (src_variable.has_value()) ? &(src_variable.value()) : nullptr;')
				lines.append(f'		return write_single_value<ValueType::{type_name},{implementing_type}>( this->dstream, key, key_length, p_src_variable );')
				lines.append(f'		}}')
				lines.append(f'')
				
				lines.append(f'	//  {array_type_name}: std::vector<{implementing_type}>' )
				lines.append(f'	template <> inline bool EntityWriter::Write<std::vector<{implementing_type}>>( const char *key, const u8 key_length, const std::vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		return write_array<ValueType::{array_type_name},{implementing_type}>(this->dstream, key, key_length, &src_variable , nullptr );')
				lines.append(f'		}}')
				lines.append(f'')
				
				lines.append(f'	//  {array_type_name}: optional_vector<{implementing_type}>' )
				lines.append(f'	template <> inline bool EntityWriter::Write<optional_vector<{implementing_type}>>( const char *key, const u8 key_length, const optional_vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		const std::vector<{implementing_type}> *p_src_variable = (src_variable.has_value()) ? &(src_variable.values()) : nullptr;')
				lines.append(f'		return write_array<ValueType::{array_type_name},{implementing_type}>(this->dstream, key, key_length, p_src_variable , nullptr );')
				lines.append(f'		}}')
				lines.append(f'')
				
				lines.append(f'	//  {array_type_name}: idx_vector<{implementing_type}>' )
				lines.append(f'	template <> inline bool EntityWriter::Write<idx_vector<{implementing_type}>>( const char *key, const u8 key_length, const idx_vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		return write_array<ValueType::{array_type_name},{implementing_type}>(this->dstream, key, key_length, &(src_variable.values()) , &(src_variable.index()) );')
				lines.append(f'		}}')
				lines.append(f'')
				
				lines.append(f'	//  {array_type_name}: optional_idx_vector<{implementing_type}>' )
				lines.append(f'	template <> inline bool EntityWriter::Write<optional_idx_vector<{implementing_type}>>( const char *key, const u8 key_length, const optional_idx_vector<{implementing_type}> &src_variable )')
				lines.append(f'		{{')
				lines.append(f'		const std::vector<{implementing_type}> *p_src_values = (src_variable.has_value()) ? &(src_variable.values()) : nullptr;')
				lines.append(f'		const std::vector<i32> *p_src_index = (src_variable.has_value()) ? &(src_variable.index()) : nullptr;')
				lines.append(f'		return write_array<ValueType::{array_type_name},{implementing_type}>(this->dstream, key, key_length, p_src_values , p_src_index );')
				lines.append(f'		}}')
				lines.append(f'')
				
	# other types which convert to existing types
	types = [ ['item_ref','uuid'], ['entity_ref','hash'] ]
	for type in types:
		type_name = type[0]
		implementing_type = type[1]

	lines.append('	};')
	hlp.write_lines_to_file("../Include/pds/EntityWriter.inl",lines)

def run():
	EntityWriter_h()
	EntityWriter_inl()