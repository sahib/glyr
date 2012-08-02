#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *


TESTCASES = [{
    # {{{
    'name': 'musicbrainz',
    'data': [{
        'options': {
            'get_type': 'albumlist',
            'artist': 'Equilibrium',
            'number': 4
            },
        'expect': lambda r: len(r) == 4
        }, {
        'options':  {
            'get_type': 'tracklist',
            'artist': 'Benele Banga Baracka'
            },
        'expect': len_equal_0
        }],
    }]
    # }}}
