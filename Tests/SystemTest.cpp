
//#include <glm/glm.hpp>
//#include <rsg/Mesh.h>
//#include <pds/Varying.h>

#include <pds/pds.h>
#include <pds/MemoryWriteStream.h>
#include <pds/EntityWriter.h>

int main()
	{
	//pds::Mesh mesh;
	
	//auto &colors = mesh.Colors().Insert( "Nej" );

	pds::MemoryWriteStream ws;
	pds::EntityWriter wr( ws );

	auto wsec = wr.BeginWriteSection( "hej", 3 );

	//ctle::idx_vector<glm::vec3> vec;
	//wsec->Write( "hoj", 3, vec );

	bool v;
	wsec->Write( "hoj", 3, v );

	wr.EndWriteSection( wsec );

	return 0;
	}

