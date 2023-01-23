# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

# hash table for allocation lookup, must be larger than the number of items to add
class AllocatorHashTable:
	def hash_function( self , data_type , container_type ):
		return ((data_type * self.hash_data_type_mult) + (container_type * self.hash_container_type_mult)) % self.hash_table_size

	def insert_into_table( self , data_type_combo_string , data_type_id , container_type_id ):
		# use hash function to generate a good starting point
		hash_val = self.hash_function( data_type_id , container_type_id )
		# find first empty slot
		while( self.hash_table[hash_val] != None ):
			hash_val = hash_val+1
			if( hash_val >= self.hash_table_size ):
				hash_val = 0
		# fill it
		self.hash_table[hash_val] = data_type_combo_string

	def __init__(self):
		self.hash_table_size = 577 
		self.hash_data_type_mult = 109
		self.hash_container_type_mult = 991
		self.hash_table = [None] * self.hash_table_size

		# fill up hash table 
		for basetype_inx in range(len(hlp.base_types)):
			basetype = hlp.base_types[basetype_inx]
			for variant_inx in range(len(basetype.variants)):
				variant_name = basetype.variants[variant_inx].implementing_type
				variant_id = ( (basetype_inx+1) << 4) + (variant_inx + 1)
				for cont in hlp.container_types:
					self.insert_into_table( f'&_dt_{variant_name}_ct_{cont.implementing_type}_DynamicTypeObject' , variant_id , cont.container_id )


