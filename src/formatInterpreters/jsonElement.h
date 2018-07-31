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

#include "json/jsoncpp.h"

class jsonElement
{
  public:
    int elementIndex = 0;
    std::string name;
    Json_gd::ArrayIndex arrayIndex = 0;
    jsonElement () noexcept {}
    jsonElement (Json_gd::Value vElement, std::string newName);

    void clear ();
    const Json_gd::Value &getElement () const { return (arraytype) ? element[arrayIndex] : element; }
    Json_gd::ArrayIndex count () const { return (arraytype) ? element.size () : Json_gd::ArrayIndex (1); }
    bool isNull () const { return (arraytype) ? element[arrayIndex].isNull () : element.isNull (); }

  private:
    Json_gd::Value element = Json_gd::nullValue;
    bool arraytype = false;
};
