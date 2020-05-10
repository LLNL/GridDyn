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
#ifndef Gauss6_h
#define Gauss6_h

#include "RungeKutta_Implicit.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|ccc}
  \frac{1}{2}-\frac{\sqrt{15}}{10}&\frac{5}{36}&\frac{2}{9}-\frac{\sqrt{15}}{15}&\frac{5}{36}-\frac{\sqrt{15}}{30}\\
  \frac{1}{2}&\frac{5}{36}+\frac{\sqrt{15}}{24}&\frac{2}{9}&\frac{5}{36}-\frac{\sqrt{15}}{24}\\
  \frac{1}{2}+\frac{\sqrt{15}}{10}&\frac{5}{36}+\frac{\sqrt{15}}{30}&\frac{2}{9}+\frac{\sqrt{15}}{15}&\frac{5}{36}\\\hline
  (6)&\frac{5}{18}&\frac{4}{9}&\frac{5}{18}
  \end{array}
  \f]
 */
    class Gauss6: public RungeKutta_Implicit {
      public:
        Gauss6(Equation* eq);
        virtual std::string GetName() { return "RK_Gauss_6"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
