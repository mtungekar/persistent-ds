# pds - Persistent data structure framework, Copyright (c) 2022 Ulrik Lindahl
# Licensed under the MIT license https://github.com/Cooolrik/pds/blob/main/LICENSE

import CodeGeneratorHelpers as hlp
import argparse

def main():
    parser = argparse.ArgumentParser(description="Code generation.")
    parser.add_argument('--clang_format', action='store_true', help='Format generated code using clang.')
    args = parser.parse_args()
    hlp.run_module(name='EntityWriter', run_clang_format= args.clang_format)
    hlp.run_module(name='EntityReader', run_clang_format=args.clang_format)
    hlp.run_module(name='DataTypes', run_clang_format=args.clang_format)
    hlp.run_module(name='ValueTypes', run_clang_format=args.clang_format)
    hlp.run_module(name='DynamicTypes', run_clang_format=args.clang_format)


if __name__ == "__main__":
    main()