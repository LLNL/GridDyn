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
#ifndef Newton_h
#define Newton_h

#include "../common/def.h"
#include "Solver.h"

namespace griddyn {
namespace paradae {
    enum NWT_error { NEWTON_NOT_CONVERGED, NEWTON_INF_NAN };

    class Newton: Solver {
        Real tol;
        bool newton_always_update_jac;

      public:
        Newton()
        {
            tol = -1;
            newton_always_update_jac = false;
        };
        Newton(int max_iter_, Real tol_ = -1, bool update = false);
        int Solve(Solver_App* app, Vector& x);
    };
}  // namespace paradae
}  // namespace griddyn

#endif
