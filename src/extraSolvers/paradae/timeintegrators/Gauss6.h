/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
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
