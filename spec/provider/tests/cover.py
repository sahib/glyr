#################################################################
# This file is part of glyr
# + a command-line tool and library to download various sort of music related metadata.
# + Copyright (C) [2011-2012]  [Christopher Pahl]
# + Hosted at: https://github.com/sahib/glyr
#
# glyr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# glyr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with glyr. If not, see <http://www.gnu.org/licenses/>.
#################################################################
#!/usr/bin/env python
# encoding: utf-8

from tests.__common__ import *

not_found_options = {
        'get_type': 'cover',
        'artist': 'Adele',
        'album':  '39',
        'title': 'Accidentally given'
        }


TESTCASES = [{
    # {{{
    'name': 'lyricswiki',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Axxis',
            'album': 'Access All Areas',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'albumart',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Pussycat Dolls',
            'album': 'Dolls Domination',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'discogs',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Nirvana',
            'album': 'Smells like Nirvana',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'lastfm',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Alestorm',
            'album': 'Black Sails at Midgnight',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'google',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Amon Amarth',
            'album': 'Fate of Norns',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'picsearch',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Amon Amarth',
            'album': 'Fate of Norns',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'rhapsody',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'knorkator',
            'album': 'hasenchartbreaker',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'jamendo',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Diablo Swing Orchestra',
            'album': 'The Butcher\'s Ballroom',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'musicbrainz',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Akrea',
            'album': 'Lebenslinie',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }, {
    # }}}
    # {{{
    'name': 'musicbrainz',
    'data': [{
        'options': {
            'get_type': 'cover',
            'artist': 'Akrea',
            'album': 'Lebenslinie',
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }]
    }
    # }}}
    ]
