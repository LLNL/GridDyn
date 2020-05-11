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
#ifndef ForwardEuler_h
#define ForwardEuler_h

#include "RungeKutta_Explicit.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|c}
  0&\\\hline
  (1)&1
  \end{array}
  \f]
 */
    class ForwardEuler: public RungeKutta_Explicit {
      public:
        ForwardEuler(Equation* eq);
        virtual std::string GetName() { return "RK_FEuler_1"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
