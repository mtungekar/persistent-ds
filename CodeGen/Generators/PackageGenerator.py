# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

from EntitiesHelpers import * 
import os
import CodeGeneratorHelpers as hlp

from ctypes import c_ulonglong 
from ctypes import c_ubyte

def CreateItemHeader(item: Item):
	packageName = item.Package.Name
	versionName = item.Version.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append(f'#include "../pdsImportsAndDefines.h"')

	# is this an alias of a previous version?
	if item.IdenticalToPreviousVersion:
		previousVersionName = item.PreviousVersion.Version.Name

		# if this is just an alias, define a using and reference back to the actual entity
		lines.append(f'#include "../{previousVersionName}/{previousVersionName}_{item.Name}.h"')
		lines.append('')
		lines.append(f'namespace {packageName}')
		lines.append('    {')
		lines.append(f'namespace {versionName}')
		lines.append('    {')
		lines.append(f'    // {item.Name} is identical to the previous version {previousVersionName}')
		lines.append(f'    using {item.Name} = {previousVersionName}::{item.Name};')
		lines.append('    };')
		lines.append('    };')		

	else:
		# not an alias, defined the whole class
		if item.IsModifiedFromPreviousVersion:
			previousVersionName = item.PreviousVersion.Version.Name
			lines.append(f'#include "../{previousVersionName}/{previousVersionName}_{item.Name}.h"')
		
		# list dependences that needs to be included in the header
		for dep in item.Dependencies:
			if dep.IncludeInHeader:
				if dep.PDSType:
					lines.append(f'#include <pds/{dep.Name}.h>')
				else:
					lines.append(f'#include "{versionName}_{dep.Name}.h"')				

		lines.append('')
		lines.append(f'namespace {packageName}')
		lines.append('    {')
		lines.append(f'namespace {versionName}')
		lines.append('    {')

		# list dependences that only needs a forward reference in the header
		for dep in item.Dependencies:
			if not dep.IncludeInHeader:
				lines.append(f'    class {dep.Name};')

		if item.IsEntity:
			lines.append(f'    class {item.Name} : public pds::Entity')
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
		lines.append(f'            static constexpr char *ItemTypeString = "{packageName}.{versionName}.{item.Name}";')
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
		lines.append(f'            static bool Write( const {item.Name} &obj, pds::EntityWriter &writer );')
		lines.append(f'            static bool Read( {item.Name} &obj, pds::EntityReader &reader );')
		lines.append('')
		lines.append(f'            static bool Validate( const {item.Name} &obj, pds::EntityValidator &validator );')
		lines.append('')
		if item.IsEntity:
			lines.append(f'            static const {item.Name} *EntitySafeCast( const pds::Entity *srcEnt );')
			lines.append(f'            static std::shared_ptr<const {item.Name}> EntitySafeCast( std::shared_ptr<const pds::Entity> srcEnt );')
			lines.append('')
		if item.IsModifiedFromPreviousVersion:
			lines.append(f'            static bool ToPrevious( {item.PreviousVersion.Version.Name}::{item.Name} &dest , const {item.Name} &source );')
			lines.append(f'            static bool FromPrevious( {item.Name} &dest , const {item.PreviousVersion.Version.Name}::{item.Name} &source );')
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
		lines.append('    };')		

	hlp.write_lines_to_file(f"{item.Package.Path}/{versionName}/{versionName}_{item.Name}.h",lines)

def ImplementClearCall(item,var):
	lines = []

	lines.append('')
	lines.append(f'        // clear variable "{var.Name}"')

	# clear all values, base values and Entities
	if var.Optional:
		lines.append(f'        obj.v_{var.Name}.reset();')
	else:
		base_type,base_variant = hlp.get_base_type_variant(var.Type)
		if base_type is not None:
			# we have a base type, add the write code directly
			lines.append(f'        obj.v_{var.Name} = {{}};')
		else:
			# clear through the MF::Clear method of the type
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

