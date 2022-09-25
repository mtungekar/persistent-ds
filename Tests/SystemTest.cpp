
#include "TestPackage/TestEntity.h"

//#include <pds/MemoryWriteStream.h>
//#include <pds/EntityWriter.h>
//#include <pds/EntityWriter.inl>

#include <glm/glm.hpp>

namespace tp = TestPackage;

int main()
	{
	pds::EntityHandler eh;

	if( eh.Initialize( "./TestFolder", { TestPackage::GetPackageRecord() } ) != pds::Status::Ok )
		return -1;

	//pds::MemoryWriteStream ws;
	//pds::EntityWriter wr( ws );
	//
	//auto wsec = wr.BeginWriteSection( "hej", 3 );

	//ctle::idx_vector<glm::vec3> vec;
	//wsec->Write( "hoj", 3, vec );
	
	auto tent = std::make_shared<tp::TestEntity>();
	tent->Name() = "hej";

	eh.AddEntity( tent );


	//bool v;
	//wsec->Write( "hoj", 3, v );
	
	//wr.EndWriteSection( wsec );
	
	return 0;
	}

