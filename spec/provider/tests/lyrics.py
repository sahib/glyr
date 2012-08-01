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
    'name': 'metrolyrics',
    'data': [{
        'options': {
            'get_type': 'lyrics',
            'artist': 'Die Apokalyptischen Reiter',
            'title': 'Friede sei mir dir'
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
