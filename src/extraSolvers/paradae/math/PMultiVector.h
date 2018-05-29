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
#ifndef PMultiVector_h
#define PMultiVector_h

#include "PVector.h"

namespace griddyn {
namespace paradae {
class SMultiVector;
class SVector;

class PMultiVector : public PVector {
  int nx;
  int ns;
public:
  PMultiVector():PVector(),nx(1),ns(0){};
  PMultiVector(const SMultiVector& v);
  PMultiVector(const Vector& v, int ns_, int nx_);
  virtual ~PMultiVector(){};
  PMultiVector& operator=(const SMultiVector& v);
  PMultiVector& operator=(const PMultiVector& v);
  void GetPVector(int i, PVector& v)const;
  void GetSVector(int i, SVector& v)const;
  int GetXSize()const{return nx;};
  int GetSSize()const{return ns;};
};
} // namespace paradae
} // namespace griddyn

#endif
