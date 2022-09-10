
#include <pds/ValueTypes.h>

#include <string>
#include <vector>
#include <set>

#include <glm/glm.hpp>

using namespace pds;

const i64 global_random_seed = 12876123876;

inline void setup_random_seed()
	{
	i64 seed = (global_random_seed == -1) ? i64( time( nullptr ) ) : global_random_seed;
	srand(uint( seed & 0xffffffff ));
	}

// add headers that you want to pre-compile here
inline u8 u8_rand() { return (u8)(rand() & 0xff); } 
inline u16 u16_rand() { return (rand() << 4) ^ rand(); } 
inline u32 u32_rand() { return u32(u16_rand()) << 16 | u32(u16_rand()); } 
inline u64 u64_rand() { return u64(u32_rand()) << 32 | u64(u32_rand()); } 
inline float float_rand() { return float(u64_rand()); } 
inline double double_rand() { return double(u64_rand()); } 

inline UUID uuid_rand() 
	{ 
	const UUID id = {u32_rand(), u16_rand(), u16_rand(), { u8_rand(), u8_rand(), u8_rand(), u8_rand(), u8_rand(), u8_rand(), u8_rand(), u8_rand() }};
	return id;
	}

inline hash hash_rand() 
	{ 
	const hash id = {u64_rand(),u64_rand(),u64_rand(),u64_rand()};
	return id;
	}

//// random string
inline std::string str_rand( size_t min_len = 0 , size_t max_len = 1000 )
	{
	size_t strl = (rand() % (max_len-min_len)) + min_len;
	std::string str;
	str.resize( strl );
	for( size_t i = 0; i < strl; ++i )
		{
		str[i] = (char)((rand() % 0x60) + 0x20); // generate ASCII values in the range 0x20 - 0x7f
		}
	return str;
	}

inline size_t capped_rand( size_t minv, size_t maxv )
	{
	if( maxv == minv )
		return minv;
	return (u64_rand() % (maxv - minv)) + minv;
	}

// -------------------------------------------

template<class T> T random_value();

template<class T> T random_value( T &dest )
	{
	dest = random_value<T>();
	}

template<class T> void random_nonzero_value( T &dest )
	{
	dest = random_value<T>();
	while( dest == data_type_information<T>::zero )
		dest = random_value<T>();
	}

// -------------------------------------------

template<class T> optional_value<T> random_optional_value()
	{
	optional_value<T> val;
	if( random_value<bool>() )
		{
		val.set( random_value<T>() );
		}
	return val;
	}

template<class T> void random_optional_value( optional_value<T> &dest )
	{
	dest = random_optional_value<T>();
	}

template<class T> void random_nonzero_optional_value( optional_value<T> &dest )
	{
	dest.set( random_value<T>() );
	while( dest.value() == data_type_information<T>::zero )
		dest.set( random_value<T>() );
	}

// -------------------------------------------

template <class T> void random_vector( std::vector<T> &dest , size_t minc = 10, size_t maxc = 1000 )
	{
	size_t len = capped_rand( minc, maxc );
	dest.resize(len);
	for( size_t i = 0; i < len; ++i )
		{
		dest[i] = random_value<T>();
		}
	}

template <class T> void random_nonzero_vector( std::vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	minc = (minc > 1) ? minc : 1;
	maxc = (maxc > 1) ? maxc : 1;
	random_vector( dest, minc, maxc );
	}

// -------------------------------------------

template<class T> void random_nonzero_optional_vector( optional_vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	dest.set();
	random_nonzero_vector<T>( dest.vector(), minc, maxc );
	}

template<class T> void random_optional_vector( optional_vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	if( random_value<bool>() )
		{
		random_nonzero_optional_vector<T>( dest, minc, maxc );
		}
	else
		{
		dest.reset();
		}
	}

// -------------------------------------------

template <class T> void random_idx_vector( idx_vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	// generate a random indexed array
	random_vector<T>( dest.values(), minc, maxc );
	i32 index_maxval = (i32)dest.values().size();
	auto indices = dest.index();
	if( index_maxval > 0 )
		{
		size_t index_len = capped_rand( minc, maxc );
		indices.resize( index_len );
		for( size_t i = 0; i < index_len; ++i )
			{
			indices[i] = (i32)capped_rand( 0, index_maxval ); // make sure all indices are actually valid
			}
		}
	else
		{
		indices.clear();
		}
	}

template <class T> void random_nonzero_idx_vector( idx_vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	minc = (minc > 1) ? minc : 1;
	maxc = (maxc > 1) ? maxc : 1;
	random_idx_vector( dest, minc, maxc );
	}

// -------------------------------------------

template<class T> void random_nonzero_optional_idx_vector( optional_idx_vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	dest.set();
	random_nonzero_idx_vector<T>( dest.vector(), minc, maxc );
	}

template<class T> void random_optional_idx_vector( optional_idx_vector<T> &dest, size_t minc = 10, size_t maxc = 1000 )
	{
	if( random_value<bool>() )
		{
		random_nonzero_optional_idx_vector( dest, minc, maxc );
		}
	else
		{
		dest.reset();
		}
	}

// -------------------------------------------