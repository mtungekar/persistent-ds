// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#pragma once

#include "pds.h"
#include "picosha2.h"
#include <vector>
#include <memory>

namespace pds
	{
	class SHA256
		{
		private:
			//void *MDData = nullptr;
			std::unique_ptr<u8[]> MData;
			picosha2::hash256_one_by_one Hasher;

		public:
			SHA256( const u8 *Data = nullptr , size_t DataLength = 0 );
			~SHA256();

			// update the SHA with the data at Data, length DataLength
			void Update( const u8 *Data, size_t DataLength );

			// get the calculated digest 
			void GetDigest( u8 DestDigest[32] );
		};
	};