# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

int_bit_range = [8,16,32,64]
float_type_range = ['float','double']
vector_dimension_range = [2,3,4] 
nonconst_const_range = ['','const ']

def print_type_information_header( type , value , value_count ):
	lines = []
	lines.append(f'\ttemplate<> struct data_type_information<{type}>')
	lines.append( '\t	{')
	lines.append(f'\t	using value_type = {value}; // the value type of {type} ( {value} )')
	lines.append(f'\t	static constexpr size_t value_count = {value_count}; // the number of values in {type} ( {value_count} )')
	lines.append(f'\t	static constexpr const char * value_name = "{value}"; // name of the value in {type} ( "{value}" ) ')
	lines.append(f'\t	static constexpr const char * type_name = "{type}"; // name of the type ( "{type}" ) ')
	lines.append(f'\t	static constexpr data_type_index type_index = data_type_index::dt_{type}; // the data type index of {type} ( dt_{type} )')
	lines.append(f'\t	static const {type} zero; // zero value of {type}')
	lines.append(f'\t	static const {type} inf; // limit inferior (minimum possible value) of {type}')
	if type != 'string':
		lines.append(f'\t	static const {type} sup; // limit superior (maximum possible value) of {type}')
	lines.append( '\t	};')
	lines.append('')
	return lines

