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
#ifndef IVanderExt_h
#define IVanderExt_h

#include "DenseMatrix.h"
#include "Vector.h"
#include "SMultiVector.h"

class IVanderExt : public DenseMatrix {
  void Build2();
  void Build3();
  void Build4();
  void Build5();
  void Build6();
  void Derivate(DenseMatrix& M) const;
public:
  IVanderExt(int n);
  void Interp(const SMultiVector& xn, const Vector& dx, SMultiVector& new_xn, Vector& new_dx, Real dt, Real Dt) const;
};

#endif
