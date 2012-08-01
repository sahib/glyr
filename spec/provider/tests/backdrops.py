#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'htbackdrops',
        'artist': 'The band they called Horse'
        }

TESTCASES = [{
    # {{{
    'name': 'htbackdrops',
    'data': [{
        'options': {
            'get_type': 'backdrops',
            'artist': 'Equilibrium'
            },
        'expect': len_greater_0
        }, {
        'options': {
            'get_type': 'backdrops',
            'artist': 'The Rolling Stones'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }
    # }}}
]
