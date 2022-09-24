# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

def CreateEntityHeader(item):
	packageName = item.Package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append('#include <pds/pds.h>')
		
	# list dependences that needs to be included in the header
	for dep in item.Dependencies:
		if dep.IncludeInHeader:
			lines.append(f'#include <{dep.PackageName}/{dep.Name}.h>')

	lines.append('')
	lines.append(f'namespace {packageName}')
	lines.append('    {')

	# list dependences that only needs a forward reference in the header
	for dep in item.Dependencies:
		if not dep.IncludeInHeader:
			lines.append(f'    class {dep.Name};')

	if item.IsEntity:
		lines.append(f'    class {item.Name} : public Entity')
	else:
		lines.append(f'    class {item.Name}')
	lines.append('        {')
	lines.append('        public:')

	# list typedefs of templates
	if len(item.Templates) > 0:
		for typ in item.Templates:
			lines.append(typ.Declaration)
		lines.append('')

	lines.append('            class MF;')
	lines.append('            friend MF;')
	lines.append('')

	if item.IsEntity:
		lines.append(f'            virtual const char *EntityTypeString() const;')
		lines.append('')

	lines.append(f'            {item.Name}() = default;')
	lines.append(f'            {item.Name}( const {item.Name} &rval );')
	lines.append(f'            {item.Name} &operator=( const {item.Name} &rval );')
	lines.append(f'            {item.Name}( {item.Name} &&rval ) = default;')
	lines.append(f'            {item.Name} &operator=( {item.Name} &&rval ) = default;')
	if item.IsEntity:
		lines.append(f'            virtual ~{item.Name}() = default;')
	else:
		lines.append(f'            ~{item.Name}() = default;')
	lines.append('')

	lines.append('            // value compare operators')
	lines.append(f'            bool operator==( const {item.Name} &rval ) const;')
	lines.append(f'            bool operator!=( const {item.Name} &rval ) const;')
	lines.append('')

	lines.append('        protected:')
	
	# list variables in item
	for var in item.Variables:
		if var.IsSimpleBaseType:
			lines.append(f'            {var.TypeString} v_{var.Name} = {{}};')
		else:
			lines.append(f'            {var.TypeString} v_{var.Name};')

	lines.append('')
	lines.append('        public:')

	# create accessor ref for variables, const and non-const versions
	for var in item.Variables:
		lines.append(f'            // accessor for referencing variable {var.Name}')
		lines.append(f'            const {var.TypeString} & {var.Name}() const {{ return this->v_{var.Name}; }}')
		lines.append(f'            {var.TypeString} & {var.Name}() {{ return this->v_{var.Name}; }}')
		lines.append('')

	lines.append('        };')

	lines.append('')
	lines.append('    class EntityWriter;')
	lines.append('    class EntityReader;')
	lines.append('    class EntityValidator;')

	lines.append('')
	lines.append(f'    class {item.Name}::MF')
	lines.append('        {')
	lines.append('        public:')
	lines.append(f'            static void Clear( {item.Name} &obj );')
	lines.append(f'            static void DeepCopy( {item.Name} &dest, const {item.Name} *source );')
	lines.append(f'            static bool Equals( const {item.Name} *lvar, const {item.Name} *rvar );')
	lines.append('')
	lines.append(f'            static bool Write( const {item.Name} &obj, EntityWriter &writer );')
	lines.append(f'            static bool Read( {item.Name} &obj, EntityReader &reader );')
	lines.append('')
	lines.append(f'            static bool Validate( const {item.Name} &obj, EntityValidator &validator );')
	lines.append('')
	if item.IsEntity:
		lines.append(f'            static const {item.Name} *EntitySafeCast( const Entity *srcEnt );')
		lines.append(f'            static std::shared_ptr<const {item.Name}> EntitySafeCast( std::shared_ptr<const Entity> srcEnt );')
		lines.append('')
	lines.append('        };')
	lines.append('')
	
	# ctors and copy operator code
	lines.append(f'    inline {item.Name}::{item.Name}( const {item.Name} &rval )')
	lines.append('        {')
	lines.append('        MF::DeepCopy( *this , &rval );')
	lines.append('        }')
	lines.append('')
	lines.append(f'    inline {item.Name} &{item.Name}::operator=( const {item.Name} &rval )')
	lines.append('        {')
	lines.append('        MF::DeepCopy( *this , &rval );')
	lines.append('        return *this;')
	lines.append('        }')
	lines.append('')
	lines.append(f'    inline bool {item.Name}::operator==( const {item.Name} &rval ) const')
	lines.append('        {')
	lines.append('        return MF::Equals( this, &rval );')
	lines.append('        }')
	lines.append('')
	lines.append(f'    inline bool {item.Name}::operator!=( const {item.Name} &rval ) const')
	lines.append('        {')
	lines.append('        return !(MF::Equals( this, &rval ));')
	lines.append('        }')
	lines.append('')

	lines.append('    };')

	hlp.write_lines_to_file(f"{item.Package.HeaderPath}/{item.Name}.h",lines)

