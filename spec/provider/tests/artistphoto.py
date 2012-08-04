#################################################################
# This file is part of glyr
# + a commnandline tool and library to download various sort of musicrelated metadata.
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
        'get_type': 'artistphoto',
        'artist': 'HorseTheBand',
        'album':  'Given, but not used.',
        'title': 'Accidentally given'
        }

TESTCASES = [{
    # {{{
    'name': 'bbcmusic',
    'data': [{
        'options': {
            'get_type': 'artistphoto',
            'artist': 'The Rolling Stones'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
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
