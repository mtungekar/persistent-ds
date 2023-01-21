// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE
#define PDS_MAIN_BUILD_FILE

#include <pds/pds.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Rpc.h>

#include "SHA256.h"
#include "DynamicTypes.h"
#include "ValueTypes.h"
#include "Varying.h"

#include "MemoryWriteStream.h"
#include "MemoryReadStream.h"

#include "EntityWriter.h"
#include "EntityReader.h"
#include "EntityValidator.h"

#include "BidirectionalMap.h"
#include "DirectedGraph.h"
#include "IndexedVector.h"
#include "ItemTable.h"

#include "DataValuePointers.h"

#include "SHA256.inl"

#include "DataTypes.inl"
#include "EntityWriter.inl"
#include "EntityReader.inl"
#include "DynamicTypes.inl"

using namespace pds;
using std::pair;
using std::make_pair;

constexpr size_t sha256_hash_size = 32;

std::wstring pds::widen( const std::string &str )
	{
	std::wstring ret;

	if( !str.empty() )
		{
		size_t wsize;
		mbstowcs_s( &wsize, nullptr, 0, str.c_str(), 0 );

		size_t alloc_wsize = wsize + 1;
		wchar_t *wstr = new wchar_t[alloc_wsize];
		size_t conv_count;
		mbstowcs_s( &conv_count, wstr, alloc_wsize, str.c_str(), wsize );

		ret = std::wstring( wstr );
		delete[] wstr;
		}

	return ret;
	}

