/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "griddyn/griddyn-config.h"
#ifndef BOOST_STATIC
#    define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE testExtra
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include "../testHelper.h"

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE(glbconfig);
