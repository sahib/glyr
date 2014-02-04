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
        'get_type': 'lyrics',
        'artist': 'The band they called Horse',
        'title': 'Its not here'
        }

TESTCASES = [{
    # {{{
    'name': 'elyrics',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'In Flames',
            'title': 'Deliver us'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lipwalk',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'In Flames',
            'title': 'Trigger'
            },
        'expect': len_greater_0
        }, {
        'options': {
            'get_type': 'lyrics',
            'artist': 'Knorkator',
            'title': 'Absolution'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lyrdb',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Knorkator',
            'title': 'Ma Baker'
            },
        'expect': len_greater_0
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
            'get_type': 'lyrics',
            'artist': 'Annihilator',
            'title': 'Coward'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lyricstime',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Knorkator',
            'title': 'Absolution'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lyricsvip',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Excrementory Grindfuckers',
            'title': 'Das Kuh'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
        # }}}
    # {{{
    'name': 'lyricswiki',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Excrementory Grindfuckers',
            'title': 'Das Kuh'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'lyrix',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Amy Whitehouse',
            'title': 'Back to Black'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
        # }}}
    # {{{
    'name': 'magistrix',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Knorkator',
            'title': 'Absolution'
            },
        'expect': len_greater_0
        }, {
        'options': {
            'get_type': 'lyrics',
            'artist': 'Lady Gaga',
            'title': 'Pokerface'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'metallum',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Equilibrium',
            'title': 'Wurzelbert'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }, {
    # }}}
    # {{{
    'name': 'chartlyrics',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Die Apokalyptischen Reiter',
            'title': 'Es wird schlimmer'
            },
        'expect': len_greater_0
        }, {
        'options': not_found_options,
        'expect': len_equal_0
        }],
    }
    ]
    # }}}
