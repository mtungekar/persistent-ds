# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

from EntitiesHelpers import *

items = []

items.append(
    Entity(
        name = "TestEntity", 
        dependencies = [],
        variables = [ Variable("string", "Name"),
                      Variable("string", "OptionalText", optional = True ) ]
        )
    )

testPackage = Package("TestPackage","../Tests/TestPackage", items )

hlp.run_module('PackageGenerator', testPackage)
