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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "utilities/stringOps.h"
#include "utilities/stringToCmdLine.h"

#include <iostream>

using namespace stringOps;

BOOST_AUTO_TEST_SUITE (stringop_tests, * boost::unit_test::label("quick"))

/** test conversion to lower case*/
BOOST_AUTO_TEST_CASE (convert_to_lower_case_test)
{
    std::string test1 = "AbCd: *Ty; ";
    auto testres = convertToLowerCase (test1);
    BOOST_CHECK (testres == "abcd: *ty; ");
    testres = convertToLowerCase ("  1234:ABC\n\t RTrt ");
    BOOST_CHECK (testres == "  1234:abc\n\t rtrt ");
}

/** test make lower case*/
BOOST_AUTO_TEST_CASE (make_lower_case_test)
{
    std::string test1 = "AbCd: *Ty; ";
    makeLowerCase (test1);
    BOOST_CHECK (test1 == "abcd: *ty; ");
    std::string test2 = "  1234:ABC\n\t RTrt ";
    makeLowerCase (test2);
    BOOST_CHECK (test2 == "  1234:abc\n\t rtrt ");
}

/** test conversion to upper case*/
BOOST_AUTO_TEST_CASE (convert_to_upper_case_test)
{
    std::string test1 = "AbCd: *Ty; ";
    auto testres = convertToUpperCase (test1);
    BOOST_CHECK (testres == "ABCD: *TY; ");
    testres = convertToUpperCase ("  1234:ABC\n\t RTrt ");
    BOOST_CHECK (testres == "  1234:ABC\n\t RTRT ");
}

/** test make upper case*/
BOOST_AUTO_TEST_CASE (make_upper_case_test)
{
    std::string test1 = "AbCd: *Ty; ";
    makeUpperCase (test1);
    BOOST_CHECK (test1 == "ABCD: *TY; ");
    std::string test2 = "  1234:ABC\n\t RTrt ";
    makeUpperCase (test2);
    BOOST_CHECK (test2 == "  1234:ABC\n\t RTRT ");
}

/** test trim*/
BOOST_AUTO_TEST_CASE (trimString_tests)
{
    std::string test1 = "AbCd: *Ty; ";
    trimString (test1);
    BOOST_CHECK (test1 == "AbCd: *Ty;");
    std::string test2 = "  \t1234:AbC\n\t RTrt\n ";
    trimString (test2);
    BOOST_CHECK (test2 == "1234:AbC\n\t RTrt");
    // test for an empty results
    std::string test3 = "  \t\n \t\t \n ";
    trimString (test3);
    BOOST_CHECK (test3.empty ());
}

/** test trim*/
BOOST_AUTO_TEST_CASE (trim_tests)
{
    std::string test1 = "AbCd: *Ty; ";
    auto testres = trim (test1);
    BOOST_CHECK (testres == "AbCd: *Ty;");
    std::string test2 = "  \t1234:AbC\n\t RTrt\n ";
    testres = trim (test2);
    BOOST_CHECK (testres == "1234:AbC\n\t RTrt");

    std::string test3 = "::  \t1234:AbC\n\t RTrt\n ::";
    // test with ':' also as a space
    testres = trim (test3, "\t\n: ");
    BOOST_CHECK (testres == "1234:AbC\n\t RTrt");

    std::string test4 = "  \t\n \t\t \n ";
    testres = trim (test4);
    BOOST_CHECK (testres.empty ());
}

/** test trim*/
BOOST_AUTO_TEST_CASE (tailString_tests)
{
    std::string test1 = "AbCd: *Ty; ";
    auto testres = getTailString (test1, '*');
    BOOST_CHECK (testres == "Ty; ");
    std::string test2 = "bob::test1:test2:4";
    testres = getTailString (test2, ':');
    BOOST_CHECK (testres == "4");

    std::string test3 = "::  \t1234:AbC\n\t RTrt\n ::";
    // test with ':' also as a space
    testres = getTailString (test3, ':');
    BOOST_CHECK (testres.empty ());

    std::string test4 = "bob::test1:test2:4";
    testres = getTailString (test4, '-');
    BOOST_CHECK (testres == test4);

    std::string test5 = "4testingBeginning";
    testres = getTailString (test5, '4');
    BOOST_CHECK (testres == "testingBeginning");

    std::string test6 = "4testingBeginning";
    testres = getTailString (test6, 'g');
    BOOST_CHECK (testres.empty ());
}

