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
#ifndef Ralston_h
#define Ralston_h

#include "RungeKutta_Explicit.h"
namespace griddyn {
namespace paradae {
/*!
  Butcher tableau:
  \f[\begin{array}{c|cc}
  0&&\\
  \frac{2}{3}&\frac{2}{3}&\\\hline
  (2)&\frac{1}{4}&\frac{3}{4}
  \end{array}
  \f]
 */
class Ralston : public RungeKutta_Explicit{
public:
  Ralston(Equation* eq);
  virtual std::string GetName(){return "RK_Ralston_2";};
};
} // namespace paradae
} // namespace griddyn

#endif
