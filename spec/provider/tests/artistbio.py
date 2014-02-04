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
    'name': 'echonest',
    'data': [{
        'options': {
            'get_type': 'artistbio',
            'artist': 'Jennifer Lopez',
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
