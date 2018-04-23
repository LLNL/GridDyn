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
#ifndef Equation_DAE_full_h
#define Equation_DAE_full_h

#include "Equation_DAE.h"

class Equation_DAE_full : public Equation_DAE {
public:
  // Redefinition of inherited virtual methods
  virtual ~Equation_DAE_full(){};
  virtual void Get_dy_from_y(Real t, const Vector& y, const Vector& state, Vector& dy){abort();};
};

#endif
