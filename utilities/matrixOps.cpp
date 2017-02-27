/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "matrixOps.h"
#include <cstring>

std::vector<double> matrixDataMultiply(matrixData<double> &ad, const double vec[])
{
	std::vector<double> res(ad.rowLimit());
	matrixDataMultiply(ad, vec, res.data());
	return res;
}

void matrixDataMultiply(matrixData<double> &ad, const double vec[], double res[])
{
	memset(res, 0, sizeof(double)*ad.rowLimit());
	auto sz = ad.size();
	ad.start();
	index_t ii = 0;
	while (ii++ < sz)
	{
		auto element = ad.next();
		res[element.row] += element.data*vec[element.col];
	}
}