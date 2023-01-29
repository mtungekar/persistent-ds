// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"

//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>

namespace pds
	{
	// Memory write stream is a write-only memory area.
	// The stream is used to write structured values. 
	// It can write out more complex types than just plain old data (POD) 
	// types, and also supports std::string, UUIDs and std::vectors of 
	// the above types.
	// Caveat: The stream is NOT thread safe, and should be accessed by 
	// only one thread at a time.
	class MemoryWriteStream
		{
		private:
			static const u64 InitialAllocationSize = 1024*1024*64; // 64MB initial size

			u8 *Data = nullptr; // the allocated data
			u64 DataSize = 0; // the size of the memory stream (not the reserved allocation)
			u64 Position = 0; // the write position in the memory stream
			
			u64 DataReservedSize = 0; // the reserved size of the allocation
			u32 PageSize = 0; // size of each page of allocation
			
			bool FlipByteOrder = false; // true if we should flip BE to LE or LE to BE

			// reserve data for at least reserveSize.
			void ReserveForSize( u64 reserveSize );
			void FreeAllocation();

			// resize (grow) the data stream. if the new size is larger than the reserved size, the allocation will be resized to fit the new size
			void Resize( u64 newSize );

			// write raw bytes to the memory stream. this will increase reserved allocation of data as needed
			void WriteRawData( const void *src, u64 count );

			// write 1,2,4 or 8 byte values and make sure they are in the correct byte order
			template <class T> void WriteValues( const T *src, u64 count );
			
		public:
			MemoryWriteStream( u64 _InitialAllocationSize = InitialAllocationSize ) { this->ReserveForSize( _InitialAllocationSize ); };
			~MemoryWriteStream() { this->FreeAllocation(); };

			// get a read-only pointer to the data
			const void *GetData() const { return this->Data; }

			// get the Size of the stream in bytes
			u64 GetSize() const;

			// Position is the current data position. the beginning of the stream is position 0. the stream grows whenever the position moves past the current end of the stream.
			u64 GetPosition() const;
			void SetPosition( u64 new_pos );

			// FlipByteOrder is set if the stream flips byte order of multibyte values 
			bool GetFlipByteOrder() const;
			void SetFlipByteOrder( bool value );

			// write one item to the memory stream. makes sure to convert endianness
			void Write( const i8 &src );
			void Write( const i16 &src );
			void Write( const i32 &src );
			void Write( const i64 &src );
			void Write( const u8 &src );
			void Write( const u16 &src );
			void Write( const u32 &src );
			void Write( const u64 &src );
			void Write( const float &src );
			void Write( const double &src );
			void Write( const uuid &src );
			void Write( const hash &src );

			// write an array of items to the memory stream. makes sure to convert endianness
			void Write( const i8 *src , u64 count );
			void Write( const i16 *src , u64 count );
			void Write( const i32 *src , u64 count );
			void Write( const i64 *src , u64 count );
			void Write( const u8 *src , u64 count );
			void Write( const u16 *src , u64 count );
			void Write( const u32 *src , u64 count );
			void Write( const u64 *src , u64 count );
			void Write( const float *src , u64 count );
			void Write( const double *src , u64 count );
			void Write( const uuid *src , u64 count );
			void Write( const hash *src , u64 count );

		};

	inline void MemoryWriteStream::ReserveForSize( u64 reserveSize )
		{
		// we need to resize the reserved data area, try doubling size
		// and if that is not enough, set to the reserveSize
		this->DataReservedSize *= 2;
		if( this->DataReservedSize < reserveSize )
			{
			this->DataReservedSize = reserveSize;
			}

		// allocate a new area
#ifdef _MSC_VER
		u8 *pNewData = (u8*)::VirtualAlloc( nullptr, this->DataReservedSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
#elif defined(__GNUC__)
		u8 *pNewData = (u8*)malloc(this->DataReservedSize);
#endif
		if( pNewData == nullptr )
			{
			throw std::bad_alloc();
			}

		// if there is an old area, copy the data and free the old area
		// the area can never shrink, so we don't need to worry about capping
		if( this->Data )
			{
			memcpy( pNewData, this->Data, this->DataSize );
			this->FreeAllocation();
			}

		// set the new pointer
		this->Data = pNewData;
		}

	inline void MemoryWriteStream::FreeAllocation()
		{
		if( this->Data ) 
			{ 
#ifdef _MSC_VER			
			::VirtualFree( this->Data, 0, MEM_RELEASE );
#elif defined(__GNUC__)
			free( this->Data );
#endif							
			this->Data = nullptr;
			}
		}


	inline void MemoryWriteStream::Resize( u64 newSize )
		{
		if( newSize > this->DataReservedSize )
			{
			this->ReserveForSize( newSize );
			}

		this->DataSize = newSize;
		}

	inline void MemoryWriteStream::WriteRawData( const void *src, u64 count )
		{
		// cap the end position
		u64 end_pos = this->Position + count;
		if( end_pos > this->DataSize )
			{
			this->Resize( end_pos );
			}

		// copy the data and move the position
		memcpy( &this->Data[this->Position] , src , count );
		this->Position = end_pos;
		}

	template <class T> inline void MemoryWriteStream::WriteValues( const T *src, u64 count )
		{
		if( this->FlipByteOrder )
			{
			// flip the byte order of the words in the dest 
			u64 pos = this->Position;
			this->WriteRawData( src, count * sizeof(T) );
			swap_byte_order<T>( (T*)(&this->Data[pos]), count );
			}
		else
			{
			// no flipping, just write as is
			this->WriteRawData( src, count * sizeof(T) );
			}
		}

	template <> inline void MemoryWriteStream::WriteValues<u8>( const u8 *src, u64 count ) 
		{ 
		this->WriteRawData( src, count ); 
		}

	inline u64 MemoryWriteStream::GetSize() const
		{
		return this->DataSize;
		}

	inline u64 MemoryWriteStream::GetPosition() const 
		{ 
		return this->Position; 
		}

	inline void MemoryWriteStream::SetPosition( u64 new_pos ) 
		{ 
		if( new_pos > DataSize )
			{
			this->Resize( new_pos );
			}
		this->Position = new_pos; 
		}

	inline bool MemoryWriteStream::GetFlipByteOrder() const 
		{ 
		return this->FlipByteOrder; 
		}

	inline void MemoryWriteStream::SetFlipByteOrder( bool value )
		{
		this->FlipByteOrder = value;
		}

	//// write one item of data, (but using the multi-values method)
	inline void MemoryWriteStream::Write( const i8 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const i16 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const i32 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const i64 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const u8 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const u16 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const u32 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const u64 &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const float &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const double &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const uuid &src ) { this->Write( &src, 1 ); }
	inline void MemoryWriteStream::Write( const hash &src ) { this->Write( &src, 1 ); }

	// 8 bit data
	inline void MemoryWriteStream::Write( const i8 *src, u64 count ) { return this->WriteValues<u8>( (const u8*)src, count ); }
	inline void MemoryWriteStream::Write( const u8 *src, u64 count ) { return this->WriteValues<u8>( src, count ); }
								   
	// 16 bit data				   
	inline void MemoryWriteStream::Write( const i16 *src, u64 count ) { return this->WriteValues<u16>( (const u16*)src, count ); }
	inline void MemoryWriteStream::Write( const u16 *src, u64 count ) { return this->WriteValues<u16>( src, count ); }
								   
	// 32 bit data				   
	inline void MemoryWriteStream::Write( const i32 *src, u64 count ) { return this->WriteValues<u32>( (const u32*)src, count ); }
	inline void MemoryWriteStream::Write( const u32 *src, u64 count ) { return this->WriteValues<u32>( src, count ); }
	inline void MemoryWriteStream::Write( const float *src, u64 count ) { return this->WriteValues<u32>( (const u32*)src, count ); }
								   
	// 64 bit data				   
	inline void MemoryWriteStream::Write( const i64 *src, u64 count ) { return this->WriteValues<u64>( (const u64*)src, count ); }
	inline void MemoryWriteStream::Write( const u64 *src, u64 count ) { return this->WriteValues<u64>( src, count ); }
	inline void MemoryWriteStream::Write( const double *src, u64 count ) { return this->WriteValues<u64>( (const u64*)src, count ); }

	// uuids
	inline void MemoryWriteStream::Write( const uuid *src, u64 count ) 
		{ 
		static_assert(sizeof(uuid)==16, "Invalid size of uuid struct, needs to be exactly 16 bytes.");

		// Write raw bytes, assumes the values are contiguous 
		// No need for byte-swapping, the uuids are always stored as raw bytes, and ordered 
		// big-endian (the order which the hex values are printed when printing a guid)
		this->WriteValues<u8>( (const u8*)src, sizeof(uuid)*count );
		}

	// hashes
	inline void MemoryWriteStream::Write( const hash *src, u64 count ) 
		{ 
		static_assert(sizeof(hash)==32, "Invalid size of hash struct, needs to be exactly 32 bytes.");

		// Write raw bytes, assumes the values are contiguous 
		// No need for byte-swapping, the hashes are always stored as raw bytes, and ordered 
		// big-endian (the order which the hex values are printed when printing a hash)
		this->WriteValues<u8>( (const u8*)src, sizeof(hash)*count );
		}
	
	};