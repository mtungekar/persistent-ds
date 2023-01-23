// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE
#pragma once

#include "SHA256.h"

using namespace pds;

SHA256::SHA256( const u8 *Data, size_t DataLength )
	{
	this->MData = std::make_unique<u8[]>(picosha2::k_digest_size);
	std::memset(MData.get(), 0,picosha2::k_digest_size );
	if( Data != nullptr )
		{
		this->Update( Data, DataLength );
		}
	}

SHA256::~SHA256()
	{
	}

void SHA256::Update( const u8 *Data, size_t DataLength )
	{
	const u8 *End = &Data[DataLength];
	while( Data < End )
		{
		size_t data_len = End - Data;
		if( data_len > INT_MAX )
			data_len = INT_MAX;
		size_t int_data_len = data_len;

		// update sha hash
		picosha2::hash256(Data,Data + DataLength, MData.get(), MData.get() + picosha2::k_digest_size );
		Data += int_data_len;
		}
	// done
	}

// get the calculated digest 
void SHA256::GetDigest( u8 DestDigest[32] )
	{
	std::memcpy(&DestDigest[0],MData.get(),32);
	}

