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

#include "arrayDataSparse.h"

#include <cassert>
#include <cmath>
#include <algorithm>

static const index_t kNullLocation((index_t)(-1));

bool compareLocRow(cLoc A, cLoc B)
{
	bool ans;
	if (std::get<adRow>(A) < std::get<adRow>(B))
	{
		ans = true;
	}
	else if (std::get<adRow>(A) > std::get<adRow>(B))
	{
		ans = false;
	}
	else       //A(0)==B(0)  so check the column
	{
		ans = (std::get<adCol>(A) < std::get<adCol>(B)) ? true : false;
	}
	return ans;
}

bool compareLocCol(cLoc A, cLoc B)
{
	bool ans;
	if (std::get<adCol>(A) < std::get<adCol>(B))
	{
		ans = true;
	}
	else if (std::get<adCol>(A) > std::get<adCol>(B))
	{
		ans = false;
	}
	else       //A(1)==B(1)  so check the row
	{
		ans = (std::get<adRow>(A) < std::get<adRow>(B)) ? true : false;
	}
	return ans;
}

bool compareLocRowQuick(cLoc A, cLoc B)
{
	return (std::get<adRow>(A) < std::get<adRow>(B));
}

bool compareLocColQuick(cLoc A, cLoc B)
{
	return (std::get<adCol>(A) < std::get<adCol>(B));
}

arrayDataSparse::arrayDataSparse(index_t startCount)
{
	data.reserve(startCount);
}
/**
* function to clear the data
*/
void arrayDataSparse::clear()
{
	data.clear();
}

void arrayDataSparse::assign(index_t X, index_t Y, double num)
{
	assert(X != kNullLocation);
	assert(X < rowLim);
	assert(Y < colLim);
	assert(std::isfinite(num));

	//data.push_back (cLoc (X, Y, num));
	data.emplace_back(X, Y, num);
}

count_t arrayDataSparse::size() const
{
	return static_cast<count_t>(data.size());
}

count_t arrayDataSparse::capacity() const
{
	return static_cast<count_t> (data.capacity());
}

void arrayDataSparse::sortIndex()
{
	std::sort(data.begin(), data.end(), compareLocCol);
	sortCount = static_cast<count_t>(data.size());
}

void arrayDataSparse::sortIndexCol()
{
	std::sort(data.begin(), data.end(), compareLocCol);
	sortCount = static_cast<count_t>(data.size());
}

void arrayDataSparse::sortIndexRow()
{
	std::sort(data.begin(), data.end(), compareLocRow);
	sortCount = static_cast<count_t>(data.size());
}
void arrayDataSparse::compact()
{
	if (data.empty())
	{
		return;
	}
	if (!isSorted())
	{
		sortIndex();
	}
	auto currentDataLocation = data.begin();
	auto testDataLocation = currentDataLocation+1;
	auto dataEnd = data.end();
	while (testDataLocation != dataEnd)
	{
		//Check if the next is equal to the previous in location 
		//if they are add them if not shift the new one to the right location and move on
		if ((std::get<adCol>(*testDataLocation) == std::get<adCol>(*currentDataLocation)) && (std::get<adRow>(*testDataLocation) == std::get<adRow>(*currentDataLocation)))
		{
			std::get<adVal>(*currentDataLocation) += std::get<adVal>(*testDataLocation);

		}
		else
		{
			++currentDataLocation;
			*currentDataLocation = *testDataLocation;
		}
		++testDataLocation;
	}
	//reduce the size and indicate that we are still sorted. 
	
		data.resize(++currentDataLocation - data.begin());
		sortCount = static_cast<count_t>(data.size());
	
}

