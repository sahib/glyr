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
import text_table

LEFT_TOP = (12, 'O/P')


class AsciiTable(object):
    'A cell based approach to drawing ascii tables'

    def __init__(self, pad_char = '?'):
        'pad_char is the char used for unset cells'
        self._rowdesc = set()
        self._columns = [LEFT_TOP]
        self._celltup = []
        self._padchar = pad_char

    def add(self, column, rowdesc, cell):
        'Add the text cell. The position is defined by column and rowdesc'
        column_tuple = (len(column), column)
        if column_tuple not in self._columns:
            self._columns.append((len(column), column))

        self._rowdesc.add(rowdesc)
        self._celltup.append((cell, rowdesc, column))

    def draw(self):
        'Draw previously added cells'
        columnlist = list(self._columns)
        table = text_table.TextTable(*columnlist)
        for rd in list(self._rowdesc):
            row = [rd] + [self._padchar] * (len(columnlist) - 1)
            for cell in self._celltup:
                # Check if the row description fits
                if cell[1] == rd:
                    cell_pos = 0
                    # Now find the corresponding column
                    for col in columnlist:
                        if cell[2] == col[1]:
                            row[cell_pos] = cell[0]
                        cell_pos += 1

            table.row(*row)

        return table.draw()


if __name__ == '__main__':
    at = AsciiTable(pad_char='*')
    at.add('Axxis', 'lyricswiki', '1')
    at.add('Adele', 'lyricswiki', '2')
    at.add('Pussycat Dolls',  'albumart', '3')
    at.add('Adele', 'albumart', '4')
    print(at.draw())
