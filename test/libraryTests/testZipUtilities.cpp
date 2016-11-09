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

//test case for workQueue

#include "zipUtilities.h"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

static const std::string zip_test_directory(GRIDDYN_TEST_DIRECTORY "/zip_tests/");

BOOST_AUTO_TEST_SUITE(zipUtilities_tests)

BOOST_AUTO_TEST_CASE(unzip_test)
{
	std::string file = zip_test_directory + "rectifier.fmu";
	std::string directory = zip_test_directory + "rectifier";
	int status=unzip(file,directory);
	BOOST_CHECK(status == 0);
	BOOST_REQUIRE(boost::filesystem::exists(directory));
	boost::filesystem::remove_all(directory);
}

BOOST_AUTO_TEST_CASE(zip_test2)
{
	//make two files with very simple text
	int fileSize1 = 1000000;
	std::vector<char> a0(fileSize1, '0');
	std::string fileZeros= zip_test_directory + "zeros.txt";
	std::ofstream outZeros(fileZeros);
	outZeros.write(a0.data(), fileSize1);
	outZeros.close();
	int fileSize2 = 981421;
	std::vector<char> a1(fileSize2, '1');
	std::string fileOnes = zip_test_directory + "ones.txt";
	std::ofstream outOnes(fileOnes);
	outOnes.write(a1.data(), fileSize2);
	outOnes.close();
	//zip them up into a zip file
	auto zipfile= zip_test_directory + "data.zip";
	auto status = zip(zipfile, std::vector<std::string>{ fileZeros,fileOnes });
	BOOST_CHECK(status ==0);
	BOOST_REQUIRE(boost::filesystem::exists(zipfile));

	//get the sizes of the original files
	auto filesize1 = boost::filesystem::file_size(fileZeros);
	auto filesize2 = boost::filesystem::file_size(fileOnes);

	auto zipsize = boost::filesystem::file_size(zipfile);
	//make sure we compressed a lot
	BOOST_CHECK(zipsize < (filesize1 + filesize2) / 40);

	//remove the files
	boost::filesystem::remove(fileZeros);
	boost::filesystem::remove(fileOnes);
	//extract them and recheck sizes
	status = unzip(zipfile, zip_test_directory);
	BOOST_CHECK(status == 0);
	BOOST_REQUIRE(boost::filesystem::exists(fileZeros));
	BOOST_REQUIRE(boost::filesystem::exists(fileOnes));

	auto filesize1b = boost::filesystem::file_size(fileZeros);
	auto filesize2b = boost::filesystem::file_size(fileOnes);

	BOOST_CHECK(filesize1 == filesize1b);
	BOOST_CHECK(filesize2 == filesize2b);
	//remove all the files
	boost::filesystem::remove(fileZeros);
	boost::filesystem::remove(fileOnes);
	boost::filesystem::remove(zipfile);

}


BOOST_AUTO_TEST_SUITE_END()