def ImplementClearCall(item,var):
	lines = []

	lines.append('')
	lines.append(f'        // clear variable "{var.Name}"')

	# clear all values, base values and Entities
	base_type,base_variant = hlp.get_base_type_variant(var.Type)
	if base_type is not None:
		# we have a base type, add the write code directly
		if var.Optional:
			lines.append(f'        obj.v_{var.Name}.reset();')
		else:
			lines.append(f'        obj.v_{var.Name} = {{}};')
	else:
		lines.append(f'        {var.Type}::MF::Clear( obj.v_{var.Name} );')

	return lines

def ImplementDeepCopyCall(item,var):
	lines = []

	lines.append('')
	lines.append(f'        // copy variable "{var.Name}"')

	# clear all base values, Entities will clear themselves
	# deep copy all values
	if var.IsBaseType:
		# we have a base type, add the copy code directly
		lines.append(f'        dest.v_{var.Name} = source->v_{var.Name};')
	else:
		# this is an item type
		if var.Optional:
			lines.append(f'        if( source->v_{var.Name}.has_value() )')
			lines.append('            {')
			lines.append(f'            dest.v_{var.Name}.set();')
			lines.append(f'            {var.Type}::MF::DeepCopy( dest.v_{var.Name}.value() , &(source->v_{var.Name}.value()) );')
			lines.append('            }')
			lines.append(f'        else')
			lines.append('            {')
			lines.append(f'            dest.v_{var.Name}.reset();')			
			lines.append('            }')
		else:
			lines.append(f'        {var.Type}::MF::DeepCopy( dest.v_{var.Name} , &(source->v_{var.Name}) );')


	return lines

def ImplementEqualsCall(item,var):
	lines = []

	lines.append(f'        // check variable "{var.Name}"')

	# do we have a base type or item?
	if var.IsBaseType:
		# we have a base type, do the compare directly
		lines.append(f'        if( lvar->v_{var.Name} != rvar->v_{var.Name} )')
		lines.append(f'            return false;')
	else:
		# not a base type, so an item. check item
		if var.Optional:
			lines.append(f'        if( !{item.Name}::{var.Type}::MF::Equals(')
			lines.append(f'            lvar->v_{var.Name}.has_value() ? &lvar->v_{var.Name}.value() : nullptr,  ')
			lines.append(f'            rvar->v_{var.Name}.has_value() ? &rvar->v_{var.Name}.value() : nullptr')
			lines.append(f'            ) )')
			lines.append('            return false;')
		else:
			lines.append(f'        if( !{item.Name}::{var.Type}::MF::Equals( &lvar->v_{var.Name} , &rvar->v_{var.Name} ) )')
			lines.append('            return false;')

	lines.append('')

	return lines

