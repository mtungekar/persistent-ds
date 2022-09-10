// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "ValueTypes.h"

namespace pds
	{
	template <class _Ty, class _Base = idx_vector<_Ty> >
	class IndexedVector : public _Base
		{
		public:
			using base_type = _Base;

			class MF;
			friend MF;

			// ctors/dtor and copy/move operators
			IndexedVector() = default;
			IndexedVector( const IndexedVector &rval ) = default;
			IndexedVector &operator=( const IndexedVector &rval ) = default;
			IndexedVector( IndexedVector &&rval ) = default;
			IndexedVector &operator=( IndexedVector &&rval ) = default;
			~IndexedVector() = default;

			// value compare operators
			bool operator==( const IndexedVector &rval ) const { return MF::Equals( this, &rval ); }
			bool operator!=( const IndexedVector &rval ) const { return !(MF::Equals( this, &rval )); }
		};

	class EntityWriter;
	class EntityReader;
	class EntityValidator;

	template<class _Ty, class _Base>
	class IndexedVector<_Ty,_Base>::MF
		{
		using _MgmCl = IndexedVector<_Ty,_Base>;

		public:
			static void Clear( _MgmCl &obj );
			static void DeepCopy( _MgmCl &dest, const _MgmCl *source );
			static bool Equals( const _MgmCl *lval, const _MgmCl *rval );

			static bool Write( const _MgmCl &obj, EntityWriter &writer );
			static bool Read( _MgmCl &obj, EntityReader &reader );

			static bool Validate( const _MgmCl &obj, EntityValidator &validator );
		};

	template<class _Ty, class _Base>
	void IndexedVector<_Ty,_Base>::MF::Clear( _MgmCl &obj )
		{
		obj.clear();
		}

	template<class _Ty, class _Base>
	void IndexedVector<_Ty,_Base>::MF::DeepCopy( _MgmCl &dest, const _MgmCl *source )
		{
		MF::Clear( dest );
		if( !source )
			return;
		dest = *source;
		}

	template<class _Ty, class _Base>
	bool IndexedVector<_Ty,_Base>::MF::Equals( const _MgmCl *lval, const _MgmCl *rval )
		{
		// early out if the pointers are equal (includes nullptr)
		if( lval == rval )
			return true;

		// early out if one of the pointers is nullptr (both can't be null because of above test)
		if( !lval || !rval )
			return false;

		// deep compare the values
		const IndexedVector<_Ty,_Base>::base_type &_lval = *lval;
		const IndexedVector<_Ty,_Base>::base_type &_rval = *rval;
		return (_lval == _rval);
		}

	template<class _Ty, class _Base>
	bool IndexedVector<_Ty,_Base>::MF::Write( const _MgmCl &obj, EntityWriter &writer )
		{
		const IndexedVector<_Ty,_Base>::base_type &_obj = obj;
		if( !writer.Write( pdsKeyMacro("Values"), _obj ) )
			return false;
		return true;
		}

	template<class _Ty, class _Base>
	bool IndexedVector<_Ty,_Base>::MF::Read( _MgmCl &obj , EntityReader &reader )
		{
		IndexedVector<_Ty,_Base>::base_type &_obj = obj;
		if( !reader.Read( pdsKeyMacro("Values"), _obj ) )
			return false;
		return true;
		}

	template<class _Ty, class _Base>
	bool IndexedVector<_Ty,_Base>::MF::Validate( const _MgmCl &obj , EntityValidator &validator )
		{
		if( obj.values().size() > (size_t)i32_sup )
			{
			pdsValidationError( ValidationError::InvalidCount ) << "This IndexedVector has too many values in the values vector. The limit is 2^31 values, which can be indexed by a 32-bit int." << pdsErrorLogEnd;
			}

		// cap the count to 32 bit int
		const u32 values_count = (u32)std::min( obj.values().size() , (size_t)i32_sup );
		for( size_t i=0; i<obj.index().size(); ++i )
			{
			if( (u32)obj.index()[i] >= values_count )
				{
				pdsValidationError( ValidationError::InvalidValue ) << "The value " << obj.index()[i] << " at position " << i << " of the index vector is out of bounds." << pdsErrorLogEnd;
				}
			}

		return true;
		}


	};