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

#include "charMappers.h"

charMapper<unsigned char> base64Mapper()
{
	charMapper<unsigned char> b64(0xFF);
	unsigned char val = 0;
	for (unsigned char c = 'A'; c <= 'Z'; ++c)
	{
		b64.addKey(c, val);
		++val;
	}
	for (unsigned char c = 'a'; c <= 'z'; ++c)
	{
		b64.addKey(c, val);
		++val;
	}
	for (unsigned char c = '0'; c <= '9'; ++c)
	{
		b64.addKey(c, val);
		++val;
	}
	b64.addKey('+', val++);
	b64.addKey('/', val);
	return b64;
}

charMapper<unsigned char> digitMapper()
{
	charMapper<unsigned char> dMap(0xFF);
	unsigned char val = 0;
	for (unsigned char c = '0'; c <= '9'; ++c)
	{
		dMap.addKey(c, val);
		++val;
	}
	return dMap;
}

charMapper<unsigned char> hexMapper()
{
	charMapper<unsigned char> dMap(0xFF);
	unsigned char val = 0;
	for (unsigned char c = '0'; c <= '9'; ++c)
	{
		dMap.addKey(c, val);
		++val;
	}
	for (unsigned char c = 'A'; c <= 'F'; ++c)
	{
		dMap.addKey(c, val);
		++val;
	}
	val = 10;
	for (unsigned char c = 'a'; c <= 'f'; ++c)
	{
		dMap.addKey(c, val);
		++val;
	}
	return dMap;
}
