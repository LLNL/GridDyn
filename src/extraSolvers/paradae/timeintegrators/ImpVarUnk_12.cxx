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
#include "ImpVarUnk_12.h"
namespace griddyn {
namespace paradae {
    ImpVarUnk_12::ImpVarUnk_12(Equation* eq, bool variable_step): RungeKutta_DIRK(eq, variable_step)
    {
        nb_steps = 4;
        order = do_varstep ? 1 : 2;
        this->InitArray();
        rk_A.Clone(DenseMatrix(nb_steps, Real(0.0)));
        rk_b.Resize(nb_steps);
        rk_binf.Resize(nb_steps);
        rk_c.Resize(nb_steps);

        rk_A(0, 0) = 1.;
        rk_A(0, 1) = 0;
        rk_A(0, 2) = 0;
        rk_A(0, 3) = 0;
        rk_A(1, 0) = 1.;
        rk_A(1, 1) = 0;
        rk_A(1, 2) = 0;
        rk_A(1, 3) = 0;
        rk_A(2, 0) = -88. / 45.;
        rk_A(2, 1) = 0;
        rk_A(2, 2) = 12. / 5.;
        rk_A(2, 3) = 0;
        rk_A(3, 0) = -407. / 75.;
        rk_A(3, 1) = 0;
        rk_A(3, 2) = 144. / 25.;
        rk_A(3, 3) = 0;
        rk_b(0) = 1. / 10.;
        rk_b(1) = 0;
        rk_b(2) = 9. / 10.;
        rk_b(3) = 0;
        rk_c(0) = 1;
        rk_c(1) = 1;
        rk_c(2) = 4. / 9.;
        rk_c(3) = 1. / 3.;
        rk_binf(0) = 1;
        rk_binf(1) = 0;
        rk_binf(2) = 0;
        rk_binf(3) = 0;
    }
}  // namespace paradae
}  // namespace griddyn
