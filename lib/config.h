/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
 * + Hosted at: https://github.com/sahib/glyr
 *
 * glyr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glyr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glyr. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/

#define GLYR_VERSION_MAJOR "0"
#define GLYR_VERSION_MINOR "9"
#define GLYR_VERSION_MICRO "1"

#define GLYR_VERSION_MAJOR_INT 0
#define GLYR_VERSION_MINOR_INT 9
#define GLYR_VERSION_MICRO_INT 1

/**
 * GLYR_CHECK_VERSION:
 * @X: Macro version.
 * @Y: Mino version.
 * @Z: Micro version,
 *
 * Version to check glyr's version.
 * Example:
 * <informalexample>
 * <programlisting>
 * #if GLYR_CHECK_VERSION(0,8,7)
 *      puts("Version is above 0.87!");
 * #endif
 * </programlisting>
 * </informalexample>
*/
#define GLYR_CHECK_VERSION(X,Y,Z) (X <= GLYR_VERSION_MAJOR_INT && Y <= GLYR_VERSION_MINOR_INT && Z <= GLYR_VERSION_MICRO_INT)

#define GLYR_VERSION_NAME  "Catholic Cat"
#define GLYR_DEBUG         FALSE

/* Message output */
#define GLYR_OUTPUT stderr
