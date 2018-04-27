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

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "utilities/string_viewOps.h"

#include <iostream>

using namespace utilities::string_viewOps;
using utilities::string_view;
BOOST_AUTO_TEST_SUITE (strViewop_tests, * boost::unit_test::label("quick"))


/** test trim*/
BOOST_AUTO_TEST_CASE (trimString_tests)
{
    string_view test1 = "AbCd: *Ty; ";
    test1 = trim (test1);
    BOOST_CHECK (test1 == "AbCd: *Ty;");
    string_view test2 = "  \t1234:AbC\n\t RTrt\n ";
    test2 = trim (test2);
    BOOST_CHECK (test2 == "1234:AbC\n\t RTrt");
    // test for an empty results
    string_view test3 = "  \t\n \t\t \n ";
    test3 = trim (test3);
    BOOST_CHECK (test3.empty ());

    // test for other characters
    string_view test4 = "%%**Bill45 *%*%";
    test4 = trim (test4, "*%");
    BOOST_CHECK (test4 == "Bill45 ");

    string_view test5 = "AbCd: *Ty; ";
    trimString (test5);
    BOOST_CHECK (test5 == "AbCd: *Ty;");
    string_view test6 = "  \t1234:AbC\n\t RTrt\n ";
    trimString (test6);
    BOOST_CHECK (test6 == "1234:AbC\n\t RTrt");
    // test for an empty results
    string_view test7 = "  \t\n \t\t \n ";
    trimString (test7);
    BOOST_CHECK (test7.empty ());

    // test for other characters
    string_view test8 = "%%**Bill45 *%*%";
    trimString (test8, "*%");
    BOOST_CHECK (test8 == "Bill45 ");
}


/** test trim*/
BOOST_AUTO_TEST_CASE (tailString_tests)
{
    string_view test1 = "AbCd: *Ty; ";
    auto testres = getTailString (test1, '*');
    BOOST_CHECK (testres == "Ty; ");
    string_view test2 = "bob::test1:test2:4";
    testres = getTailString (test2, ':');
    BOOST_CHECK (testres == "4");

    string_view test3 = "::  \t1234:AbC\n\t RTrt\n ::";
    // test with ':' also as a space
    testres = getTailString (test3, ':');
    BOOST_CHECK (testres.empty ());

    string_view test4 = "bob::test1:test2:4";
    testres = getTailString (test4, '-');
    BOOST_CHECK (testres == test4);

    string_view test5 = "4testingBeginning";
    testres = getTailString (test5, '4');
    BOOST_CHECK (testres == "testingBeginning");

    string_view test6 = "4testingBeginning";
    testres = getTailString (test6, 'g');
    BOOST_CHECK (testres.empty ());
}

/** simple split line test*/
BOOST_AUTO_TEST_CASE (splitline_test1)
{
    string_view test1 = "alpha, bravo, charlie";
    auto testres = split (test1);

    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "alpha");
    BOOST_CHECK (testres[1] == " bravo");
    BOOST_CHECK (testres[2] == " charlie");


    testres = split (test1, ", ", delimiter_compression::on);

    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "alpha");
    BOOST_CHECK (testres[1] == "bravo");
    BOOST_CHECK (testres[2] == "charlie");
}

/** simple split line test for multiple tokens*/
BOOST_AUTO_TEST_CASE (splitline_test2)
{
    string_view test1 = "alpha, bravo,;, charlie,";
    auto testres = split (test1);

    BOOST_REQUIRE (testres.size () == 6);
    BOOST_CHECK (testres[2].empty ());
    BOOST_CHECK (testres[3].empty ());
    BOOST_CHECK (testres[5].empty ());


    testres = split (test1, ";, ", delimiter_compression::on);

    BOOST_REQUIRE (testres.size () == 3);
}

/** simple split line and trim test*/
BOOST_AUTO_TEST_CASE (splitline_test3)
{
    string_view test1 = " alpha,   bravo ; charlie ";
    auto testres = split (test1);
    trim (testres);
    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "alpha");
    BOOST_CHECK (testres[1] == "bravo");
    BOOST_CHECK (testres[2] == "charlie");
}

