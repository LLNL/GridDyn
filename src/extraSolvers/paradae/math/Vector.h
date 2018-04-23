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
#pragma once

#include <iostream>
#include <cstring>
#include "../common/def.h"

class Vector {
protected:
  int m;
  Real* data;
public:
  Vector():m(0),data(nullptr){};
  virtual ~Vector(){};
  void Fill(Real fill_=0);

  Real operator()(int i) const;
  Real& operator()(int i);
  Vector& operator+=(const Vector& v);
  Vector& operator-=(const Vector& v);
  Vector& operator*=(Real alpha);
  Vector& AXPBY(Real alpha, Real beta, const Vector& x);
  void CopyData(const Vector& v);

  Real Norm2() const;
  Real NormInf() const;
  int GetM()const{return m;};
  Real* GetData()const{return data;};

  void dump() const;
  void dump(std::ostream& output) const;
  void dump(std::string filename) const;
};

std::ostream& operator<<(std::ostream& output, const Vector& vec);

