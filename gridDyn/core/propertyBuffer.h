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

#ifndef PROPERTY_BUFFER_H_
#define PROPERTY_BUFFER_H_

#include <string>
#include <vector>
#include <utility>
/** class for temporarily holding object properties if the object has delayed initialization or something to that effect*/
class propertyBuffer
{
private:
	std::vector<std::pair<std::string, std::string>> stringProperties;
	std::vector<std::pair<std::string, double>> doubleProperties;
	std::vector<std::pair<std::string, int>> intProperties;
	std::vector<std::pair<std::string, bool>> flagProperties;
public:
	propertyBuffer();
	propertyBuffer(const propertyBuffer &buf);
	propertyBuffer(propertyBuffer &&buf);
	propertyBuffer &operator=(const propertyBuffer &buf);
	propertyBuffer &operator=(propertyBuffer &&buf);

	void set(const std::string &param, const std::string &val);
	void set(const std::string &param, double val);
	void set(const std::string &param, int val);
	void setFlag(const std::string &param, bool val=true);

	template <class X>
	void apply(X *obj)
	{
		for (auto &sprop : stringProperties)
		{
			obj->set(sprop.first, sprop.second);
		}
		for (auto &sprop : doubleProperties)
		{
			obj->set(sprop.first, sprop.second);
		}
		for (auto &sprop : intProperties)
		{
			obj->set(sprop.first, sprop.second);
		}
		for (auto &sprop : flagProperties)
		{
			obj->setFlag(sprop.first, sprop.second);
		}
	}

	void clear();

};


#endif