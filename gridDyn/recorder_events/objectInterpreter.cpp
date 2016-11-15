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
#include "objectInterpreter.h"



objInfo::objInfo ()
{
}

objInfo::objInfo (const std::string &Istring, gridCoreObject *obj)
{
  LoadInfo (Istring, obj);
}


void objInfo::LoadInfo (const std::string &Istring, gridCoreObject *obj)
{
  //get the object which to grab from
  size_t rlc = Istring.find_last_of (":?");
  if (rlc != std::string::npos)
    {
      m_obj = locateObject (Istring.substr(0, rlc), obj);

      m_field = convertToLowerCase(Istring.substr (rlc + 2, std::string::npos));
    }
  else
    {
      m_obj = obj;
      m_field = convertToLowerCase(Istring);
    }

  rlc = m_field.find_first_of ('(');
  if (rlc != std::string::npos)
    {
      size_t rlc2 = m_field.find_last_of (')');
      m_unitType = gridUnits::getUnits (m_field.substr (rlc + 1, rlc2 - rlc - 1));
      m_field = convertToLowerCase(m_field.substr (0, rlc));
    }
}


gridCoreObject* locateObject (std::string Istring, const gridCoreObject *rootObj, bool rootSearch)
{
  gridCoreObject *obj = nullptr;
  std::string mname = Istring;
  std::string secName = "_";
  //get the object which to grab from
  size_t rlc = Istring.find_first_of (":/?");
  char sep = ' ';
  if (rlc != std::string::npos)
    {
      mname = Istring.substr (0, rlc);
      secName = Istring.substr (rlc + 1);
      sep = Istring[rlc];
    }

  if (mname == rootObj->getName ())
    {
      obj = const_cast<gridCoreObject *> (rootObj);
    }
  else if ((mname[0] == '@')||(mname[0] == '/')) //implies searching the parent object as well
    {
      mname.erase (0);
      obj = rootObj->find (mname);
      if (!obj)
        {
          if (rootObj->getParent ())
            {
              obj = rootObj->getParent ()->find (mname);
            }
        }

    }
  else
    {
      obj = rootObj->find (mname);
      if (!(obj))
        {
          size_t rlc2 = mname.find_last_of ("#$!");
          if (rlc2 != std::string::npos)
            {
              std::string type = mname.substr (0, rlc2);
              std::string num = mname.substr (rlc2 + 1);
              int onum = intRead (num, -1);
              if (onum >= 0)
                {
                  makeLowerCase (type);
                  switch (mname[rlc2])
                    {
                    case '$':              //$ means get by id
                      obj = rootObj->findByUserID (type, onum);
                      break;
                    case '#':
                    case '!':
                      obj = rootObj->getSubObject (type, onum);
                      break;
					default:
						break;
                    }
                }
            }
          else if (rootSearch)
            {
              gridCoreObject *rootObject2 = rootObj->find ("root");
              obj = rootObject2->find (mname);
            }
        }
    }
  if ((sep == '/') && (obj))  //we have a '/' so go into the sub model
  {
	  obj = locateObject(secName, obj, false);
  }
  else if ((secName[0] == ':') && (obj))  //we have a double colon so go deeper in the object using the found object as the base
    {
      obj = locateObject (secName.substr (1), obj, false);
    }
  return obj;
}


gridCoreObject * findMatchingObject(gridCoreObject *obj, gridCoreObject *root)
{
	gridCoreObject *par = obj;
	
	stringVec stackNames;
	while (par->getName() != root->getName())
	{
		stackNames.push_back(par->getName());
		par = par->getParent();
		if (!par)
		{
			break;
		}
		
	}
	//now trace back through the new root object
	gridCoreObject *matchObj = root;
	while (!stackNames.empty())
	{
		matchObj=matchObj->find(stackNames.back());
		stackNames.pop_back();
		if (!matchObj)
		{
			break;
		}
	}
	if (matchObj)
	{
		if (matchObj->getName() == obj->getName())
		{
			return matchObj;
		}
	}
	return nullptr;
}
