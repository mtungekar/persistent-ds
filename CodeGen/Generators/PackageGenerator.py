# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

from ctypes import c_ulonglong 
from ctypes import c_ubyte 

def CreateItemHeader(item):
	packageName = item.Package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#pragma once')
	lines.append('')
		
	# list dependences that needs to be included in the header
	for dep in item.Dependencies:
		if dep.IncludeInHeader:
			lines.append(f'#include <{dep.PackageName}/{dep.Name}.h>')

	lines.append('')
	lines.append(f'#include "{packageName}.h"')

	lines.append('')
	lines.append(f'namespace {packageName}')
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
	lines.append(f'            static constexpr char *ItemTypeString = "{packageName}.{item.Name}";')
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

	hlp.write_lines_to_file(f"{item.Package.Path}/{item.Name}.h",lines)

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


def CreateItemSource(item):
	packageName = item.Package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <glm/glm.hpp>')
	lines.append('')	
	lines.append(f'#include <pds/EntityWriter.h>')
	lines.append(f'#include <pds/EntityReader.h>')
	lines.append(f'#include <pds/EntityValidator.h>')
	lines.append('')
	lines.append(f'#include "{item.Name}.h"')
		
	# include dependences that were forward referenced in the header
	for dep in item.Dependencies:
		if not dep.IncludeInHeader:
			lines.append(f'#include <{dep.PackageName}/{dep.Name}.h>')
		
	lines.append('')
	lines.append(f'namespace {packageName}')
	lines.append('    {')

	lines.append('')
	if item.IsEntity:
		lines.append(f'    const char *{item.Name}::EntityTypeString() const {{ return {item.Name}::ItemTypeString; }}')
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
	lines.append(f'    bool {item.Name}::MF::Write( const {item.Name} &obj, pds::EntityWriter &writer )')
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
	lines.append(f'    bool {item.Name}::MF::Read( {item.Name} &obj, pds::EntityReader &reader )')
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

	lines.append('    };')
	hlp.write_lines_to_file(f"{item.Package.Path}/{item.Name}.inl",lines)


# static and constant hash table for entity lookup, (must be larger than the number of entities to add)
# Fowler–Noll–Vo FNV-1a hash function  https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
class EntityHashTable:
	def hash_function( self , entity_name ):
		hash = c_ulonglong(0xcbf29ce484222325)
		for ch in entity_name:
			hash = c_ulonglong(hash.value ^ c_ubyte(ord(ch)).value)
			hash = c_ulonglong(hash.value * c_ulonglong(0x00000100000001B3).value)
		return hash.value

	def insert_into_table( self , package_name , entity_name ):
		# use hash function to generate a good starting point
		hash_pos = self.hash_function( package_name + "." + entity_name ) % self.hash_table_size
		# find first empty slot
		while( self.hash_table[hash_pos] != None ):
			hash_pos = hash_pos+1
			if( hash_pos >= self.hash_table_size ):
				hash_pos = 0
		# fill it
		self.hash_table[hash_pos] = entity_name

	def __init__(self, items):
		self.hash_table_size = 37 
		self.hash_data_type_mult = 109
		self.hash_container_type_mult = 991
		self.hash_table = [None] * self.hash_table_size

		# fill up hash table, only fill with entities
		for item in items:
			if item.IsEntity:
				self.insert_into_table( item.Package.Name , item.Name );

def CreatePackageHandler_inl( package ):
	packageName = package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('#include <pds/pds.h>')
	lines.append('#include <pds/Varying.h>')
	lines.append('')

	for item in package.Items:
		if item.IsEntity:
			lines.append(f'#include "{item.Name}.h"')

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

	for item in package.Items:
		if item.IsEntity:
			lines.append(f'    // {item.Name}' )
			lines.append(f'    static const class _et_{item.Name}_EntityType : public _entityTypeClass' )
			lines.append(f'        {{' )
			lines.append(f'        public:' )
			lines.append(f'            virtual const char *EntityTypeString() const {{ return {item.Name}::ItemTypeString; }}' )
			lines.append(f'            virtual std::shared_ptr<pds::Entity> New() const {{ return std::make_shared<{item.Name}>(); }}')
			lines.append(f'            virtual void Clear( pds::Entity *obj ) const {{ {item.Name}::MF::Clear( *(({item.Name}*)obj) ); }}')
			lines.append(f'            virtual bool Equals( const pds::Entity *lval , const pds::Entity *rval ) const {{ return {item.Name}::MF::Equals( (({item.Name}*)lval) , (({item.Name}*)rval) ); }}')
			lines.append(f'            virtual bool Write( const pds::Entity *obj, pds::EntityWriter &writer ) const {{ return {item.Name}::MF::Write( *(({item.Name}*)obj) , writer ); }}')
			lines.append(f'            virtual bool Read( pds::Entity *obj, pds::EntityReader &reader ) const {{ return {item.Name}::MF::Read( *(({item.Name}*)obj) , reader ); }}')
			lines.append(f'            virtual bool Validate( const pds::Entity *obj, pds::EntityValidator &validator ) const {{ return {item.Name}::MF::Validate( *(({item.Name}*)obj) , validator ); }}')
			lines.append(f'        }} _et_{item.Name}_EntityTypeObject;' )
			lines.append('')

	# allocate and print hash table
	hash_table = EntityHashTable( package.Items )

	# print it 
	lines.append('    // Hash table with the type entity handler objects')
	lines.append(f'    static const _entityTypeClass *_entityTypeClassHashTable[{hash_table.hash_table_size}] = ')
	lines.append('        {')
	for idx in range(hash_table.hash_table_size):
		if hash_table.hash_table[idx] == None:
			lines.append('        nullptr,')
		else:
			lines.append(f'        &_et_{hash_table.hash_table[idx]}_EntityTypeObject,')
	lines.append('        };')
	lines.append('')
	lines.append('    // hash table lookup of entityType')
	lines.append('    static const _entityTypeClass *_findEntityTypeClass( const char *typeNameString )')
	lines.append('        {')
	lines.append('        // calculate hash value using Fowler–Noll–Vo FNV-1a hash function')
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
	#lines.append('        // entity was not found (this should never happen unless testing)')
	#lines.append('        pdsErrorLog << "_findEntityTypeClass: Invalid entity parameter { " << typeNameString << " } " << pdsErrorLogEnd;')
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

	hlp.write_lines_to_file(f"{package.Path}/{packageName}.h",lines)

def CreatePackageSourceFile( package ):
	packageName = package.Name

	lines = []
	lines.extend( hlp.generate_header() )
	lines.append('')
	lines.append('// All pds imports and typedefs')
	lines.append(f'#include "{packageName}.h"')
	lines.append('')

	lines.append('// Needed inline code from pds')
	lines.append('#include <pds/EntityReader.inl>')
	lines.append('#include <pds/EntityWriter.inl>')
	lines.append('')
	
	lines.append('// All headers and code for this package')
	for item in package.Items:
		lines.append(f'#include "{item.Name}.h"')

	for item in package.Items:
		lines.append(f'#include "{item.Name}.inl"')

	lines.append(f'#include "{packageName}PackageHandler.inl"')

	hlp.write_lines_to_file(f"{package.Path}/{packageName}.cpp",lines)

def run( package ):
	CreatePackageHeader( package )
	CreatePackageSourceFile( package )
	CreatePackageHandler_inl( package )
	for item in package.Items:
		CreateItemHeader( item )
		CreateItemSource( item )