def ImplementWriterCall(item,var):
	lines = []

	# do we have a base type or item?
	if var.IsBaseType:
		# we have a base type, add the write code directly
		lines.append(f'        // write variable "{var.Name}"')
		lines.append(f'        success = writer.Write<{var.TypeString}>( pdsKeyMacro("{var.Name}") , obj.v_{var.Name} );')
		lines.append(f'        if( !success )')
		lines.append(f'            return false;')
		lines.append('')
	else:
		# not a base type, so an item. add a block
		lines.append(f'        // write section "{var.Name}"')
		lines.append(f'        success = (section_writer = writer.BeginWriteSection( pdsKeyMacro("{var.Name}") ));')
		lines.append('        if( !success )')
		lines.append('            return false;')
		if var.Optional:
			lines.append(f'        if( obj.v_{var.Name}.has_value() )')
			lines.append('            {')
			lines.append(f'            if( !{item.Name}::{var.Type}::MF::Write( obj.v_{var.Name}.value(), *section_writer ) )')
			lines.append('                return false;')
			lines.append('            }')
		else:
			lines.append(f'        if( !{item.Name}::{var.Type}::MF::Write( obj.v_{var.Name}, *section_writer ) )')
			lines.append('            return false;')
		lines.append('        writer.EndWriteSection( section_writer );')
		lines.append('        section_writer = nullptr;')
		lines.append('')

	return lines

def ImplementReaderCall(item,var):
	lines = []

	if var.Optional:
		value_can_be_null = "true"
	else:
		value_can_be_null = "false"

	# do we have a base type or item?
	if var.IsBaseType:
		# we have a base type, add the read code directly
		lines.append(f'        // read variable "{var.Name}"')
		lines.append(f'        success = reader.Read<{var.TypeString}>( pdsKeyMacro("{var.Name}") , obj.v_{var.Name} );')
		lines.append(f'        if( !success )')
		lines.append(f'            return false;')
		lines.append('')
	else:
		# not a base type, so an item. add a block
		lines.append(f'        // read section "{var.Name}"')
		lines.append(f'        std::tie(section_reader,success) = reader.BeginReadSection( pdsKeyMacro("{var.Name}") , {value_can_be_null} );')
		lines.append('        if( !success )')
		lines.append('            return false;')
		lines.append('        if( section_reader )')
		lines.append('            {')
		if var.Optional:
			lines.append(f'            obj.v_{var.Name}.set();')
			lines.append(f'            if( !{item.Name}::{var.Type}::MF::Read( obj.v_{var.Name}.value(), *section_reader ) )')
		else:
			lines.append(f'            if( !{item.Name}::{var.Type}::MF::Read( obj.v_{var.Name}, *section_reader ) )')
		lines.append('                return false;')
		lines.append('            reader.EndReadSection( section_reader );')
		lines.append('            section_reader = nullptr;')
		lines.append('            }')
		if var.Optional:
			lines.append('        else')
			lines.append(f'            obj.v_{var.Name}.reset();')
		lines.append('')

	return lines

def ImplementVariableValidatorCall(item,var):
	lines = []

	# validate all values, base values and Entities
	base_type,base_variant = hlp.get_base_type_variant(var.Type)
	if base_type is None:
		lines.append(f'        // validate variable "{var.Name}"')
		if var.Optional:
			lines.append(f'        if( obj.v_{var.Name}.has_value() )')
			lines.append('            {')
			lines.append(f'            success = {var.Type}::MF::Validate( obj.v_{var.Name}.value() , validator );')
			lines.append('            if( !success )')
			lines.append('                return false;')
			lines.append('            }')
		else:
			lines.append(f'        success = {var.Type}::MF::Validate( obj.v_{var.Name} , validator );')
			lines.append('        if( !success )')
			lines.append('            return false;')
		lines.append('')

	return lines


