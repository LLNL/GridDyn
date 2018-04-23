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
#ifndef Radau3_h
#define Radau3_h

#include "RungeKutta_Implicit.h"

/*!
  Butcher tableau:
  \f[\begin{array}{c|cc}
  \frac{1}{3}&\frac{5}{12}&-\frac{1}{12}\\
  1&\frac{3}{4}&\frac{1}{4}\\\hline
  (3)&\frac{3}{4}&\frac{1}{4}
  \end{array}
  \f]
 */
class Radau3 : public RungeKutta_Implicit{
public:
  Radau3(Equation* eq);
  virtual std::string GetName(){return "RK_Radau_3";};
};

#endif
