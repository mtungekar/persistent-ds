// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#include "Tests.h"

#include <pds/SHA256.h>
TEST( TypeTests , StandardTypes )
	{
	EXPECT_EQ(sizeof(u8) , 1);
	EXPECT_EQ(sizeof(u16) , 2);
	EXPECT_EQ(sizeof(u32) , 4);
	EXPECT_EQ(sizeof(u64) , 8);
	EXPECT_EQ(sizeof(i8) , 1);
	EXPECT_EQ(sizeof(i16) , 2);
	EXPECT_EQ(sizeof(i32) , 4);
	EXPECT_EQ(sizeof(i64) , 8);
	EXPECT_EQ(sizeof(uint) , 4);
	EXPECT_EQ(sizeof(uuid) , 16);
	EXPECT_EQ(sizeof(hash) , 32);

//#ifdef _MSC_VER
//	// test widen()
//	std::string str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
//	std::wstring expected_wstr = L"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
//	std::wstring wstr = widen( str );
//	EXPECT_EQ( wstr , expected_wstr );
//
//	// test full_path()
//	std::wstring rel_path( L"." );
//	std::wstring fpath = full_path( rel_path );
//	wchar_t currpath[MAX_PATH];
//	GetCurrentDirectoryW( MAX_PATH, currpath );
//	std::wstring expected_fpath( currpath );
//	EXPECT_EQ( fpath , expected_fpath );
//#endif
	}

TEST( TypeTests , ByteswapFunctions )
	{
	const u16 u16val = 0x6712;
	const u32 u32val = 0x93671028;
	const u64 u64val = 0x2047239209461749;

	const u8 u16_be[] = {0x67, 0x12};
	const u8 u32_be[] = {0x93, 0x67, 0x10, 0x28};
	const u8 u64_be[] = {0x20, 0x47, 0x23, 0x92, 0x09, 0x46, 0x17, 0x49};

	u16 t16 = value_from_bigendian<u16>( u16_be );
	EXPECT_EQ(t16 , u16val);
	u32 t32 = value_from_bigendian<u32>( u32_be );
	EXPECT_EQ(t32 , u32val);
	u64 t64 = value_from_bigendian<u64>( u64_be );
	EXPECT_EQ(t64 , u64val);
			
	u8 res[8];
	bigendian_from_value<u16>( res , u16val );
	EXPECT_EQ(memcmp(res,u16_be,sizeof(u16_be)) , 0);
	bigendian_from_value<u32>( res , u32val );
	EXPECT_EQ(memcmp(res,u32_be,sizeof(u32_be)) , 0);
	bigendian_from_value<u64>( res , u64val );
	EXPECT_EQ(memcmp(res,u64_be,sizeof(u64_be)) , 0);

	t16 = u16val;
	swap_byte_order( &t16 );
	EXPECT_EQ( ((t16 >> 0) & 0xff) , ((u16val >>  8) & 0xff) );
	EXPECT_EQ( ((t16 >> 8) & 0xff) , ((u16val >>  0) & 0xff) );
				
	t32 = u32val;
	swap_byte_order( &t32 );
	EXPECT_EQ( ((t32 >>  0) & 0xff) , ((u32val >> 24) & 0xff) );
	EXPECT_EQ( ((t32 >>  8) & 0xff) , ((u32val >> 16) & 0xff) );
	EXPECT_EQ( ((t32 >> 16) & 0xff) , ((u32val >>  8) & 0xff) );
	EXPECT_EQ( ((t32 >> 24) & 0xff) , ((u32val >>  0) & 0xff) );

	t64 = u64val;
	swap_byte_order( &t64 );
	EXPECT_EQ( ((t64 >>  0) & 0xff) , ((u64val >> 56) & 0xff) );
	EXPECT_EQ( ((t64 >>  8) & 0xff) , ((u64val >> 48) & 0xff) );
	EXPECT_EQ( ((t64 >> 16) & 0xff) , ((u64val >> 40) & 0xff) );
	EXPECT_EQ( ((t64 >> 24) & 0xff) , ((u64val >> 32) & 0xff) );
	EXPECT_EQ( ((t64 >> 32) & 0xff) , ((u64val >> 24) & 0xff) );
	EXPECT_EQ( ((t64 >> 40) & 0xff) , ((u64val >> 16) & 0xff) );
	EXPECT_EQ( ((t64 >> 48) & 0xff) , ((u64val >>  8) & 0xff) );
	EXPECT_EQ( ((t64 >> 56) & 0xff) , ((u64val >>  0) & 0xff) );
	}

