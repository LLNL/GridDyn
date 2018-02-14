/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef CORE_PROPERTIES_H_
#define CORE_PROPERTIES_H_

#include "coreObject.h"
#include "utilities/dataDictionary.h"

/** these objects are intended to capture extra properties about a coreObject that are not in the common definition
such as position information, metadata, etc
*/
template <class PropertyType>
class coreObjectProperty
{
private:
	std::string name_;
	utilities::dataDictionary<id_type_t, PropertyType> dictionary;
public:
	coreObjectProperty(const std::string &name) :name_(name)
	{

	}
	void set(coreObject *obj, PropertyType data)
	{
		dictionary.update(obj->getID(), data);
	}
	PropertyType query(coreObject *obj)
	{
		return dictionary.query(obj->getID());
	}
	void clearProperty(coreObject *obj)
	{
		dictionary.erase(obj->getID());
	}
};
/** @brief loads a position object
*@details I don't know what a grid Position object looks like yet
@param[in] npos a gridPositionObject
*/
//void loadPosition (std::shared_ptr<gridPositionInfo> npos);

#endif