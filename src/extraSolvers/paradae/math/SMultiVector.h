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
#ifndef SMultiVector_h
#define SMultiVector_h

#include "SVector.h"
class PVector;

class SMultiVector : public SVector {
  int nx;
  int ns;
public:
  SMultiVector():SVector(),nx(1),ns(0){};
  SMultiVector(int ns_, int nx_, Real fill_=0);
  SMultiVector(const SMultiVector& mv);
  SMultiVector(const Vector& v);
  virtual ~SMultiVector(){};
  void Free();
  void Resize(int ns_, int nx_, Real fill_=0);
  SMultiVector& operator=(const SMultiVector& v);
  SMultiVector& operator=(const Vector& v);
  void GetPVector(int i, PVector& v)const;
  void GetSVector(int i, SVector& v)const;
  void PushFront(const SVector& v);
  void PushBack(const SVector& v);
  void PushBack(Real v);
  void PushBack(int v){PushBack(Real(v));};
  void PopFront(SVector& v);
  void PopBack(SVector& v);
  void PopFront();
  void PopBack();
  void PushAndPop(const SVector& v);
  void PushAndPop(Real v);
  void PushAndPop(int v){PushAndPop(Real(v));};
  int GetXSize()const{return nx;};
  int GetSSize()const{return ns;};
};

#endif
