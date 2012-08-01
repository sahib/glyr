#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

TESTCASES = [{
    # {{{
    'name': 'musicbrainz',
    'data': [{
        'options': {
            'get_type': 'tracklist',
            'artist': 'Equilibrium',
            'album': 'Sagas',
            'number': 13
            },
        'expect': lambda r: len(r) == 13
        }, {
        'options':  {
            'get_type': 'tracklist',
            'artist': 'Adele',
            'album': '39'
            },
        'expect': len_equal_0
        }],
    }]
    # }}}