def CreateEntitySource(item):
	packageName = item.Package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <glm/glm.hpp>')
	lines.append('')	
	lines.append(f'#include <pds/pds.h>')
	lines.append(f'#include <pds/EntityWriter.h>')
	lines.append(f'#include <pds/EntityReader.h>')
	lines.append(f'#include <pds/EntityValidator.h>')
	lines.append('')
	lines.append(f'#include <{packageName}/{item.Name}.h>')
		
	# include dependences that were forward referenced in the header
	for dep in item.Dependencies:
		if not dep.IncludeInHeader:
			lines.append(f'#include <{dep.PackageName}/{dep.Name}.h>')
		
	lines.append('')
	lines.append(f'namespace {packageName}')
	lines.append('    {')
	
	if item.IsEntity:
		lines.append(f'    const char *{item.Name}::EntityTypeString() const {{ return "{item.Name}"; }}')
		lines.append('')

	# check if there are entities in the variable list
	vars_have_item = False
	for var in item.Variables:
		base_type,base_variant = hlp.get_base_type_variant(var.Type)
		if base_type is None:
			vars_have_item = True
			break
	
	# clear code
	lines.append(f'    void {item.Name}::MF::Clear( {item.Name} &obj )')
	lines.append('        {')
	lines.append('        // direct clear calls on variables and Entities')
	for var in item.Variables:
		lines.extend(ImplementClearCall(item,var))
	lines.append('        }')
	lines.append('')

	# deep copy code
	lines.append(f'    void {item.Name}::MF::DeepCopy( {item.Name} &dest, const {item.Name} *source )')
	lines.append('        {')
	lines.append('        // just call Clear if source is nullptr')
	lines.append('        if( !source )')
	lines.append('            {')
	lines.append('            MF::Clear( dest );')
	lines.append('            return;')
	lines.append('            }')
	for var in item.Variables:
		lines.extend(ImplementDeepCopyCall(item,var))
	lines.append('        }')
	lines.append('')

	# equals code
	lines.append(f'    bool {item.Name}::MF::Equals( const {item.Name} *lvar, const {item.Name} *rvar )')
	lines.append('        {')
	lines.append('        // early out if pointers are equal')
	lines.append('        if( lvar == rvar )')
	lines.append('            return true;')
	lines.append('')
	lines.append('        // early out if one of the pointers is nullptr')
	lines.append('        if( !lvar || !rvar )')
	lines.append('            return false;')
	lines.append('')
	for var in item.Variables:
		lines.extend(ImplementEqualsCall(item,var))
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')

	# writer code
	lines.append(f'    bool {item.Name}::MF::Write( const {item.Name} &obj, EntityWriter &writer )')
	lines.append('        {')
	lines.append('        bool success = true;')
	if vars_have_item:
		lines.append('        EntityWriter *section_writer = nullptr;')
	lines.append('')
	for var in item.Variables:
		lines.extend(ImplementWriterCall(item,var))
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	
	# reader code
	lines.append(f'    bool {item.Name}::MF::Read( {item.Name} &obj, EntityReader &reader )')
	lines.append('        {')
	lines.append('        bool success = true;')
	if vars_have_item:
		lines.append('        EntityReader *section_reader = nullptr;')
	lines.append('')
	for var in item.Variables:
		lines.extend(ImplementReaderCall(item,var))
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	
	# validator code
	lines.append(f'    bool {item.Name}::MF::Validate( const {item.Name} &obj, EntityValidator &validator )')
	lines.append('        {')
	lines.append('        bool success = true;')
	lines.append('')
	for var in item.Variables:
		lines.extend(ImplementVariableValidatorCall(item,var))
	for validation in item.Validations:
		lines.extend( validation.GenerateValidationCode(item,'        ') )
		lines.append('')
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')

	# entity code
	if item.IsEntity:
		lines.append(f'    const {item.Name} *{item.Name}::MF::EntitySafeCast( const Entity *srcEnt )')
		lines.append('        {')
		lines.append(f'        if( srcEnt && std::string(srcEnt->EntityTypeString()) == "{item.Name}" )')
		lines.append('            {')
		lines.append(f'            return (const {item.Name} *)(srcEnt);')
		lines.append('            }')
		lines.append('        return nullptr;')
		lines.append('        }')
		lines.append('')
		lines.append(f'    std::shared_ptr<const {item.Name}> {item.Name}::MF::EntitySafeCast( std::shared_ptr<const Entity> srcEnt )')
		lines.append('        {')
		lines.append(f'        if( srcEnt && std::string(srcEnt->EntityTypeString()) == "{item.Name}" )')
		lines.append('            {')
		lines.append(f'            return std::static_pointer_cast<const {item.Name}>(srcEnt);')
		lines.append('            }')
		lines.append('        return nullptr;')
		lines.append('        }')
		lines.append('')

	lines.append('    };')
	hlp.write_lines_to_file(f"{item.Package.SrcPath}/{item.Name}.cpp",lines)

def run( package ):
	for item in package.Items:
		CreateEntityHeader( item )
		CreateEntitySource( item )

