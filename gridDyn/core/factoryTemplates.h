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

#ifndef FACTORY_TEMPLATES_H_
#define FACTORY_TEMPLATES_H_

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <type_traits>

/**create a factory for a specific type of helper component*/
template <class parentClass>
class classFactory;

/** @brief factory for building types of various components that interact with GridDyn
*/
template<class parentClass>
class coreClassFactory
{
	typedef std::map<std::string, classFactory<parentClass> *> fMap;
	std::string m_defaultType;
	public:
	~coreClassFactory()
	{
		
	}

	/** @brief get a shared pointer to the core object factory*/
	static std::shared_ptr<coreClassFactory> instance()
	{
		static std::shared_ptr<coreClassFactory> factory = std::shared_ptr<coreClassFactory>(new coreClassFactory());
		return factory;
	}

	/** @brief register a type factory with the coreObjectFactory
	@param[in] name the string identifier to the factory
	@param[in] tf the type factory to place in the map
	*/
	void registerFactory(const std::string  name, classFactory<parentClass> *tf)
	{
		auto ret = m_factoryMap.emplace(name, tf);
		if (ret.second == false)
		{
			ret.first->second = tf;
		}
	}

	/** @brief register a type factory with the coreObjectFactory
	gets the name to use in the mapping from the type factory itself
	@param[in] tf the type factory to place in the map
	*/
	void registerFactory(classFactory<parentClass> *tf)
	{
		registerFactory(tf->name, tf);
	}

	void setDefault(const std::string &type)
	{
		if (type.empty())
		{
			return;
		}
		auto mfind = m_factoryMap.find(type);
		if (mfind != m_factoryMap.end())
		{
			m_defaultType = type;
		}
	}

	/** @brief get a listing of the factory names*/
	std::vector<std::string> getFactoryNames()
	{
		std::vector<std::string> tnames;
		tnames.reserve(m_factoryMap.size());
		for (auto tname : m_factoryMap)
		{
			tnames.push_back(tname.first);
		}
		return tnames;
	}


	/** @brief create an object from a given objectType and typeName
	@param[in] typeName  the specific type to create
	@return the created coreObject */
	std::shared_ptr<parentClass> createObject(const std::string &typeName)
	{
		auto mfind = m_factoryMap.find(typeName);
		if (mfind != m_factoryMap.end())
		{
			return m_factoryMap[typeName]->makeObject();
		}
		if (!m_defaultType.empty())
		{
			return m_factoryMap[m_defaultType]->makeObject();

		}
		return nullptr;
	}

	/** @brief create an object from the specific type with a name of objName
	@param[in] typeName  the specific type to create
	@param[in] objName  the name of the object to create
	@return the created coreObject */
	std::shared_ptr<parentClass> createObject(const std::string &typeName, const std::string &objName)
	{
		auto mfind = m_factoryMap.find(typeName);
		if (mfind != m_factoryMap.end())
		{
			return m_factoryMap[typeName]->makeObject(objName);
		}
		if (!m_defaultType.empty())
		{
			return m_factoryMap[m_defaultType]->makeObject(objName);

		}
		return nullptr;
	}

	/** @brief get a specific type factory
	@param[in] typeName the name of the typeFactory to get
	@return a shared pointer to a specific type Factory
	*/
	classFactory<parentClass> *getFactory(const std::string &typeName)
	{
		if (typeName.empty())
		{
			return m_factoryMap[m_defaultType];
		}
			auto mfind = m_factoryMap.find(typeName);
			if (mfind != m_factoryMap.end())
			{
				return m_factoryMap[typeName];
			}
			return nullptr;
	}

	/** @brief check if a specific object category is valid*/
	bool isValidObject(const std::string &typeName)
	{
		auto mfind = m_factoryMap.find(typeName);
		return (mfind != m_factoryMap.end());
	}

	

private:
	coreClassFactory()
	{
		
	}

	fMap m_factoryMap;  //!< the main map from string to the classFactory

};

template <class parentClass>
class classFactory
{
public:
	std::string name;

	classFactory(const std::string &keyName) :name(keyName)
	{
		coreClassFactory<parentClass>::instance()->registerFactory(keyName, this);
	}
	classFactory(const std::vector<std::string> &names) :name(names[0])
	{
		auto cfac = coreClassFactory<parentClass>::instance();
		for (auto &nn:names)
		{
			cfac->registerFactory(nn, this);
		}
	}
	classFactory(const std::vector<std::string> &names, const std::string &defType) :name(names[0])
	{
		auto cfac = coreClassFactory<parentClass>::instance();
		for (auto &nn : names)
		{
			cfac->registerFactory(nn, this);
		}
		cfac->setDefault(defType);
	}
	virtual ~classFactory()
	{}
	virtual std::shared_ptr<parentClass> makeObject()
	{
		return std::make_shared<parentClass>();
	}
	virtual std::shared_ptr<parentClass> makeObject(const std::string &newObjectName)
	{
		return std::make_shared<parentClass>(newObjectName);
	}

};

template <class childClass, class parentClass>
class childClassFactory :public classFactory<parentClass> 
{
	static_assert (std::is_base_of<parentClass, childClass>::value, "factory classes must have parent child class relationship");
public:

	childClassFactory(const std::string &keyName) :classFactory<parentClass>(keyName)
	{
		
	}
	childClassFactory(const std::vector<std::string> &names) :classFactory<parentClass>(names)
	{

	}
	childClassFactory(const std::vector<std::string> &names, const std::string &defType) :classFactory<parentClass>(names,defType)
	{
		
	}
	virtual std::shared_ptr<parentClass> makeObject() override
	{
		return std::make_shared<childClass>();
	}
	virtual std::shared_ptr<parentClass> makeObject(const std::string &newObjectName) override
	{
		return std::make_shared<childClass>(newObjectName);
	}

	std::shared_ptr<childClass> makeClassObject()
	{
		return std::make_shared<childClass>();
	}
};

template <class childClass, class parentClass, class argType>
class childClassFactoryArg :public classFactory<parentClass>
{
	static_assert (std::is_base_of<parentClass, childClass>::value, "factory classes must have parent child class relationship");
	static_assert (!std::is_same<argType, std::string>::value, "arg type cannot be a std::string");
private:
	argType argVal;
public:
	

	childClassFactoryArg(const std::string &keyName, argType iArg) :classFactory<parentClass>(keyName), argVal(iArg)
	{

	}
	childClassFactoryArg(const std::vector<std::string> &names, argType iArg) :classFactory<parentClass>(names), argVal(iArg)
	{

	}
	childClassFactoryArg(const std::vector<std::string> &names, const std::string &defType, argType iArg) :classFactory<parentClass>(names, defType), argVal(iArg)
	{

	}
	std::shared_ptr<parentClass> makeObject() override
	{
		return std::make_shared<childClass>(argVal);
	}
	std::shared_ptr<parentClass> makeObject(const std::string &newObjectName) override
	{
		return std::make_shared<childClass>(newObjectName,argVal);
	}
	std::shared_ptr<childClass> makeClassObject()
	{
		return std::make_shared<childClass>(argVal);
	}
};
#endif