/** simple split line test*/
BOOST_AUTO_TEST_CASE (splitline_test1)
{
    std::string test1 = "alpha, bravo, charlie";
    auto testres = splitline (test1);

    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "alpha");
    BOOST_CHECK (testres[1] == " bravo");
    BOOST_CHECK (testres[2] == " charlie");

    testres = splitline (test1, ", ", delimiter_compression::on);

    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "alpha");
    BOOST_CHECK (testres[1] == "bravo");
    BOOST_CHECK (testres[2] == "charlie");
}

/** simple split line test for multiple tokens*/
BOOST_AUTO_TEST_CASE (splitline_test2)
{
    std::string test1 = "alpha, bravo,;, charlie,";
    auto testres = splitline (test1);

    BOOST_REQUIRE (testres.size () == 6);
    BOOST_CHECK (testres[2].empty ());
    BOOST_CHECK (testres[3].empty ());
    BOOST_CHECK (testres[5].empty ());

    std::string test2 = "alpha, bravo,;, charlie,";
    testres = splitline (test1, ";, ", delimiter_compression::on);

    BOOST_REQUIRE (testres.size () == 4);
    BOOST_CHECK (testres[3].empty ());
}

/** simple split line test*/
BOOST_AUTO_TEST_CASE (splitline_test3)
{
    std::string test1 = " alpha,   bravo ; charlie ";
    auto testres = splitline (test1);
    trim (testres);
    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "alpha");
    BOOST_CHECK (testres[1] == "bravo");
    BOOST_CHECK (testres[2] == "charlie");
}

/**remove quotes test test*/
BOOST_AUTO_TEST_CASE (removeQuotes_test)
{
    std::string test1 = "\'remove quotes\'";
    auto testres = removeQuotes (test1);

    BOOST_CHECK (testres == "remove quotes");
    std::string test2 = "\"remove quotes \"";
    testres = removeQuotes (test2);

    BOOST_CHECK (testres == "remove quotes ");

    std::string test3 = "\"remove quotes \'";
    testres = removeQuotes (test3);

    BOOST_CHECK (testres == "\"remove quotes \'");

    std::string test4 = "   \" remove quotas \"  ";
    testres = removeQuotes (test4);

    BOOST_CHECK (testres == " remove quotas ");
}

BOOST_AUTO_TEST_CASE (appendInteger_tests)
{
    std::string str1 = "tail_";
    appendInteger (str1, 456u);
    BOOST_CHECK (str1 == "tail_456");

    std::string str2 = "tail_";
    appendInteger (str2, 0);
    BOOST_CHECK (str2 == "tail_0");

    std::string str3 = "tail";
    appendInteger (str3, -34);
    BOOST_CHECK (str3 == "tail-34");

    std::string str4 = "tail";
    appendInteger (str4, 12345678ull);
    BOOST_CHECK (str4 == "tail12345678");

    // only use the integer part of the number if it is less than 1,000,000,000
    std::string str5 = "num=";
    appendInteger (str5, 98.2341);
    BOOST_CHECK (str5 == "num=98");

    // the result here is not well defined so this check is mainly to make sure it didn't crash
    std::string str6 = "num=";
    appendInteger (str6, 45e34);
    BOOST_CHECK (str6 != "num=");

    std::string str7 = "long num_";
    appendInteger (str7, 1234567890123456ll);
    BOOST_CHECK (str7 == "long num_1234567890123456");

    std::string str8 = "long num_";
    appendInteger (str8, -1234567890123ll);
    BOOST_CHECK (str8 == "long num_-1234567890123");
}

