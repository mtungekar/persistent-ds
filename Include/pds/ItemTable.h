// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"
#include "EntityWriter.h"
#include "EntityReader.h"
#include "EntityValidator.h"

namespace pds
	{
	enum ItemTableFlags : uint
		{
		ZeroKeys = 0x1, // if set, validation will allow zero value keys (0 for ints, all 0 in a uuids, empty strings) 
		NullEntities = 0x2, // if set, validation will allow that null entities exist in the registry
		};

	// ItemTable holds a map of key values to unique memory mapped objects. This is the main holder of most objects in ISD.
	template<class _Kty, class _Ty, uint _Flags = 0, class _MapTy = std::map<_Kty, std::unique_ptr<_Ty>>>
	class ItemTable
		{
		public:
			using key_type = _Kty;
			using mapped_type = _Ty;
			using map_type = _MapTy;

			using value_type = typename map_type::value_type;
			using iterator = typename map_type::iterator;
			using const_iterator = typename map_type::const_iterator;

			static const bool type_no_zero_keys = (_Flags & ItemTableFlags::ZeroKeys) == 0;
			static const bool type_no_null_entities = (_Flags & ItemTableFlags::NullEntities) == 0;

			class MF;
			friend MF;

			// ctors/dtor and copy/move operators
			ItemTable() = default;
			ItemTable( const ItemTable &rval ) { MF::DeepCopy( *this , &rval ); }
			ItemTable &operator=( const ItemTable &rval ) { MF::DeepCopy( *this , &rval ); return *this; }
			ItemTable( ItemTable &&rval ) = default;
			ItemTable &operator=( ItemTable &&rval ) = default;
			~ItemTable() = default;

			// value compare operators
			bool operator==( const ItemTable &rval ) const { return MF::Equals( this, &rval ); }
			bool operator!=( const ItemTable &rval ) const { return !(MF::Equals( this, &rval )); }

		private:
			map_type v_Entries;

		public:
			// returns the number of entries in the ItemTable
			size_t Size() const noexcept { return this->v_Entries.size(); }

			// direct access to the Entries map
			map_type &Entries() noexcept { return this->v_Entries; }
			const map_type &Entries() const noexcept { return this->v_Entries; }

			// index access operator. node, dereferences the mapped value, so will throw if the value does not exist.
			mapped_type &operator[]( const key_type &key ) { return *(this->v_Entries[key].get()); }
			const mapped_type &operator[]( const key_type &key ) const { return *(this->v_Entries[key].get()); }

			// insert a key and new empty value, returns reference to value
			mapped_type &Insert( const key_type &key ) { this->v_Entries.emplace( key, std::make_unique<mapped_type>() ); return *(this->v_Entries[key].get()); }
		};

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	class ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF
		{
		using _MgmCl = ItemTable<_Kty,_Ty,_Flags,_MapTy>;

		public:
			static void Clear( _MgmCl &obj );
			static void DeepCopy( _MgmCl &dest, const _MgmCl *source );
			static bool Equals( const _MgmCl *lval, const _MgmCl *rval );

			static bool Write( const _MgmCl &obj, EntityWriter &writer );
			static bool Read( _MgmCl &obj, EntityReader &reader );

			static bool Validate( const _MgmCl &obj, EntityValidator &validator );

			// additional validation with external data
			template<class _Table> static bool ValidateAllKeysAreContainedInTable( const _MgmCl &obj, EntityValidator &validator, const _Table &otherTable, const char *otherTableName );

			// support methods for validation
			static bool ContainsKey( const _MgmCl &obj, const _Kty &key );
		};

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	void ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF::Clear( _MgmCl &obj )
		{
		obj.v_Entries.clear();
		}

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	void ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF::DeepCopy( _MgmCl &dest, const _MgmCl *source )
		{
		MF::Clear( dest );
		if( !source )
			return;

		for( const auto & ent : source->v_Entries )
			{
			// make a new copy of the value, if original is not nullptr
			dest.v_Entries.emplace( ent.first , std::unique_ptr<_Ty>( (ent.second) ? new _Ty(*ent.second) : nullptr ) );
			}
		}

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	bool ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF::Equals( const _MgmCl *lval, const _MgmCl *rval )
		{
		// early out if the pointers are equal (includes nullptr)
		if( lval == rval )
			return true;

		// early out if one of the pointers is nullptr (both can't be null because of above test)
		if( !lval || !rval )
			return false;

		// early out if the sizes are not the same 
		if( lval->Size() != rval->Size() )
			return false;

		// compare all the entries
		auto lval_it = lval->v_Entries.begin();
		while( lval_it != lval->v_Entries.end() )
			{
			// find the key in the right object, should always find
			auto rval_it = rval->v_Entries.find( lval_it->first );
			if( rval_it == rval->v_Entries.end() )
				return false;

			// compare values ptrs 
			// if pointers are not equal, check contents
			auto lval_value_ptr = lval_it->second.get();
			auto rval_value_ptr = rval_it->second.get();
			if( lval_value_ptr != rval_value_ptr )
				{
				// first, make sure both pointers are valid (since they are not both nullptr)
				if( !lval_value_ptr || !rval_value_ptr )
					return false;

				// dereference and compare contents
				if( !((*lval_value_ptr) == (*rval_value_ptr)) )
					return false;
				}

			// step
			++lval_it;
			}

		return true;
		}


	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	bool ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF::Write( const _MgmCl &obj, EntityWriter &writer )
		{
		// collect the keys into a vector, and store in stream as an array
		std::vector<_Kty> keys(obj.v_Entries.size());
		size_t index = 0;
		for( auto it = obj.v_Entries.begin(); it != obj.v_Entries.end(); ++it, ++index )
			{
			keys[index] = it->first;
			}
		if( !writer.Write( pdsKeyMacro("IDs"), keys ) )
			return false;
		keys.clear();

		// create a sections array for the entities
		EntityWriter *section_writer = writer.BeginWriteSectionsArray( pdsKeyMacro("Entities"), obj.v_Entries.size() );
		if( !section_writer )
			return false;

		// write out all the entities as an array
		// for each non-empty entity, call the write method of the entity
		index = 0;
		for( auto it = obj.v_Entries.begin(); it != obj.v_Entries.end(); ++it, ++index )
			{
			if( !writer.BeginWriteSectionInArray( section_writer, index ) )
				return false;
			if( it->second )
				{
				if( !_Ty::MF::Write( *(it->second), *(section_writer) ) )
					return false;
				}
			if( !writer.EndWriteSectionInArray( section_writer, index ) )
				return false;
			}

		// sanity check, make sure all sections were written
		pdsSanityCheckDebugMacro( index == obj.v_Entries.size() );

		// end the Entries sections array
		if( !writer.EndWriteSectionsArray( section_writer ) )
			return false;

		return true;
		}

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	bool ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF::Read( _MgmCl &obj , EntityReader &reader )
		{
		EntityReader *section_reader = {};
		size_t map_size = {};
		bool success = {};
		typename _MgmCl::iterator it = {};

		// read in the keys as a vector
		std::vector<_Kty> keys;
		if( !reader.Read( pdsKeyMacro("IDs"), keys ) )
			return false;

		// begin the named sections array
		std::tie( section_reader, map_size, success ) = reader.BeginReadSectionsArray( pdsKeyMacro("Entities"), false );
		if( !success )
			return false;
		pdsSanityCheckDebugMacro( section_reader );
		if( map_size != keys.size() )
			{
			pdsErrorLog << "Invalid size in ItemTable, the Keys and Entities arrays do not match in size." << pdsErrorLogEnd;
			return false;
			}

		// read in all the entities, push into map as key-value pairs
		obj.v_Entries.clear();
		for( size_t index = 0; index < map_size ; ++index )
			{
			bool has_data = false;
			if( !reader.BeginReadSectionInArray( section_reader, index, &has_data ) )
				return false;

			if( has_data )
				std::tie(it,success) = obj.v_Entries.emplace( keys[index], std::make_unique<_Ty>() );
			else 
				std::tie(it,success) = obj.v_Entries.emplace( keys[index], nullptr );

			if( !success )
				{
				pdsErrorLog << "Failed inserting key-value pair in ItemTable" << pdsErrorLogEnd;
				return false;
				}

			if( it->second )
				{
				if( !_Ty::MF::Read( *(it->second), *(section_reader) ) )
					return false;
				}

			if( !reader.EndReadSectionInArray( section_reader, index ) )
				return false;
			}

		// end the sections array
		if( !reader.EndReadSectionsArray( section_reader ) )
			return false;

		return true;
		}

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	bool ItemTable<_Kty,_Ty,_Flags,_MapTy>::MF::Validate( const _MgmCl &obj , EntityValidator &validator )
		{
		// check if a zero key exists in the dictionary
		if( _MgmCl::type_no_zero_keys )
			{
			if( obj.v_Entries.find( data_type_information<_Kty>::zero ) != obj.v_Entries.end() )
				{
				pdsValidationError( ValidationError::NullNotAllowed ) << "This Directory has a zero-value key, which is not allowed. (DictionaryFlags::NoZeroKeys)" << pdsErrorLogEnd;
				}
			}

		for( auto it = obj.v_Entries.begin(); it != obj.v_Entries.end(); ++it )
			{
			// check value
			if( it->second )
				{
				if( !_Ty::MF::Validate( *(it->second) , validator ) )
					return false;
				}
			else if( _MgmCl::type_no_null_entities )
				{
				// value is empty, and this is not allowed in this dictionary
				pdsValidationError( ValidationError::NullNotAllowed ) << "Non allocated entities (values) are not allowed in this Directory. (DictionaryFlags::NoNullEntities)" << pdsErrorLogEnd;
				}
			}

		return true;
		}

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	template<class _Table>
	bool ItemTable<_Kty, _Ty, _Flags, _MapTy>::MF::ValidateAllKeysAreContainedInTable( const _MgmCl &obj, EntityValidator &validator, const _Table &otherTable, const char *otherTableName )
		{
		for( auto it = obj.v_Entries.begin(); it != obj.v_Entries.end(); ++it )
			{
			if( !_Table::MF::ContainsKey( otherTable, it->first ) )
				{
				pdsValidationError( ValidationError::MissingObject ) << "The key " << it->first << " is missing in " << otherTableName << pdsErrorLogEnd;
				}
			}
		return true;
		}

	template<class _Kty, class _Ty, uint _Flags, class _MapTy>
	bool ItemTable<_Kty, _Ty, _Flags, _MapTy>::MF::ContainsKey( const _MgmCl &obj, const _Kty &key )
		{
		return obj.v_Entries.find( key ) != obj.v_Entries.end();
		}


	};