TEST( TypeTests , HexStringFunctions )
	{
	uuid uuidval = { 0xb2b1a2a178563412 , 0xc8c7c6c5c4c3c2c1 };
	std::string expected_hexuuidval = "12345678-a1a2-b1b2-c1c2-c3c4c5c6c7c8";
	EXPECT_EQ( value_to_hex_string( uuidval ) , expected_hexuuidval );

	const u8 hashdata[32] = {
		0xf6,0x48,0x54,0x2d,0xf8,0xcc,0xf2,0x1f,
		0xd3,0x4e,0x95,0xf6,0x7d,0xf5,0xf2,0xb4,
		0xf2,0x72,0x72,0xaa,0x14,0xf5,0x03,0x09,
		0x0c,0xc4,0x76,0x6f,0xe2,0x78,0xc4,0xb5
		};
	hash hashval;
	memcpy( &hashval, hashdata, 32 );
	std::string expected_hexhashval = "f648542df8ccf21fd34e95f67df5f2b4f27272aa14f503090cc4766fe278c4b5";
	EXPECT_EQ( value_to_hex_string( hashval ) , expected_hexhashval );

	u8 u8val = 0x13;
	std::string expected_hexu8 = "13";
	EXPECT_EQ( value_to_hex_string( u8val ) , expected_hexu8 );

	u16 u16val = 0x1234;
	std::string expected_hexu16 = "1234";
	EXPECT_EQ( value_to_hex_string( u16val ) , expected_hexu16 );

	u32 u32val = 0x0218a782;
	std::string expected_hexu32 = "0218a782";
	EXPECT_EQ( value_to_hex_string( u32val ) , expected_hexu32 );

	u64 u64val = 0x35023218a7828505;
	std::string expected_hexu64 = "35023218a7828505";
	EXPECT_EQ( value_to_hex_string( u64val ) , expected_hexu64 );
	}

TEST( TypeTests , SHA256Hashing )
	{
	if( true )
		{
		SHA256 sha;

		u8 testdata[] = {
			0x34,0x2b,0x1f,0x3e,0x61,
			0x4b,0x03,0x4b,0x02,0x36,
			0x05,0x5c,0x17,0x29,0x3d,
			0x53,0x0e,0x5e,0x5b,0x4d,
			0x52,0x5f,0x12,0x20,0x0a,
			0x56,0x31,0x3b,0x2c,0x06,
			0x51,0x28,0x28,0x5d,0x05,
			0x59,0x2b,0x41,0x0d,0x1f,
			0x01,0x01,0x1b,0x1f,0x09,
			0x2c,0x13,0x01,0x46,0x19,
			0x05,0x3e,0x3c,0x2d,0x58,
			0x16,0x5f,0x19,0x0f,0x07,
			0x39,0x48,0x46,0x4b,0x23,
			0x06,0x15,0x0b,0x44,0x18,
			0x0e,0x38,0x56,0x0e,0x0a,
			0x0e,0x54,0x43,0x0a,0x31,
			0x2d,0x51,0x0d,0x2a,0x5a,
			0x09,0x06,0x10,0x23,0x24,
			0x23,0x33,0x2e,0x1d,0x56,
			0x48,0x2f,0x4a,0x33,0x06
			};

		sha.Update( testdata, sizeof( testdata ) );

		u8 calc_hash[32];
		sha.GetDigest( calc_hash );

		u8 expected_hash[32] = {
			0xf6,0x48,0x54,0x2d,0xf8,0xcc,0xf2,0x1f,
			0xd3,0x4e,0x95,0xf6,0x7d,0xf5,0xf2,0xb4,
			0xf2,0x72,0x72,0xaa,0x14,0xf5,0x03,0x09,
			0x0c,0xc4,0x76,0x6f,0xe2,0x78,0xc4,0xb5
			};

		EXPECT_EQ( memcmp( calc_hash, expected_hash, 32 ) , 0 );
		}
	}

