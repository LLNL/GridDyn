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

#ifndef JSON_ELEMENT_H_
#define JSON_ELEMENT_H_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

class jsonElement
{
public:
	int elementIndex = 0;
	std::string name;
	Json::ArrayIndex arrayIndex = 0;
	jsonElement()
	{
	}
	jsonElement(Json::Value vElement, std::string newName);
	
	jsonElement(const jsonElement &ye);
	jsonElement(jsonElement &&ye);
	jsonElement &operator=(const jsonElement& ye);
	jsonElement &operator=(jsonElement &&ye);
	void clear();
	const Json::Value &getElement() const
	{
		return (arraytype) ? element[arrayIndex] : element;
	}
	Json::ArrayIndex count() const
	{
		return (arraytype) ? element.size() : Json::ArrayIndex(1);
	}
	bool isNull() const
	{
		return (arraytype) ? element[arrayIndex].isNull() : element.isNull();
	}
private:
	Json::Value element = Json::nullValue;
	bool arraytype = false;

};


#endif
