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
#ifndef VirtualVector_h
#define VirtualVector_h

#include "../common/def.h"

namespace griddyn {
namespace paradae {
class Vector;

class VirtualVector {
public:
  virtual int GetM() const = 0;
  virtual Real* GetData() = 0;
  virtual ~VirtualVector(){};
  virtual void SetSubVec(int i, const Vector& vec) = 0;
  virtual Vector GetSubVec(int i, int l)const = 0;
};
} // namespace paradae
} // namespace griddyn

#endif
