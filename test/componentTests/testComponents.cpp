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


#include "griddyn/griddyn-config.h"
#ifndef GRIDDYN_BOOST_STATIC
#define BOOST_TEST_DYN_LINK
#endif


#define BOOST_TEST_MODULE coreObject testComponents
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include <boost/test/unit_test.hpp>

#include "../testHelper.h"

BOOST_GLOBAL_FIXTURE (glbconfig);
