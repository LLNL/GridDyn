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

#ifndef OBJECT_INTERPRETER_H_
#define OBJECT_INTERPRETER_H_

#include "gridCore.h"
#include "units.h"
#include <string>
#include <functional>

/** @brief class for constructing some info about an object
 generall used for interpreting an object string with object and field references and possibly units as well
*/
class objInfo
{
public:
  gridCoreObject *m_obj = nullptr; //!< pointer to the object being referenced
  std::string m_field;  //!< the field referenced
  gridUnits::units_t m_unitType = gridUnits::defUnit; //!< the units corresponding to the reference

  /** @brief default constructor*/
  objInfo ();
  /** @brief constuctor with the string to interpret and a base object to begin the search process for
  @param[in] Istring the input string containing the object and field reference
  @param[in] obj the object used as the basis for the search if needed
  */
  objInfo (std::string Istring, gridCoreObject *obj);

  /** @brief load a string similar to the constructor except with an existing object
   the string should be of the form objA::subObject:field(units) const
  @param[in] Istring the input string containing the object and field reference
          @param[in] obj the object used as the basis for the search if needed
          */
  void LoadInfo (std::string Istring, gridCoreObject *obj);
};

/** @brief locate a specific object by name
 the string should be of the form obj::subobj:field  field is optional but "::" defines parent child relationships along
the search path
obj and subobj descriptions can take a number of forms
either the name or specific description if a parent can only contain 1 of that type of object
or type#N  where type is the type of subobject and N is the index number starting from 0
or type$ID  where type is the type of subobject and ID is the user ID of the object
@param[in] Istring the string containing the the object name
@param[in] rootObj the object where the search is started
@param[in] rootSearch is set to true and the object can't be located from rootObj then the function will attempt to locate a root object and start the search over from there.
*/
gridCoreObject* locateObject (std::string Istring, const gridCoreObject *rootObj, bool rootSearch = true);





#endif