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

#ifndef GRIDDYN_PLUGIN_API_H_

#define GRIDDYN_PLUGIN_API_H_

#include <string>

class gridDynPlugInApi
{
  public:
    virtual std::string name () const = 0;
    virtual void load () = 0;
    virtual void load (const std::string &sectionName) = 0;

    virtual ~gridDynPlugInApi () {}
};

#endif
