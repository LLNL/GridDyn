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

charMapper::charMapper()
{
	key.fill(0);
}

charMapper::charMapper(const std::string &pmap)
{
	key.fill(0);
	if (pmap == "numericstart") //fill with all the values that would not preclude a string from containing a valid number
	{
		key['0'] = 1;
		key['1'] = 1;
		key['2'] = 1;
		key['3'] = 1;
		key['4'] = 1;
		key['5'] = 1;
		key['6'] = 1;
		key['7'] = 1;
		key['8'] = 1;
		key['9'] = 1;
		key['+'] = 1;
		key['-'] = 1;
		key[' '] = 1;
		key['\t'] = 1;
		key['.'] = 1;
		key['\n'] = 1;
		key['\r'] = 1;
		key['\0'] = 1;
	}
	else if (pmap == "numeric") //load the characters that can be contained in a string of a number
	{
		key['0'] = 1;
		key['1'] = 1;
		key['2'] = 1;
		key['3'] = 1;
		key['4'] = 1;
		key['5'] = 1;
		key['6'] = 1;
		key['7'] = 1;
		key['8'] = 1;
		key['9'] = 1;
		key['+'] = 1;
		key['-'] = 1;
		key[' '] = 1;
		key['e'] = 1;
		key['.'] = 1;
	}
}

int charMapper::getKey(unsigned char x)const
{
	return key[x];
}

int charMapper::operator[](unsigned char x)const
{
	return key[x];
}