def DataTypes_h():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('// DataTypes.h - All basic data types which are used by pds')
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append('// UUID and HASH definitions. Define PDS_SKIP_UUID_AND_HASH if you wish to roll your own UUID and HASH definitions.')
	lines.append('#ifndef PDS_SKIP_UUID_AND_HASH')
	lines.append('')
	lines.extend( hlp.inline_file( 'InlinedCode/uuid_hash_header.inl' ) )
	lines.append('')
	lines.append('#endif//PDS_SKIP_UUID_AND_HASH')
	lines.append('')

	lines.append('#include <glm/fwd.hpp>')
	lines.append('')

	lines.append('namespace pds')
	lines.append('    {')

	# typedef base integer types
	lines.append(f"\t// scalar types")
	for bit_size in int_bit_range:
		lines.append(f"\ttypedef std::int{bit_size}_t i{bit_size};")
	for bit_size in int_bit_range:
		lines.append(f"\ttypedef std::uint{bit_size}_t u{bit_size};")
	lines.append('')
	lines.append(f"\ttypedef std::string string;")
	lines.append(f"\ttypedef UUID uuid;")
	lines.append(f"\ttypedef HASH hash;")
	lines.append('')

	# const min/max values of the standard types
	lines.append('\t// scalar types, zero value, minimum possible value ("inf", limit inferior) and maximum possible value ("sup", limit superior)')
	lines.append('\tconstexpr bool bool_zero = false;')
	lines.append('\tconstexpr bool bool_inf = false;')
	lines.append('\tconstexpr bool bool_sup = true;')
	for bit_size in int_bit_range:
		lines.append(f"\tconstexpr i{bit_size} i{bit_size}_zero = 0;")
		lines.append(f"\tconstexpr i{bit_size} i{bit_size}_inf = INT{bit_size}_MIN;")
		lines.append(f"\tconstexpr i{bit_size} i{bit_size}_sup = INT{bit_size}_MAX;")
	for bit_size in int_bit_range:
		lines.append(f"\tconstexpr u{bit_size} u{bit_size}_zero = 0;")
		lines.append(f"\tconstexpr u{bit_size} u{bit_size}_inf = 0;")
		lines.append(f"\tconstexpr u{bit_size} u{bit_size}_sup = UINT{bit_size}_MAX;")
	lines.append('')
	lines.append('\tconstexpr float float_zero = 0.0f;')
	lines.append('\tconstexpr float float_inf = -FLT_MAX;')
	lines.append('\tconstexpr float float_sup = FLT_MAX;')
	lines.append('\tconstexpr float double_zero = 0.0;')
	lines.append('\tconstexpr double double_inf = -DBL_MAX;')
	lines.append('\tconstexpr double double_sup = DBL_MAX;')
	lines.append('')
	lines.append('\tconst string string_zero;')
	lines.append('\tconst string string_inf;')
	lines.append('')
	lines.append('\tconstexpr uuid uuid_zero = {0,0,0,{0,0,0,0,0,0,0,0}};')
	lines.append('\tconstexpr uuid uuid_inf = {0,0,0,{0,0,0,0,0,0,0,0}};')
	lines.append('\tconstexpr uuid uuid_sup = {0xffffffff,0xffff,0xffff,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}};')
	lines.append('')
	lines.append('\tconstexpr hash hash_zero = {0,0,0,0};')
	lines.append('\tconstexpr hash hash_inf = {0,0,0,0};')
	lines.append('\tconstexpr hash hash_sup = {~0Ui64,~0Ui64,~0Ui64,~0Ui64};')
	lines.append('')

	# typedef vector types
	lines.append(f"\t// vector types")
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.append(f"\ttypedef glm::vec<{vec_dim},glm::i{bit_size},glm::packed_highp> i{bit_size}vec{vec_dim};")
	lines.append('')
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.append(f"\ttypedef glm::vec<{vec_dim},glm::u{bit_size},glm::packed_highp> u{bit_size}vec{vec_dim};")
	lines.append('')
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::vec<{vec_dim},glm::f32,glm::packed_highp> fvec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::vec<{vec_dim},glm::f64,glm::packed_highp> dvec{vec_dim};")
	lines.append('')
	
	# typedef matrix types
	lines.append(f"\t// matrix types")
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::mat<{vec_dim},{vec_dim},glm::f32,glm::packed_highp> fmat{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::mat<{vec_dim},{vec_dim},glm::f64,glm::packed_highp> dmat{vec_dim};")
	lines.append('')

	# typedef quaternions
	lines.append(f"\t// quaternion types")
	lines.append(f"\ttypedef glm::qua<glm::f32,glm::packed_highp> fquat;")
	lines.append(f"\ttypedef glm::qua<glm::f64,glm::packed_highp> dquat;")
	lines.append('')

	# typedef standard types
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::ivec{vec_dim} ivec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::uvec{vec_dim} uvec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::vec{vec_dim} vec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\ttypedef glm::mat{vec_dim} mat{vec_dim};")
	lines.append('')

	# inline entity_ref and item_ref
	lines.extend( hlp.inline_file( 'InlinedCode/entity_ref.inl' ) )
	lines.extend( hlp.inline_file( 'InlinedCode/item_ref.inl' ) )

	# enum of all data types
	lines.append('\t// all value type indices')
	lines.append('\tenum class data_type_index')
	lines.append('\t\t{')
	for basetype_inx in range(len(hlp.base_types)):
		basetype = hlp.base_types[basetype_inx]
		for variant_inx in range(len(basetype.variants)):
			variant_name = basetype.variants[variant_inx].implementing_type
			variant_id = ( (basetype_inx+1) << 4) + (variant_inx + 1)
			lines.append(f'\t\tdt_{variant_name} = {hex(variant_id)},')
	lines.append('\t\t};')
	lines.append('')

	# type information on all types
	lines.append('\t// type_information stores information on the standard types in PDS')
	lines.append('\ttemplate <class T> struct data_type_information;')
	lines.append('')

	# scalar type info
	lines.extend(print_type_information_header("bool","bool",1))
	for bit_size in int_bit_range:
		lines.extend(print_type_information_header(f"i{bit_size}",f"i{bit_size}",1))
	for bit_size in int_bit_range:
		lines.extend(print_type_information_header(f"u{bit_size}",f"u{bit_size}",1))
	lines.extend(print_type_information_header("float","float",1))
	lines.extend(print_type_information_header("double","double",1))

	# vector type info
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.extend(print_type_information_header(f"i{bit_size}vec{vec_dim}",f"i{bit_size}",vec_dim))
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.extend(print_type_information_header(f"u{bit_size}vec{vec_dim}",f"u{bit_size}",vec_dim))	
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_header(f"fvec{vec_dim}",'float',vec_dim))	
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_header(f"dvec{vec_dim}",'double',vec_dim))	

	# matrix type info
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_header(f"fmat{vec_dim}",'float',vec_dim*vec_dim))	
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_header(f"dmat{vec_dim}",'double',vec_dim*vec_dim))	

	# quaternions info
	lines.extend(print_type_information_header('fquat','float',4))
	lines.extend(print_type_information_header('dquat','double',4))

	# uuid info
	lines.extend(print_type_information_header('uuid','uuid',1))
	lines.extend(print_type_information_header('item_ref','item_ref',1))

	# hash info
	lines.extend(print_type_information_header('hash','hash',1))
	lines.extend(print_type_information_header('entity_ref','entity_ref',1))

	# string info
	lines.extend(print_type_information_header('string','string',1))

	# end of pds namespace
	lines.append('    };')
	
	# define stuff in std namespace
	lines.append('')
	lines.append('// inject hash functions into std')
	lines.append('template<>')
	lines.append('struct std::hash<pds::item_ref>')
	lines.append('    {')
	lines.append('    std::size_t operator()(pds::item_ref const& val) const noexcept')
	lines.append('        {')
	lines.append('        return std::hash<UUID>{}( UUID( val ) );')
	lines.append('        }')
	lines.append('    };')
	lines.append('')
	lines.append('template<>')
	lines.append('struct std::hash<pds::entity_ref>')
	lines.append('    {')
	lines.append('    std::size_t operator()(pds::entity_ref const& val) const noexcept')
	lines.append('        {')
	lines.append('        return std::hash<HASH>{}( HASH( val ) );')
	lines.append('        }')
	lines.append('    };')

	# end of file
	hlp.write_lines_to_file("../Include/pds/DataTypes.h",lines)

