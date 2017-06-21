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
#ifndef STRING_TO_CMD_LINE
#define STRING_TO_CMD_LINE
#pragma once

#include <string>
#include <vector>

namespace utilities
{
class stringToCmdLine
{
public:
	stringToCmdLine(const std::string &cmdString);
	void load(const std::string &cmdString);

	int getArgCount() const { return argCount; }
	auto getArgV() { return stringPtrs.data(); }

private:
	std::vector<std::string> stringCap;
	std::vector<char *> stringPtrs;
	int argCount;
	
};
} // namespace utilities
#endif
