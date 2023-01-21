// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE
#pragma once

#include "SHA256.h"

// directly include librock_sha256.c from the Dependencies folder
// silence warning we can't control
#pragma warning( push )
#pragma warning( disable : 4456 )
#include <librock_sha256.c>
#pragma warning( pop )

using namespace pds;

SHA256::SHA256( const u8 *Data, size_t DataLength )
	{
	this->MDData = malloc( librock_SHA256_Init( 0 ) );
	librock_SHA256_Init( (librock_SHA256_CTX *)this->MDData );
	if( Data != nullptr )
		{
		this->Update( Data, DataLength );
		}
	}

SHA256::~SHA256()
	{
	free( this->MDData );
	}

void SHA256::Update( const u8 *Data, size_t DataLength )
	{
	const u8 *End = &Data[DataLength];

	// run until end, in blocks of INT_MAX
	while( Data < End )
		{
		size_t data_len = End - Data;
		if( data_len > INT_MAX )
			data_len = INT_MAX;
		int int_data_len = (int)data_len;

		// update sha hash
		librock_SHA256_Update( (librock_SHA256_CTX *)this->MDData, Data, int_data_len );

		Data += int_data_len;
		}

	// done
	}

// get the calculated digest 
void SHA256::GetDigest( u8 DestDigest[32] )
	{
	librock_SHA256_StoreFinal( DestDigest, (librock_SHA256_CTX *)this->MDData );
	}