def print_type_information_source( type , value , value_count ):
	lines = []
	
	zero = inf = sup = ''
	for i in range(value_count):
		zero += f'{value}_zero'
		inf += f'{value}_inf'
		sup += f'{value}_sup'
		if i < value_count-1:
			zero += ','
			inf += ','
			sup += ','

	lines.append(f'\tconst {type} data_type_information<{type}>::zero = {type}({zero}); // zero value of {type}')
	lines.append(f'\tconst {type} data_type_information<{type}>::inf = {type}({inf}); // limit inferior (minimum bound) of {type}')
	if type != 'string':
		lines.append(f'\tconst {type} data_type_information<{type}>::sup = {type}({sup}); // limit superior (maximum bound) of {type}')
	lines.append('')
	return lines

def DataTypes_inl():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <glm/glm.hpp>')
	lines.append('#include <glm/gtc/type_ptr.hpp>')
	lines.append('')
	lines.append('#include <pds/pds.h>')
	lines.append('')
	lines.extend( hlp.inline_file( 'InlinedCode/uuid_hash_source.inl' ) )
	lines.append('')
	lines.append('namespace pds')
	lines.append('    {')

	# scalar type info
	lines.extend(print_type_information_source("bool","bool",1))
	for bit_size in int_bit_range:
		lines.extend(print_type_information_source(f"i{bit_size}",f"i{bit_size}",1))
	for bit_size in int_bit_range:
		lines.extend(print_type_information_source(f"u{bit_size}",f"u{bit_size}",1))
	lines.extend(print_type_information_source("float","float",1))
	lines.extend(print_type_information_source("double","double",1))

	# vector type info
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.extend(print_type_information_source(f"i{bit_size}vec{vec_dim}",f"i{bit_size}",vec_dim))
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.extend(print_type_information_source(f"u{bit_size}vec{vec_dim}",f"u{bit_size}",vec_dim))	
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_source(f"fvec{vec_dim}",'float',vec_dim))	
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_source(f"dvec{vec_dim}",'double',vec_dim))	

	# matrix type info
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_source(f"fmat{vec_dim}",'float',vec_dim*vec_dim))	
	for vec_dim in vector_dimension_range:
		lines.extend(print_type_information_source(f"dmat{vec_dim}",'double',vec_dim*vec_dim))	

	# quaternions info
	lines.extend(print_type_information_source('fquat','float',4))
	lines.extend(print_type_information_source('dquat','double',4))

	# other types that are atomic
	same_type_range = ['uuid','entity_ref','hash','item_ref','string']
	for type in same_type_range:
		lines.extend(print_type_information_source(type,type,1))
	lines.append('    };')

	hlp.write_lines_to_file("../Include/pds/DataTypes.inl",lines)

def DataValuePointers_h():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append('#include <pds/pds.h>')
	lines.append('')

	lines.extend( hlp.generate_push_and_disable_warnings( [4201] , [] ) )
	lines.append('')
	lines.append('#include <glm/gtc/type_ptr.hpp>')
	lines.append('')
	lines.append('namespace pds')
	lines.append('    {')

	# type pointer functions (return pointer to first item in each type)
	lines.append(f"\t// item pointer functions, returns a pointer to the first item of each type")
	lines.append('')
	for bit_size in int_bit_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}i{bit_size} *value_ptr( {const_type}i{bit_size} &value ) {{ return &value; }}")
	lines.append('')
	for bit_size in int_bit_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}u{bit_size} *value_ptr( {const_type}u{bit_size} &value ) {{ return &value; }}")
	lines.append('')
	for float_type in float_type_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}{float_type} *value_ptr( {const_type}{float_type} &value ) {{ return &value; }}")
	lines.append('')
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			for const_type in nonconst_const_range:
				lines.append(f"\tinline {const_type}i{bit_size} *value_ptr( {const_type}i{bit_size}vec{vec_dim} &value ) {{ return glm::value_ptr(value); }}")
	lines.append('')
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			for const_type in nonconst_const_range:
				lines.append(f"\tinline {const_type}u{bit_size} *value_ptr( {const_type}u{bit_size}vec{vec_dim} &value ) {{ return glm::value_ptr(value); }}")
	lines.append('')

	# vectors
	for vec_dim in vector_dimension_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}float *value_ptr( {const_type}fvec{vec_dim} &value ) {{ return glm::value_ptr(value); }}")
	for vec_dim in vector_dimension_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}double *value_ptr( {const_type}dvec{vec_dim} &value ) {{ return glm::value_ptr(value); }}")
	lines.append('')

	# matrices
	for vec_dim in vector_dimension_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}float *value_ptr( {const_type}fmat{vec_dim} &value ) {{ return glm::value_ptr(value); }}")
	for vec_dim in vector_dimension_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}double *value_ptr( {const_type}dmat{vec_dim} &value ) {{ return glm::value_ptr(value); }}")
	lines.append('')

	# quaternions
	for const_type in nonconst_const_range:
		lines.append(f"\tinline {const_type}float *value_ptr( {const_type}fquat &value ) {{ return glm::value_ptr(value); }}")
	for const_type in nonconst_const_range:
		lines.append(f"\tinline {const_type}double *value_ptr( {const_type}dquat &value ) {{ return glm::value_ptr(value); }}")
	lines.append('')

	# other types that have no inner item pointer
	same_type_range = ['uuid','hash','string']
	for type in same_type_range:
		for const_type in nonconst_const_range:
			lines.append(f"\tinline {const_type}{type} *value_ptr( {const_type}{type} &value ) {{ return &value; }}")

	# end of namespace
	lines.append('    };')
	lines.append('')

	# reenable warning
	lines.extend( hlp.generate_pop_warnings() )
	
	hlp.write_lines_to_file("../Include/pds/DataValuePointers.h",lines)

