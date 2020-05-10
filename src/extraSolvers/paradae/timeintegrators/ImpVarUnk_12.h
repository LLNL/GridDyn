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
#pragma once

#include "RungeKutta_DIRK.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|cccc}
  1&1&&&\\
  1&1&&&\\
  \frac{4}{9}&-\frac{88}{45}&0&\frac{12}{5}&\\
  \frac{1}{3}&-\frac{407}{75}&0&\frac{144}{25}&\\\hline
  (2)&\frac{1}{10}&0&\frac{9}{10}&0\\
  (1)&1&0&0&0
  \end{array}
  \f]
 */
    class ImpVarUnk_12: public RungeKutta_DIRK {
      public:
        ImpVarUnk_12(Equation* eq, bool variable_step = false);
        virtual std::string GetName() { return "RK_Imp_12"; };
    };
}  // namespace paradae
}  // namespace griddyn
