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
#ifndef Gauss4_h
#define Gauss4_h

#include "RungeKutta_Implicit.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|cc}
  \frac{1}{2}-\frac{\sqrt{3}}{6}&\frac{1}{4}&\frac{1}{4}-\frac{\sqrt{3}}{6}\\
  \frac{1}{2}+\frac{\sqrt{3}}{6}&\frac{1}{4}+\frac{\sqrt{3}}{6}&\frac{1}{4}\\\hline
  (4)&\frac{1}{2}&\frac{1}{2}
  \end{array}
  \f]
 */
    class Gauss4: public RungeKutta_Implicit {
      public:
        Gauss4(Equation* eq);
        virtual std::string GetName() { return "RK_Gauss_4"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