def DynamicTypes_inl(run_clang_format):
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <pds/pds.h>')
	lines.append('#include <pds/EntityWriter.h>')
	lines.append('#include <pds/EntityReader.h>')
	lines.append('#include <pds/DynamicTypes.h>')
	lines.append('')
	lines.append('#include <pds/ValueTypes.inl>')
	lines.append('')
	lines.append('namespace pds')
	lines.append('    {')
	lines.append('namespace dynamic_types')
	lines.append('    {')
	lines.append('    struct type_combo')
	lines.append('        {')
	lines.append('        data_type_index data_type = {};')
	lines.append('        container_type_index container_type = {};')
	lines.append('        bool operator==(const type_combo& other) const { return other.data_type == this->data_type && other.container_type == this->container_type; }')
	lines.append('        };')
	lines.append('')
	
	lines.append('    // dynamic allocation functors for items')
	lines.append('    class _dynamicTypeClass')
	lines.append('        {')
	lines.append('        public:')
	lines.append('            virtual type_combo Type() const = 0;')
	lines.append('            virtual void *New() const = 0;')
	lines.append('            virtual void Delete( void *data ) const = 0;')
	lines.append('            virtual void Clear( void *data ) const = 0;')
	lines.append('            virtual bool Write( const char *key, const u8 key_length , EntityWriter &writer , const void *data ) const = 0;')
	lines.append('            virtual bool Read( const char *key, const u8 key_length , EntityReader &reader , void *data ) const = 0;')
	lines.append('            virtual void Copy( void *dest , const void *src ) const = 0;')
	lines.append('            virtual bool Equals( const void *dataA , const void *dataB ) const = 0;')
	lines.append('        };')

	lines.append('')

	def generate_dynamic_object_function( base_type_name , implementing_type , container_type , item_type , num_items_per_object , base_type_combo ):
		lines = []
		lines.append(f'    // {base_type_combo}' )
		lines.append(f'    static const class _dt_{implementing_type}_ct_{container_type}_DynamicTypeClass : public _dynamicTypeClass' )
		lines.append(f'        {{' )
		lines.append(f'        public:' )
		lines.append(f'            virtual type_combo Type() const {{ return {{ combined_type_information<{base_type_combo}>::type_index , combined_type_information<{base_type_combo}>::container_index }}; }}' )
		lines.append(f'            virtual void *New() const {{ return new {base_type_combo}(); }}' )
		lines.append(f'            virtual void Delete( void *data ) const {{ delete (({base_type_combo}*)(data)); }}' )
		lines.append(f'            virtual void Clear( void *data ) const {{ clear_combined_type(*(({base_type_combo}*)data)); }}' )
		lines.append(f'            virtual bool Write( const char *key, const u8 key_length , EntityWriter &writer , const void *data ) const {{ return writer.Write<{base_type_combo}>( key , key_length , *((const {base_type_combo}*)data) ); }}' )
		lines.append(f'            virtual bool Read( const char *key, const u8 key_length , EntityReader &reader , void *data ) const {{ return reader.Read<{base_type_combo}>( key , key_length , *(({base_type_combo}*)data) ); }}' )
		lines.append(f'            virtual void Copy( void *dest , const void *src ) const {{ *(({base_type_combo}*)dest) = *((const {base_type_combo}*)src); }}' )
		lines.append(f'            virtual bool Equals( const void *dataA , const void *dataB ) const {{ return *((const {base_type_combo}*)dataA) == *((const {base_type_combo}*)dataB); }}' )
		lines.append(f'        }} _dt_{implementing_type}_ct_{container_type}_DynamicTypeObject;' )
		lines.append(f'')
		return lines
	lines.extend( hlp.function_for_all_basetype_combos( generate_dynamic_object_function ))

	# allocate and print hash table
	hash_table = AllocatorHashTable()

	# print it 
	lines.append('    // Hash table with the type allocator objects')
	lines.append(f'    static const _dynamicTypeClass *_dynamicTypeClassHashTable[{hash_table.hash_table_size}] = ')
	lines.append('        {')
	for idx in range(hash_table.hash_table_size):
		if hash_table.hash_table[idx] == None:
			lines.append('        nullptr,')
		else:
			lines.append(f'        {hash_table.hash_table[idx]},')
	lines.append('        };')
	lines.append('')
	lines.append('    // hash table lookup of typeCombo')
	lines.append('    static const _dynamicTypeClass *_findTypeClass( type_combo typeCombo )')
	lines.append('        {')
	lines.append(f'        size_t hashValue = ((((size_t)typeCombo.data_type) * {hash_table.hash_data_type_mult}) + (((size_t)typeCombo.container_type) * {hash_table.hash_container_type_mult})) % {hash_table.hash_table_size};')
	lines.append('        while( _dynamicTypeClassHashTable[hashValue] != nullptr )')
	lines.append('            {')
	lines.append('            type_combo type = _dynamicTypeClassHashTable[hashValue]->Type();')
	lines.append(f'            if( type == typeCombo )')
	lines.append('                return _dynamicTypeClassHashTable[hashValue];')
	lines.append('            ++hashValue;')
	lines.append(f'            if(hashValue >= {hash_table.hash_table_size})')
	lines.append('                hashValue = 0;')
	lines.append('            }')
	lines.append('        pdsErrorLog << "Invalid typeCombo parameter { " << (int)typeCombo.data_type << " , " << (int)typeCombo.container_type << " } " << pdsErrorLogEnd;')
	lines.append('        return nullptr;')
	lines.append('        }')
	lines.append('')
	lines.append('    std::tuple<void*, bool> new_type( data_type_index dataType , container_type_index containerType )')
	lines.append('        {')
	lines.append('        void* data = {};')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( ta )')
	lines.append('            data = ta->New();')
	lines.append('        return std::tuple<void*, bool>(data, data != nullptr);')
	lines.append('        }')
	lines.append('')
	lines.append('    bool delete_type( data_type_index dataType , container_type_index containerType , void *data )')
	lines.append('        {')
	lines.append('        if( !data )')
	lines.append('            {')
	lines.append('            pdsErrorLog << "Invalid parameter, data must be a pointer to existing type" << pdsErrorLogEnd;')
	lines.append('            return false;')
	lines.append('            }')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( !ta )')
	lines.append('            return false;')
	lines.append('        ta->Delete( data );')
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	lines.append('    bool clear( data_type_index dataType , container_type_index containerType , void *data )')
	lines.append('        {')
	lines.append('        if( !data )')
	lines.append('            {')
	lines.append('            pdsErrorLog << "Invalid parameter, data must be a pointer to existing type" << pdsErrorLogEnd;')
	lines.append('            return false;')
	lines.append('            }')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( !ta )')
	lines.append('            return false;')
	lines.append('        ta->Clear( data );')
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	lines.append('    bool write( data_type_index dataType , container_type_index containerType , const char *key, const u8 key_length , EntityWriter &writer , const void *data )')
	lines.append('        {')
	lines.append('        if( !data )')
	lines.append('            {')
	lines.append('            pdsErrorLog << "Invalid parameter, data must be a pointer to existing type" << pdsErrorLogEnd;')
	lines.append('            return false;')
	lines.append('            }')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( !ta )')
	lines.append('            return false;')
	lines.append('        return ta->Write( key , key_length , writer , data );')
	lines.append('        }')
	lines.append('')
	lines.append('    bool read( data_type_index dataType , container_type_index containerType , const char *key, const u8 key_length , EntityReader &reader , void *data )')
	lines.append('        {')
	lines.append('        if( !data )')
	lines.append('            {')
	lines.append('            pdsErrorLog << "Invalid parameter, data must be a pointer to existing type" << pdsErrorLogEnd;')
	lines.append('            return false;')
	lines.append('            }')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( !ta )')
	lines.append('            return false;')
	lines.append('        return ta->Read( key , key_length , reader , data );')
	lines.append('        }')
	lines.append('')
	lines.append('    bool copy( data_type_index dataType , container_type_index containerType , void *dest , const void *src )')
	lines.append('        {')
	lines.append('        if( !dest || !src )')
	lines.append('            {')
	lines.append('            pdsErrorLog << "Invalid parameter, dest and src must be pointers to existing types" << pdsErrorLogEnd;')
	lines.append('            return false;')
	lines.append('            }')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( !ta )')
	lines.append('            return false;')
	lines.append('        ta->Copy( dest , src );')
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	lines.append('    bool equals( data_type_index dataType , container_type_index containerType , const void *dataA , const void *dataB )')
	lines.append('        {')
	lines.append('        if( !dataA || !dataB )')
	lines.append('            {')
	lines.append('            pdsErrorLog << "Invalid parameter, dataA and dataB must be pointers to existing types" << pdsErrorLogEnd;')
	lines.append('            return false;')
	lines.append('            }')
	lines.append('        const _dynamicTypeClass *ta = _findTypeClass( { dataType, containerType } );')
	lines.append('        if( !ta )')
	lines.append('            return false;')
	lines.append('        return ta->Equals( dataA , dataB );')
	lines.append('        }')
	lines.append('')

	# end of namespace
	lines.append('    };')
	lines.append('    };')
	hlp.write_lines_to_file("../Include/pds/DynamicTypes.inl",lines, run_clang_format)
	


