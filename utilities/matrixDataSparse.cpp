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

#include "matrixDataSparse.h"



static const index_t kNullLocation;



matrixDataSparse::matrixDataSparse(index_t startCount)

/**
* function to clear the data
*/
void matrixDataSparse::clear()


void matrixDataSparse::assign(index_t X, index_t Y, double num)


count_t matrixDataSparse::size() const


count_t matrixDataSparse::capacity() const


void matrixDataSparse::sortIndex()


void matrixDataSparse::sortIndexCol()


void matrixDataSparse::sortIndexRow()

void matrixDataSparse::compact()


std::vector<count_t> matrixDataSparse::columnCount()


index_t matrixDataSparse::rowIndex(index_t N) const


index_t matrixDataSparse::colIndex(index_t N) const


double matrixDataSparse::val(index_t N) const


double matrixDataSparse::at(index_t rowN, index_t colN) const



void matrixDataSparse::start()


matrixElement<double> matrixDataSparse::next()


bool matrixDataSparse::moreData()


void matrixDataSparse::scale(double factor, index_t startIndex, count_t count)



void matrixDataSparse::translateRow(index_t origRow, index_t newRow)


void matrixDataSparse::filter(index_t rowTest)


void matrixDataSparse::translateCol(index_t origCol, index_t newCol)



void matrixDataSparse::scaleRow(index_t row, const double factor)


void matrixDataSparse::scaleCol(index_t col, const double factor)


void matrixDataSparse::copyTranslateRow(matrixDataSparse *a2, index_t origRow, index_t newRow)


void matrixDataSparse::copyTranslateCol(matrixDataSparse *a2, index_t origCol, index_t newCol)


void matrixDataSparse::merge(matrixDataSparse *a2)



void matrixDataSparse::cascade(matrixDataSparse *a2, index_t element)


void matrixDataSparse::copyReplicate(matrixDataSparse *a2, index_t origCol, std::vector<index_t> newIndices, std::vector<double> mult)


void matrixDataSparse::transpose()


void matrixDataSparse::diagMultiply(std::vector<double> diag)


std::vector<index_t> findMissing(matrixDataSparse *ad)
{

}

std::vector<double> matrixDataSparse::vectorMult(std::vector<double> V)


std::vector<std::vector<index_t>> findRank(matrixDataSparse *ad)


#include <fstream>
void matrixDataSparse::saveFile(double time, const std::string &filename, bool append)

