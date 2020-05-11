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

#include <vector>

/** mode defining how to get partial derivatives for Jacobian calculations*/
enum class refMode_t : unsigned char {
    direct = 0,
    level1 = 1,
    level2 = 2,
    level3 = 3,
    level4 = 4,
    level5 = 5,
    level6 = 6,
    level7 = 7,
    level8 = 8,

};

/** data class for storing variable information for states and outputs*/
class vInfo {
  public:
    int varIndex = -1;  //!< the actual variable index in the fmi
    int index = -1;  //!< the local index into a matrix
    bool isState = false;  //!< defining if it is a state [for output only]
    refMode_t refMode =
        refMode_t::direct;  //!< the mode to use for computing the partial derivatives
    std::vector<int> inputDep;  //!< the inputs on which the calculation depends
    std::vector<int> stateDep;  //!< the states on which the calculation depends
};