def ImplementToPreviousCall(item:Item , mapping:Mapping):
	lines = []	

	# if code inject, do that and return
	if type(mapping) is CustomCodeMapping:
		lines.append(mapping.ToPrevious)
		return lines

	# if it is a deleted variable, just return empty
	if type(mapping) is DeletedVariable:
		return []

	# not custom code, so there is exactly one variable
	variableName = mapping.Variables[0]
	
	# find variable in item
	variable = next( (var for var in item.Variables if var.Name == variableName) , None )
	if variable == None:
		return []

	# validate all values, base values and Entities
	base_type,base_variant = hlp.get_base_type_variant(variable.Type)

	if type(mapping) is NewVariable: # if this is a new variable, not much we can do converting back
		return []

	if type(mapping) is RenamedVariable: # renamed or same variable, copy to the previous name in the dest
		if base_type is None:
			lines.append(f'        success = {variable.Type}::MF::Copy( dest.{mapping.PreviousName}() , obj.v_{variable.Name} );')
			lines.append('        if( !success )')
			lines.append('            return false;')
		else:
			lines.append(f'        dest.{mapping.PreviousName}() = obj.v_{variable.Name};')

	return lines

def ImplementFromPreviousCall(item:Item , mapping:Mapping):
	lines = []	

	# if code inject, do that and return
	if type(mapping) is CustomCodeMapping:
		lines.append(mapping.FromPrevious)
		return lines

	# if it is a deleted variable, just return empty
	if type(mapping) is DeletedVariable:
		return []

	# not custom code, so there is exactly one variable
	variableName = mapping.Variables[0]
	
	# find variable in item
	variable = next( (var for var in item.Variables if var.Name == variableName) , None )
	if variable == None:
		return []

	# validate all values, base values and Entities
	base_type,base_variant = hlp.get_base_type_variant(variable.Type)

	if type(mapping) is NewVariable: # if this is a new variable, clear it
		return ImplementClearCall(item,variable)

	if type(mapping) is RenamedVariable: # renamed or same variable, copy to the previous name in the dest
		if base_type is None:
			lines.append(f'        success = {variable.Type}::MF::Copy( obj.v_{variable.Name} , src.{mapping.PreviousName}() );')
			lines.append('        if( !success )')
			lines.append('            return false;')
		else:
			lines.append(f'        obj.v_{variable.Name} = src.{mapping.PreviousName}();')

	return lines