# used by CreatePackageHeader to list all needed defines in pds
def ListPackageHeaderDefines():
	lines = []

	# typedef base integer types
	lines.append(f"\t// scalar types")
	for bit_size in int_bit_range:
		lines.append(f"\tusing pds::i{bit_size};")
	for bit_size in int_bit_range:
		lines.append(f"\tusing pds::u{bit_size};")
	lines.append('')

	lines.append(f"\t// ids, hashes and strings")
	lines.append(f"\tusing pds::uuid;")
	lines.append(f"\tusing pds::hash;")
	lines.append('\tusing std::string;')
	lines.append('')

	lines.append(f"\t// container types")
	lines.append('\tusing std::vector;')
	lines.append('\tusing ctle::idx_vector;')
	lines.append('\tusing ctle::optional_idx_vector;')
	lines.append('\tusing ctle::optional_value;')
	lines.append('\tusing ctle::optional_vector;')
	lines.append('')

	# typedef vector types
	lines.append(f"\t// vector types")
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.append(f"\tusing pds::i{bit_size}vec{vec_dim};")
	lines.append('')
	for bit_size in int_bit_range:
		for vec_dim in vector_dimension_range:
			lines.append(f"\tusing pds::u{bit_size}vec{vec_dim};")
	lines.append('')
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::fvec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::dvec{vec_dim};")
	lines.append('')
	
	# typedef matrix types
	lines.append(f"\t// matrix types")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::fmat{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::dmat{vec_dim};")
	lines.append('')

	# typedef quaternions
	lines.append(f"\t// quaternion types")
	lines.append(f"\tusing pds::fquat;")
	lines.append(f"\tusing pds::dquat;")
	lines.append('')

	# typedef standard types
	lines.append(f"\t// standard types from glm")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::ivec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::uvec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::vec{vec_dim};")
	for vec_dim in vector_dimension_range:
		lines.append(f"\tusing pds::mat{vec_dim};")
	lines.append('')

	# inline entity_ref and item_ref
	lines.append(f"\tusing pds::entity_ref;")
	lines.append(f"\tusing pds::item_ref;")
	lines.append('')

	# enum of all data types
	lines.append('\t// value type index enums')
	lines.append('\tusing pds::data_type_index;')
	lines.append('')

	# standard PDS data classes
	lines.append('\t// data classes')
	lines.append('\tusing pds::IndexedVector;')
	lines.append('\tusing pds::ItemTable;')
	lines.append('\tusing pds::Varying;')
	lines.append('\tusing pds::DirectedGraph;')
	lines.append('\tusing pds::BidirectionalMap;')
	lines.append('')

	# type information on all types
	lines.append('\t// type information templates')
	lines.append('\ttemplate <class _Ty> using data_type_information = pds::data_type_information<_Ty>;')
	lines.append('\ttemplate <class _Ty> using combined_type_information = pds::combined_type_information<_Ty>;')
	
	# might not need, wait with defining this one
	#lines.append('\ttemplate <class _Ty> using clear_combined_type = pds::clear_combined_type<_Ty>;')
	return lines

def run():
	DataTypes_h()
	DataTypes_inl()
	DataValuePointers_h()
