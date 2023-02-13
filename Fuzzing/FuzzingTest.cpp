#include<iostream>
#include <vector>
#include <string>


#include <pds/pds.h>
#include <pds/pds.inl>
#include <ctle/uuid.h>


void RandomFileDataReadTest(const uint8_t *Data, size_t Size)
{
	pds::MemoryReadStream rs( Data, Size, false );
	pds::EntityReader er( rs );

		// try reading some values. calls are allowed to fail with error, but not crash
		pds::i32 v;
		er.Read( "i", 1, v );
		std::vector<pds::i8> dv;
        std::cout<<"dv\n";
		er.Read( "dv", 2, dv );
		std::string ds;
        std::cout<<"ds\n";
		er.Read( "ds", 2, ds );
			
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  RandomFileDataReadTest(Data, Size);
  return 0;
}
