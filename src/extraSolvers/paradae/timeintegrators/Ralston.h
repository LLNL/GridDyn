/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
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
    class Ralston: public RungeKutta_Explicit {
      public:
        Ralston(Equation* eq);
        virtual std::string GetName() { return "RK_Ralston_2"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
