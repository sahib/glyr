#!/usr/bin/env python
# encoding: utf-8

import os
import sys
import traceback
import executor

TESTS_DIR = 'tests'


def parse_only_argument(argument):
    """
    Parse the argument of --only into a dictionary
    Example: --only 'albumlist:musicbrainz|cover:google,lastfm'
    """
    allowed_modules = {}

    for include_modules in sys.argv[2].split('|'):
        parseme = include_modules.split(':')
        module_name = parseme[0]
        if len(parseme) > 1:
            allowed_modules[module_name] = [x.strip() for x in parseme[1].split(',')]
        else:
            allowed_modules[module_name] = []

    return allowed_modules


def main():
    """
    Import all testfiles and run some of them.
    """
    test_modules = []
    allowed_modules = {}

    # Create a results dir
    try:
        os.mkdir('results')
    except OSError:
        pass

    # Try to import all files in tests/
    for test_file in os.listdir(TESTS_DIR):
        if not test_file.startswith('__'):
            full_path = '.'.join([TESTS_DIR, test_file[:-3]])
            try:
                test_modules.append((full_path, __import__(full_path, fromlist=['TESTCASES'])))
            except:
                print('Error while loading', full_path, traceback.format_exc())
                sys.exit(-1)

    # Parse --only string, and pass it further
    if len(sys.argv) > 2 and sys.argv[1] == '--only':
        allowed_modules = parse_only_argument(sys.argv[2])

    # Now run through them, and execute all specified ones.
    for module in test_modules:
        module_name = module[0].split('.')[-1]
        if len(allowed_modules) == 0 or module_name in allowed_modules:
            print('Running:', module[0], ':\n')
            try:
                executor.test_by_dictionary(module[1].TESTCASES,
                        includes=allowed_modules.get(module_name) or [],
                        colored=True)
            except:
                print('Error while executing tests:', full_path,
                        traceback.format_exc())
                sys.exit(-2)
            print('')

if __name__ == '__main__':
    main()
