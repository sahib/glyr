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
