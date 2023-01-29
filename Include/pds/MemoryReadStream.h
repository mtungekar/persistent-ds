// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"

#include <vector>

namespace pds
	{
	// Memory read stream is a wrapper around a read-only memory area.
	// The stream is used to read structured values, and automatically 
	// does byte order swapping. It can read in more complex types than
	// just plain old data (POD) types, such as std::string, UUIDs and
	// std::vectors of the above types.
	// Caveat: The stream is NOT thread safe, and should be accessed by 
	// only one thread at a time.
	class MemoryReadStream
		{
		private:
			const u8 *Data = nullptr;
			u64 DataSize = 0;
			u64 DataPosition = 0;
			bool FlipByteOrder = false; // true if we should flip BE to LE or LE to BE

			// read raw bytes from the memory stream
			u64 ReadRawData( void *dest, u64 count );

			// read 1,2,4 or 8 byte values and make sure they are in the correct byte order
			template <class T> u64 ReadValues( T *dest, u64 count );
			template <> u64 ReadValues<u8>( u8 *dest, u64 count );

		public:
			MemoryReadStream( const void *_Data, u64 _DataSize, bool _FlipByteOrder = false ) : Data( (u8*)_Data ), DataSize( _DataSize ), FlipByteOrder(_FlipByteOrder) {};

			// get the Size of the stream in bytes
			u64 GetSize() const;

			// Position is the current data position. the beginning of the stream is position 0. the position will not move past the end of the stream.
			u64 GetPosition() const;
			bool SetPosition( u64 new_pos );

			// if at EOF return true, which means that Position >= Size
			bool IsEOF() const;

			// FlipByteOrder is set if the stream flips byte order of multibyte values 
			bool GetFlipByteOrder() const;
			void SetFlipByteOrder( bool value );

			// Peek at the next byte in the stream, without modifing the Position or any data. If the Position is beyond the end of the stream, the value will be 0
			u8 Peek() const;

			// read one item from the memory stream. makes sure to convert endianness
			template <class T> T Read();
			template <> i8 Read<i8>();
			template <> i16 Read<i16>();
			template <> i32 Read<i32>();
			template <> i64 Read<i64>();
			template <> u8 Read<u8>();
			template <> u16 Read<u16>();
			template <> u32 Read<u32>();
			template <> u64 Read<u64>();
			template <> float Read<float>();
			template <> double Read<double>();
			template <> uuid Read<uuid>();
			template <> hash Read<hash>();

			// read a number of items from the memory stream. makes sure to convert endianness
			u64 Read( i8 *dest , u64 count );
			u64 Read( i16 *dest , u64 count );
			u64 Read( i32 *dest , u64 count );
			u64 Read( i64 *dest , u64 count );
			u64 Read( u8 *dest , u64 count );
			u64 Read( u16 *dest , u64 count );
			u64 Read( u32 *dest , u64 count );
			u64 Read( u64 *dest , u64 count );
			u64 Read( float *dest , u64 count );
			u64 Read( double *dest , u64 count );
			u64 Read( uuid *dest , u64 count );
			u64 Read( hash *dest , u64 count );
		};

	inline u8 MemoryReadStream::Peek() const
		{
		if( this->DataPosition >= this->DataSize )
			return 0;
		else
			return this->Data[this->DataPosition];
		}

	inline u64 MemoryReadStream::ReadRawData( void *dest, u64 count )
		{
		// cap the end position
		u64 end_pos = this->DataPosition + count;
		if( end_pos > this->DataSize )
			{
			end_pos = this->DataSize;
			count = end_pos - this->DataPosition;
			}

		// copy the data and move the position
		memcpy( dest, &this->Data[this->DataPosition], count );
		this->DataPosition = end_pos;
		return count;
		}

	template <class T> inline u64 MemoryReadStream::ReadValues( T *dest, u64 count )
		{
		u64 readc = this->ReadRawData( dest, count * sizeof(T) ) / sizeof(T);
		if( !this->FlipByteOrder )
			return readc;

		// flip the byte order of the words in the dest
		swap_byte_order<T>( dest, count );
		return readc;
		}

	template <> inline u64 MemoryReadStream::ReadValues<u8>( u8 *dest, u64 count ) 
		{ 
		return ReadRawData( dest, count ); 
		}

	inline u64 MemoryReadStream::GetSize() const
		{
		return this->DataSize;
		}

	inline u64 MemoryReadStream::GetPosition() const 
		{ 
		return this->DataPosition; 
		}

	inline bool MemoryReadStream::SetPosition( u64 new_pos ) 
		{ 
		if( new_pos > DataSize )
			{
			return false;
			}
		this->DataPosition = new_pos; 
		return true;
		}

	inline bool MemoryReadStream::IsEOF() const 
		{ 
		return this->DataPosition >= this->DataSize; 
		}

	inline bool MemoryReadStream::GetFlipByteOrder() const 
		{ 
		return this->FlipByteOrder; 
		}

	inline void MemoryReadStream::SetFlipByteOrder( bool value )
		{
		this->FlipByteOrder = value;
		}

	// read one item of data
	template <> inline i8 MemoryReadStream::Read<i8>() { i8 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline i16 MemoryReadStream::Read<i16>() { i16 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline i32 MemoryReadStream::Read<i32>() { i32 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline i64 MemoryReadStream::Read<i64>() { i64 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline u8 MemoryReadStream::Read<u8>() { u8 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline u16 MemoryReadStream::Read<u16>() { u16 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline u32 MemoryReadStream::Read<u32>() { u32 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline u64 MemoryReadStream::Read<u64>() { u64 dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline float MemoryReadStream::Read<float>() { float dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline double MemoryReadStream::Read<double>() { double dest = 0; this->Read( &dest, 1 ); return dest; }
	template <> inline uuid MemoryReadStream::Read<uuid>() { uuid dest = {}; this->Read( &dest, 1 ); return dest; }
	template <> inline hash MemoryReadStream::Read<hash>() { hash dest = {}; this->Read( &dest, 1 ); return dest; }

	// 8 bit data
	inline u64 MemoryReadStream::Read( i8 *dest, u64 count ) { return this->ReadValues<u8>( (u8*)dest, count ); }
	inline u64 MemoryReadStream::Read( u8 *dest, u64 count ) { return this->ReadValues<u8>( dest, count ); }

	// 16 bit data
	inline u64 MemoryReadStream::Read( i16 *dest, u64 count ) { return this->ReadValues<u16>( (u16*)dest, count ); }
	inline u64 MemoryReadStream::Read( u16 *dest, u64 count ) { return this->ReadValues<u16>( dest, count ); }

	// 32 bit data
	inline u64 MemoryReadStream::Read( i32 *dest, u64 count ) { return this->ReadValues<u32>( (u32*)dest, count ); }
	inline u64 MemoryReadStream::Read( u32 *dest, u64 count ) { return this->ReadValues<u32>( dest, count ); }
	inline u64 MemoryReadStream::Read( float *dest, u64 count ) { return this->ReadValues<u32>( (u32*)dest, count ); }

	// 64 bit data
	inline u64 MemoryReadStream::Read( i64 *dest, u64 count ) { return this->ReadValues<u64>( (u64*)dest, count ); }
	inline u64 MemoryReadStream::Read( u64 *dest, u64 count ) { return this->ReadValues<u64>( dest, count ); }
	inline u64 MemoryReadStream::Read( double *dest, u64 count ) { return this->ReadValues<u64>( (u64*)dest, count ); }

	// uuids
	inline u64 MemoryReadStream::Read( uuid *dest, u64 count ) 
		{ 
		static_assert(sizeof(uuid)==16, "Invalid size of uuid struct, needs to be exactly 16 bytes.");

		// Read raw bytes, assumes the values are contiguous 
		// No need for byte-swapping, the uuids are always stored as raw bytes, and ordered 
		// big-endian (the order which the hex values are printed when printing a uuid)
		// return number of successfully read full items
		return this->ReadValues<u8>( (u8 *)dest, sizeof( uuid ) * count ) / sizeof(uuid); 
		}

	// hashes
	inline u64 MemoryReadStream::Read( hash *dest, u64 count ) 
		{ 
		static_assert(sizeof(hash)==32, "Invalid size of hash struct, needs to be exactly 32 bytes.");

		// Read raw bytes, assumes the values are contiguous 
		// No need for byte-swapping, the hashes are always stored as raw bytes, and ordered 
		// big-endian (the order which the hex values are printed when printing a hash)
		// return number of successfully read full items
		return this->ReadValues<u8>( (u8 *)dest, sizeof( hash ) * count ) / sizeof(hash); 
		}

	};