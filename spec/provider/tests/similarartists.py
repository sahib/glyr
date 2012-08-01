#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'similarartists',
        'artist': 'The band they called Horse'
        }

TESTCASES = [{
    # {{{
    'name': 'lastfm',
    'data': [{
        'options': {
            'get_type': 'similarartists',
            'artist': 'Equilibrium'
            },
        'expect': len_greater_0
        }, {
        'options': {
            'get_type': 'similarartists',
            'artist': 'Falkenbach',
            'number': 3
            },
        'expect': lambda r: len(r) == 3
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    },
    # }}}
]
