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
#ifndef SVector_h
#define SVector_h

#include "Vector.h"

namespace griddyn {
namespace paradae {
class PVector;

class SVector : public Vector {
public:
  SVector():Vector(){};
  SVector(int m_, Real fill_=0);
  SVector(const SVector& v):SVector((Vector)v){};
  SVector(const Vector& v);
  virtual ~SVector();
  void Free();
  void Resize(int m_, Real fill_=0);
  void Append(Real alpha);
  bool operator==(const SVector& v)const;
  SVector& operator=(const SVector& v);
  SVector& operator=(const PVector& v);

  static SVector Rand(int n, Real a=0, Real b=1);
};
} // namespace paradae
} // namespace griddyn

#endif
