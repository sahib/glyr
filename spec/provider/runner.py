#!/usr/bin/env python
# encoding: utf-8

import os
import sys
import traceback
import executor

TESTS_DIR = 'tests'


def main():
    test_modules = []
    colored_output = False

    for test_file in os.listdir(TESTS_DIR):
        if not test_file.startswith('__'):
            full_path = '.'.join([TESTS_DIR, test_file[:-3]])
            try:
                test_modules.append((full_path, __import__(full_path)))
            except:
                print('Error while loading', full_path, traceback.format_exc())

    if len(sys.argv) > 0 and sys.argv[1] == '--color':
        colored_output = True

    for module in test_modules:
        print('Running:', module[0], ':\n')
        try:
            executor.test_by_dictionary(module[1].cover.TESTCASES, colored_output)
        except:
            print('Error while executing tests:', full_path,
                    traceback.format_exc())
        print('')


if __name__ == '__main__':
    main()
