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
    }, {
    # }}}
    # {{{
    'name': 'echonest',
    'data': [{
        'options': {
            'get_type': 'albumreview',
            'artist': 'Of Monsters and Men',
            'album': 'My Head is an Animal'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }
    # }}}
]
