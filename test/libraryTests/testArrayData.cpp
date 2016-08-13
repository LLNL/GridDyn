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






//test case for gridCoreObject object

#include "testHelper.h"
#include "gridDynTypes.h"
#include "arrayDataSparseSM.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <iostream>
#include <random>

BOOST_AUTO_TEST_SUITE(arrayData_tests)

BOOST_AUTO_TEST_CASE(test_block_compute)
{
	blockCompute<2, sparse_ordering::column_ordered> bc1;
	bc1.setMaxIndex(0, 20);
	std::vector<index_t> colcnt(6,0);
	for (index_t pp = 0; pp < 20; ++pp)
	{
		++colcnt[bc1.blockIndexGen(0, pp)];
	}
	BOOST_CHECK(colcnt[0] == 2);
	BOOST_CHECK(colcnt[1] == 8);
	BOOST_CHECK(colcnt[2] == 8);
	BOOST_CHECK(colcnt[3] == 2);

	blockCompute<2, sparse_ordering::row_ordered> bc2;
	bc2.setMaxIndex(20, 0);
	std::vector<index_t> colcnt2(6, 0);
	for (index_t pp = 0; pp < 20; ++pp)
	{
		++colcnt2[bc2.blockIndexGen(pp, 0)];
	}
	BOOST_CHECK(colcnt2[0] == 2);
	BOOST_CHECK(colcnt2[1] == 8);
	BOOST_CHECK(colcnt2[2] == 8);
	BOOST_CHECK(colcnt2[3] == 2);
}


BOOST_AUTO_TEST_CASE(test_block_compute2)
{
	blockCompute<3, sparse_ordering::column_ordered> bc1;
	bc1.setMaxIndex(7893, 7893);
	std::vector<index_t> colcnt(10, 0);
	for (index_t pp = 0; pp < 7893; ++pp)
	{
		++colcnt[bc1.blockIndexGen(pp, pp)];
	}
	BOOST_CHECK(colcnt[0] == 874);
	BOOST_CHECK(colcnt[1] == 1024);
	BOOST_CHECK(colcnt[2] == 1024);
	BOOST_CHECK(colcnt[3] == 1024);
	BOOST_CHECK(colcnt[4] == 1024);
	BOOST_CHECK(colcnt[5] == 1024);
	BOOST_CHECK(colcnt[6] == 1024);
	BOOST_CHECK(colcnt[7] == 875);
	BOOST_CHECK(colcnt[8] == 0);

	
}

BOOST_AUTO_TEST_CASE(test_keygen)
{
	keyCompute<std::uint32_t, sparse_ordering::column_ordered> kc1;
	

	auto key1 = kc1.keyGen(45, 1);
	BOOST_CHECK(key1 == (1 << 16) + 45);
	keyCompute<std::uint32_t, sparse_ordering::row_ordered> kc2;
	auto key2 = kc2.keyGen(45, 1);
	BOOST_CHECK(key2 == (45 << 16) + 1);

	BOOST_CHECK(kc1.row(key1) == 45);
	BOOST_CHECK(kc1.col(key1) == 1);
	BOOST_CHECK(kc2.row(key2) == 45);
	BOOST_CHECK(kc2.col(key2) == 1);

	keyCompute<std::uint64_t, sparse_ordering::column_ordered> kc3;


	auto key3 = kc3.keyGen(45, 1);
	BOOST_CHECK(key3 == ((std::uint64_t)(1) << 32) + 45);
	keyCompute<std::uint64_t, sparse_ordering::row_ordered> kc4;
	auto key4 = kc4.keyGen(45, 1);
	BOOST_CHECK(key4 == ((std::uint64_t)(45) << 32) + 1);

	BOOST_CHECK(kc3.row(key3) == 45);
	BOOST_CHECK(kc3.col(key3) == 1);
	BOOST_CHECK(kc4.row(key4) == 45);
	BOOST_CHECK(kc4.col(key4) == 1);
}

BOOST_AUTO_TEST_CASE(test_array_1)
{
	arrayDataSparseSMB<4, std::uint64_t> bigarray;
	bigarray.setColLimit(1000000);
	bigarray.setRowLimit(1000000);
	bigarray.reserve(200000);

	std::default_random_engine generator;
	std::uniform_int_distribution<std::uint32_t> distribution(1, 999998);
	for (size_t pp = 0; pp < 199997; ++pp)
	{
		auto index1 = distribution(generator);
		auto index2 = distribution(generator);
		bigarray.assign(index1, index2, 1.0);
	}
	bigarray.assign(0, 0, 3.27);
	bigarray.assign(999999, 999999, 6.129);

	bigarray.compact();
	
	bigarray.start();

	auto A = bigarray.next();
	BOOST_CHECK(A.col == 0);
	BOOST_CHECK(A.row == 0);
	BOOST_CHECK(A.data == 3.27);
	auto pcol = A.col;
	for (size_t pp = 1; pp < bigarray.size(); ++pp)
	{
		A = bigarray.next();
		if (A.col < pcol)
		{
			BOOST_CHECK(A.col < pcol);
		}
		pcol = A.col;
	}
	BOOST_CHECK(A.col == 999999);
	BOOST_CHECK(A.row == 999999);
	BOOST_CHECK(A.data == 6.129);
}

BOOST_AUTO_TEST_CASE(test_array_2)
{
	arrayDataSparseSMB<3, std::uint64_t,double,sparse_ordering::row_ordered> bigarray;
	bigarray.setColLimit(1000000);
	bigarray.setRowLimit(1000000);
	bigarray.reserve(200000);

	std::default_random_engine generator;
	std::uniform_int_distribution<std::uint32_t> distribution(1, 999998);
	for (size_t pp = 0; pp < 199997; ++pp)
	{
		auto index1 = distribution(generator);
		auto index2 = distribution(generator);
		bigarray.assign(index1, index2, 1.0);
	}
	bigarray.assign(0, 0, 3.27);
	bigarray.assign(999999, 999999, 6.129);

	bigarray.compact();

	bigarray.start();

	auto A = bigarray.next();
	BOOST_CHECK(A.col == 0);
	BOOST_CHECK(A.row == 0);
	BOOST_CHECK(A.data == 3.27);
	auto prow = A.row;
	for (size_t pp = 1; pp < bigarray.size(); ++pp)
	{
		A = bigarray.next();
		if (A.row < prow)
		{
			BOOST_CHECK(A.row < prow);
		}
		prow = A.row;
	}
	BOOST_CHECK(A.col == 999999);
	BOOST_CHECK(A.row == 999999);
	BOOST_CHECK(A.data == 6.129);
}

BOOST_AUTO_TEST_SUITE_END()