BOOST_AUTO_TEST_CASE (splitLineQuotes_tests)
{
    std::string test1 = "454, 345, happy; frog";
    auto testres = splitlineQuotes (test1);
    trim (testres);
    BOOST_REQUIRE (testres.size () == 4);
    BOOST_CHECK (testres[0] == "454");
    BOOST_CHECK (testres[1] == "345");
    BOOST_CHECK (testres[2] == "happy");
    BOOST_CHECK (testres[3] == "frog");
    std::string test2 = " \'alpha,   bravo\' ; charlie ";
    auto testres2 = splitlineQuotes (test2);
    trim (testres2);
    BOOST_REQUIRE (testres2.size () == 2);
    BOOST_CHECK (testres2[0] == "\'alpha,   bravo\'");
    BOOST_CHECK (testres2[1] == "charlie");

    std::string test3 = " \"test1\",\'test2\' ; \"charlie\",";
    auto testres3 = splitlineQuotes (test3);
    trim (testres3);
    BOOST_REQUIRE (testres3.size () == 4);
    BOOST_CHECK (testres3[0] == "\"test1\"");
    BOOST_CHECK (testres3[1] == "\'test2\'");
    BOOST_CHECK (testres3[2] == "\"charlie\"");
    BOOST_CHECK (testres3[3].empty ());

    testres3 = splitlineQuotes (test3, default_delim_chars, default_quote_chars, delimiter_compression::on);
    BOOST_REQUIRE (testres3.size () == 3);

    std::string test4 = "\"'part1' and,; 'part2'\",\"34,45,56\"";
    auto testres4 = splitlineQuotes (test4);
    BOOST_REQUIRE (testres4.size () == 2);
    BOOST_CHECK (testres4[1] == "\"34,45,56\"");

    std::string test5 = "\"part1'\" and \"part2\",\"34,45,56\"";
    auto testres5 = splitlineQuotes (test5);
    BOOST_REQUIRE (testres5.size () == 2);
    BOOST_CHECK (testres5[1] == "\"34,45,56\"");

    std::string test6 = "--arg1 --arg2=bob --arg3=\"string1 string2\" --arg3=\"bob\"";
    auto testres6 = splitlineQuotes (test6, " \t");
    BOOST_REQUIRE (testres6.size () == 4);
    BOOST_CHECK (testres6[2] == "--arg3=\"string1 string2\"");
}

BOOST_AUTO_TEST_CASE (splitLineBracket_tests)
{
    std::string test1 = "(454, 345), happy; frog";
    auto testres = splitlineBracket (test1);
    trim (testres);
    BOOST_REQUIRE (testres.size () == 3);
    BOOST_CHECK (testres[0] == "(454, 345)");
    BOOST_CHECK (testres[1] == "happy");
    BOOST_CHECK (testres[2] == "frog");
    std::string test2 = " \'alpha,   bravo\' ; charlie ";
    // the default bracket split should recognize strings as well
    auto testres2 = splitlineBracket (test2);
    trim (testres2);
    BOOST_REQUIRE (testres2.size () == 2);
    BOOST_CHECK (testres2[0] == "\'alpha,   bravo\'");
    BOOST_CHECK (testres2[1] == "charlie");

    std::string test3 = "$45,34,45$;$23.45,34,23.3$";
    auto testres3 = splitlineBracket (test3, ";,", "$");
    trim (testres3);
    BOOST_REQUIRE (testres3.size () == 2);
    BOOST_CHECK (testres3[0] == "$45,34,45$");
    BOOST_CHECK (testres3[1] == "$23.45,34,23.3$");
}

BOOST_AUTO_TEST_CASE (string2cmdLine_test)
{
    utilities::StringToCmdLine cmdargs ("--arg1 --arg2=bob --arg3=\"string1 string2\" --arg3=\"\"bob\"\"");

    BOOST_CHECK_EQUAL (cmdargs.getArgCount (), 5);

    auto args = cmdargs.getArgV ();
    BOOST_CHECK_EQUAL ("--arg1", args[1]);
    BOOST_CHECK_EQUAL ("--arg3=string1 string2", args[3]);
	BOOST_CHECK_EQUAL("--arg3=\"bob\"", args[4]);
}
BOOST_AUTO_TEST_SUITE_END ()
