/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
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

#ifndef GLYR_MISC_HH
#define GLYR_MISC_HH

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * SECTION:misc
     * @short_description: Random utils that may be useful for users of the API for different reasons
     * @title: Misc
     * @section_id:
     * @stability: Stable
     * @include: glyr/misc.h
     *
     * Provides different functions to help with certain tasks
     * may run into in certain cases.
     * This includes:
     * <itemizedlist>
     * <listitem>
     * <para>
     * String utils (e.g. glyr_levenshtein_strcmp())
     * </para>
     * </listitem>
     * </itemizedlist>
     *
     */

    /**
    * glyr_levenshtein_strcmp:
    * @string: first string to compare
    * @other: second string to compare
    *
    * Computes the levenshtein distance betwenn @string and @other.
    * See Also: http://de.wikipedia.org/wiki/Levenshtein-Distanz
    *
    * In very simple words this means: glyr_levenshtein_strcmp() checks if two
    * string are 'similar', the similarity is returned as int from
    * 0 (== total match) to MAX(strlen(string),strlen(other))
    *
    * This is fully UTF-8 aware and calls g_utf8_normalize() beforehand.
    *
    * Example:
    * <note>
    * <para>
    * Equilibrium <=> Aqquilibrim
    * will return 3 since:
    * Equilibrium -> Aquilibrium // one Edit:   'E' -> 'A'
    *
    * Aquilibrium -> Aquilibrim  // one Delete: 'u' -> ''
    *
    * Aquilibrim  -> Aqquilibrim // one Insert: ''  -> 'q'
    *
    * </para>
    * </note>
    *
    * Returns: the levenshtein distance (number of Edits, Deletes and Inserts needed to turn string to other)
    */
    size_t glyr_levenshtein_strcmp (const char * string, const char * other);

    /**
    * glyr_levenshtein_strnormcmp:
    * @string: first string to compare
    * @other: second string to compare
    *
    * Same as levenshtein_strcmp, but tries to normalize the two strings as
    * best as it can (includes strdown, stripping html,
    * utf8 normalization, stripping stuff like (CD 1)
    * and Clapton, Eric -> Eric Clapton
    *
    * For very small strings the function may return very high values in order
    * to prevent accidental matches. See below.
    *
    * internally glyr_levenshtein_strcmp() is used, so this is UTF-8 aware as well.
    *
    * Example:
    * <note>
    * <para>
    * Adios <=> Weiß or 19 <=> 21 return 4 or 2 for glyr_levenshtein_strcmp()
    * (and may pass a max threshold of e.g. 4),
    * but a lot higher value with glyr_levenshtein_strnormcmp()
    * </para>
    * </note>
    *
    * Returns: the levenshtein distance
    */
    size_t glyr_levenshtein_strnormcmp (const char * string, const char * other);

#ifdef __cplusplus
}
#endif

#endif