def CreateItemSource(item):
	packageName = item.Package.Name
	versionName = item.Version.Name

	# if this is an aliased entity, dont generate an inl file
	if item.IdenticalToPreviousVersion:
		return

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <glm/glm.hpp>')
	lines.append('')	
	lines.append(f'#include <pds/EntityWriter.h>')
	lines.append(f'#include <pds/EntityReader.h>')
	lines.append(f'#include <pds/EntityValidator.h>')
	lines.append('')
	lines.append(f'#include "{versionName}_{item.Name}.h"')
		
	# include dependences that were forward referenced in the header
	for dep in item.Dependencies:
		if not dep.IncludeInHeader:
			if dep.PDSType:
				lines.append(f'#include <pds/{dep.Name}.h>')
			else:
				lines.append(f'#include "{versionName}_{dep.Name}.h"')

	lines.append('')
	lines.append(f'namespace {packageName}')
	lines.append('    {')
	lines.append(f'namespace {versionName}')
	lines.append('    {')

	lines.append('')
	if item.IsEntity:
		lines.append(f'    const char *{item.Name}::EntityTypeString() const {{ return {item.Name}::ItemTypeString; }}')
		lines.append('')

	# check if there are entities in the variable list, which means we need to add entity writers/readers/validators
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
	lines.append(f'    bool {item.Name}::MF::Write( const {item.Name} &obj, pds::EntityWriter &writer )')
	lines.append('        {')
	lines.append('        bool success = true;')
	if vars_have_item:
		lines.append('        pds::EntityWriter *section_writer = nullptr;')
	lines.append('')
	for var in item.Variables:
		lines.extend(ImplementWriterCall(item,var))
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	
	# reader code
	lines.append(f'    bool {item.Name}::MF::Read( {item.Name} &obj, pds::EntityReader &reader )')
	lines.append('        {')
	lines.append('        bool success = true;')
	if vars_have_item:
		lines.append('        pds::EntityReader *section_reader = nullptr;')
	lines.append('')
	for var in item.Variables:
		lines.extend(ImplementReaderCall(item,var))
	lines.append('        return true;')
	lines.append('        }')
	lines.append('')
	
	# validator code
	lines.append(f'    bool {item.Name}::MF::Validate( const {item.Name} &obj, pds::EntityValidator &validator )')
	lines.append('        {')

	# setup validation lines first, and see if there are any lines generated
	validation_lines = []
	for var in item.Variables:
		validation_lines.extend(ImplementVariableValidatorCall(item,var))
	for validation in item.Validations:
		validation_lines.extend( validation.GenerateValidationCode(item,'        ') )
		validation_lines.append('')

	# if we have validation lines, setup the support code
	if len(validation_lines) > 0:
		lines.append('        bool success = {};')
		lines.append('')
		lines.extend( validation_lines )
		lines.append('')
	else:
		lines.append('        // no validation, just reference the objects to silence warnings, and return')
		lines.append('        obj;')
		lines.append('        validator;')

	lines.append('        return true;')
	lines.append('        }')
	lines.append('')

	# entity code
	if item.IsEntity:
		lines.append(f'    const {item.Name} *{item.Name}::MF::EntitySafeCast( const pds::Entity *srcEnt )')
		lines.append('        {')
		lines.append(f'        if( srcEnt && std::string(srcEnt->EntityTypeString()) == {item.Name}::ItemTypeString )')
		lines.append('            {')
		lines.append(f'            return (const {item.Name} *)(srcEnt);')
		lines.append('            }')
		lines.append('        return nullptr;')
		lines.append('        }')
		lines.append('')
		lines.append(f'    std::shared_ptr<const {item.Name}> {item.Name}::MF::EntitySafeCast( std::shared_ptr<const pds::Entity> srcEnt )')
		lines.append('        {')
		lines.append(f'        if( srcEnt && std::string(srcEnt->EntityTypeString()) == {item.Name}::ItemTypeString )')
		lines.append('            {')
		lines.append(f'            return std::static_pointer_cast<const {item.Name}>(srcEnt);')
		lines.append('            }')
		lines.append('        return nullptr;')
		lines.append('        }')
		lines.append('')

	# modified item code
	if item.IsModifiedFromPreviousVersion:
		lines.append(f'    bool {item.Name}::MF::ToPrevious( {item.PreviousVersion.Version.Name}::{item.Name} &dest , const {item.Name} &obj )')
		lines.append('        {')
		lines.append('        bool success = {};')
		lines.append('')			
		for mapping in item.Mappings:
			lines.extend(ImplementToPreviousCall(item,mapping))	
		lines.append('')			
		lines.append('        return success;')
		lines.append('        }')
		lines.append('')
		lines.append(f'    bool {item.Name}::MF::FromPrevious( {item.Name} &obj , const {item.PreviousVersion.Version.Name}::{item.Name} &src )')
		lines.append('        {')
		lines.append('        bool success = {};')
		lines.append('')			
		for mapping in item.Mappings:
			lines.extend(ImplementFromPreviousCall(item,mapping))	
		lines.append('')			
		lines.append('        return success;')
		lines.append('        }')

	lines.append('    };')
	lines.append('    };')	
	hlp.write_lines_to_file(f"{item.Package.Path}/{versionName}/{versionName}_{item.Name}.inl",lines)


# static and constant hash table for entity lookup, (must be larger than the number of entities to add)
# Fowler–Noll–Vo FNV-1a hash function  https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
class EntityHashTable:
	
	# select the next prime number above the item count as the table size
	def calc_table_size( self , item_count ):
		# start with item_count*2 (but minimum 20), then count up to the next prime
		table_size = max( item_count*2 , 20 )
		while True:
			is_prime = True
			for num in range(2, int(table_size**0.5) + 1):
				if table_size % num == 0:
					is_prime = False
					break
			if is_prime:
				return table_size
			table_size += 1

	def hash_function( self , entity_name ):
		hash = c_ulonglong(0xcbf29ce484222325)
		for ch in entity_name:
			hash = c_ulonglong(hash.value ^ c_ubyte(ord(ch)).value)
			hash = c_ulonglong(hash.value * c_ulonglong(0x00000100000001B3).value)
		return hash.value

	def insert_into_table( self , package_name , version_name , entity_name ):
		# use hash function to generate a good starting point
		hash_pos = self.hash_function( package_name + "." + version_name + "." + entity_name ) % self.hash_table_size
		# find first empty slot
		while( self.hash_table[hash_pos] != None ):
			hash_pos = hash_pos+1
			if( hash_pos >= self.hash_table_size ):
				hash_pos = 0
		# fill it
		self.hash_table[hash_pos] = version_name + "_" + entity_name

	def __init__(self, package):
		item_count = 0
		for version in package.Versions:
			item_count = len(version.Items)
		
		self.hash_table_size = self.calc_table_size( item_count ) 
		self.hash_data_type_mult = 109
		self.hash_container_type_mult = 991
		self.hash_table = [None] * self.hash_table_size

		# fill up hash table, only fill with entities
		for version in package.Versions:
			for item in version.Items:
				if item.IsEntity and not item.IdenticalToPreviousVersion and not item.IsDeleted:
					self.insert_into_table( package.Name , version.Name , item.Name );

