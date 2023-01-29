// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#define PDS_IMPLEMENTATION

#include "TestPackA/TestEntityA.h"
#include "TestPackA/TestEntityB.h"

#include <pds/pds.h>

using namespace TestPackA;

// disable warnings in code we cannot control
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4189 )
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#endif

int main()
	{
	pds::EntityHandler eh;

	if( eh.Initialize( "./testfolder", { GetPackageRecord() } ) != pds::Status::Ok )
		return -1;
	
	auto pentA = std::make_shared<TestEntityA>();
	TestEntityA &entA = *pentA;
	auto pentB = std::make_shared<TestEntityB>();
	TestEntityB &entB = *pentB;
	
	entA.Name() = "hej";
	entA.TestVariableA().set();
	entA.TestVariableA().value().Insert( pds::item_ref_zero );
	entA.TestVariableA().value()[pds::item_ref_zero].Name() = "Ullebulle";
	
	//entB.Name() = "hej";
	
	eh.AddEntity( pentA );
	eh.AddEntity( pentB );

	auto e1ref = pds::entity_ref( hex_string_to_value<hash>( "5771e7bb72582619919523b8bc5567a6e17678cdb82f79c2d9e7ce93aa8ddfe6" ) );
	auto e2ref = pds::entity_ref( hex_string_to_value<hash>( "89b1d8e9e5ac248c2e154f4c212d5a8ea9b9d43408a502a5e55339feb32e50f0" ) );
	
	eh.LoadEntity( e1ref );
	eh.LoadEntity( e2ref );
	
	auto l1 = eh.GetLoadedEntity( e1ref );
	auto l2 = eh.GetLoadedEntity( e2ref );
	
	//auto pentA = TestEntityA::MF::EntitySafeCast( l1 );
	//auto pentB = TestEntityB::MF::EntitySafeCast( l2 );
	
	////auto t3ent = std::make_shared<tp3::TestEntity>();
	////t3ent->Name3() = te->Name2();
	////t3ent->OptionalText3() = te->OptionalText2();
	////auto e3ref = eh.AddEntity( t3ent ).first;

	return 0;
	}