std::vector<count_t> arrayDataSparse::columnCount()
{   
	if (!isSorted())
	{
		sortIndexCol();
	}
	auto dataEnd = data.end();
	std::vector<count_t> colCount(std::get<adRow>(*(dataEnd - 1)), 0);
	count_t cnt = 1;

	index_t testRow = std::get<adRow>(data.front());
	for (auto testData = data.begin(); testData != dataEnd; ++testData)
	{
		if (testRow != std::get<adRow>(*testData))
		{
			colCount[testRow] = cnt;
			cnt = 0;
			testRow = std::get<adRow>(*testData);
		}
		++cnt;
	}
	colCount[testRow] = cnt;
	return colCount;

}

index_t arrayDataSparse::rowIndex(index_t N) const
{
	return std::get<adRow>(data[N]);
}

index_t arrayDataSparse::colIndex(index_t N) const
{
	return std::get<adCol>(data[N]);
}

double arrayDataSparse::val(index_t N) const
{
	return std::get<adVal>(data[N]);
}

double arrayDataSparse::at(index_t rowN, index_t colN) const
{  
	if (isSorted())
	{
		auto res = std::lower_bound(data.begin(), data.end(), cLoc(rowN, colN, 0.0), compareLocCol);
		if (res == data.end())
		{
			return 0.0;
		}
		if ((std::get<adRow>(*res) == rowN) && (std::get<adCol>(*res) == colN))
		{
			return std::get<adVal>(*res);
		}
		else
		{
			return 0.0;
		}
	}
	else
	{
		for (const auto &rv : data)
		{
			if ((std::get<adRow>(rv) == rowN) && (std::get<adCol>(rv) == colN))
			{
				return std::get<adVal>(rv);
			}
		}
		return 0.0;
	}
	
}


void arrayDataSparse::start()
{
	cptr = data.begin();
}

data_triple<double> arrayDataSparse::next()
{
	data_triple<double> tp{ std::get<adRow>(*cptr), std::get<adCol>(*cptr), std::get<adVal>(*cptr) };
	++cptr;
	return tp;
}

bool arrayDataSparse::moreData()
{
	return (cptr != data.end());
}

void arrayDataSparse::scale(double factor, index_t startIndex, count_t count)
{
	if (startIndex >= data.size())
	{
		return;
	}
	auto res = data.begin() + startIndex;
	auto term = data.end();
	if (count < data.size())
	{
		term = data.begin() + std::min(startIndex + count, static_cast<count_t> (data.size()));
	}
	while (res != term)
	{
		std::get<adVal>(*res) *= factor;
		++res;
	}
}


void arrayDataSparse::translateRow(index_t origRow, index_t newRow)
{
	auto res = data.begin();
	auto term = data.end();
	while (res != term)
	{
		if (std::get<adRow>(*res) == origRow)
		{
			std::get<adRow>(*res) = newRow;
		}
		++res;
	}
}

void arrayDataSparse::filter(index_t rowTest)
{
	if (data.empty())
	{
		return;
	}
	auto dvb = data.begin();
	auto dv2 = dvb;
	auto dvend = data.end();
	index_t row, col;
	while (dv2 != dvend)
	{
		row = std::get<adRow>(*dv2);
		col = std::get<adCol>(*dv2);
		if ((row < rowLim) && (row != rowTest))
		{
			if (col < colLim)
			{
				*dvb = *dv2;
				++dvb;
			}
		}
		++dv2;
	}
	data.resize(dvb - data.begin());
}

void arrayDataSparse::translateCol(index_t origCol, index_t newCol)
{
	auto res = data.begin();
	auto term = data.end();
	while (res != term)
	{
		if (std::get<adCol>(*res) == origCol)
		{
			std::get<adCol>(*res) = newCol;
		}
		++res;
	}
}


void arrayDataSparse::scaleRow(index_t row, const double factor)
{
	auto res = data.begin();
	auto term = data.end();
	while (res != term)
	{
		if (std::get<adRow>(*res) == row)
		{
			std::get<adVal>(*res) *= factor;
		}
		++res;
	}
}