def CreatePackageHandler_inl( package: Package ):
	packageName = package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <pds/pds.h>')
	lines.append('#include <pds/Varying.h>')
	lines.append('')

	for version in package.Versions:
		for item in version.Items:
			if not item.IsDeleted and not item.IdenticalToPreviousVersion and item.IsEntity:
				lines.append(f'#include "{version.Name}/{version.Name}_{item.Name}.h"')

	lines.append('')
	lines.append(f'namespace {packageName}')
	lines.append('    {')
	lines.append('namespace entity_types')
	lines.append('    {')
	
	lines.append('    // dynamic allocation functors for items')
	lines.append('    class _entityTypeClass')
	lines.append('        {')
	lines.append('        public:')
	lines.append('            virtual const char *EntityTypeString() const = 0;')
	lines.append('            virtual std::shared_ptr<pds::Entity> New() const = 0;')
	lines.append('            virtual void Clear( pds::Entity *obj ) const = 0;')
	lines.append('            virtual bool Equals( const pds::Entity *lval , const pds::Entity *rval ) const = 0;')
	lines.append('            virtual bool Write( const pds::Entity *obj, pds::EntityWriter &writer ) const = 0;')
	lines.append('            virtual bool Read( pds::Entity *obj, pds::EntityReader &reader ) const = 0;')
	lines.append('            virtual bool Validate( const pds::Entity *obj, pds::EntityValidator &validator ) const = 0;')
	lines.append('        };')
	lines.append('')

	# add all (unique) entities of all versions
	for version in package.Versions:
		for item in version.Items:
			if item.IsEntity and not item.IdenticalToPreviousVersion and not item.IsDeleted:
				namespacedItemName = f'{version.Name}::{item.Name}'
				lines.append(f'    // {namespacedItemName}' )
				lines.append(f'    static const class _et_{version.Name}_{item.Name}_EntityType : public _entityTypeClass' )
				lines.append(f'        {{' )
				lines.append(f'        public:' )
				lines.append(f'            virtual const char *EntityTypeString() const {{ return {namespacedItemName}::ItemTypeString; }}' )
				lines.append(f'            virtual std::shared_ptr<pds::Entity> New() const {{ return std::make_shared<{namespacedItemName}>(); }}')
				lines.append(f'            virtual void Clear( pds::Entity *obj ) const {{ {namespacedItemName}::MF::Clear( *(({namespacedItemName}*)obj) ); }}')
				lines.append(f'            virtual bool Equals( const pds::Entity *lval , const pds::Entity *rval ) const {{ return {namespacedItemName}::MF::Equals( (({namespacedItemName}*)lval) , (({namespacedItemName}*)rval) ); }}')
				lines.append(f'            virtual bool Write( const pds::Entity *obj, pds::EntityWriter &writer ) const {{ return {namespacedItemName}::MF::Write( *(({namespacedItemName}*)obj) , writer ); }}')
				lines.append(f'            virtual bool Read( pds::Entity *obj, pds::EntityReader &reader ) const {{ return {namespacedItemName}::MF::Read( *(({namespacedItemName}*)obj) , reader ); }}')
				lines.append(f'            virtual bool Validate( const pds::Entity *obj, pds::EntityValidator &validator ) const {{ return {namespacedItemName}::MF::Validate( *(({namespacedItemName}*)obj) , validator ); }}')
				lines.append(f'        }} _et_{version.Name}_{item.Name}_EntityTypeObject;' )
				lines.append('')

	# allocate and print hash table
	hash_table = EntityHashTable( package )

	# print it 
	lines.append('    // Hash table with the type entity handler objects')
	lines.append(f'    static const _entityTypeClass *_entityTypeClassHashTable[{hash_table.hash_table_size}] = ')
	lines.append('        {')
	for row_start in range(0,hash_table.hash_table_size,10):
		row_str = ''
		row_end = min(row_start+10,hash_table.hash_table_size)
		for idx in range(row_start,row_end):
			if hash_table.hash_table[idx] == None:
				row_str += 'nullptr,'
			else:
				row_str += f'&_et_{hash_table.hash_table[idx]}_EntityTypeObject,'
		lines.append('        ' + row_str + f' // items {row_start} to {row_end-1}' )
	lines.append('        };')
	lines.append('')
	lines.append('    // hash table lookup of entityType')
	lines.append('    static const _entityTypeClass *_findEntityTypeClass( const char *typeNameString )')
	lines.append('        {')
	lines.append('        // calculate hash value using Fowler-Noll-Vo FNV-1a hash function')
	lines.append('        u64 hash = 0xcbf29ce484222325;')
	lines.append('        for( const char *chP = typeNameString ; *chP != \'\\0\' ; ++chP )')
	lines.append('            {')
	lines.append('            hash ^= (u8)(*chP);')
	lines.append('            hash *= (u64)(0x00000100000001B3);')
	lines.append('            }')
	lines.append('')
	lines.append('        // look for entry in table. ')
	lines.append(f'        u64 hashValue = hash % {hash_table.hash_table_size};')
	lines.append('        while( _entityTypeClassHashTable[hashValue] != nullptr )')
	lines.append('            {')
	lines.append('            if( strcmp( _entityTypeClassHashTable[hashValue]->EntityTypeString() , typeNameString ) == 0 )')
	lines.append('                return _entityTypeClassHashTable[hashValue];')
	lines.append('            ++hashValue;')
	lines.append(f'            if(hashValue >= {hash_table.hash_table_size})')
	lines.append('                hashValue = 0;')
	lines.append('            }')
	lines.append('')
	lines.append('        // entity was not found (this should never happen unless testing)')
	lines.append('        pdsErrorLog << "_findEntityTypeClass: Invalid entity parameter { " << typeNameString << " } " << pdsErrorLogEnd;')
	lines.append('        return nullptr;')
	lines.append('        }')
	lines.append('')
	
	lines.append('\t};')
	
	lines.append("""
	static const class CreatePackageHandler : public pds::EntityHandler::PackageRecord
		{
		public:
			virtual std::shared_ptr<pds::Entity> New( const char *entityTypeString ) const
				{
				if( !entityTypeString )
					{
					pdsErrorLog << "Invalid parameter, data must be name of entity type" << pdsErrorLogEnd;
					return nullptr;
					}
				const entity_types::_entityTypeClass *ta = entity_types::_findEntityTypeClass( entityTypeString );
				if( !ta )
					return nullptr;
				return ta->New();
				}
		
			virtual bool Write( const pds::Entity *obj, pds::EntityWriter &writer ) const
				{
				if( !obj )
					{
					pdsErrorLog << "Invalid parameter, data must be a pointer to allocated object" << pdsErrorLogEnd;
					return false;
					}
				const entity_types::_entityTypeClass *ta = entity_types::_findEntityTypeClass( obj->EntityTypeString() );
				if( !ta )
					return false;
				return ta->Write( obj , writer );
				}
		
			virtual bool Read( pds::Entity *obj, pds::EntityReader &reader ) const
				{
				if( !obj )
					{
					pdsErrorLog << "Invalid parameter, data must be a pointer to allocated object" << pdsErrorLogEnd;
					return false;
					}
				const entity_types::_entityTypeClass *ta = entity_types::_findEntityTypeClass( obj->EntityTypeString() );
				if( !ta )
					return false;
				return ta->Read( obj , reader );
				}
		
			virtual bool Validate( const pds::Entity *obj, pds::EntityValidator &validator ) const
				{
				if( !obj )
					{
					pdsErrorLog << "Invalid parameter, data must be a pointer to allocated object" << pdsErrorLogEnd;
					return false;
					}
				const entity_types::_entityTypeClass *ta = entity_types::_findEntityTypeClass( obj->EntityTypeString() );
				if( !ta )
					return false;
				return ta->Validate( obj , validator );
				}
				
		} _createPackageHandlerObject;

	const pds::EntityHandler::PackageRecord *GetPackageRecord() { return &_createPackageHandlerObject; }
	""")

	# end of namespace
	lines.append('\t};')
	hlp.write_lines_to_file(f"{package.Path}/{packageName}PackageHandler.inl",lines)


