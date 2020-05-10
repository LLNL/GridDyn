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
#include "Kutta3.h"
namespace griddyn {
namespace paradae {
    Kutta3::Kutta3(Equation* eq): RungeKutta_Explicit(eq, false)
    {
        nb_steps = 3;
        order = 3;
        rk_A.Clone(DenseMatrix(nb_steps, Real(0.0)));
        rk_b.Resize(nb_steps);
        rk_binf.Resize(nb_steps);
        rk_c.Resize(nb_steps);

        rk_A(1, 0) = Real(1. / 2.);
        rk_A(2, 0) = Real(-1.);
        rk_A(2, 1) = Real(2.);
        rk_b(0) = Real(1. / 6.);
        rk_b(1) = Real(2. / 3.);
        rk_b(2) = Real(1. / 6.);
        rk_c(0) = Real(0.);
        rk_c(1) = Real(1. / 2.);
        rk_c(2) = Real(1.);
    }
}  // namespace paradae
}  // namespace griddyn
