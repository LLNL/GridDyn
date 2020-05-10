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
#ifndef DormPrince_45_h
#define DormPrince_45_h

#include "RungeKutta_Explicit.h"
namespace griddyn {
namespace paradae {
    /*!
  Butcher tableau:
  \f[\begin{array}{c|ccccccc}
  0&&&&&&&\\
  \frac{1}{5}&\frac{1}{5}&&&&&&\\
  \frac{3}{10}&\frac{3}{40}&\frac{9}{40}&&&&&\\
  \frac{4}{5}&\frac{44}{45}&-\frac{56}{15}&\frac{32}{9}&&&&\\
  \frac{8}{9}&\frac{19372}{6561}&-\frac{25360}{2187}&\frac{64448}{6561}&-\frac{212}{729}&&&\\
  1&\frac{9017}{3168}&-\frac{355}{33}&\frac{46732}{5247}&\frac{49}{176}&-\frac{5103}{18656}&&\\
  1&\frac{35}{384}&0&\frac{500}{1113}&\frac{125}{192}&-\frac{2187}{6784}&\frac{11}{84}&\\\hline
  (5)&\frac{35}{384}&0&\frac{500}{1113}&\frac{125}{192}&-\frac{2187}{6784}&\frac{11}{84}&0\\
  (4)&\frac{5179}{57600}&0&\frac{7571}{16695}&\frac{393}{640}&-\frac{92097}{339200}&\frac{187}{2100}&\frac{1}{40}
  \end{array}
  \f]
 */
    class DormPrince_45: public RungeKutta_Explicit {
      public:
        DormPrince_45(Equation* eq, bool variable_step = false);
        virtual std::string GetName() { return "RK_ExpDP_45"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
