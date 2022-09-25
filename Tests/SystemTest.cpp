// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#include "TestPack1/TestEntity.h"
#include "TestPack2/TestEntity.h"
#include "TestPack3/TestEntity.h"

#include <glm/glm.hpp>

#include <pds/pds.inl>

namespace tp1 = TestPack1;
namespace tp2 = TestPack2;
namespace tp3 = TestPack3;

int main()
	{
	pds::EntityHandler eh;

	if( eh.Initialize( "./TestFolder", { TestPack1::GetPackageRecord() , TestPack2::GetPackageRecord() , TestPack3::GetPackageRecord() } ) != pds::Status::Ok )
		return -1;
	
	//auto tent = std::make_shared<tp1::TestEntity>();
	//tent->Name() = "hej";
	//auto e1ref = eh.AddEntity( tent ).first;
	//
	//auto t2ent = std::make_shared<tp2::TestEntity>();
	//t2ent->Name2() = "hej2";
	//auto e2ref = eh.AddEntity( t2ent ).first;

	auto e1ref = pds::entity_ref( hex_string_to_value<hash>( "0fb9f008ebcfb6a4dd569aeb088a3313543f8b3705e77f3510cda50049c158c9" ) );
	auto e2ref = pds::entity_ref( hex_string_to_value<hash>( "55b2138e51229e7b5eb3e6ada3ab41f9684af1a61b64a9e0fd27eead2ab53514" ) );
	
	eh.LoadEntity( e1ref );
	eh.LoadEntity( e2ref );

	auto l1 = eh.GetLoadedEntity( e1ref );
	auto l2 = eh.GetLoadedEntity( e2ref );

	auto te = tp2::TestEntity::MF::EntitySafeCast( l2 );

	auto t3ent = std::make_shared<tp3::TestEntity>();
	t3ent->Name3() = te->Name2();
	t3ent->OptionalText3() = te->OptionalText2();

	auto e3ref = eh.AddEntity( t3ent ).first;

	return 0;
	}

