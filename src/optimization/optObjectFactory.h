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

#pragma once

#include "gridOptObjects.h"
#include <map>
#include <vector>
#include <memory>

#include <type_traits>

namespace griddyn
{
// class definitions for the object factories that can create the objects
//cFactory is a virtual base class for object Construction functions
class optFactory
{
public:
	std::string  name;
	int m_level = 0;
    optFactory(const std::string &component, const std::string &objName, int level = 0) : name(objName), m_level(level)
	{

	}
	optFactory(const stringVec &component, const std::string &objName, int level = 0) : name(objName), m_level(level)
	{

	}
	virtual gridOptObject * makeObject(coreObject *obj) = 0;
	virtual gridOptObject * makeObject() = 0;
	virtual void prepObjects(count_t /*count*/, coreObject * /*obj*/)
	{
	}
	virtual count_t remainingPrepped() const
	{
		return 0;
	}
	virtual bool testObject(coreObject *)
	{
		return true;
	}
};

using optMap = std::map<std::string, optFactory *>;

class optComponentFactory
{
public:
	std::string  name;
	optComponentFactory()
	{
	}
	optComponentFactory(const std::string &typeName);
	~optComponentFactory();
	stringVec getObjNames();
	gridOptObject * makeObject(coreObject *obj);
	gridOptObject * makeObject(const std::string &objType);
	gridOptObject * makeObject();
	void registerFactory(optFactory *optFac);
	bool isValidObject(const std::string &objName);
	optFactory * getFactory(const std::string &objName);
protected:
	optMap m_factoryMap;
	std::vector<optFactory *> m_factoryList;
};

//create a high level object factory for the coreObject class
using optfMap = std::map<std::string, std::shared_ptr<optComponentFactory>>;

class coreOptObjectFactory
{
public:
	/** public destructor
	* Destructor must be public to work with shared_ptr
	*/
	~coreOptObjectFactory()
	{
	}
	static std::shared_ptr<coreOptObjectFactory> instance();
	void registerFactory(const std::string &name, std::shared_ptr<optComponentFactory> tf);
	void registerFactory(std::shared_ptr<optComponentFactory> tf);
	stringVec getFactoryNames();
	stringVec getObjNames(const std::string &typeName);
	gridOptObject * createObject(const std::string &optComponet, const std::string &objName);
	gridOptObject * createObject(const std::string &optComponent, coreObject *obj);
	gridOptObject * createObject(coreObject *obj);
	gridOptObject * createObject(const std::string &objName);
	std::shared_ptr<optComponentFactory> getFactory(const std::string &optComponent);
	bool isValidType(const std::string &obComponent);
	bool isValidObject(const std::string &optComponent, const std::string &objName);
	void setDefaultType(const std::string &defComponent);
	void prepObjects(const std::string &optComponent, const std::string &optName, count_t numObjects, coreObject *baseObj);
	void prepObjects(const std::string &objName, count_t numObjects, coreObject *baseObj);
private:
	coreOptObjectFactory()
	{
	}

	optfMap m_factoryMap;
	std::string m_defaultType;

};

/*** template class for opt object ownership*/
template <class Ntype, class gdType>
class gridOptObjectHolder : public coreObject
{
	static_assert (std::is_base_of<gridOptObject, Ntype>::value, "opt class must have a base class of gridOptObject");
	static_assert (std::is_base_of<coreObject, gdType>::value, "gridDyn class must have base class type of coreObject");
private:
	std::vector<Ntype> objArray;
	count_t next = 0;
	count_t objCount = 0;
public:
	gridOptObjectHolder(count_t objs) : objArray(objs), objCount(objs)
	{
		for (auto &so : objArray)
		{
			so.addOwningReference();
		}
	}
	Ntype * getNext()
	{
		Ntype *obj = nullptr;
		if (next < objCount)
		{
			obj = &(objArray[next]);
			++next;
		}
		return obj;
	}

	count_t remaining() const
	{
		return objCount - next;
	}


};



//opt factory is a template class that inherits from cFactory to actually to the construction of a specific object
template <class Ntype, class gdType>
class optObjectFactory : public optFactory
{
	static_assert (std::is_base_of<gridOptObject, Ntype>::value, "opt class must have a base class of gridOptObject");
	static_assert (std::is_base_of<coreObject, gdType>::value, "gridDyn class must have base class type of coreObject");
private:
	bool useBlock = false;
	gridOptObjectHolder<Ntype, gdType> *gOOH = nullptr;
public:
	optObjectFactory(const std::string &component, const std::string &objName, int level = 0, bool makeDefault = false) : optFactory(component, objName, level)
	{

		auto coof = coreOptObjectFactory::instance();
		auto fac = coof->getFactory(component);
		fac->registerFactory(this);
		if (makeDefault)
		{
			coof->setDefaultType(component);
		}

	}

	optObjectFactory(const stringVec &components, const std::string &objName, int level = 0, bool makeDefault = false) : optFactory(components[0], objName, level)
	{
		auto coof = coreOptObjectFactory::instance();
		for (auto &tname : components)
		{
			coof->getFactory(tname)->registerFactory(this);
		}
		if (makeDefault)
		{
			coof->setDefaultType(components[0]);
		}
	}

	bool testObject(coreObject *obj) override
	{
		if (dynamic_cast<gdType *> (obj))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	gridOptObject * makeObject(coreObject *obj) override
	{
		gridOptObject *ret = nullptr;
		if (useBlock)
		{
			ret = gOOH->getNext();
			if (ret == nullptr)
			{                     //means the block was used up
				useBlock = false;
				gOOH = nullptr;
			}
			else
			{
				ret->add(obj);
			}
		}
		if (ret == nullptr)
		{
			ret = new Ntype(obj);
		}
		return ret;
	}

	gridOptObject * makeObject() override
	{
		gridOptObject *ret = nullptr;
		if (useBlock)
		{
			ret = gOOH->getNext();
			if (ret == nullptr)
			{                     //means the block was used up
				useBlock = false;
				gOOH = nullptr;
			}
		}
		if (ret == nullptr)
		{
			ret = new Ntype();
		}
		return ret;
	}

	Ntype * makeTypeObject(coreObject *obj)
	{
		Ntype *ret = nullptr;
		if (useBlock)
		{
			ret = gOOH->getNext();
			if (ret == nullptr)
			{                     //means the block was used up
				useBlock = false;
				gOOH = nullptr;
			}
			else
			{
				ret->add(obj);
			}
		}
		if (ret == nullptr)
		{
			ret = new Ntype(obj);
		}
		return ret;
	}

	virtual void prepObjects(count_t count, coreObject *obj) override
	{
		auto root = obj->getRoot();
		gOOH = new gridOptObjectHolder<Ntype, gdType>(count);
		root->add(gOOH);
		useBlock = true;

	}
	virtual count_t remainingPrepped() const override
	{
		return (gOOH) ? (gOOH->remaining()) : 0;
	}
};

}// namespace griddyn
