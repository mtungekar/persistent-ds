// include GUID stuff from windows
#ifdef _WIN32
#include <objbase.h>
#define __INLINE_ISEQUAL_GUID
#include <guiddef.h>
#else
#include <uuid/uuid.h>
#endif//_WIN32

#include<functional>
#include<cstdint>
#include<iostream>
#include<cstring>
#include<float.h>
#include<array>
#include<algorithm>
#include<sstream>
#include<iomanip>

// define UUID

namespace pds
	{
	struct uuid
		{
		public:
			/*
			Default constructor
			*/
			uuid() = default;
			
			/*
			Default copy
			*/
			
			uuid(const uuid& other) = default;
			/*
			Default assignment
			*/
			
			uuid& operator=(const uuid& other) = default;
			/*
			Default move
			*/
			
			uuid(uuid&& other) = default;
			/*
			Default move assignment
			*/
			
			uuid& operator=(uuid&& other) = default;

			/*
			Equals
			*/
			bool operator==(const uuid& other) const
				{
				return (*this).byteData == other.byteData;
				}
			
			/*
			Not Equals
			*/
			bool operator!=(const uuid& other) const
				{
				return !(*this == other);
				}

			
			constexpr explicit uuid(const std::array<uint8_t,16>& data)
				:byteData(data)
				{
				}
			/*
			Explicit move construction from byte array
			*/
			constexpr explicit uuid(std::array<uint8_t,16>&& data)
				:byteData(std::move(data))
				{
				}
			constexpr uint8_t hex_to_digit(char c)
				{
				// 0-9
				if (c > 47 && c < 58)
					return c - 48;
				// a-f
				if (c > 96 && c < 103)
					return c - 87;
				return c;
				}
			 explicit uuid(const std::string& data)
				{
				std::string temp(data);
				temp.erase(std::remove(temp.begin(),temp.end(),'-'),temp.end());
				std::transform(temp.begin(),temp.end(), temp.begin(), [](char c){return ::tolower(c);});
				std::string::iterator it = temp.begin();
				for(auto byte_index=0; byte_index<16;++byte_index)
					{
					byteData[byte_index] = (hex_to_digit(temp[byte_index*2]) << 4) + hex_to_digit(temp[byte_index*2+1]);
					}
				}
#ifdef _WIN32
			explicit uuid (const GUID& data)
				{
				/* 
				Windows GUID structure is mixendian. 
				Data blocks (1,2,3) are little endian while block 4 is big endian.
				Store everything as bigendian in our uuid byte structure.
				*/
				byteData[0] = static_cast<uint8_t>(data.Data1 >> 24);
				byteData[1] = static_cast<uint8_t>(data.Data1 >> 16);
				byteData[2] = static_cast<uint8_t>(data.Data1 >> 8);
				byteData[3] = static_cast<uint8_t>(data.Data1);
				byteData[4] = static_cast<uint8_t>(data.Data2 >> 8);
				byteData[5] = static_cast<uint8_t>(data.Data2);
				byteData[6] = static_cast<uint8_t>(data.Data3 >> 8);
				byteData[7] = static_cast<uint8_t>(data.Data3);
				memcpy(&byteData[8],&data.Data4,8);
				}
#endif
			
			operator std::string() const
				{
				return str();
				}
			const std::array<uint8_t,16>& bytes() const
				{
				return byteData;
				}
			constexpr inline uint8_t get_high_nibble(uint8_t byte) const
				{
				return ((byte >> 4) & 0xF);
				}
			constexpr inline uint8_t get_low_nibble(uint8_t byte) const
				{
				return (byte & 0xF);
				}
			inline std::string str() const
				{
				std::string result;
				static const char hexchars[] = "0123456789abcdef";
				for(auto& c: byteData)
					{
					result+= hexchars[get_high_nibble(c)];
					result+= hexchars[get_low_nibble(c)];
					if(result.size() == 8 || result.size() == 13 || result.size() == 18 || result.size() == 23)
						result+="-";
					}
				return result;
				}
			inline bool is_valid() const
				{
				uuid temp;
				return *this != temp;
				}

    

#ifdef _WIN32
			static uuid new_guid()
				{
				GUID newId;
				::CoCreateGuid(&newId);
				return uuid(newId);
				}
#endif

    		inline void reset()
    			{
        		std::fill(byteData.begin(),byteData.end(),0);
    			}
			

		private:
			std::array<uint8_t,16> byteData{0};

			friend bool operator<(const uuid &left, const uuid &right);
			friend std::ostream& operator<<( std::ostream &os, const uuid& _uuid);
		};

		inline bool operator<(const uuid& left, const uuid& Right)
			{
			return memcmp(&left.bytes()[0], &Right.bytes()[0],16) < 0;
			}
	
	}



template<>
struct std::hash<pds::uuid>
	{
	std::size_t operator()(pds::uuid const& val) const noexcept
		{
		static_assert(sizeof( std::size_t ) == sizeof( std::uint64_t ), "Code is assuming 64 bit size_t" );

		const std::size_t *ptr = (const std::size_t *)&val;
		return ptr[0] ^ ptr[1];
		}
	};



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