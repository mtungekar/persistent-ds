# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE


import CodeGeneratorHelpers as hlp

def CombinedTypes_h():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append('#include "pds.h"')
	lines.append('')
	lines.append('#include <ctle/idx_vector.h>')
	lines.append('#include <ctle/optional_idx_vector.h>')
	lines.append('#include <ctle/optional_value.h>')
	lines.append('#include <ctle/optional_vector.h>')
	lines.append('')
	lines.append('// Lists information of combined types: a combination of data type and container, ')
	lines.append('// even when the container is "none" and the data type is used directly')
	lines.append('')
	lines.append('namespace pds')
	lines.append('    {')
	lines.append('    using std::vector;')
	lines.append('    using ctle::idx_vector;')
	lines.append('    using ctle::optional_idx_vector;')
	lines.append('    using ctle::optional_value;')
	lines.append('    using ctle::optional_vector;')
	lines.append('')
	lines.append('    // type_information stores information on the combined types in PDS')
	lines.append('    template <class _Ty> struct combined_type_information;')
	lines.append('')
	lines.append('    // clears the combined type')
	lines.append('    template <class _Ty> void clear_combined_type( _Ty& type );')
	lines.append('')

	def generate_type_information( base_type_name , implementing_type , container_type , item_type , num_items_per_object , base_type_combo ):
		lines = []
		lines.append(f'    // {base_type_combo}' )
		lines.append(f'    template<> struct combined_type_information<{base_type_combo}>' )
		lines.append(f'        {{' )
		lines.append(f'        using data_type = {implementing_type};' )
		lines.append(f'        static constexpr data_type_index type_index = data_type_index::dt_{implementing_type}; // the data type index of {base_type_combo} ( dt_{implementing_type} )')
		lines.append(f'        static constexpr container_type_index container_index = container_type_index::ct_{container_type}; // the container type index of {base_type_combo} ( dt_{container_type} )')
		lines.append(f'        }};' )
		lines.append(f'')
		return lines
	lines.extend( hlp.function_for_all_basetype_combos( generate_type_information ))

	# end of namespaces
	lines.append('    };')
	hlp.write_lines_to_file("../Include/pds/ValueTypes.h",lines)

def CombinedTypes_inl():
	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include "ValueTypes.h"')
	lines.append('')
	lines.append('#include <glm/gtc/type_ptr.hpp>')
	lines.append('')
	lines.append('namespace pds')
	lines.append('    {')

	def generate_clear_combined_type( base_type_name , implementing_type , container_type , item_type , num_items_per_object , base_type_combo ):
		lines = []
		if( container_type == 'none' ):
			lines.append(f'    template <> void clear_combined_type<{base_type_combo}>( {base_type_combo} &type ) {{ type = data_type_information<{base_type_combo}>::zero; }}' )
		elif( container_type == 'vector' or container_type == 'idx_vector' ):
			lines.append(f'    template <> void clear_combined_type<{base_type_combo}>( {base_type_combo} &type ) {{ type.clear(); }}' )
		else:
			lines.append(f'    template <> void clear_combined_type<{base_type_combo}>( {base_type_combo} &type ) {{ type.reset(); }}' )
		return lines
	lines.extend( hlp.function_for_all_basetype_combos( generate_clear_combined_type ))


	# end of namespace
	lines.append('    };')
	hlp.write_lines_to_file("../Include/pds/ValueTypes.inl",lines)
	
def run():
	CombinedTypes_h()
	CombinedTypes_inl()
