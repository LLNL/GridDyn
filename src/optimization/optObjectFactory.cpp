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

#include "optObjectFactory.h"

namespace griddyn
{
optComponentFactory::optComponentFactory (const std::string &component) : name (component)
{

}

optComponentFactory::~optComponentFactory () = default;

void optComponentFactory::registerFactory (optFactory *optFac)
{
    auto ret = m_factoryMap.insert (std::pair<std::string, optFactory *> (optFac->name, optFac));
    if (ret.second == false)
    {
        ret.first->second = optFac;
    }
    m_factoryList.push_back (optFac);
}


stringVec optComponentFactory::getObjNames ()
{
    stringVec tnames;
    tnames.reserve (m_factoryMap.size ());
    for (auto tname : m_factoryMap)
    {
        tnames.push_back (tname.first);
    }
    return tnames;
}

gridOptObject *optComponentFactory::makeObject ()
{
    return nullptr;
}

bool optComponentFactory::isValidObject (const std::string &objName)
{
    auto mfind = m_factoryMap.find (objName);
    return (mfind != m_factoryMap.end ());
}

optFactory *optComponentFactory::getFactory (const std::string &typeName)
{
    auto mfind = m_factoryMap.find (typeName);
    if (mfind != m_factoryMap.end ())
    {
        return m_factoryMap[typeName];
    }
    return nullptr;
}

gridOptObject *optComponentFactory::makeObject (const std::string &type)
{
    gridOptObject *obj;
    auto mfind = m_factoryMap.find (type);
    if (mfind != m_factoryMap.end ())
    {
        obj = m_factoryMap[type]->makeObject ();
        return obj;
    }
    return nullptr;
}

gridOptObject *optComponentFactory::makeObject (coreObject *obj)
{
    gridOptObject *oo;

    int mxLevel = -1;
    auto ofm = m_factoryList[0];
    for (auto &of:m_factoryList)
    {
        if (of->m_level > mxLevel)
        {
            if (of->testObject (obj))
            {
                ofm = of;
                mxLevel = of->m_level;
            }
        }
    }

    if (mxLevel >= 0)
    {
        oo = ofm->makeObject (obj);
        return oo;
    }
    return nullptr;
}


//create a high level object factory for the coreObject class
std::shared_ptr<coreOptObjectFactory> coreOptObjectFactory::instance ()
{
    static std::shared_ptr<coreOptObjectFactory> factory = std::shared_ptr<coreOptObjectFactory> (new coreOptObjectFactory ());
    return factory;
}

void coreOptObjectFactory::registerFactory (const std::string &name, std::shared_ptr<optComponentFactory> tf)
{
    m_factoryMap[name] = tf;
}

stringVec coreOptObjectFactory::getFactoryNames ()
{
    stringVec factoryNames;
    factoryNames.reserve (m_factoryMap.size ());
    for (auto factoryName : m_factoryMap)
    {
        factoryNames.push_back (factoryName.first);
    }
    return factoryNames;
}

std::vector < std::string> coreOptObjectFactory::getObjNames (const std::string &factoryName)
{
    auto mfind = m_factoryMap.find (factoryName);
    if (mfind != m_factoryMap.end ())
    {
        return m_factoryMap[factoryName]->getObjNames ();
    }
    return {};
}

gridOptObject *coreOptObjectFactory::createObject (const std::string &optType, const std::string &typeName)
{
    gridOptObject *oo;
    auto mfind = m_factoryMap.find (optType);
    if (mfind != m_factoryMap.end ())
    {
        oo = m_factoryMap[optType]->makeObject (typeName);
        return oo;
    }
    return nullptr;
}

gridOptObject *coreOptObjectFactory::createObject (const std::string &optType, coreObject *obj)
{
    gridOptObject *oo;
    auto mfind = m_factoryMap.find (optType);
    if (mfind != m_factoryMap.end ())
    {
        oo = m_factoryMap[optType]->makeObject (obj);
        return oo;
    }
    return nullptr;
}

gridOptObject *coreOptObjectFactory::createObject (coreObject *obj)
{
    if (m_defaultType.empty ())
    {
        return nullptr;
    }
    gridOptObject *oo;
    auto mfind = m_factoryMap.find (m_defaultType);
    if (mfind != m_factoryMap.end ())
    {
        oo = m_factoryMap[m_defaultType]->makeObject (obj);
        return oo;
    }
    return nullptr;
}

gridOptObject *coreOptObjectFactory::createObject (const std::string &typeName)
{
    if (m_defaultType.empty ())
    {
        return nullptr;
    }
    gridOptObject *oo;
    auto mfind = m_factoryMap.find (m_defaultType);
    if (mfind != m_factoryMap.end ())
    {
        oo = m_factoryMap[m_defaultType]->makeObject (typeName);
        return oo;
    }
    return nullptr;
}

std::shared_ptr<optComponentFactory> coreOptObjectFactory::getFactory (const std::string &factoryName)
{
    auto mfind = m_factoryMap.find (factoryName);
    if (mfind != m_factoryMap.end ())
    {
        return (m_factoryMap[factoryName]);
    }
    else       //make a new factory
    {
        auto tf = std::make_shared<optComponentFactory> ();
        tf->name = factoryName;
        m_factoryMap.insert (std::pair<std::string, std::shared_ptr<optComponentFactory>> (factoryName, tf));
        return tf;
    }
}

bool coreOptObjectFactory::isValidType (const std::string &optType)
{
    auto mfind = m_factoryMap.find (optType);
    return (mfind != m_factoryMap.end ());
}

bool coreOptObjectFactory::isValidObject (const std::string &optType, const std::string &objName)
{
    auto mfind = m_factoryMap.find (optType);
    if (mfind != m_factoryMap.end ())
    {
        return mfind->second->isValidObject (objName);
    }
    return false;
}

void coreOptObjectFactory::setDefaultType (const std::string &defType)
{
    auto mfind = m_factoryMap.find (defType);
    if (mfind != m_factoryMap.end ())
    {
        m_defaultType = defType;
    }
}

void coreOptObjectFactory::prepObjects (const std::string &optType, const std::string &typeName, count_t numObjects, coreObject *obj)
{
    auto mfind = m_factoryMap.find (optType);
    if (mfind != m_factoryMap.end ())
    {
        auto obfact = m_factoryMap[optType]->getFactory (typeName);
        if (obfact)
        {
            obfact->prepObjects (numObjects, obj);
        }
    }
}

void coreOptObjectFactory::prepObjects (const std::string &typeName, count_t numObjects, coreObject *obj)
{
    if (m_defaultType.empty ())
    {
        return;
    }

    auto obfact = m_factoryMap[m_defaultType]->getFactory (typeName);
    if (obfact)
    {
        obfact->prepObjects (numObjects, obj);
    }
}

}// namespace griddyn
