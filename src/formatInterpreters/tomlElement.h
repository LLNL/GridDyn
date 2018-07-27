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

#include "toml/toml.h"

class tomlElement
{
  public:
    int elementIndex = 0;
    std::string name;
    int arrayIndex = 0;
    tomlElement () = default;
    tomlElement (toml::Value vElement, std::string newName);

    void clear ();
    const toml::Value &getElement () const { return (arraytype) ? *element.find (arrayIndex) : element; }
    int count () const { return (arraytype) ? static_cast<int> (element.size ()) : 1; }
    bool isNull () const { return (arraytype) ? element.find (arrayIndex) != nullptr : element.empty (); }

  private:
    toml::Value element;
    bool arraytype = false;
};
