#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'artistphoto',
        'artist': 'HorseTheBand',
        'album':  'Given, but not used.',
        'title': 'Accidentally given'
        }

TESTCASES = [{
    # {{{
    'name': 'discogs',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'Nirvana'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'flickr',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'Die Ärzte'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'google',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'DeVildRiVeR'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lastfm',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'Alestorm'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'singerpictures',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'Equilibrium'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'rhapsody',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'In Flames'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }
    ]