from .DataTypes import ListPackageHeaderDefines

# create a header for the package, which has all needed references and definitions
def CreatePackageHeader( package ):
	packageName = package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
	lines.append(f'#include <pds/pds.h>')
	lines.append(f'#include <pds/ValueTypes.h>')
	lines.append('')
		
	lines.append('')
	lines.append(f'namespace {packageName}')
	lines.append('\t{')
	lines.extend( ListPackageHeaderDefines() )
	lines.append('')
	lines.append('\tconst pds::EntityHandler::PackageRecord *GetPackageRecord();')
	lines.append('\t};')

	hlp.write_lines_to_file(f"{package.Path}/pdsImportsAndDefines.h",lines)

def CreatePackageSourceFile( package: Package ):
	packageName = package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('// All pds imports and typedefs')
	lines.append(f'#include "pdsImportsAndDefines.h"')
	lines.append('')

	lines.append('// Needed inline code from pds')
	lines.append('#include <pds/EntityReader.inl>')
	lines.append('#include <pds/EntityWriter.inl>')
	lines.append('')
	
	lines.append('// All versions of this package')
	for version in package.Versions:
		for item in version.Items:
			if not item.IsDeleted:
				lines.append(f'#include "{version.Name}/{version.Name}_{item.Name}.h"')
		lines.append('')
	
	lines.append('// Include all inl implementations of all versions')
	for version in package.Versions:		
		# include inl files for all new items in version
		for item in version.Items:
			if not item.IsDeleted and not item.IdenticalToPreviousVersion:
				lines.append(f'#include "{version.Name}/{version.Name}_{item.Name}.inl"')
	lines.append('')

	lines.append('// Include the package handler for this package')
	lines.append(f'#include "{packageName}PackageHandler.inl"')

	hlp.write_lines_to_file(f"{package.Path}/{packageName}.cpp",lines)

