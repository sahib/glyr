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

'''text_table - Simple Eyecandy ASCII Tables

>>> from text_table import TextTable
>>> t = TextTable(5, 10)
>>> t.row('foobarbaz', 'yadayadayada')
>>> print t.draw()

.-------+------------.
| foob- | yadayaday- |
| arbaz | ada        |
'-------+------------'

>>> t = TextTable((5, 'abc'), (10, 'def'))
>>> t.row('foobarbaz', 'yadayadayada')
>>> print t.draw()

.-------+------------.
| abc   | def        |
+-------+------------+
| foob- | yadayaday- |
| arbaz | ada        |
'-------+------------'



Ported from Sebastian Riedel's Text::SimpleTable[1] to Python by Swaroop C H[2].
License is same as that of Text::SimpleTable - Artistic or GPL license.

[1]: http://search.cpan.org/~sri/Text-SimpleTable/lib/Text/SimpleTable.pm
[2]: http://www.swaroopch.info
'''

__version__ = '0.02'

TOP_LEFT        = '.-'
TOP_BORDER      = '-'
TOP_SEPARATOR   = '-+-'
TOP_RIGHT       = '-.'

MIDDLE_LEFT     = '+-'
MIDDLE_BORDER   = '-'
MIDDLE_SEPARATOR= '-+-'
MIDDLE_RIGHT    = '-+'

LEFT_BORDER     = '| '
SEPARATOR       = ' | '
RIGHT_BORDER    = ' |'

BOTTOM_LEFT     = "'-"
BOTTOM_SEPARATOR= '-+-'
BOTTOM_BORDER   = '-'
BOTTOM_RIGHT    = "-'"

WRAP            = '-'


class TextTable:
    '''Text ASCII table drawing.'''

    def __init__(self, *args, **kwargs):
        cache = []
        max = 0
        for arg in args:
            if isinstance(arg, tuple) or isinstance(arg, list):
                assert len(arg) == 2
                width, name = arg
            else:
                assert isinstance(arg, int)
                width = arg
                name = None

            if name is None or len(name) == 0:
                title = []
            else:
                title = self._wrap(name, width)

            col = [width, [], title]

            if max < len(col[2]):
                max = len(col[2])

            cache.append(col)

        for col in cache:
            while len(col[2]) < max:
                col[2].append('')

        self.columns = cache


    def row(self, *args, **kwargs):
        texts = list(args)
        size = len(self.columns) - 1
        if size < 0:
            return
        for i in range(0, size):
            if size <= len(texts):
                break
            texts.append('')

        cache = []
        max = 0
        for i in range(0, size+1):
            text = texts.pop(0)
            column = self.columns[i]
            width = column[0]
            pieces = self._wrap(text, width)
            cache.append(pieces)
            if max < len(pieces):
                max = len(pieces)

        for col in cache:
            while len(col) < max:
                col.append('')

        for i in range(0, size+1):
            column = self.columns[i]
            store = column[1]
            store.extend(cache[i])


    def draw(self):
        if self.columns is None or len(self.columns) == 0:
            return

        rows = len(self.columns[0][1]) - 1
        columns = len(self.columns) - 1
        output = ''

        # Top border
        for j in range(0, columns+1):
            column = self.columns[j]
            width = column[0]
            text = TOP_BORDER * width
            if j == 0 and columns == 0:
                text = TOP_LEFT + text + TOP_RIGHT
            elif j == 0:
                text = TOP_LEFT + text + TOP_SEPARATOR
            elif j == columns:
                text = text + TOP_RIGHT
            else:
                text = text + TOP_SEPARATOR
            output += text
        output += '\n'

        title = 0
        for column in self.columns:
            if title < len(column[2]):
                title = len(column[2])

        if title:
            for i in range(0, title):
                for j in range(0, columns+1):
                    column = self.columns[j]
                    width = column[0]
                    text = column[2][i] or ''
                    format = '%%-%ds' % width       # "%-${width}s"
                    text = format % text
                    if j == 0 and columns == 0:
                        text = LEFT_BORDER + text + RIGHT_BORDER
                    elif j == 0:
                        text = LEFT_BORDER + text + SEPARATOR
                    elif j == columns:
                        text = text + RIGHT_BORDER
                    else:
                        text = text + SEPARATOR
                    output += text
                output += '\n'

            # Title separator
            for j in range(0, columns+1):
                column = self.columns[j]
                width = column[0]
                text = MIDDLE_BORDER * width
                if j == 0 and columns == 0:
                    text = MIDDLE_LEFT + text + MIDDLE_RIGHT
                elif j == 0:
                    text = MIDDLE_LEFT + text + MIDDLE_SEPARATOR
                elif j == columns:
                    text = text + MIDDLE_RIGHT
                else:
                    text = text + MIDDLE_SEPARATOR
                output += text
            output += '\n'

        # Rows
        for i in range(0, rows+1):
            for j in range(0, columns+1):
                column = self.columns[j]
                width = column[0]
                text = column[1][i] or ''
                format = '%%-%ss' % width       # "%-${width}s"
                text = format % text
                if j == 0 and columns == 0:
                    text = LEFT_BORDER + text + RIGHT_BORDER
                elif j == 0:
                    text = LEFT_BORDER + text + SEPARATOR
                elif j == columns:
                    text = text + RIGHT_BORDER
                else:
                    text = text + SEPARATOR
                output += text
            output += '\n'

        # Bottom border
        for j in range(0, columns+1):
            column = self.columns[j]
            width = column[0]
            text = BOTTOM_BORDER * width
            if j == 0 and columns == 0:
                text = BOTTOM_LEFT + text + BOTTOM_RIGHT
            elif j == 0:
                text = BOTTOM_LEFT + text + BOTTOM_SEPARATOR
            elif j == columns:
                text = text + BOTTOM_RIGHT
            else:
                text = text + BOTTOM_SEPARATOR
            output += text
        output += '\n'

        return output


    def _wrap(self, text, width):
        cache = []
        parts = text.split('\n')
        for part in parts:
            while len(part) > width:
                subtext = part[:width-len(WRAP)]
                part = part[width-len(WRAP):]
                cache.append(subtext + WRAP)
            if part:
                cache.append(part)

        return cache


if __name__ == '__main__':
    # Example 1
    # t = TextTable(5, 10)
    # t.row('foobarbaz', 'yadayadayada')
    # print t.draw()

    # Example 2
    t = TextTable((5, 'abc'), (10, 'def'))
    t.row('foobarbaz', 'yadayadayada')
    print(t.draw())


