/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// test case for operatingBoundary class

#include "utilities/OperatingBoundary.h"

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>

BOOST_AUTO_TEST_SUITE(operatingBoundary_tests, *boost::unit_test::label("quick"))

using namespace utilities;
BOOST_AUTO_TEST_CASE(opbound_test1)
{
    OperatingBoundary Bound(-10, 10, 0, 5);

    auto lim = Bound.getLimits(0.0);
    BOOST_CHECK_CLOSE(lim.first, 0.0, 0.000001);
    BOOST_CHECK_CLOSE(lim.second, 5.0, 0.000001);

    BOOST_CHECK_SMALL(Bound.dMaxROC(0.0), 0.000001);
    BOOST_CHECK_SMALL(Bound.dMinROC(0.0), 0.0000001);
}

BOOST_AUTO_TEST_SUITE_END()
