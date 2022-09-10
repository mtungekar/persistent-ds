// include GUID stuff from windows
#ifdef _WIN32
#define __INLINE_ISEQUAL_GUID
#include <guiddef.h>
#endif//_WIN32

// define UUID
#ifndef UUID_DEFINED
#define UUID_DEFINED
typedef GUID UUID;
#ifdef _WIN32
#ifndef uuid_t
#define uuid_t UUID
#endif//uuid_t
#endif//_WIN32

inline bool operator<( const UUID &Left, const UUID &Right ) 
	{
	return memcmp( &Left, &Right, sizeof( UUID ) ) < 0;
	};

std::ostream &operator<<( std::ostream &os, const UUID &_uuid );

template<>
struct std::hash<UUID>
	{
	std::size_t operator()(UUID const& val) const noexcept
		{
		static_assert(sizeof( std::size_t ) == sizeof( std::uint64_t ), "Code is assuming 64 bit size_t" );

		const std::size_t *ptr = (const std::size_t *)&val;
		return ptr[0] ^ ptr[1];
		}
	};

#endif//UUID_DEFINED

// define hash for sha256 message digests
#ifndef HASH_DEFINED
#define HASH_DEFINED

struct HASH
	{
	union
		{
		std::uint64_t _digest_q[4];
		std::uint8_t digest[32];
		};
	};

inline bool operator<( const HASH &Left, const HASH &Right ) 
	{
	return memcmp( &Left, &Right, sizeof( HASH ) ) < 0;
	};

inline bool operator==( const HASH &Left, const HASH &Right ) 
	{
	return (Left._digest_q[0] == Right._digest_q[0])
		&& (Left._digest_q[1] == Right._digest_q[1])
		&& (Left._digest_q[2] == Right._digest_q[2])
		&& (Left._digest_q[3] == Right._digest_q[3]);
	};

inline bool operator!=( const HASH &Left, const HASH &Right ) 
	{
	return (Left._digest_q[0] != Right._digest_q[0])
		|| (Left._digest_q[1] != Right._digest_q[1])
		|| (Left._digest_q[2] != Right._digest_q[2])
		|| (Left._digest_q[3] != Right._digest_q[3]);
	};

std::ostream &operator<<( std::ostream &os, const HASH &_hash );

template<>
struct std::hash<HASH>
	{
	std::size_t operator()(HASH const& val) const noexcept
		{
		static_assert(sizeof( std::size_t ) == sizeof( std::uint64_t ), "Code is assuming 64 bit size_t" );

		return val._digest_q[0] 
			 ^ val._digest_q[1] 
			 ^ val._digest_q[2] 
			 ^ val._digest_q[3];
		}
	};

#endif//HASH_DEFINED