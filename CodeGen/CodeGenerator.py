# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp

hlp.run_module('EntityWriter')
hlp.run_module('EntityReader')
hlp.run_module('DataTypes')
hlp.run_module('ValueTypes')
hlp.run_module('DynamicTypes')

import Testing 
