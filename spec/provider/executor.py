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

import plyr
import renderer
import termcolor
import threading
import os


PRINT_CHARS = [('Y', 'green'), ('X', 'red'), ('#', 'yellow')]
THREAD_LIST = []
DRAW_LOCK = threading.Lock()


def _run_query(provider, canvas, options, expect):
    'Do the actual plyr work'
    draw_string = 'X'

    # Only block realted to plyr.
    qry = plyr.Query(**options)
    qry.providers = [provider]
    results = qry.commit()

    # Write results to results/ folder
    ctr = 0
    for item in results:
        item.write(os.path.join('results', '{art}_{alb}_{tit}_{typ}_{prv}_{nbr}'.format(
            art = options.get('artist', ''),
            alb = options.get('album', ''),
            tit = options.get('title', ''),
            typ = options.get('get_type', ''),
            prv = provider,
            nbr = str(ctr)
            )))
        ctr += 1

    if expect(results):
        draw_string = 'Y'

    # Make sure no add() is done in parallel
    DRAW_LOCK.acquire()
    canvas.add(options['artist'], provider, draw_string)
    DRAW_LOCK.release()


def test_by_dictionary(testcases, includes=[], colored=True):
    'Input a dictionary with testcases and print the results (colored?)'
    canvas = renderer.AsciiTable(PRINT_CHARS[2][0])
    THREAD_LIST = []

    # Go through all testcases, and start a thread fro them
    for testcase in testcases:
        if len(includes) == 0 or testcase['name'] in includes:
            for subcase in testcase['data']:
                qry_thread = threading.Thread(target=_run_query,
                        args=(testcase['name'], canvas,
                            subcase['options'], subcase['expect']))

                THREAD_LIST.append(qry_thread)
                qry_thread.start()

    # Wait for all of them to join, and remove them eventually.
    for thread in THREAD_LIST:
        thread.join()

    # Make sure to be reentrant
    THREAD_LIST = []

    # Render the table
    block = canvas.draw()

    # Colorize it maybe
    if colored:
        for char_color in PRINT_CHARS:
            block = block.replace(char_color[0], termcolor.colored(
                char_color[0], color=char_color[1]))

    # Finally, print it.
    print(block)
