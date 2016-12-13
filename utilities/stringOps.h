/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef STRINGOPS_H_
#define STRINGOPS_H_


#include <string>
#include <vector>
#include <stdexcept>
#include <array>
//!< typedef for convenience
typedef std::vector<std::string> stringVector;

/** @brief convert a string to lower case as a new string
@param[in] input  the string to convert
@return the string with all upper case converted to lower case
*/
std::string convertToLowerCase (const std::string &input);
/** @brief convert a string to upper case as a new string
@param[in] input  the string to convert
@return the string with all lower case letters converted to upper case
*/
std::string convertToUpperCase (const std::string &input);
/** @brief make a string lower case
@param[in,out] input  the string to convert
*/
void makeLowerCase (std::string &input);

/** @brief make a string upper case
@param[in,out] input  the string to convert
*/
void makeUpperCase (std::string &input);

/** @brief trim whitespace characters from a string at the beginning and end of the string
@param[in,out] input  the string to convert
*/
void trimString(std::string &input);

/** @brief trim whitespace characters from a string
@param[in] input the string to trim;
@param[in] whitespace  the definition of whitespace characters defaults to " \t\n"
@return the trimmed string
*/

std::string trim(const std::string& input, const std::string& whitespace = " \t\n");

/** @brief get a string that comes after the last of a specificed seperator
@param[in] input  the string to separate
@param[in] sep the separation character
@return  the tail string or the string that comes after the last sep character 
if not found returns the entire string
*/
std::string getTailString (const std::string &input, char sep);
/** @brief get a string that comes after the last of a specificed seperator
@param[in] input  the string to separate
@param[in] sep the separation character
@return  the tail string or the string that comes after the last sep character
if not found returns the entire string
*/
std::string getTailString(const std::string &input, const std::string &sep);

enum class delimiter_compression
{
	on,
	off,
};

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters together
@return a vector of strings separated by the delimiters characters
*/
stringVector splitline(const std::string &line, const std::string &delimiters = ",;", delimiter_compression compression=delimiter_compression::off);

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[in] del the delimiter character
@return a vector of strings separated by the delimiters characters
*/
stringVector splitline(const std::string &line, char del);

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[out] strVec vector to place the strings
@param[in] del the delimiter character

*/
void splitline(const std::string &line, stringVector &strVec, char del );