void arrayDataSparse::scaleCol(index_t col, const double factor)
{
	auto res = data.begin();
	auto term = data.end();
	while (res != term)
	{
		if (std::get<adCol>(*res) == col)
		{
			std::get<adVal>(*res) *= factor;
		}
		++res;
	}
}

void arrayDataSparse::copyTranslateRow(arrayDataSparse *a2, index_t origRow, index_t newRow)
{
	auto res = a2->data.begin();
	auto term = a2->data.end();

	while (res != term)
	{
		if (std::get<adRow>(*res) == origRow)
		{
			data.emplace_back(newRow, std::get<adCol>(*res), std::get<adVal>(*res));
		}
		++res;
	}
}

void arrayDataSparse::copyTranslateCol(arrayDataSparse *a2, index_t origCol, index_t newCol)
{
	auto res = a2->data.begin();
	auto term = a2->data.end();

	while (res != term)
	{
		if (std::get<adCol>(*res) == origCol)
		{
			data.emplace_back(std::get<adRow>(*res), newCol, std::get<adVal>(*res));
		}
		++res;
	}
}

void arrayDataSparse::merge(arrayDataSparse *a2)
{
	data.insert(data.end(), a2->data.begin(), a2->data.end());
}


void arrayDataSparse::cascade(arrayDataSparse *a2, index_t element)
{
	auto term = data.size();
	size_t mm, kk, nn = 0;
	double keyval = 0;
	while (nn != term)
	{
		if (std::get<adCol>(data[nn]) == element)
		{
			mm = 0;
			keyval = std::get<adVal>(data[nn]);
			for (kk = 0; kk < a2->data.size(); kk++)
			{
				if (std::get<adRow>(a2->data[kk]) == element)
				{
					if (mm == 0)
					{
						std::get<adCol>(data[nn]) = std::get<adCol>(a2->data[kk]);
						std::get<adVal>(data[nn]) = std::get<adVal>(a2->data[kk]) * keyval;
						++mm;
					}
					else
					{
						//data.push_back (cLoc (std::get<adRow> (data[nn]), std::get<adCol> (a2->data[kk]), keyval * std::get<adVal> (a2->data[kk])));
						data.emplace_back(std::get<adRow>(data[nn]), std::get<adCol>(a2->data[kk]), keyval * std::get<adVal>(a2->data[kk]));
						++mm;
					}
				}
			}
		}
		++nn;
	}
}

void arrayDataSparse::copyReplicate(arrayDataSparse *a2, index_t origCol, std::vector<index_t> newIndices, std::vector<double> mult)
{
	auto res = a2->data.begin();
	auto term = a2->data.end();

	while (res != term)
	{
		if (std::get<adCol>(*res) == origCol)
		{
			for (index_t nn = 0; nn<newIndices.size(); ++nn)
			{
				//data.push_back(cLoc(std::get<adRow>(*res), newIndices[nn], std::get<adVal>(*res)*mult[nn]));
				data.emplace_back(std::get<adRow>(*res), newIndices[nn], std::get<adVal>(*res)*mult[nn]);
			}
		}
		else
		{
			data.push_back(*res);
		}
		++res;
	}
}

void arrayDataSparse::transpose()
{
	size_t kk;
	int t1;
	for (kk = 0; kk < data.size(); kk++)
	{
		t1 = std::get<adCol>(data[kk]);
		std::get<adCol>(data[kk]) = std::get<adRow>(data[kk]);
		std::get<adRow>(data[kk]) = t1;
	}
}

void arrayDataSparse::diagMultiply(std::vector<double> diag)
{
	size_t kk;
	for (kk = 0; kk < data.size(); kk++)
	{
		std::get<adVal>(data[kk]) *= diag[std::get < adCol >(data[kk])];
	}
}

