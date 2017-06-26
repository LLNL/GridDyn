#pragma once/*
* LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef STRINGVIEWOPS_H_
#define STRINGVIEWOPS_H_

#include "string_viewDef.h"

// disable a funny warning (bug in visual studio 2015)
#ifdef _MSC_VER
#if _MSC_VER >= 1900
#pragma warning(disable : 4592)
#endif
#endif

namespace utilities
{

namespace string_viewOps
{
const string_view whiteSpaceCharacters (" \t\n\r\0\v\f");

/** @brief trim whitespace characters from a string at the beginning and end of the string
@param[in,out] input  the string to convert
@param[in] trimCharacters the characters to potentially tr
*/
void trimString (string_view &input, string_view trimCharacters = whiteSpaceCharacters);

/** @brief trim whitespace characters from a string
@param[in] input the string to trim;
@param[in] trimCharacters  the definition of characters defaults to  \t\n\r\0\v\f
@return the trimmed string
*/
string_view trim (string_view input, string_view trimCharacters = whiteSpaceCharacters);

/** @brief trim the whitespace from a vector of string_views
@param[in] input the vector of strings to trim;
@param[in] trimCharacters  the definition of characters defaults to  \t\n\r\0\v\f
*/
void trim (string_viewVector &input, string_view trimCharacters = whiteSpaceCharacters);

/** @brief remove outer quotes from a string
only quotes around the edges are removed along with whitespace outside the quotes whitespace is also trimmed
@param[in] source  the original string
@return  the string with quotes removed
*/
string_view removeQuotes (string_view str);

/** @brief remove outer brackets from a string
outer brackets include [({<  and their matching pair whitespace is also trimmmed
@param[in] source  the original string
@return  the string with outer brackets removed
*/
string_view removeBrackets(string_view str);

/** @brief get a string that comes after the last of a specified separator
@param[in] input  the string to separate
@param[in] separationCharacter the separation character
@return  the tail string or the string that comes after the last separation character
if not found returns the entire string
*/
string_view getTailString (string_view input, char separationCharacter);
/** @brief get a stringView that comes after the last of a specified separator
@param[in] input  the string to separate
@param[in] separationCharacters the separation characters
@return  the tail string or the string that comes after the last separation character
if not found returns the entire string
*/
string_view getTailString (string_view input, string_view separationCharacters);


const string_view default_delim_chars (",;");
const string_view default_quote_chars (R"raw('"`)raw");
const string_view default_bracket_chars (R"raw([{(<'"`)raw");

enum class delimiter_compression
{
    on,
    off,
};

/** @brief split a line into a vector of stringViews
@param[in] line  the string to split
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
@return a vector of strings separated by the delimiters characters
*/

string_viewVector split (string_view str,
                         string_view delimiters = default_delim_chars,
                         delimiter_compression compression = delimiter_compression::off);
/** @brief split a line into a vector of strings taking into account quote characters
the delimiter characters are allowed inside the brackets and the resulting vector will take the brackets into
account
@param[in] line  the string to split
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
@return a vector of strings separated by the delimiters characters accounting for bracketing characters
*/
string_viewVector splitlineQuotes (string_view line,
                                   string_view delimiters = default_delim_chars,
                                   string_view quoteChars = default_quote_chars,
                                   delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings taking into account bracketing characters
bracket characters include "()","{}","[]","<>" as well as quote characters ' and "
the delimiter characters are allowed inside the brackets and the resulting vector will take the brackets into
account
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters
together
@return a vector of strings separated by the delimiters characters accounting for bracketing characters
*/
string_viewVector splitlineBracket (string_view line,
                                    string_view delimiters = default_delim_chars,
                                    string_view bracketChars = default_bracket_chars,
                                    delimiter_compression compression = delimiter_compression::off);

/** @brief merge two stringViews together
@details this will only work if these are from a single original string
the check is the gap between the start of one string and another is less than 5 characters
@param[in] string1 the first string
@param[in] string2 the second string
@return a new string view of the combined string

*/
string_view merge (string_view string1, string_view string2);

/** @brief extract a trailing number from a string return the number and the string without the number
@param[in] input the string to extract the information from
@param[out]  the leading string with the numbers removed
@param[in]  the default number to return if no trailing number was found
@return the numerical value of the trailing number*/
int trailingStringInt (string_view input, string_view &output, int defNum = -1);

/** @brief extract a trailing number from a string
@param[in] input the string to extract the information from
@param[in]  the default number to return if no trailing number was found
@return the numerical value of the trailing number*/
int trailingStringInt (string_view input, int defNum = -1);
/** convert a stringView to an integer
@details this function does not handle any complexities it ignores all non digit characters
including negative signs and returns a positive integer or 0 basically the number you would get if
all non integer characters were erased*/
int toIntSimple (string_view input);
}
}
#endif
