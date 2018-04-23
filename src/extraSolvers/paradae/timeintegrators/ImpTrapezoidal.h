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
#ifndef ImpTrapezoidal_h
#define ImpTrapezoidal_h

#include "RungeKutta_DIRK.h"

/*!
  Butcher tableau:
  \f[\begin{array}{c|cc}
  0&&\\
  1&\frac{1}{2}&\frac{1}{2}\\\hline
  (2)&\frac{1}{2}&\frac{1}{2}
  \end{array}
  \f]
 */
class ImpTrapezoidal : public RungeKutta_DIRK {
public:
  ImpTrapezoidal(Equation* eq);
  virtual std::string GetName(){return "RK_ImpTrap_2";};
};

#endif