/**remove quotes test*/
BOOST_AUTO_TEST_CASE (removeQuotes_test)
{
    string_view test1 = "\'remove quotes\'";
    auto testres = removeQuotes (test1);

    BOOST_CHECK (testres == "remove quotes");
    string_view test2 = "\"remove quotes \"";
    testres = removeQuotes (test2);

    BOOST_CHECK (testres == "remove quotes ");

    string_view test3 = "\"remove quotes \'";
    testres = removeQuotes (test3);

    BOOST_CHECK (testres == "\"remove quotes \'");

    string_view test4 = "   \" remove quotas \"  ";
    testres = removeQuotes (test4);

    BOOST_CHECK (testres == " remove quotas ");
}

BOOST_AUTO_TEST_CASE (splitLineQuotes_tests)
{
    string_view test1 = "454, 345, happy; frog";
    auto testres = splitlineQuotes (test1);
    trim (testres);
    BOOST_REQUIRE (testres.size () == 4);
    BOOST_CHECK (testres[0] == "454");
    BOOST_CHECK (testres[1] == "345");
    BOOST_CHECK (testres[2] == "happy");
    BOOST_CHECK (testres[3] == "frog");
    string_view test2 = " \'alpha,   bravo\' ; charlie ";
    auto testres2 = splitlineQuotes (test2);
    trim (testres2);
    BOOST_REQUIRE (testres2.size () == 2);
    BOOST_CHECK (testres2[0] == "\'alpha,   bravo\'");
    BOOST_CHECK (testres2[1] == "charlie");

    string_view test3 = " \"test1\",\'test2\' ; \"charlie\",";
    auto testres3 = splitlineQuotes (test3);
    trim (testres3);
    BOOST_REQUIRE (testres3.size () == 4);
    BOOST_CHECK (testres3[0] == "\"test1\"");
    BOOST_CHECK (testres3[1] == "\'test2\'");
    BOOST_CHECK (testres3[2] == "\"charlie\"");
    BOOST_CHECK (testres3[3].empty ());

    testres3 = splitlineQuotes (test3, default_delim_chars, default_quote_chars, delimiter_compression::on);
    BOOST_REQUIRE (testres3.size () == 3);


    string_view test4 = "\"'part1' and,; 'part2'\",\"34,45,56\"";
    auto testres4 = splitlineQuotes (test4);
    BOOST_REQUIRE (testres4.size () == 2);
    BOOST_CHECK (testres4[1] == "\"34,45,56\"");

    string_view test5 = "\"part1'\" and \"part2\",\"34,45,56\"";
    auto testres5 = splitlineQuotes (test5);
    BOOST_REQUIRE (testres5.size () == 2);
    BOOST_CHECK (testres5[1] == "\"34,45,56\"");
}

BOOST_AUTO_TEST_CASE (splitLineBracket_tests)
{
    string_view test1 = "(454, 345), happy; frog";
    auto testres = splitlineBracket (test1);
    trim (testres);
    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "(454, 345)");
    BOOST_CHECK (testres[1] == "happy");
    BOOST_CHECK (testres[2] == "frog");
    string_view test2 = " \'alpha,   bravo\' ; charlie ";
    // the default bracket split should recognize strings as well
    auto testres2 = splitlineBracket (test2);
    trim (testres2);
    BOOST_REQUIRE (testres2.size () == 2);
    BOOST_CHECK (testres2[0] == "\'alpha,   bravo\'");
    BOOST_CHECK (testres2[1] == "charlie");

    string_view test3 = "$45,34,45$;$23.45,34,23.3$";
    auto testres3 = splitlineBracket (test3, ";,", "$");
    trim (testres3);
    BOOST_REQUIRE (testres3.size () == 2);
    BOOST_CHECK (testres3[0] == "$45,34,45$");
    BOOST_CHECK (testres3[1] == "$23.45,34,23.3$");
}
BOOST_AUTO_TEST_SUITE_END ()
