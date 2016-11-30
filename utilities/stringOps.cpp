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

#include "stringOps.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
//#pragma GCC diagnostic warning "-w"
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#pragma GCC diagnostic pop
#else
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#endif

#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <cctype>

#ifndef TRIM
#define TRIM(X) boost::algorithm::trim (X)
#endif

stringVector splitlineTrim (const std::string &line, const std::string &delimiters,delimiter_compression compression)
{
  stringVector strVec;
  auto comp = (compression==delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
  boost::algorithm::split (strVec, line, boost::is_any_of (delimiters), comp);
  for (auto &str : strVec)
    {
      TRIM (str);
    }
  return strVec;
}

stringVector splitlineTrim(const std::string &line, char del)
{
	stringVector strVec;
	boost::algorithm::split(strVec, line, boost::is_from_range(del, del));
	for (auto &str : strVec)
	{
		TRIM(str);
	}
	return strVec;
}

void splitlineTrim(const std::string &line, stringVector &strVec, const std::string &delimiters, delimiter_compression compression)
{
	auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
	boost::algorithm::split(strVec, line, boost::is_any_of(delimiters), comp);
	for (auto &str : strVec)
	{
		TRIM(str);
	}
}

void splitlineTrim(const std::string &line, stringVector &strVec, char del)
{
	boost::algorithm::split(strVec, line, boost::is_from_range(del, del));
	for (auto &str : strVec)
	{
		TRIM(str);
	}
}

stringVector splitline(const std::string &line, const std::string &delimiters, delimiter_compression compression)
{
	stringVector strVec;
	auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
	boost::algorithm::split(strVec, line, boost::is_any_of(delimiters), comp);
	return strVec;
}

stringVector splitline(const std::string &line, char del)
{
	stringVector strVec;
	boost::algorithm::split(strVec, line, boost::is_from_range(del, del));
	return strVec;
}

void splitline(const std::string &line, stringVector &strVec, const std::string &delimiters, delimiter_compression compression)
{
	auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
	boost::algorithm::split(strVec, line, boost::is_any_of(delimiters), comp);
}

void splitline(const std::string &line, stringVector &strVec, char del)
{
	boost::algorithm::split(strVec, line, boost::is_from_range(del, del));
}


stringVector splitlineBracket (const std::string &line, const std::string &delimiters, delimiter_compression compression)
{
  size_t qloc = line.find_first_of ("\"\'{[(<");
 
  if (qloc == std::string::npos)
    {
      return splitline (line, delimiters,compression);
    }
  stringVector strVec;
  qloc = line.find_first_of ("\"\'");
  if (qloc != std::string::npos)
    {
      stringVector astring;
	  auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
      boost::algorithm::split (astring, line, boost::is_any_of (delimiters), comp);
      bool inquote = false;
      std::string tstring;
      char qtype = line[qloc];
      for (auto &str : astring)
        {
          if (inquote)
            {
              qloc = str.find_first_of (qtype);
              if (qloc != std::string::npos)
                {
					tstring.push_back(delimiters[0]);
					tstring.push_back(' ');
					tstring += str;
                  qloc = str.find_first_of ("\"\'", qloc + 1);
                  if (qloc == std::string::npos)
                    {
                      TRIM (tstring);
                      strVec.push_back (tstring);
                      inquote = false;
                    }
                  else
                    {
                      qtype = str[qloc];
                      qloc = str.find_first_of (qtype, qloc + 1);
                      if (qloc != std::string::npos)
                        {
                          TRIM (tstring);
                          strVec.push_back (str);
                          inquote = false;
                        }
                      else                 //we still didn't terminate the string
                        {

                        }
                    }
                }
            }
          else
            {
              qloc = str.find_first_of ("\"\'");
              if (qloc == std::string::npos)
                {
                  TRIM (str);
                  strVec.push_back (str);
                }
              else
                {
                  qtype = str[qloc];
                  qloc = str.find_first_of (qtype, qloc + 1);
                  if (qloc != std::string::npos)
                    {
                      TRIM (str);
                      strVec.push_back (str);
                    }
                  else               //we didn't terminate the string
                    {
                      tstring = str;
                      inquote = true;
                    }
                }
            }

        }
      //now remove the quotes if need be
      for (auto &str : strVec)
        {
          if ((str[0] == '"') && (str[str.size () - 1] == '"'))
            {
              str = str.substr (1, str.size () - 2);
            }
          else if ((str[0] == '\'') && (str[str.size () - 1] == '\''))
            {
              str = str.substr (1, str.size () - 2);
            }
        }
    }
  else
    {
		auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
      boost::algorithm::split (strVec, line, boost::is_any_of (delimiters), comp);
    }


  //now look for the brackets
  bool inbracket = false;
  std::string tstring;
  stringVector bstrings;
  stringVector tpairs = {"()","{}","[]","<>"};
  //test for matching parenthesis
  for (auto &tp : tpairs)
    {
      qloc = line.find_first_of (tp[0]);
      if (qloc != std::string::npos)
        {
          for (auto &str : strVec)
            {
              if (inbracket)
                {
				  tstring.push_back(delimiters[0]);
				  tstring.push_back(' ');
				  tstring += str;
                }
              else
                {
                  tstring = str;
                }
              auto ob = std::count (str.begin (), str.end (), tp[0]);
              auto cb = std::count (str.begin (), str.end (), tp[1]);
              if (ob > cb)
                {
                  inbracket = true;
                }
              else
                {
                  inbracket = false;
                  bstrings.push_back (tstring);
                }
            }
          if (inbracket)
            {
              bstrings.push_back (tstring);
            }
          strVec = bstrings;
        }
    }
  return strVec;

}

stringVector splitlineBracketTrim(const std::string &line, const std::string &delimiters, delimiter_compression compression)
{
	auto strVec = splitlineBracket(line, delimiters,compression);
	for (auto &str : strVec)
	{
		TRIM(str);
	}
	return strVec;

}

void trimString(std::string &input)
{
	TRIM(input);
}

std::string trim(const std::string& input, const std::string& whitespace)
{
	const auto strStart = input.find_first_not_of(whitespace);
	if (strStart == std::string::npos)
	{
		return ""; // no content
	}

	const auto strEnd = input.find_last_not_of(whitespace);

	return input.substr(strStart, strEnd - strStart + 1);
}

std::string convertToLowerCase (const std::string &input)
{
  std::string out (input);
  std::transform (input.begin (), input.end (), out.begin (), ::tolower);
  return out;
}

std::string convertToUpperCase (const std::string &input)
{
  std::string out (input);
  std::transform (input.begin (), input.end (), out.begin (), ::toupper);
  return out;
}

void makeLowerCase (std::string &input)
{
  std::transform (input.begin (), input.end (), input.begin (), ::tolower);
}

void makeUpperCase (std::string &input)
{
  std::transform (input.begin (), input.end (), input.begin (), ::toupper);
}


int trailingStringInt (const std::string &input, std::string &output,int defNum)
{
  

  if (!isdigit (input.back ()))
    {
      output = input;
      return defNum;
    }
  int num = defNum;
  auto pos1 = input.find_last_not_of ("0123456789");
  if (pos1 == std::string::npos) // in case the whole thing is a number
    {
      output.clear();
      num = std::stol (input);
    }
  else
    {
      if (pos1 == input.length () - 2)
        {
          num = input.back () - '0';
        }
      else
        {
          num = std::stol (input.substr (pos1 + 1));
        }

      if ((input[pos1] == '_') || (input[pos1] == '#'))
        {
          output = input.substr (0, pos1);
        }
      else
        {
          output = input.substr (0, pos1 + 1);
        }
    }

  return num;
}


int trailingStringInt (const std::string &input, int defNum)
{
 

  if (!isdigit (input.back ()))
    {
      return defNum;
    }

  auto pos1 = input.find_last_not_of ("0123456789");
  if (pos1 == std::string::npos)       // in case the whole thing is a number
    {
      return std::stol (input);
    }
  else
    {
      if (pos1 == input.length () - 2)
        {
          return input.back () - '0';
        }
      else
        {
          return std::stol (input.substr (pos1 + 1));
        }
    }
}




static const charMapper<int> numCheck("numericstart");

int intRead(const std::string &V, int def)
{
	if ((V.empty()) || (numCheck[V[0]]==0))
	{
		return def;
	}
	try
	{
		return std::stoi(V);
	}
	catch (std::invalid_argument)
	{
		return def;
	}
}

unsigned long indexRead(const std::string &V, unsigned long def)
{
	if ((V.empty()) || (numCheck[V[0]] == 0))
	{
		return def;
	}
	try
	{
		return std::stoul(V);
	}
	catch (std::invalid_argument)
	{
		return def;
	}
}

double  doubleRead(const std::string &V, double def)
{
	if ((V.empty()) || (numCheck[V[0]] == 0))
	{
		return def;
	}
	try
	{
		return std::stod(V);
	}
	catch (std::invalid_argument)
	{
		return def;
	}
}

int intReadComplete(const std::string &V, int def)
{
	
	if ((V.empty()) || (numCheck[V[0]] == 0))
	{
		return def;
	}
	try
	{
		size_t rem;
		int res = std::stoi(V,&rem);
		while (rem < V.length())
		{
			if (!(isspace(V[rem])))
			{
				res = def;
				break;
			}
			++rem;
		}
		return res;
	}
	catch (std::invalid_argument)
	{
		return def;
	}
}


double  doubleReadComplete(const std::string &V, double def)
{
	if ((V.empty()) || (numCheck[V[0]] == 0))
	{
		return def;
	}
	try
	{
		size_t rem;
		double res=std::stod(V,&rem);
		while (rem < V.length())
		{
			if (!(isspace(V[rem])))
			{
				res = def;
				break;
			}
			++rem;
		}
		return res;
		
	}
	catch (std::invalid_argument)
	{
		return def;
	}
}

long long  longlongReadComplete(const std::string &V, long long def)
{
	if ((V.empty()) || (numCheck[V[0]] == 0))
	{
		return def;
	}
	try
	{
		size_t rem;
		long long res = std::stoll(V, &rem);
		while (rem < V.length())
		{
			if (!(isspace(V[rem])))
			{
				res = def;
				break;
			}
			++rem;
		}
		return res;

	}
	catch (std::invalid_argument)
	{
		return def;
	}
}

void removeQuotes(std::string &str)
{
	trimString(str);
	auto q1 = str.find_first_of("" "\'");
	if (q1 == 0)
	{
		auto q2 = str.find_last_of("" "\'");
		if (q2 != q1)
		{
			str = str.substr(q1 + 1, q2 - 1);
			removeQuotes(str);
		}
	}
}

std::string getTailString(const std::string &input, char sep)
{
	auto tc = input.find_last_of(sep);
	std::string ret = (tc == std::string::npos) ? input : input.substr(tc + 1);
	return ret;
}

int findCloseStringMatch(const stringVector &testStrings, const stringVector &iStrings, string_match_type_t smatch)
{
	std::string lct; //lower case test string
	std::string lcis;//lower case input string
	stringVector lciStrings = iStrings;
	//make all the input strings lower case
	for (auto &st : lciStrings)
	{
		makeLowerCase(st);
	}
	for (auto &ts : testStrings)
	{
		lct = convertToLowerCase(ts);
		for (int kk = 0; kk < static_cast<int>(lciStrings.size()); ++kk)
		{
			lcis = lciStrings[kk];
			switch (smatch)
			{
			case string_match_exact:
				if (lcis == lct)
				{
					return kk;
				}
				break;
			case string_match_begin:
				if (lct.compare(0, lct.length(), lcis) == 0)
				{
					return kk;
				}
				break;
			case string_match_end:
				if (lct.length() > lcis.length())
				{
					continue;
				}
				if (lcis.compare(lcis.length()-lct.length(), lct.length(), lct) == 0)
				{
					return kk;
				}
				break;
			case string_match_close:
				if (lct.length() == 1)//special case
				{ //we are checking if the single character is isolated from other other alphanumeric characters
					auto bf = lcis.find(lct);
					while (bf != std::string::npos)
					{
						if (bf == 0)
						{
							if ((isspace(lcis[bf+1]))||(ispunct(lcis[bf + 1])))
							{
								return kk;
							}
						}
						else if (bf == lcis.length() - 1)
						{
							if ((isspace(lcis[bf - 1])) || (ispunct(lcis[bf - 1])))
							{
								return kk;
							}
						}
						else
						{
							if ((isspace(lcis[bf - 1])) || (ispunct(lcis[bf - 1])))
							{
								if ((isspace(lcis[bf + 1])) || (ispunct(lcis[bf + 1])))
								{
									return kk;
								}
							}
						}
						bf = lcis.find(lct,bf+1);
					}
				}
				else
				{
					auto bf = lcis.find(lct);
					if (bf != std::string::npos)
					{
						return kk;
					}
					auto nstr = removeChar(lcis, '_');
					bf = lcis.find(nstr);
					if (bf != std::string::npos)
					{
						return kk;
					}
				}
				break;

			}
		}
	}
	return -1;
}

std::string removeChars(const std::string& source, const std::string& remchars) {
	std::string result = "";
	result.reserve(source.length());
	for (auto sc:source) {
		bool foundany = false;
		for (auto tc : remchars)
		{
			if (sc == tc)
			{
				foundany = true;
				break;
			}
		}
		if (!foundany) {
			result.push_back(sc);
		}
	}
	return result;
}


std::string removeChar(const std::string& source, char remchar) {
	std::string result;
	result.reserve(source.length());
	for (auto sc : source)
	{
		if (sc != remchar)
		{
			result.push_back(sc);
		}
	}
	return result;
}

std::string characterReplace(const std::string &source, char key, std::string repStr)
{
	std::string result = "";
	result.reserve(source.length());
	for (auto sc : source) {
		if (sc == key)
		{
			for (auto rc : repStr)
			{
				result.push_back(rc);
			}
		}
		else
		{
			result.push_back(sc);
		}
	}
	return result;
}
std::string xmlCharacterCodeReplace(std::string str)
{
	std::string out = str;
	auto tt = out.find("&gt;");
	while (tt != std::string::npos)
	{
		out.replace(tt, 4, ">");
		tt = out.find("&gt;");
	}
	tt = out.find("&lt;");
	while (tt != std::string::npos)
	{
		out.replace(tt, 4, "<");
		tt = out.find("&lt;");
	}
	tt = out.find("&amp;");
	while (tt != std::string::npos)
	{
		out.replace(tt, 5, "&");
		tt =out.find("&amp;");
	}
	return out;
}