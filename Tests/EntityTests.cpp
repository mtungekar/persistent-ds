// pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
// Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

#include "Tests.h"

#include <pds/EntityValidator.h>

#include "TestPackA/TestEntityA.h"

TEST( EntityTests , EntityManagementBasicTests )
	{
	// only using TestPack1 in this test
	using TestPackA::TestEntityA;

	TestEntityA ent1;
	TestEntityA ent2;
	EntityValidator validator;

	// set a value, they should differ
	ent1.Name() = random_value<string>();
	EXPECT_FALSE( TestEntityA::MF::Equals( &ent1, &ent2 ) );

	// clear values, they should be the same
	TestEntityA::MF::Clear( ent1 );
	EXPECT_TRUE( TestEntityA::MF::Equals( &ent1, &ent2 ) );

	// set values, validate, should be fine
	ent1.Name() = random_value<string>();
	ent1.OptionalText().set(random_value<string>());
	EXPECT_TRUE( TestEntityA::MF::Validate( ent1 , validator ) );
	EXPECT_EQ( validator.GetErrorCount() , uint(0) );

	// set random values on ent2, should differ
	ent2.Name() = random_value<string>();
	ent2.OptionalText().set(random_value<string>());
	EXPECT_FALSE( TestEntityA::MF::Equals( &ent1, &ent2 ) );
	
	// copy ent1 to ent2, compare, should be the same
	ent2 = ent1;
	EXPECT_TRUE( TestEntityA::MF::Equals( &ent1, &ent2 ) );

	// reset the OptionalText on ent2, copy to ent1
	ent2.OptionalText().reset();
	ent1 = ent2;
	EXPECT_TRUE( TestEntityA::MF::Equals( &ent1, &ent2 ) );

	// clear Name on ent1
	ent1.Name() = {};
	ent2 = ent1;
	EXPECT_TRUE( TestEntityA::MF::Equals( &ent1, &ent2 ) );

	// clear ent2, should be the same
	TestEntityA::MF::Clear( ent2 );
	EXPECT_TRUE( TestEntityA::MF::Equals( &ent1, &ent2 ) );
	}
