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

#ifndef OBJECT_INTERPRETER_H_
#define OBJECT_INTERPRETER_H_

#include "coreObject.h"
#include "units.h"

/** @brief class for constructing some info about an object
 generally used for interpreting an object string with object and field references and possibly units as well
*/
class objInfo
{
public:
  coreObject *m_obj = nullptr; //!< pointer to the object being referenced
  std::string m_field;  //!< the field referenced
  gridUnits::units_t m_unitType = gridUnits::defUnit; //!< the units corresponding to the reference

  /** @brief default constructor*/
  objInfo ();
  /** @brief constructor with the string to interpret and a base object to begin the search process for
  @param[in] Istring the input string containing the object and field reference
  @param[in] obj the object used as the basis for the search if needed
  */
  objInfo (const std::string &Istring, const coreObject *obj);

  /** @brief load a string similar to the constructor except with an existing object
   the string should be of the form objA::subObject:field(units) const
  @param[in] Istring the input string containing the object and field reference
          @param[in] obj the object used as the basis for the search if needed
          */
  void LoadInfo (const std::string &Istring, const coreObject *obj);
};

/** @brief locate a specific object by name
 the string should be of the form obj::subobj:field, or /obj/subobj?field,  field is optional but "::" or "/" defines parent child relationships along
the search path
obj and subobj descriptions can take a number of forms
either the name or specific description if a parent can only contain 1 of that type of object
or type#N  where type is the type of subObject and N is the index number starting from 0
or type$ID  where type is the type of subObject and ID is the user ID of the object
@param[in] Istring the string containing the object name
@param[in] rootObj the object where the search is started
@param[in] rootSearch is set to true and the object can't be located from rootObj then the function will attempt to locate a root object and start the search over from there.
*/
coreObject* locateObject (const std::string &Istring, const coreObject *rootObj, bool rootSearch = true);

/** @brief locate a matching object in a new tree
meant to target cloning operations where pointers need to be mapped to a new hierarchy
@param[in] obj the existing object
@param[in] root the root of the new tree to locate a corresponding object
*/
coreObject * findMatchingObject(coreObject *obj, coreObject *root);



#endif