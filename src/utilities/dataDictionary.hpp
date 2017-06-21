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

#ifndef DATA_DICTIONARY_H_
#define DATA_DICTIONARY_H_
#pragma once

#include "mapOps.hpp"
#include <mutex>

namespace utilities
{
template <typename keyType, typename dataType>
class dataDictionary
{
  private:
    std::unordered_map<keyType, dataType> Vals;
    std::mutex datalock;

  public:
    dataDictionary () = default;

    void update (keyType key, const dataType &data)
    {
        std::lock_guard<std::mutex> updateLock (datalock);
        Vals[key] = data;
    }
    void copy (keyType origKey, keyType newKey)
    {
        auto floc = Vals.find (origKey);
        if (floc != Vals.end ())
        {
            std::lock_guard<std::mutex> updateLock (datalock);
            Vals[newKey] = floc->second;
        }
    }
    dataType query (keyType key) { return mapFind (Vals, key, dataType ()); }
};

}  // namespace utilities
#endif
