/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
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

#include "string_viewOps.h"
#include "generic_string_ops.hpp"
#include <stdexcept>
#include <algorithm>

namespace utilities
{
	namespace string_viewOps
	{
		void trimString(string_view &input, string_view trimCharacters)
		{
			input.remove_suffix(input.length() - std::min(input.find_last_not_of(trimCharacters) + 1, input.size()));
			input.remove_prefix(std::min(input.find_first_not_of(trimCharacters), input.size()));
		}

		string_view trim(string_view input, string_view trimCharacters)
		{
			const auto strStart = input.find_first_not_of(trimCharacters);
			if (strStart == std::string::npos)
			{
				return string_view(); // no content
			}

			const auto strEnd = input.find_last_not_of(trimCharacters);

			return input.substr(strStart, strEnd - strStart + 1);
		}

		void trim(string_viewVector &input, string_view trimCharacters)
		{
			for (auto &istr : input)
			{
				istr = trim(istr, trimCharacters);
			}
		}

		string_view getTailString(string_view input, char separatonCharacter)
		{
			auto tc = input.find_last_of(separatonCharacter);
			string_view ret = (tc == string_view::npos) ? input : input.substr(tc + 1);
			return ret;
		}

		string_view getTailString(string_view input, string_view separatonCharacters)
		{
			auto tc = input.find_last_of(separatonCharacters);
			string_view ret = (tc == string_view::npos) ? input : input.substr(tc + 1);
			return ret;
		}

		string_view removeQuotes(string_view str)
		{
			string_view ret = trim(str);
			if (ret.empty())
			{
				return ret;
			}
			if ((ret.front() == '\"') || (ret.front() == '\'') || (ret.front() == '`'))
			{
				if (ret.back() == ret.front())
				{
					return ret.substr(1, ret.size() - 2);
				}
			}
			return ret;
		}

		string_view merge(string_view string1, string_view string2)
		{
			int diff = (string2.data() - string1.data()) + static_cast<int>(string1.length());
			if ((diff >= 0) && (diff < 24)) //maximum of 24 bytes between the strings
			{
				return string_view(string1.data(), diff + string1.length() + string2.length());
			}
			else if (string1.empty())
			{
				return string2;
			}
			else if (string2.empty())
			{
				return string1;
			}
			throw(std::out_of_range("unable to merge string_views"));
		}


		string_viewVector split(string_view str, string_view delims, delimiter_compression compression)
		{
			return generalized_string_split(str, delims, (compression == delimiter_compression::on));

		}

		static const auto pmap = pairMapper();

		string_viewVector splitlineQuotes(string_view line, string_view delimiters, string_view quoteChars, delimiter_compression compression)
		{
			bool compress = (compression == delimiter_compression::on);
			return generalized_section_splitting(line, delimiters, quoteChars, pmap, compress);

		}


		string_viewVector splitlineBracket(string_view line, string_view delimiters, string_view bracketChars, delimiter_compression compression)
		{
			bool compress = (compression == delimiter_compression::on);
			return generalized_section_splitting(line, delimiters, bracketChars, pmap, compress);

		}

		int toIntSimple(string_view input)
		{
			int ret = 0;
			for (auto c : input)
			{
				if (isdigit(c))
				{
					ret = 10 * ret + (c - '0');
				}
			}
			return ret;
		}

		static string_view digits("0123456789");
		int trailingStringInt(string_view input, string_view &output, int defNum)
		{


			if (!isdigit(input.back()))
			{
				output = input;
				return defNum;
			}
			int num = defNum;
			auto pos1 = input.find_last_not_of(digits);
			if (pos1 == string_view::npos) // in case the whole thing is a number
			{
				output = string_view{};
				num = toIntSimple(input);
			}
			else
			{
				if (pos1 == input.length() - 2)
				{
					num = input.back() - '0';
				}
				else
				{
					num = toIntSimple(input.substr(pos1 + 1));
				}

				if ((input[pos1] == '_') || (input[pos1] == '#'))
				{
					output = input.substr(0, pos1);
				}
				else
				{
					output = input.substr(0, pos1 + 1);
				}
			}

			return num;
		}


		int trailingStringInt(string_view input, int defNum)
		{


			if (!isdigit(input.back()))
			{
				return defNum;
			}

			auto pos1 = input.find_last_not_of(digits);
			if (pos1 == string_view::npos)       // in case the whole thing is a number
			{
				return toIntSimple(input);
			}
			else
			{
				if (pos1 == input.length() - 2)
				{
					return input.back() - '0';
				}
				else
				{
					return toIntSimple(input.substr(pos1 + 1));
				}
			}
		}

	}
}