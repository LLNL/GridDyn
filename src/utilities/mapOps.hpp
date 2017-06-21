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

#ifndef MAP_OPS_H_
#define MAP_OPS_H_
#pragma once

#include "optionalDef.hpp"
#include <map>
#include <unordered_map>

template <class X1, class X2>
inline X2 mapFind (const std::map<X1, X2> &mapS, const X1 &val, const X2 &defVal)
{
    auto map_it = mapS.find (val);
    return (map_it != mapS.end ()) ? map_it->second : defVal;
}

template <class X1, class X2>
utilities::optional<X2> mapFind (const std::map<X1, X2> &mapS, const X1 &val)
{
    auto map_it = mapS.find (val);
    if (map_it != mapS.end ())
    {
        return map_it->second;
    }
    return {};
}

template <class X1, class X2>
inline X2 mapFind (const std::unordered_map<X1, X2> &mapS, const X1 &val, const X2 &defVal)
{
    auto map_it = mapS.find (val);
    return (map_it != mapS.end ()) ? map_it->second : defVal;
}

template <class X1, class X2>
utilities::optional<X2> mapFind (const std::unordered_map<X1, X2> &mapS, const X1 &val)
{
    auto map_it = mapS.find (val);
    if (map_it != mapS.end ())
    {
        return map_it->second;
    }
    return {};
}
#endif