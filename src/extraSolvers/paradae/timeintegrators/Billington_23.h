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
#ifndef Billington_23_h
#define Billington_23_h

#include "RungeKutta_SDIRK.h"

namespace griddyn {
namespace paradae {
/*!
  Butcher tableau:
  \f[\begin{array}{c|ccc}
  1-\frac{\sqrt{2}}{2}&1-\frac{\sqrt{2}}{2}&&\\
  -18+27\frac{\sqrt{2}}{2}&-19+14\sqrt{2}&1-\frac{\sqrt{2}}{2}&\\
  2-\frac{\sqrt{2}}{2}&\frac{53-5\sqrt{2}}{62}&\frac{9+5\sqrt{2}}{62}&1-\frac{\sqrt{2}}{2}\\\hline
  (3)&\frac{263-95\sqrt{2}}{186}&\frac{47+33\sqrt{2}}{186}&\frac{-2+\sqrt{2}}{3}\\
  (2)&\frac{53-5\sqrt{2}}{62}&\frac{9+5\sqrt{2}}{62}&
  \end{array}
  \f]
 */
class Billington_23 : public RungeKutta_SDIRK {
public:
  Billington_23(Equation* eq, bool variable_step=false);
  virtual std::string GetName(){return "RK_ImpBi_23";};
};
} // namespace paradae
} // namespace griddyn

#endif
