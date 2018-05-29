/*
* LLNS Copyright Start
* Copyright (c) 2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#ifndef paradaeArrayData_h
#define paradaeArrayData_h

#include "utilities/matrixData.hpp"
#include "../math/SparseMatrix.h"

namespace griddyn {
namespace paradae {
class paradaeArrayData : public matrixData<double>
{
 private:
  SparseMatrix* J=nullptr;
 public:
  paradaeArrayData(){};
  paradaeArrayData(SparseMatrix* mat);

  void clear () ;
  void assign (index_t X, index_t Y, double num) ;
  void setMatrix (SparseMatrix* mat);
  count_t size () const ;
  count_t capacity () const ;
  index_t rowIndex (index_t N) const ;
  index_t colIndex (index_t N) const ;
  double val (index_t N) const ;
  double at (index_t rowN, index_t colN) const ;
  matrixElement<double> element (index_t N) const;
};
} // namespace paradae
} // namespace griddyn

#endif
