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
#include "ImpMidPoint.h"
namespace griddyn {
namespace paradae {
    ImpMidPoint::ImpMidPoint(Equation* eq): RungeKutta_DIRK(eq, false)
    {
        nb_steps = 1;
        order = 2;
        this->InitArray();
        rk_A.Clone(DenseMatrix(nb_steps, Real(1. / 2.)));
        rk_b.Resize(nb_steps, 1.0);
        rk_binf.Resize(nb_steps);
        rk_c.Resize(nb_steps, 0.5);
    }
}  // namespace paradae
}  // namespace griddyn
