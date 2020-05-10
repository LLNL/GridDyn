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
#ifndef RungeKutta_Explicit_h
#define RungeKutta_Explicit_h

#include "RungeKutta.h"
namespace griddyn {
namespace paradae {
    class RungeKutta_Explicit: public RungeKutta {
      public:
        RungeKutta_Explicit(Equation* eq, bool varstep);
        ~RungeKutta_Explicit(){};
        virtual bool SolveInnerSteps(Real t, Real used_dt, const Vector& x0, SMultiVector& allK);
    };
}  // namespace paradae
}  // namespace griddyn

#endif
