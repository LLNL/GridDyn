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