def CreateDefaultVersionReferencesAndHeaders( version: Version ):
	package = version.Package

	for item in version.Items:
		if item.IsEntity and not item.IsDeleted:
			lines = []
			
			# point at the latest implemented version of the entity
			implementVersionName = version.Name
			if item.IdenticalToPreviousVersion:
				implementVersionName = item.PreviousVersion.Version.Name

			lines.extend( hlp.generate_header() )
			lines.append('')
			lines.append('#pragma once')
			lines.append('')
			lines.append(f'#include "{implementVersionName}/{implementVersionName}_{item.Name}.h"')
			lines.append(f'namespace {package.Name}')
			lines.append('\t{')
			lines.append(f'\tusing {item.Name} = {implementVersionName}::{item.Name};' )
			lines.append('\t}')

			hlp.write_lines_to_file(f"{package.Path}/{item.Name}.h",lines)

def FindAndCreateDefaultVersionReferencesAndHeaders( package: Package , defaultVersion:str ):
	# if we want default version headers and references directly in the Package
	if defaultVersion != None:
		if defaultVersion == "Latest":

			# look for a version which does not have a later version
			hasFoundALatest = False
			for version in package.Versions:	
				# for each version, check if any other version points at it
				hasLater = False
				for laterVersion in package.Versions:	
					if laterVersion == version:
						continue
					if laterVersion.PreviousVersion == version:
						hasLater = True
						break
				if not hasLater:
					if hasFoundALatest:
						print('Error: The package has "Latest" set as selected default version, but there are more than one leaf versions.')
						exit(1)
					CreateDefaultVersionReferencesAndHeaders( version )
					hasFoundALatest = True
					break

			# make sure one was found
			if not hasFoundALatest:
				print('Error: The package has "Latest" set as selected default version, but no leaf version was found.')
				exit(1)

		else:
			# set a specific version as the latest
			hasFoundVersion = False
			for version in package.Versions:
				if version.Name == defaultVersion:
					CreateDefaultVersionReferencesAndHeaders( version )
					hasFoundVersion = True
					break
			
			# make sure one was found
			if not hasFoundVersion:
				print(f'Error: The package has "{defaultVersion}" set as selected default version, but no leaf version was found.')
				exit(1)


def run( package: Package, defaultVersion:str = None ):
	
	os.makedirs(package.Path, exist_ok=True)
	for version in package.Versions:
		os.makedirs(package.Path + '/' + version.Name , exist_ok=True)

	CreatePackageHeader( package )
	CreatePackageSourceFile( package )
	CreatePackageHandler_inl( package )
	
	# generate all items
	for version in package.Versions:
		for item in version.Items:
			if not item.IsDeleted:
				CreateItemHeader( item )
				CreateItemSource( item )
	
	FindAndCreateDefaultVersionReferencesAndHeaders( package, defaultVersion )
