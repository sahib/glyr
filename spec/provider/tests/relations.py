#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'relations',
        'artist': 'The band they called Horse'
        }

TESTCASES = [{
    # {{{
    'name': 'musicbrainz',
    'data': [{
        'options': {
            'get_type': 'relations',
            'artist': 'Metallica'
            },
        'expect': len_greater_0
        }, {
        'options': {
            'get_type': 'relations',
            'artist': 'Metallica',
            'album': 'Some kind of Monster',
            'number': 2
            },
        'expect': lambda r: len(r) == 3
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    },
    # }}}
]
