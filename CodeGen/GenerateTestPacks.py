# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

from EntitiesHelpers import *

def GenTestPack1():
	items = []

	items.append(
		Entity(
			name = "TestEntity", 
			dependencies = [],
			variables = [ Variable("string", "Name"),
						  Variable("string", "OptionalText", optional = True ) ]
			)
		)

	hlp.run_module('PackageGenerator', Package("TestPack1","../Tests/TestPack1", items ) )

def GenTestPack2():
	items = []

	items.append(
		Entity(
			name = "TestEntity", 
			dependencies = [],
			variables = [ Variable("string", "Name2"),
						  Variable("string", "OptionalText2", optional = True ) ]
			)
		)

	hlp.run_module('PackageGenerator', Package("TestPack2","../Tests/TestPack2", items ) )

def GenTestPack3():
	items = []

	items.append(
		Entity(
			name = "TestEntity", 
			dependencies = [],
			variables = [ Variable("string", "Name3"),
						  Variable("string", "OptionalText3", optional = True ) ]
			)
		)

	hlp.run_module('PackageGenerator', Package("TestPack3","../Tests/TestPack3", items ) )

GenTestPack1()
GenTestPack2()
GenTestPack3()