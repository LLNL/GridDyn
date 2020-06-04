/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SDIRK_12_h
#define SDIRK_12_h

#include "RungeKutta_SDIRK.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|cc}
  1&1&\\
  0&-1&1\\\hline
  (2)&\frac{1}{2}&\frac{1}{2}\\
  (1)&1&0
  \end{array}
  \f]
 */
    class SDIRK_12: public RungeKutta_SDIRK {
      public:
        SDIRK_12(Equation* eq, bool variable_step = false);
        virtual std::string GetName() { return "RK_SDIRK_12"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
