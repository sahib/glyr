#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'artistbio',
        'artist': 'The band they called Horse',
        'album': 'The album you all know.'
        }

TESTCASES = [{
    # {{{
    'name': 'lastfm',
    'data': [{
        'options': {
            'get_type': 'artistbio',
            'artist': 'Die Ärzte',
            'language': 'de'
            },
        'expect': lambda r: len(r) == 1 and r[0].size > 35000
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lyricsreg',
    'data': [{
        'options': {
            'get_type': 'artistbio',
            'artist': 'Adele'
            },
        'expect': lambda r: len(r) == 1 and r[0].size > 1000
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'bbcmusic',
    'data': [{
        'options': {
            'get_type': 'artistbio',
            'artist': 'The Rolling Stones'
            },
        'expect': lambda r: len(r) == 1 and r[0].size > 1500
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }
    # }}}
]
