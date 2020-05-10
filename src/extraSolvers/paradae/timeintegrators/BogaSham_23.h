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
#ifndef BogaSham_23_h
#define BogaSham_23_h

#include "RungeKutta_Explicit.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|cccc}
  0&&&&\\
  \frac{1}{2}&\frac{1}{2}&&&\\
  \frac{3}{4}&0&\frac{3}{4}&&\\
  1&\frac{2}{9}&\frac{1}{3}&\frac{4}{9}&\\\hline
  (3)&\frac{2}{9}&\frac{1}{3}&\frac{4}{9}&0\\
  (2)&\frac{7}{24}&\frac{1}{4}&\frac{1}{3}&\frac{1}{8}
  \end{array}
  \f]
 */
    class BogaSham_23: public RungeKutta_Explicit {
      public:
        BogaSham_23(Equation* eq, bool variable_step = false);
        virtual std::string GetName() { return "RK_ExpBS_23"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
