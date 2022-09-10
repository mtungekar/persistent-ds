// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"
#include <ctle/bimap.h>

namespace pds
	{
	template <class _Kty, class _Vty, class _Base = ctle::bimap<_Kty,_Vty> >
	class BidirectionalMap : public _Base
		{
		public:
			using base_type = _Base;

			class MF;
			friend MF;

			// ctors/dtor and copy/move operators
			BidirectionalMap() = default;
			BidirectionalMap( const BidirectionalMap &rval ) = default;
			BidirectionalMap &operator=( const BidirectionalMap &rval ) = default;
			BidirectionalMap( BidirectionalMap &&rval ) = default;
			BidirectionalMap &operator=( BidirectionalMap &&rval ) = default;
			~BidirectionalMap() = default;

			// value compare operators
			bool operator==( const BidirectionalMap &rval ) const { return MF::Equals( this, &rval ); }
			bool operator!=( const BidirectionalMap &rval ) const { return !(MF::Equals( this, &rval )); }
		};

	class EntityWriter;
	class EntityReader;
	class EntityValidator;

	template<class _Kty, class _Vty, class _Base>
	class BidirectionalMap<_Kty,_Vty,_Base>::MF
		{
		using _MgmCl = BidirectionalMap<_Kty,_Vty,_Base>;

		public:
			static void Clear( _MgmCl &obj );
			static void DeepCopy( _MgmCl &dest, const _MgmCl *source );
			static bool Equals( const _MgmCl *lval, const _MgmCl *rval );

			static bool Write( const _MgmCl &obj, EntityWriter &writer );
			static bool Read( _MgmCl &obj, EntityReader &reader );

			static bool Validate( const _MgmCl &obj, EntityValidator &validator );

			// support methods for validation
			static bool ContainsKey( const _MgmCl &obj, const _Kty &key );
		};

	template<class _Kty, class _Vty, class _Base>
	void BidirectionalMap<_Kty,_Vty,_Base>::MF::Clear( _MgmCl &obj )
		{
		obj.clear();
		}

	template<class _Kty, class _Vty, class _Base>
	void BidirectionalMap<_Kty,_Vty,_Base>::MF::DeepCopy( _MgmCl &dest, const _MgmCl *source )
		{
		MF::Clear( dest );
		if( !source )
			return;
		dest = *source;
		}

	template<class _Kty, class _Vty, class _Base>
	bool BidirectionalMap<_Kty,_Vty,_Base>::MF::Equals( const _MgmCl *lval, const _MgmCl *rval )
		{
		// early out if the pointers are equal (includes nullptr)
		if( lval == rval )
			return true;

		// early out if one of the pointers is nullptr (both can't be null because of above test)
		if( !lval || !rval )
			return false;

		// deep compare the values
		const BidirectionalMap<_Kty,_Vty,_Base>::base_type &_lval = *lval;
		const BidirectionalMap<_Kty,_Vty,_Base>::base_type &_rval = *rval;
		return (_lval == _rval);
		}

	template<class _Kty, class _Vty, class _Base>
	bool BidirectionalMap<_Kty,_Vty,_Base>::MF::Write( const _MgmCl &obj, EntityWriter &writer )
		{
		// enumerate all keys and values to two separate vectors
		std::vector<_Kty> keys(obj.size());
		std::vector<_Vty> values(obj.size());
		size_t index = 0;
		for( auto it = obj.begin(); it != obj.end(); ++it, ++index )
			{
			keys[index] = it->first;
			values[index] = it->second;
			}

		// write vectors 
		if( !writer.Write( pdsKeyMacro("Keys"), keys ) )
			return false;
		if( !writer.Write( pdsKeyMacro("Values"), values ) )
			return false;

		return true;
		}

	template<class _Kty, class _Vty, class _Base>
	bool BidirectionalMap<_Kty,_Vty,_Base>::MF::Read( _MgmCl &obj , EntityReader &reader )
		{
		obj.clear();

		std::vector<_Kty> keys;
		std::vector<_Vty> values;

		// read in vectors with keys and values
		if( !reader.Read( pdsKeyMacro("Keys"), keys ) )
			return false;
		if( !reader.Read( pdsKeyMacro("Values"), values ) )
			return false;

		// insert into map
		for( size_t index = 0; index < keys.size(); ++index )
			{
			obj.insert( keys[index], values[index] );
			}

		return true;
		}

	template<class _Kty, class _Vty, class _Base>
	bool BidirectionalMap<_Kty,_Vty,_Base>::MF::Validate( const _MgmCl &obj , EntityValidator &validator )
		{
		return true;
		}

	template<class _Kty, class _Vty, class _Base>
	bool BidirectionalMap<_Kty, _Vty, _Base>::MF::ContainsKey( const _MgmCl &obj, const _Kty &key )
		{
		return obj.contains_key( key );
		}

	};