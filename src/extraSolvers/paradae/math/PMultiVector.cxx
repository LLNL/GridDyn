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
#include "PMultiVector.h"
#include "SMultiVector.h"
#include "SVector.h"

namespace griddyn {
namespace paradae {
using namespace std;

PMultiVector::PMultiVector(const SMultiVector& v)
{
  m=v.GetM();
  data=v.GetData();
  nx=v.GetXSize();
  ns=v.GetSSize();
}

PMultiVector::PMultiVector(const Vector& v, int ns_, int nx_)
{
#ifdef CHECK_MEM_OP
  if ( ns_*nx_!=v.GetM() )
    {
      cerr << "Error in PMultiVector::PMultiVector(v[" << v.GetM() << "]," << ns_ << "," << nx_ << ")" << endl;
      abort();
    }
#endif
  m=v.GetM();
  data=v.GetData();
  nx=nx_;
  ns=ns_;
}

PMultiVector& PMultiVector::operator=(const SMultiVector& v)
{
  m=v.GetM();
  data=v.GetData();
  nx=v.GetXSize();
  ns=v.GetSSize();
  return *this;
}

PMultiVector& PMultiVector::operator=(const PMultiVector& v)
{
  m=v.m;
  data=v.data;
  nx=v.nx;
  ns=v.ns;
  return *this;
}

void PMultiVector::GetPVector(int i, PVector& v) const
{
#ifdef CHECK_MEM_OP
  if ( i<0 || i>=ns )
    {
      cerr << "Error in PMultiVector[" << ns << "," << nx << "]::GetPVector(" << i << ")" << endl;
      abort();
    }
#endif
  v.Set(nx,data+i*nx);
}

void PMultiVector::GetSVector(int i, SVector& v) const
{
#ifdef CHECK_MEM_OP
  if ( i<0 || i>=ns )
    {
      cerr << "Error in PMultiVector[" << ns << "," << nx << "]::GetSVector(" << i << ")" << endl;
      abort();
    }
#endif
  v.Resize(nx);
  memcpy(v.GetData(),data+i*nx,nx*sizeof(Real));
}
} // namespace paradae
} // namespace griddyn
