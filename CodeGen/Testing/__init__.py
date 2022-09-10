# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

from EntitiesHelpers import *

Items = []

Items.append(
    Entity(
        name = "TestEntity", 
        dependencies = [],
        variables = [ Variable("string", "Name"),
                      Variable("string", "OptionalText", optional = True ) ]
        )
    )

hlp.run_module('EntityGenerator', "../Include/pds" ,  "../Src" , Items)
