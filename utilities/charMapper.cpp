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
	if (pmap == "numericstart")
	{
		key['0'] = 1.0;
		key['1'] = 1.0;
		key['2'] = 1.0;
		key['3'] = 1.0;
		key['4'] = 1.0;
		key['5'] = 1.0;
		key['6'] = 1.0;
		key['7'] = 1.0;
		key['8'] = 1.0;
		key['9'] = 1.0;
		key['+'] = 1.0;
		key['-'] = 1.0;
		key[' '] = 1.0;
		key['\t'] = 1.0;
		key['.'] = 1.0;
	}
	else if (pmap == "numeric")
	{
		key['0'] = 1.0;
		key['1'] = 1.0;
		key['2'] = 1.0;
		key['3'] = 1.0;
		key['4'] = 1.0;
		key['5'] = 1.0;
		key['6'] = 1.0;
		key['7'] = 1.0;
		key['8'] = 1.0;
		key['9'] = 1.0;
		key['+'] = 1.0;
		key['-'] = 1.0;
		key[' '] = 1.0;
		key['e'] = 1.0;
		key['\t'] = 1.0;
		key['.'] = 1.0;
	}
}

int charMapper::getKey(unsigned char x)const
{
	return key[x];
}