/** @brief split a line into a vector of strings
@param[in] line  the string to spit
@param[out] strVec vector to place the strings
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters together
*/
void splitline(const std::string &line, stringVector &strVec, const std::string &delimiters = ",;", delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings and include a trim operation on the substrings
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters together
@return a vector of strings separated by the delimiters characters
*/
stringVector splitlineTrim(const std::string &line, const std::string &delimiters = ",;", delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings and include a trim operation on the substrings
@param[in] line  the string to spit
@param[in] del the delimiter character
@return a vector of strings separated by the delimiters characters
*/
stringVector splitlineTrim(const std::string &line, char del);

/** @brief split a line into a vector of strings and include a trim operation on the substrings
@param[in] line  the string to spit
@param[out] strVec vector to place the strings
@param[in] del the delimiter character

*/
void splitlineTrim(const std::string &line, stringVector &strVec, char del);

/** @brief split a line into a vector of strings and include a trim operation on the substrings
@param[in] line  the string to spit
@param[out] strVec vector to place the strings
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters together
*/
void splitlineTrim(const std::string &line, stringVector &strVec, const std::string &delimiters = ",;", delimiter_compression compression = delimiter_compression::off);


/** @brief split a line into a vector of strings taking into account bracketing characters
 bracket characters include "()","{}","[]","<>" as well as quote characters ' and "
the delimiter characters are allowed inside the brackets and the resulting vector will take the brackets into account
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters together
@return a vector of strings separated by the delimiters characters accounting for bracketing characters
*/
stringVector splitlineBracket (const std::string &line, const std::string &delimiters = ",;", delimiter_compression compression = delimiter_compression::off);

/** @brief split a line into a vector of strings taking into account bracketing characters and Trim the resulting strings for whitespace
 bracket characters include "()","{}","[]","<>" as well as quote characters ' and "
the delimiter characters are allowed inside the brackets and the resulting vector will take the brackets into account
@param[in] line  the string to spit
@param[in]  delimiters a string containing the valid delimiter characters
@param[in] compression default off,  if set to delimiter_compression::on will merge multiple sequential delimiters together
@return a vector of strings separated by the delimiters characters accounting for bracketing characters
*/
stringVector splitlineBracketTrim(const std::string &line, const std::string &delimiters = ",;", delimiter_compression compression = delimiter_compression::off);


int intRead(const std::string &V, int def = 0);

/** @brief convert a string to an unsigned long
@param[in] V  the string to convert
@param[in] def  the default value to return if the conversion fails
@return the numerical result of the conversion or the def value
*/
unsigned long indexRead(const std::string &V, unsigned long def = (unsigned long)(-1));

/** @brief convert a string to a double
@param[in] V  the string to convert
@param[in] def  the default value to return if the conversion fails
@return the numerical result of the conversion or the def value
*/
double  doubleRead(const std::string &V, double def = 0);
/** @brief extract a trailing number from a string return the number and the string without the number
@param[in] input the string to extract the information from
@param[out]  the leading string with the numbers removed
@param[in]  the default number to return if no trailing number was found
@return the numerical value of the trailing number*/
int trailingStringInt (const std::string &input, std::string &output, int defNum = -1);

/** @brief extract a trailing number from a string
@param[in] input the string to extract the information from
@param[in]  the default number to return if no trailing number was found
@return the numerical value of the trailing number*/
int trailingStringInt (const std::string &input, int defNum = -1);

/** @brief convert a string to an integer making sure the complete string was converted
@param[in] V  the string to convert
@param[in] def  the default value to return if the conversion fails
@return the numerical result of the conversion or the def value
*/
int intReadComplete (const std::string &V, int def = 0);


/** @brief convert a string to a double making sure the complete string was converted
@param[in] V  the string to convert
@param[in] def  the default value to return if the conversion fails
@return the numerical result of the conversion or the def value
*/
double  doubleReadComplete (const std::string &V, double def = 0);


/**@brief enumeration for string close matches
*/
enum string_match_type_t
{
  string_match_close, string_match_begin, string_match_end,string_match_exact
};

/** @brief find a close match in a vector of strings to a test string
 function searches for any of the testStrings in the istrings vec based on the smatch parameter and returns 
the index into the istrings vector
@param[in] testStrings the vector of atrings to search for
@param[in] iString the string library to search through 
@param[in] smatch the matching type
@return the index of the match or -1 if no match is found
*/
int findCloseStringMatch (const stringVector &testStrings, const stringVector &istrings, string_match_type_t smatch = string_match_close);

/** @brief remove a set of characters from a string
@param[in] source  the original string
@param[in] remchars the characters to remove
@return  the string with the specificed character removed
*/
std::string removeChars (const std::string& source, const std::string& remchars);

/** @brief remove a particular character from a string
@param[in] source  the original string
@param[in] remchar the character to remove
@return  the string with the specificed character removed
*/
std::string removeChar(const std::string& source, char remchar);

/** @brief remove quotes from a string
 only quotes around the edges are removed along with whitespace outside the quotes
@param[in] source  the original string
@return  the string with quotes removed
*/
void removeQuotes(std::string &str);

/** @brief replace a particular key character with a different string
@param[in] source  the original string
@param[in] key the character to replace
@param[in]  the string to replace the key with
@return  the string after the specified replacement
*/
std::string characterReplace (const std::string &source, char key, std::string repStr);

/** @brief replace XML character codes with the appropriate character
@param[in] str  the string to do the replacement on
@return the string with the character codes removed and replaced with the appropriate character
*/
std::string xmlCharacterCodeReplace(std::string str);

/** small helper class to map characters to values*/
template<typename V>
class charMapper
{
private:
	std::array<V, 256> key; //!< the character map
public:
	/** default constructor*/
	charMapper(V defVal=V(0))
	{
		key.fill(defVal);
	}
		
	void addKey(unsigned char x, V val)
	{
		key[x] = val;
	}
	/** @brief the main constructor 
	 *@param[in] pmap a string containing a description of the map to use*/
	explicit charMapper(const std::string &pmap)
	{
		key.fill(0);
		if (pmap == "numericstart") //fill with all the values that would not preclude a string from containing a valid number
		{
			key['0'] = V(1);
			key['1'] = V(1);
			key['2'] = V(1);
			key['3'] = V(1);
			key['4'] = V(1);
			key['5'] = V(1);
			key['6'] = V(1);
			key['7'] = V(1);
			key['8'] = V(1);
			key['9'] = V(1);
			key['+'] = V(1);
			key['-'] = V(1);
			key[' '] = V(1);
			key['\t'] = V(1);
			key['.'] = V(1);
			key['\n'] = V(1);
			key['\r'] = V(1);
			key['\0'] = V(1);
		}
		else if (pmap == "numeric") //load the characters that can be contained in a string of a number
		{
			key['0'] = V(1);
			key['1'] = V(1);
			key['2'] = V(1);
			key['3'] = V(1);
			key['4'] = V(1);
			key['5'] = V(1);
			key['6'] = V(1);
			key['7'] = V(1);
			key['8'] = V(1);
			key['9'] = V(1);
			key['+'] = V(1);
			key['-'] = V(1);
			key[' '] = V(1);
			key['e'] = V(1);
			key['.'] = V(1);
		}
	}
	/** get the value assigned to a character
	 * @param[in] x the character to test or convert
	 * @return the resulting value,  0 if nothing in particular is specified in a given map
	 */
	V at(unsigned char x)const
	{
		return key[x];
	}
	/** get the value assigned to a character by bracket notation
	* @param[in] x the character to test or convert
	* @return the resulting value,  0 if nothing in particular is specified in a given map
	*/
	V operator[](unsigned char x) const
	{
		return key[x];
	}

};



#endif
