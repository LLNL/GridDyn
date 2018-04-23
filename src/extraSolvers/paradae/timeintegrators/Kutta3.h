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

#include "RungeKutta_Explicit.h"

/*!
  Butcher tableau:
  \f[\begin{array}{c|ccc}
  0&&&\\
  \frac{1}{2}&\frac{1}{2}&&\\
  1&-1&2&\\\hline
  (3)&\frac{1}{6}&\frac{2}{3}&\frac{1}{6}
  \end{array}
  \f]
 */
class Kutta3 : public RungeKutta_Explicit{
public:
  Kutta3(Equation* eq);
  virtual std::string GetName(){return "RK_Kutta_3";};
};
