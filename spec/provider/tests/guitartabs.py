#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'guitartabs',
        'artist': 'The band they called Horse',
        'title': 'Tabditab'
        }

TESTCASES = [{
    # {{{
    'name': 'guitaretab',
    'data': [{
        'options': {
            'get_type': 'guitartabs',
            'artist': 'Van Halen',
            'title': 'Cathedral'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'chordie',
    'data': [{
        'options': {
            'get_type': 'guitartabs',
            'artist': 'Van Halen',
            'title': 'Cathedral'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    },
    # }}}
]
