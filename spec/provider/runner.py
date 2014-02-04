#################################################################
# This file is part of glyr
# + a command-line tool and library to download various sort of music related metadata.
# + Copyright (C) [2011-2012]  [Christopher Pahl]
# + Hosted at: https://github.com/sahib/glyr
#
# glyr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# glyr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with glyr. If not, see <http://www.gnu.org/licenses/>.
#################################################################
#!/usr/bin/env python
# encoding: utf-8

import os
import sys
import traceback
import executor

TESTS_DIR = 'tests'
USAGE = '''
Usage: runner.py [--help|-h] [--only|-o]

--help | -h   Print this text and exit.
--only | -o   Restrict test to certain providers and getters.

              Example:

                --only 'cover'
                --only 'cover|albumlist'
                --only 'cover:google'
                --only 'cover:google,lastfm'

With no arguments all tests are run.
'''


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

    if '--help' in sys.argv or '-h' in sys.argv:
        print(USAGE)
        sys.exit(0)

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
    if len(sys.argv) > 2 and ('--only' in sys.argv or '-o' in sys.argv):
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
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted.')
