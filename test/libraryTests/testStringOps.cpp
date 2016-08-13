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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "stringOps.h"

#include <iostream>

BOOST_AUTO_TEST_SUITE(utility_tests)

/** test conversion to lower case*/
BOOST_AUTO_TEST_CASE(convert_to_lower_case_test)
{
	std::string test1 = "AbCd: *Ty; ";
	auto testres = convertToLowerCase(test1);
	BOOST_CHECK(testres == "abcd: *ty; ");
	testres = convertToLowerCase("  1234:ABC\n\t RTrt ");
	BOOST_CHECK(testres == "  1234:abc\n\t rtrt ");
}

/** test make lower case*/
BOOST_AUTO_TEST_CASE(make_lower_case_test)
{
	std::string test1 = "AbCd: *Ty; ";
	makeLowerCase(test1);
	BOOST_CHECK(test1 == "abcd: *ty; ");
	std::string test2 = "  1234:ABC\n\t RTrt ";
	makeLowerCase(test2);
	BOOST_CHECK(test2 == "  1234:abc\n\t rtrt ");
}

/** test conversion to upper case*/
BOOST_AUTO_TEST_CASE(convert_to_upper_case_test)
{
	std::string test1 = "AbCd: *Ty; ";
	auto testres = convertToUpperCase(test1);
	BOOST_CHECK(testres == "ABCD: *TY; ");
	testres = convertToUpperCase("  1234:ABC\n\t RTrt ");
	BOOST_CHECK(testres == "  1234:ABC\n\t RTRT ");
}

/** test make upper case*/
BOOST_AUTO_TEST_CASE(make_upper_case_test)
{
	std::string test1 = "AbCd: *Ty; ";
	makeUpperCase(test1);
	BOOST_CHECK(test1 == "ABCD: *TY; ");
	std::string test2 = "  1234:ABC\n\t RTrt ";
	makeUpperCase(test2);
	BOOST_CHECK(test2 == "  1234:ABC\n\t RTRT ");
}


/** test trim*/
BOOST_AUTO_TEST_CASE(trimString_tests)
{
	std::string test1 = "AbCd: *Ty; ";
	trimString(test1);
	BOOST_CHECK(test1 == "AbCd: *Ty;");
	std::string test2 = "  \t1234:AbC\n\t RTrt\n ";
	trimString(test2);
	BOOST_CHECK(test2 == "1234:AbC\n\t RTrt");
	//test for an empty results
	std::string test3 = "  \t\n \t\t \n ";
	trimString(test3);
	BOOST_CHECK(test3.empty());
}

/** test trim*/
BOOST_AUTO_TEST_CASE(trim_tests)
{
	std::string test1 = "AbCd: *Ty; ";
	auto testres=trim(test1);
	BOOST_CHECK(testres == "AbCd: *Ty;");
	std::string test2 = "  \t1234:AbC\n\t RTrt\n ";
	testres=trim(test2);
	BOOST_CHECK(testres == "1234:AbC\n\t RTrt");

	std::string test3 = "::  \t1234:AbC\n\t RTrt\n ::";
	//test with ':' also as a space
	testres = trim(test3,"\t\n: ");
	BOOST_CHECK(testres == "1234:AbC\n\t RTrt");

	std::string test4 = "  \t\n \t\t \n ";
	testres=trim(test4);
	BOOST_CHECK(testres.empty());
}


/** test trim*/
BOOST_AUTO_TEST_CASE(tailString_tests)
{
	std::string test1 = "AbCd: *Ty; ";
	auto testres = getTailString(test1,'*');
	BOOST_CHECK(testres == "Ty; ");
	std::string test2 = "bob::test1:test2:4";
	testres = getTailString(test2, ':');
	BOOST_CHECK(testres == "4");

	std::string test3 = "::  \t1234:AbC\n\t RTrt\n ::";
	//test with ':' also as a space
	testres = getTailString(test3, ':');
	BOOST_CHECK(testres.empty());

	std::string test4 = "bob::test1:test2:4";
	testres = getTailString(test4,'-');
	BOOST_CHECK(testres==test4);

	std::string test5 = "4testingBeginning";
	testres = getTailString(test5, '4');
	BOOST_CHECK(testres == "testingBeginning");

	std::string test6 = "4testingBeginning";
	testres = getTailString(test6, 'g');
	BOOST_CHECK(testres.empty() );
}

/** simple split line test*/
BOOST_AUTO_TEST_CASE(splitline_test1)
{
	std::string test1 = "alpha, bravo, charlie";
	auto testres = splitline(test1);

	BOOST_CHECK(testres.size()==3);
	BOOST_CHECK(testres[0] == "alpha");
	BOOST_CHECK(testres[1] == " bravo");
	BOOST_CHECK(testres[2] == " charlie");


	std::string test2 = "alpha, bravo, charlie";
	testres = splitline(test1,", ",delimiter_compression::on);

	BOOST_CHECK(testres.size() == 3);
	BOOST_CHECK(testres[0] == "alpha");
	BOOST_CHECK(testres[1] == "bravo");
	BOOST_CHECK(testres[2] == "charlie");

}

/** simple split line test for multiple tokens*/
BOOST_AUTO_TEST_CASE(splitline_test2)
{
	std::string test1 = "alpha, bravo,;, charlie,";
	auto testres = splitline(test1);

	BOOST_CHECK(testres.size() == 6);
	BOOST_CHECK(testres[2].empty());
	BOOST_CHECK(testres[3].empty());
	BOOST_CHECK(testres[5].empty());


	std::string test2 = "alpha, bravo,;, charlie,";
	testres = splitline(test1, ";, ", delimiter_compression::on);

	BOOST_CHECK(testres.size() == 4);
	BOOST_CHECK(testres[3].empty());

}

/** simple split line test*/
BOOST_AUTO_TEST_CASE(splitline_test3)
{
	std::string test1 = " alpha,   bravo ; charlie ";
	auto testres = splitlineTrim(test1);

	BOOST_CHECK(testres.size() == 3);
	BOOST_CHECK(testres[0] == "alpha");
	BOOST_CHECK(testres[1] == "bravo");
	BOOST_CHECK(testres[2] == "charlie");
}
BOOST_AUTO_TEST_SUITE_END()