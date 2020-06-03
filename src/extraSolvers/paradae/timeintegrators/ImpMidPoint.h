/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ImpMidPoint_h
#define ImpMidPoint_h

#include "RungeKutta_DIRK.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|c}
  \frac{1}{2}&\frac{1}{2}\\\hline
  (2)&1
  \end{array}
  \f]
 */
    class ImpMidPoint: public RungeKutta_DIRK {
      public:
        ImpMidPoint(Equation* eq);
        virtual std::string GetName() { return "RK_ImpMidPoint_2"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
