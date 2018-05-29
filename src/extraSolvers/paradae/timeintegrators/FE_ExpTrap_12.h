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
#ifndef FE_ExpTrap_12_h
#define FE_ExpTrap_12_h

#include "RungeKutta_Explicit.h"

/*!
  Butcher tableau:
  \f[\begin{array}{c|cc}
  0&&\\
  1&1&\\\hline
  (2)&\frac{1}{2}&\frac{1}{2}\\
  (1)&1&0
  \end{array}
  \f]
 */
class FE_ExpTrap_12 : public RungeKutta_Explicit{
public:
  FE_ExpTrap_12(Equation* eq, bool variable_step=false);
  virtual std::string GetName(){return "RK_ExpFE_12";};
};

#endif
