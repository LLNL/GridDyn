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
#include "RungeKutta_Explicit.h"

#include "../math/PVector.h"
namespace griddyn {
namespace paradae {
    RungeKutta_Explicit::RungeKutta_Explicit(Equation* eq, bool varstep): RungeKutta(eq, varstep) {}

    bool RungeKutta_Explicit::SolveInnerSteps(Real t,
                                              Real used_dt,
                                              const Vector& x0,
                                              SMultiVector& allK)
    {
        SVector xi(x0);
        PVector Kj;

        for (int i = 0; i < nb_steps; i++) {
            xi.CopyData(x0);
            for (int j = 0; j < i; j++) {
                allK.GetPVector(j, Kj);
                xi.AXPBY(used_dt * rk_A(i, j), 1.0, Kj);
            }

            allK.GetPVector(i, Kj);
            equation->Get_dy_from_y(t + rk_c(i) * used_dt, xi, pstate, Kj);
        }

        return true;
    }
}  // namespace paradae
}  // namespace griddyn
