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

#include "griddyn/gridDynDefinitions.hpp"
#include "utilities/stringConversion.h"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <type_traits>

BOOST_AUTO_TEST_SUITE (stringconversion_tests, * boost::unit_test::label("quick"))

/** test conversion to lower case*/
BOOST_AUTO_TEST_CASE (simple_integer_conversions_test)
{
    auto a = numeric_conversion<int> ("457", -1);
    BOOST_CHECK_EQUAL (a, 457);
    auto b = numeric_conversion<long long> ("-457", -1);
    BOOST_CHECK_EQUAL (b, -457);
    static_assert (std::is_same<decltype (b), long long>::value, "conversion types do not match");
    auto c = numeric_conversion<unsigned char> ("25", 0xFF);
    BOOST_CHECK_EQUAL (c, 25);
    auto d = numeric_conversion<short> ("-7629", 0xFF);
    BOOST_CHECK_EQUAL (d, -7629);
    BOOST_CHECK (sizeof (d) == 2);

    auto e = numeric_conversion<unsigned int> ("-1", 0);
    BOOST_CHECK_EQUAL (e, static_cast<unsigned int> (-1));

    auto f = numeric_conversion ("FF3q", -234);
    BOOST_CHECK_EQUAL (f, -234);
}


BOOST_AUTO_TEST_CASE (simple_floating_point_conversions_test)
{
    const double closeDef = 0.0000000001;
    auto a = numeric_conversion<float> ("457", -1);
    BOOST_CHECK_EQUAL (a, 457);
    auto b = numeric_conversion<double> ("234.123131", -1);
    BOOST_CHECK_CLOSE (b, 234.123131, closeDef);
    static_assert (std::is_same<decltype (b), double>::value, "conversion types do not match");
    auto c = numeric_conversion<double> (".456", 0xFF);
    BOOST_CHECK_CLOSE (c, .456, closeDef);
    auto d = numeric_conversion<long double> ("45.456e27", 0xFF);
    BOOST_CHECK_CLOSE (d, 45.456e27, closeDef);
    BOOST_CHECK (sizeof (d) == sizeof (long double));

    auto e = numeric_conversion<double> ("-456.234", 0);
    BOOST_CHECK_CLOSE (e, -456.234, closeDef);

    auto f = numeric_conversion<double> ("-23E-2", 0);
    BOOST_CHECK_CLOSE (f, -0.23, closeDef);

    auto g = numeric_conversion ("FF3q", 45.34);
    BOOST_CHECK_CLOSE (g, 45.34, closeDef);
}


BOOST_AUTO_TEST_SUITE_END ()
