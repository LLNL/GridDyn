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
#ifndef PVector_h
#define PVector_h

#include "Vector.h"

namespace griddyn {
namespace paradae {
class PVector : public Vector {
public:
  PVector():Vector(){};
  PVector(const Vector& v);
  virtual ~PVector(){};
  PVector& operator=(const Vector& v);
  PVector& operator=(const PVector& v);
  void Set(int m_, Real* data_);
};
} // namespace paradae
} // namespace griddyn

#endif
