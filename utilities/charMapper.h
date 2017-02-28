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
#pragma once
#ifndef CHARMAPPERS_H_
#define CHARMAPPERS_H_

#include <array>
/** small helper class to map characters to values*/
template<typename V>
class charMapper
{
private:
	std::array<V, 256> key; //!< the character map
public:
	/** default constructor*/
	explicit charMapper(V defVal = V(0))
	{
		key.fill(defVal);
	}
	/** update a the value returned from a key query
	@details this is purposely distinct from the [] operator to make it an error to 
	try to assign something that way
	*/
	void addKey(unsigned char x, V val)
	{
		key[x] = val;
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

charMapper<bool> numericMapper();
charMapper<bool> numericStartMapper();
charMapper<bool> numericEndMapper();
charMapper<unsigned char> base64Mapper();
charMapper<unsigned char> digitMapper();
charMapper<unsigned char> hexMapper();
charMapper<unsigned char> pairMapper();
#endif
