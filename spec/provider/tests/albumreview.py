#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'albumreview',
        'artist': 'The band they called Horse',
        'album': 'The album you all know.'
        }

TESTCASES = [{
    # {{{
    'name': 'metallum',
    'data': [{
        'options': {
            'get_type': 'albumreview',
            'artist': 'Die Apokalyptischen Reiter',
            'album': 'Riders on the Storm'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }
    # }}}
]