TEST( TypeTests , Test_item_ref )
	{
	item_ref ref;
	EXPECT_TRUE( !ref );
	uuid val = ref;
	EXPECT_TRUE( val == uuid_zero );
	EXPECT_TRUE( val == item_ref::null() );
	EXPECT_TRUE( ref == item_ref() );

	item_ref ref2 = item_ref::make_ref();
	EXPECT_TRUE( ref != ref2 );
	EXPECT_TRUE( !(ref == ref2) );
	EXPECT_TRUE( ref < ref2 ); // ref is zero, ref2 must be more 
	val = ref2;
	EXPECT_TRUE( val != uuid_zero );
	EXPECT_TRUE( uuid_zero < val );

	ref = std::move( ref2 );
	EXPECT_TRUE( ref != ref2 );
	EXPECT_TRUE( !(ref == ref2) );
	EXPECT_TRUE( ref2 < ref ); // ref2 is zero, ref must be more 

	ref2 = ref;
	EXPECT_TRUE( ref == ref2 );
	EXPECT_TRUE( !(ref != ref2) );
	EXPECT_TRUE( !(ref2 < ref) ); 
	}

TEST( TypeTests , Test_entity_ref )
	{
	entity_ref ref;
	EXPECT_TRUE( !ref );
	hash val = ref;
	EXPECT_EQ( val , hash_zero );
	EXPECT_EQ( entity_ref(val) , entity_ref::null() );
	EXPECT_EQ( ref , entity_ref() );

	entity_ref ref2 = random_value<hash>();
	EXPECT_NE( ref ,  ref2 );
	EXPECT_TRUE( !(ref == ref2) );
	EXPECT_LT( ref , ref2 ); // ref is zero, ref2 must be more 
	val = ref2;
	EXPECT_NE( val , hash_zero );
	EXPECT_LT( hash_zero , val );

	ref = std::move( ref2 );
	EXPECT_NE( ref , ref2 );
	EXPECT_TRUE( !(ref == ref2) );
	EXPECT_LT( ref2 , ref ); // ref2 is zero, ref must be more 

	ref2 = ref;
	EXPECT_EQ( ref , ref2 );
	EXPECT_TRUE( !(ref != ref2) );
	EXPECT_TRUE( !(ref2 < ref) ); 
	}

template<typename K, typename V>
std::map<V, K> inverse_map(std::map<K, V> &map)
	{
	std::map<V, K> inv;
	std::for_each(map.begin(), map.end(),
		[&inv] (const std::pair<K, V> &p) {
		inv.emplace(p.second, p.first);
		});
	return inv;
	}

// this tests the hash function, the less than operator and the equals operator
template<class K> void TestSetAndMapWithKey()
	{ 
	const size_t item_count = 100;

	std::map<K, size_t> mmap;
	std::unordered_map<K, size_t> umap;
	std::set<K> mset;
	std::unordered_set<K> uset;
	for( size_t i = 0; i < item_count; ++i )
		{
		K key = random_value<K>();
		mmap.emplace( key, i );
		umap.emplace( key, i );
		mset.emplace( key );
		uset.emplace( key );
		}
		
	EXPECT_EQ( item_count , mmap.size() );
	EXPECT_EQ( item_count , umap.size() );
	EXPECT_EQ( item_count , mset.size() );
	EXPECT_EQ( item_count , uset.size() );

	auto lookup = inverse_map( mmap );

	for( size_t i = 0; i < item_count; ++i )
		{
		// lookup key from value
		bool found = lookup.find(i) != lookup.end();
		EXPECT_TRUE( found );
		K key = lookup[i];
		
		// make sure that the key is in map and umap, and returns same value
		found = mmap.find( key ) != mmap.end();
		EXPECT_TRUE( found );
		EXPECT_EQ( mmap[key] , i );
		found = umap.find( key ) != umap.end();
		EXPECT_TRUE( found );
		EXPECT_EQ( umap[key] , i );

		// make sure the key is in the two sets
		found = mset.find( key ) != mset.end();
		EXPECT_TRUE( found );
		found = uset.find( key ) != uset.end();
		EXPECT_TRUE( found );
		}
	}

TEST( TypeTests , Test_functors )
	{
	setup_random_seed();

	for( uint pass_index = 0; pass_index < global_number_of_passes; ++pass_index )
		{
		TestSetAndMapWithKey<uuid>();
		TestSetAndMapWithKey<hash>();
		}
	}
