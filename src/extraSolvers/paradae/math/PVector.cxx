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
#include "PVector.h"
#include "SVector.h"
#include <iostream>
#include <cstring>

using namespace std;

PVector::PVector(const Vector& v)
{
  m=v.GetM();
  data=v.GetData();
}

PVector& PVector::operator=(const Vector& v)
{
  m=v.GetM();
  data=v.GetData();
  return *this;
}

PVector& PVector::operator=(const PVector& v)
{
  m=v.m;
  data=v.data;
  return *this;
}

void PVector::Set(int m_,Real* data_)
{
  m=m_;
  data=data_;
}
