/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef BackwardEuler_h
#define BackwardEuler_h

#include "RungeKutta_DIRK.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|c}
  1&1\\\hline
  (1)&1
  \end{array}
  \f]
 */
    class BackwardEuler: public RungeKutta_DIRK {
      public:
        BackwardEuler(Equation* eq);
        virtual std::string GetName() { return "RK_BEuler_1"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
