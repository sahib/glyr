#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'similarsongs',
        'artist': 'The band they called Horse',
        'title': 'The album you all know.'
        }

TESTCASES = [{
    # {{{
    'name': 'lastfm',
    'data': [{
        'options': {
            'get_type': 'similarsongs',
            'artist': 'Equilibrium',
            'title': 'Wurzelbert'
            },
        'expect': len_greater_0
        }, {
        'options': {
            'get_type': 'similarsongs',
            'artist': 'In Flames',
            'title': 'Trigger',
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