// writes array of bytes to string of hex values. the hex values will be
// in the same order as the bytes, so if you need to convert a litte-endian
// word into hex, be sure to flip the byte order before.
std::string pds::bytes_to_hex_string( const void *bytes, size_t count )
	{
	static const char hexchars[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	std::string ret;
	const u8 *p = (const u8 *)bytes;
	for( size_t i = 0; i < count; ++i )
		{
		ret += hexchars[((*p) >> 4) & 0xf]; // high nibble
		ret += hexchars[(*p) & 0xf]; // low nibble
		++p;
		}
	return ret;
	}

template <> std::string pds::value_to_hex_string<u8>( u8 value )
	{
	return bytes_to_hex_string( &value, sizeof( value ) );
	}

template <> std::string pds::value_to_hex_string<u16>( u16 value )
	{
	bigendian_from_value( (u8 *)&value, value ); // in-place make sure big endian
	return bytes_to_hex_string( &value, sizeof( value ) );
	}

template <> std::string pds::value_to_hex_string<u32>( u32 value )
	{
	bigendian_from_value( (u8 *)&value, value ); // in-place make sure big endian
	return bytes_to_hex_string( &value, sizeof( value ) );
	}

template <> std::string pds::value_to_hex_string<u64>( u64 value )
	{
	bigendian_from_value( (u8 *)&value, value ); // in-place make sure big endian
	return bytes_to_hex_string( &value, sizeof( value ) );
	}

template <> std::string pds::value_to_hex_string<uuid>( uuid value )
	{
	std::string ret;

	ret += value_to_hex_string<u32>( value.Data1 );
	ret += "-";
	ret += value_to_hex_string<u16>( value.Data2 );
	ret += "-";
	ret += value_to_hex_string<u16>( value.Data3 );
	ret += "-";
	ret += bytes_to_hex_string( value.Data4, 2 );
	ret += "-";
	ret += bytes_to_hex_string( &value.Data4[2], 6 );

	return ret;
	}

template <> std::string pds::value_to_hex_string<hash>( hash value )
	{
	static_assert(sizeof( hash ) == 32, "Error: hash is assumed to be of size 32.");
	// note: no need to swap order of bytes. 
	// The hash is always ordered the same, regardless of the hardware (basically a big-endian 256 bit value)
	return bytes_to_hex_string( &value, 32 );
	}

static inline u8 decode_hex_char( char c )
	{
	if( c >= '0' && c <= '9' )
		return c - '0';
	else if( c >= 'a' && c <= 'f' )
		return (c - 'a') + 10;
	else if( c >= 'A' && c <= 'F' )
		return (c - 'A') + 10;

	pdsRuntimeCheck( false, Status::EParam, "invalid hex character c" );
	}

// retrieves bytes from a hex string of known length.
// note: the count is equal to the number of bytes, and the hex string is assumed to be twice the count (since two hex values is combined into one byte)
void pds::hex_string_to_bytes( void *bytes, const char *hex_string, size_t count )
	{
	pdsRuntimeCheck( bytes, Status::EParam, "bytes cannot be nullptr" );

	u8 *p = (u8 *)bytes;
	for( size_t i = 0; i < count; ++i )
		{
		p[i] = decode_hex_char( hex_string[i * 2 + 0] ) << 4 
			| decode_hex_char( hex_string[i * 2 + 1] );
		}
	}

template <> u8 pds::hex_string_to_value<u8>( const char *hex_string )
	{
	u8 ret;
	hex_string_to_bytes( &ret, hex_string, sizeof( u8 ) );
	return ret;
	}

template <> u16 pds::hex_string_to_value<u16>( const char *hex_string )
	{
	u8 bytes[sizeof( u16 )];
	hex_string_to_bytes( bytes, hex_string, sizeof( u16 ) );
	return value_from_bigendian<u16>( bytes );
	}

template <> u32 pds::hex_string_to_value<u32>( const char *hex_string )
	{
	u8 bytes[sizeof( u32 )];
	hex_string_to_bytes( bytes, hex_string, sizeof( u32 ) );
	return value_from_bigendian<u32>( bytes );
	}

template <> u64 pds::hex_string_to_value<u64>( const char *hex_string )
	{
	u8 bytes[sizeof( u64 )];
	hex_string_to_bytes( bytes, hex_string, sizeof( u64 ) );
	return value_from_bigendian<u64>( bytes );
	}

template <> uuid pds::hex_string_to_value<uuid>( const char *hex_string )
	{
	pdsRuntimeCheck( hex_string[8] == '-'
		&& hex_string[13] == '-'
		&& hex_string[18] == '-'
		&& hex_string[23] == '-', Status::EParam, "hex_string_to_value ill-formated hex_string" );

	uuid value;
	value.Data1 = hex_string_to_value<u32>( &hex_string[0] );
	value.Data2 = hex_string_to_value<u16>( &hex_string[9] );
	value.Data3 = hex_string_to_value<u16>( &hex_string[14] );
	hex_string_to_bytes( &value.Data4[0], &hex_string[19] , 2 );
	hex_string_to_bytes( &value.Data4[2], &hex_string[24] , 6 );
	return value;
	}

template <> hash pds::hex_string_to_value<hash>( const char *hex_string )
	{
	static_assert(sizeof( hash ) == 32, "Error: hash is assumed to be of size 32.");
	hash value;
	hex_string_to_bytes( &value, hex_string, 32 );
	// note: no need to swap order of bytes. 
	// The hash is always ordered the same, regardless of the hardware (basically a big-endian 256 bit value)
	return value;
	}

std::wstring pds::full_path( const std::wstring &path )
	{
	std::wstring ret;

	wchar_t *buffer = new wchar_t[32768];
	DWORD len = GetFullPathNameW( path.c_str(), 32768, buffer, nullptr );
	if( len > 0 )
		{
		ret = std::wstring( buffer );
		}
	delete[] buffer;

	return ret;
	}

item_ref item_ref::make_ref()
	{
	item_ref ref;

	RPC_STATUS stat = ::UuidCreate( &ref.id_m );
	if( stat != RPC_S_OK
		|| ref.id_m == uuid_zero )
		{
		throw std::exception( "Failed to generate a uuid through ::UuidCreate()" );
		}

	return ref;
	}

static std::shared_ptr<Entity> entityNew( const std::vector<const EntityHandler::PackageRecord*> &records , const char *entityTypeString )
	{
	if( !entityTypeString )
		{
		pdsErrorLog << "Invalid parameter, entityTypeString must be a pointer to a string" << pdsErrorLogEnd;
		return nullptr;
		}

	for( size_t i = 0; i < records.size(); ++i )
		{
		auto ret = records[i]->New( entityTypeString );
		if( ret )
			return ret;
		}

	pdsErrorLog << "Unrecognized entity, cannot allocate entity of type: " << entityTypeString << " is not registered with any package." << pdsErrorLogEnd;
	return nullptr;
	}

static bool entityWrite( const std::vector<const EntityHandler::PackageRecord*> &records , const Entity *obj, EntityWriter &writer )
	{
	if( !obj )
		{
		pdsErrorLog << "Invalid parameter, obj must be a pointer to an allocated object" << pdsErrorLogEnd;
		return false;
		}

	for( size_t i = 0; i < records.size(); ++i )
		{
		auto ret = records[i]->Write( obj, writer );
		if( ret )
			return ret;
		}

	pdsErrorLog << "Unrecognized entity, " << obj->EntityTypeString() << " is not registered with any package." << pdsErrorLogEnd;
	return false;
	}

static bool entityRead( const std::vector<const EntityHandler::PackageRecord*> &records , Entity *obj, EntityReader &reader )
	{
	if( !obj )
		{
		pdsErrorLog << "Invalid parameter, obj must be a pointer to an allocated object" << pdsErrorLogEnd;
		return false;
		}

	for( size_t i = 0; i < records.size(); ++i )
		{
		auto ret = records[i]->Read( obj, reader );
		if( ret )
			return ret;
		}

	pdsErrorLog << "Unrecognized entity, " << obj->EntityTypeString() << " is not registered with any package." << pdsErrorLogEnd;
	return false;
	}

static bool entityValidate( const std::vector<const EntityHandler::PackageRecord*> &records , const Entity *obj, EntityValidator &validator )
	{
	if( !obj )
		{
		pdsErrorLog << "Invalid parameter, obj must be a pointer to an allocated object" << pdsErrorLogEnd;
		return false;
		}

	for( size_t i = 0; i < records.size(); ++i )
		{
		auto ret = records[i]->Validate( obj, validator );
		if( ret )
			return ret;
		}

	pdsErrorLog << "Unrecognized entity, " << obj->EntityTypeString() << " is not registered with any package." << pdsErrorLogEnd;
	return false;
	}

void EntityHandler::InsertEntity( const entity_ref &ref, const std::shared_ptr<const Entity> &entity )
	{
	ctle::readers_writer_lock::write_guard guard( this->EntitiesLock );

	this->Entities.emplace( ref, entity );
	}

Status EntityHandler::Initialize( const std::string &path , const std::vector<const PackageRecord*> &records )
	{
	if( !this->Path.empty() )
		{
		return Status::EAlreadyInitialized;
		}
	if( records.empty() )
		{
		return Status::EParam; // must have at least one record
		}

	std::wstring wpath = widen( path );

	// make path absolute
	wpath = full_path( wpath );

	// make sure it is a directory 
	DWORD file_attributes = GetFileAttributesW( wpath.c_str() );
	if(    (file_attributes == INVALID_FILE_ATTRIBUTES)
		|| (file_attributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
		{
		pdsErrorLog << "Invalid path: " << path << pdsErrorLogEnd;
		return Status::EParam; // invalid path
		}

	this->Path = wpath;

	// copy the package records
	this->Records = records;

	return Status::Ok;
	}

Status EntityHandler::ReadTask( EntityHandler *pThis, const entity_ref ref )
	{
	const uint hash_size = 32;

	// skip if entity already is loaded
	if( pThis->IsEntityLoaded( ref ) )
		{
		return Status::Ok;
		}

	// create the file name and path from the hash
	const std::wstring fileName = widen( value_to_hex_string( hash( ref ) ) ) + L".dat";
	const std::wstring filePath = pThis->Path + L"\\" + fileName;

	// open the file
	HANDLE file_handle = ::CreateFileW( filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr );
	if( file_handle == INVALID_HANDLE_VALUE )
		{
		// failed to open the file
		return Status::ECantOpen;
		}

	// get the size
	LARGE_INTEGER dfilesize = {};
	if( !::GetFileSizeEx( file_handle, &dfilesize ) )
		{
		// failed to get the size
		return Status::ECantOpen;
		}
	u64 total_bytes_to_read = dfilesize.QuadPart;

	// cant be less in size than the size of the hash at the end
	if( total_bytes_to_read < hash_size )
		{
		return Status::ECorrupted;
		}

	// read in all of the file
	std::vector<u8> allocation;
	allocation.resize( total_bytes_to_read );
	if( allocation.size() != total_bytes_to_read )
		{
		// failed to allocate the memory
		return Status::ECantAllocate;
		}
	u8 *buffer = allocation.data();

	u64 bytes_read = 0;
	while( bytes_read < total_bytes_to_read )
		{
		// check how much to read and cap each read at UINT_MAX
		u64 bytes_left = total_bytes_to_read - bytes_read;
		u32 bytes_to_read_this_time = UINT_MAX;
		if( bytes_left < UINT_MAX )
			bytes_to_read_this_time = (u32)bytes_left;

		// read in bytes into the memory allocation
		DWORD bytes_that_were_read = 0;
		if( !::ReadFile( file_handle, &buffer[bytes_read], bytes_to_read_this_time, &bytes_that_were_read, nullptr ) )
			{
			// failed to read
			return Status::ECantRead;
			}

		// update number of bytes that were read
		bytes_read += bytes_that_were_read;
		}

	::CloseHandle( file_handle );

	// calculate the sha256 hash on the data, and make sure it compares correctly with the hash
	SHA256 sha( buffer, total_bytes_to_read );
	hash digest;
	sha.GetDigest( digest.digest );
	if( digest != hash( ref ) )
		{
		// sha hash does not compare correctly, file is corrupted
		return Status::ECorrupted;
		}

	// set up a memory stream and deserializer
	MemoryReadStream rstream( buffer, total_bytes_to_read, false );
	EntityReader reader( rstream );

	// read file header and deserialize the entity
	bool result = {};
	EntityReader *sectionReader;
	std::tie( sectionReader, result ) = reader.BeginReadSection( pdsKeyMacro( "EntityFile" ), false );
	if( !result )
		return Status::ECorrupted;
	std::string entityTypeString;
	result = sectionReader->Read<std::string>( pdsKeyMacro( "EntityType" ), entityTypeString );
	if( !result )
		return Status::ECorrupted;
	std::shared_ptr<Entity> entity = entityNew( pThis->Records , entityTypeString.c_str() );
	if( !entity )
		return Status::ENotInitialized;
	result = entityRead( pThis->Records , entity.get(), *sectionReader );
	if( !result )
		return Status::ECorrupted;
	result = reader.EndReadSection( sectionReader );
	if( !result )
		return Status::ECorrupted;

	// transfer into the Entities map 
	pThis->InsertEntity( ref, entity );

	// done
	return Status::Ok;
	}

std::future<Status> EntityHandler::LoadEntityAsync( const entity_ref &ref )
	{
	return std::async( ReadTask, this, ref );
	}

Status EntityHandler::LoadEntity( const entity_ref &ref )
	{
	auto futr = this->LoadEntityAsync( ref );
	futr.wait();
	return futr.get();
	}

Status EntityHandler::UnloadNonReferencedEntities()
	{
	ctle::readers_writer_lock::write_guard guard( this->EntitiesLock );

	auto it = this->Entities.begin();
	while( it != this->Entities.end() )
		{
		// if this entity is only held by us, remove it, else skip to next
		if( it->second.use_count() == 1 )
			{
			it = this->Entities.erase( it );
			}
		else
			{
			++it;
			}
		}

	return Status::Ok;
	}


bool EntityHandler::IsEntityLoaded( const entity_ref &ref )
	{
	ctle::readers_writer_lock::read_guard guard( this->EntitiesLock );

	return this->Entities.find( ref ) != this->Entities.end();
	}

std::shared_ptr<const Entity> EntityHandler::GetLoadedEntity( const entity_ref &ref )
	{
	ctle::readers_writer_lock::read_guard guard( this->EntitiesLock );

	const auto it = this->Entities.find( ref );
	if( it == this->Entities.end() )
		return nullptr;

	return it->second;
	}

std::pair<entity_ref, Status> EntityHandler::WriteTask( EntityHandler *pThis, std::shared_ptr<const Entity> entity )
	{
	EntityValidator validator;
	MemoryWriteStream wstream;
	EntityWriter writer( wstream );

	// make sure the entity is valid
	if( !entityValidate( pThis->Records , entity.get(), validator ) )
		return std::pair<entity_ref, Status>( {}, Status::ECorrupted );
	if( validator.GetErrorCount() > 0 )
		return std::pair<entity_ref, Status>( {}, Status::EInvalid );

	// serialize to a stream
	EntityWriter *sectionWriter = writer.BeginWriteSection( pdsKeyMacro( "EntityFile" ) );
	if( !sectionWriter )
		return std::pair<entity_ref, Status>( {}, Status::EUndefined );
	sectionWriter->Write<std::string>( pdsKeyMacro( "EntityType" ), entity->EntityTypeString() );
	if( !entityWrite( pThis->Records , entity.get(), *sectionWriter ) )
		return std::pair<entity_ref, Status>( {}, Status::EUndefined );
	if( !writer.EndWriteSection( sectionWriter ) )
		return std::pair<entity_ref, Status>( {}, Status::EUndefined );

	// calculate the sha256 hash on the data
	SHA256 sha( (u8 *)wstream.GetData(), wstream.GetSize() );
	hash digest;
	sha.GetDigest( digest.digest );

	// create the file name and path from the hash
	const std::wstring fileName = widen( value_to_hex_string( digest ) ) + L".dat";
	const std::wstring filePath = pThis->Path + L"\\" + fileName;

	// open the file
	HANDLE fileHandle = ::CreateFileW( filePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr );
	if( fileHandle != INVALID_HANDLE_VALUE )
		{
		// write all of the file
		const u8 *writeBuffer = (u8 *)wstream.GetData();
		const u64 totalBytesToWrite = wstream.GetSize();

		u64 bytesWritten = 0;
		while( bytesWritten < totalBytesToWrite )
			{
			// check how much to write, capped at UINT_MAX
			u64 bytesToWrite = std::min<u64>( totalBytesToWrite - bytesWritten, UINT_MAX );

			// read in bytes into the memory allocation
			DWORD numBytesWritten = 0;
			if( !::WriteFile( fileHandle, &writeBuffer[bytesWritten], (DWORD)bytesToWrite, &numBytesWritten, nullptr ) )
				{
				// failed to read
				return std::pair<entity_ref, Status>( {}, Status::ECantWrite );
				}

			// update number of bytes that were read
			bytesWritten += numBytesWritten;
			}

		::CloseHandle( fileHandle );
		}
	else
		{
		// file open failed. if it is because the file already exists, that is ok
		// all other issues, return error
		DWORD errorCode = GetLastError();
		if( errorCode != ERROR_FILE_EXISTS )
			{
			// failed to open the file
			return std::pair<entity_ref, Status>( {}, Status::ECantOpen );
			}
		}

	// transfer into the Entities map 
	pThis->InsertEntity( entity_ref( digest ), entity );

	// done
	return std::pair<entity_ref, Status>( entity_ref( digest ), Status::Ok );
	}

std::future<std::pair<entity_ref, Status>> EntityHandler::AddEntityAsync( const std::shared_ptr<const Entity> &entity )
	{
	return std::async( WriteTask, this, entity );
	}

std::pair<entity_ref, Status> EntityHandler::AddEntity( const std::shared_ptr<const Entity> &entity )
	{
	auto futr = this->AddEntityAsync( entity );
	futr.wait();
	return futr.get();
	}