std::vector<index_t> findMissing(arrayDataSparse *ad)
{
	std::vector<index_t> missing;
	ad->sortIndexCol();
	ad->compact();
	ad->sortIndexRow();
	index_t pp = 0;
	bool good = false;
	for (index_t kk = 0; kk < ad->rowLimit(); ++kk)
	{
		good = false;
		while ((pp < ad->size()) && (ad->rowIndex(pp) <= kk))
		{
			if ((ad->rowIndex(pp) == kk) && (std::isnormal(ad->val(pp))))
			{
				good = true;
				++pp;
				break;
			}

			++pp;
			if (pp >= ad->size())
			{
				break;
			}
		}
		if (!good)
		{
			missing.push_back(kk);
		}
	}
	return missing;
}

std::vector<double> arrayDataSparse::vectorMult(std::vector<double> V)
{
	sortIndexRow();
	index_t nn;
	auto maxRow = std::get<adRow>(data.back());
	std::vector<double> out(maxRow, 0);
	auto res = data.begin();
	auto term = data.end();
	for (nn = 0; nn <= maxRow; ++nn)
	{
		while ((res != term) && (std::get<adRow>(*res) == nn))
		{
			out[nn] += std::get<adVal>(*res) * V[std::get < adCol >(*res)];
			++res;
		}
	}
	return out;
}

std::vector<std::vector<index_t>> findRank(arrayDataSparse *ad)
{
	std::vector<index_t> vr, vt;
	std::vector<double> vq, vtq;
	std::vector<std::vector<index_t>> mrows;
	ad->sortIndexCol();
	ad->compact();
	ad->sortIndexRow();
	double factor = 0;
	index_t pp = 0;
	index_t qq = 0;
	auto mp = ad->size();
	bool good = false;
	for (index_t kk = 0; kk < ad->rowLimit() - 1; ++kk)
	{
		vr.clear();
		vq.clear();
		while (ad->rowIndex(pp) == kk)
		{
			vr.push_back(ad->colIndex(pp));
			vq.push_back(ad->val(pp));
			++pp;
		}
		qq = pp;
		for (index_t nn = kk + 1; nn < ad->rowLimit(); ++nn)
		{
			vt.clear();
			vtq.clear();
			good = false;
			if (ad->colIndex(qq) != vr[0])
			{
				good = true;
			}
			while (ad->rowIndex(qq) == nn)
			{

				if (!good)
				{
					vt.push_back(ad->colIndex(qq));
					vtq.push_back(ad->val(qq));
				}
				++qq;
				if (qq >= mp)
				{
					break;
				}
			}
			if (!good)
			{
				continue;
			}
			if (vr.size() != vtq.size())
			{
				continue;
			}

			for (size_t jj = 0; jj < vr.size(); ++jj)
			{
				if (vt[jj] != vr[jj])
				{
					good = true;
					break;
				}
				else if (jj == 0)
				{
					factor = vtq[jj] / vq[jj];
				}
				else if (std::abs(vtq[jj] / vq[jj] - factor) > 0.000001)
				{
					good = true;
					break;
				}
			}
			if (good)
			{
				continue;
			}
			else
			{
				mrows.push_back({ kk,nn });
			}
		}
	}
	return mrows;
}

#include <fstream>
void arrayDataSparse::saveFile(double time, const std::string &filename, bool append)
{
	sortIndexCol();
	compact();
	std::ofstream  bFile;

	if (append)
	{
		bFile.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	}
	else
	{
		bFile.open(filename.c_str(), std::ios::out | std::ios::binary);
	}
	bFile.write((char *)(&time), sizeof(double));
	count_t count = size();
	bFile.write((char *)(&count), sizeof(count_t));
	for (auto &dve : data)
	{
		bFile.write((char *)(&(std::get<adRow>(dve))), sizeof(index_t));
		bFile.write((char *)(&(std::get<adCol>(dve))), sizeof(index_t));
		bFile.write((char *)(&(std::get<adVal>(dve))), sizeof(double));
	}
	bFile.close();
}