def DynamicTypesTests_cpp(run_clang_format):
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('#include "Tests.h"')
	lines.append('#include <pds/DynamicTypes.h>')
	lines.append('#include <pds/EntityWriter.inl>')
	lines.append('#include <pds/EntityReader.inl>')
	lines.append('')
	lines.append('template<class _Ty> void DynamicValueTester()')
	lines.append('    {')
	lines.append('    constexpr pds::data_type_index type_index = pds::data_type_information<_Ty>::type_index;')
	lines.append('    void *dataA = {};')
	lines.append('    void *dataB = {};')
	lines.append('    bool ret = {};')
	lines.append('')

	for cont in hlp.container_types:
		lines.append(f'    // test container type: {cont.implementing_type} ')
		lines.append(f'    constexpr pds::container_type_index ct_{cont.implementing_type} = container_type_index::ct_{cont.implementing_type};')
		lines.append('')
		lines.append('    // create objects of the type, one dynamically typed in the heap and one statically typed on the stack')
		lines.append(f'    std::tie(dataA,ret) = pds::dynamic_types::new_type( type_index , ct_{cont.implementing_type} );')
		lines.append('    EXPECT_TRUE( ret );')
		if cont.is_template:
			lines.append(f'    pds::{cont.implementing_type}<_Ty> ct_{cont.implementing_type}_valueB;')
		else:
			lines.append(f'    _Ty ct_{cont.implementing_type}_valueB = pds::data_type_information<_Ty>::zero;')

		lines.append(f'    dataB = &ct_{cont.implementing_type}_valueB;')
		lines.append('')
		lines.append('    // clear A, make sure they match')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::clear( type_index , ct_{cont.implementing_type} , dataA ) );')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::equals( type_index , ct_{cont.implementing_type} , dataA, dataB ) );')
		lines.append('')
		lines.append('    // give A a random non-zero value')
		if cont.is_template:
			lines.append(f'    random_nonzero_{cont.implementing_type}<_Ty>( *(({cont.implementing_type}<_Ty>*)dataA) );')
		else:
			lines.append(f'    random_nonzero_value<_Ty>( *((_Ty*)dataA) );')
		lines.append('')
		lines.append('    // try comparing, clearing and copying')
		lines.append(f'    EXPECT_FALSE( pds::dynamic_types::equals( type_index , ct_{cont.implementing_type} , dataA, dataB ) );')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::copy( type_index , ct_{cont.implementing_type} , dataB, dataA ) );')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::equals( type_index , ct_{cont.implementing_type} , dataA, dataB ) );')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::clear( type_index , ct_{cont.implementing_type} , dataA ) );')
		lines.append(f'    EXPECT_FALSE( pds::dynamic_types::equals( type_index , ct_{cont.implementing_type} , dataA, dataB ) );')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::copy( type_index , ct_{cont.implementing_type} , dataB, dataA ) );')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::equals( type_index , ct_{cont.implementing_type} , dataA, dataB ) );')
		lines.append('')
		lines.append('    // delete the heap allocated data')
		lines.append(f'    EXPECT_TRUE( pds::dynamic_types::delete_type( type_index , ct_{cont.implementing_type} , dataA ) );')
		lines.append('')

	lines.append('    }')
	lines.append('')

	lines.append('TEST( DynamicTypesTests , DynamicTypes )')
	lines.append('    {')

	lines.append('    setup_random_seed();')
	lines.append('    for( uint pass_index=0; pass_index<(2*global_number_of_passes); ++pass_index )')
	lines.append('        {')

	for basetype in hlp.base_types:
		for var in basetype.variants:
			lines.append(f'        DynamicValueTester<{var.implementing_type}>();')

	lines.append('        }')
	lines.append('    }')

	hlp.write_lines_to_file("../Tests/DynamicTypesTests.cpp",lines, run_clang_format)	
	
def run(**kwargs):
	run_clang_format = kwargs['run_clang_format']
	DynamicTypes_inl(run_clang_format)
	DynamicTypesTests_cpp(run_clang_format)