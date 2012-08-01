#!/usr/bin/env python
# encoding: utf-8

import plyr
import renderer
import termcolor
import threading

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

    if expect(results):
        draw_string = 'Y'

    # Make sure no add() is done in parallel
    DRAW_LOCK.acquire()
    canvas.add(options['artist'], provider, draw_string)
    DRAW_LOCK.release()


def test_by_dictionary(testcases, colored=True):
    'Input a dictionary with testcases and print the results (colored?)'
    canvas = renderer.AsciiTable(PRINT_CHARS[2][0])
    THREAD_LIST = []
    for testcase in testcases:
        for subcase in testcase['data']:
            qry_thread = threading.Thread(target=_run_query,
                    args=(testcase['name'], canvas,
                          subcase['options'], subcase['expect']))

            THREAD_LIST.append(qry_thread)
            qry_thread.start()

    for thread in THREAD_LIST:
        thread.join()
        THREAD_LIST.remove(thread)

    block = canvas.draw()

    if colored:
        for char_color in PRINT_CHARS:
            block = block.replace(char_color[0], termcolor.colored(
                char_color[0], color=char_color[1]))

    print(block)
