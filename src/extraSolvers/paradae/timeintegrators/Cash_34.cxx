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
#include "Cash_34.h"

//  0.435866521508  | 0.435866521508
// -0.7             |-1.13586652150   0.435866521508
//  0.8             | 1.08543330679  -0.721299828287   0.435866521508
//  0.924556761814  | 0.416349501547  0.190984004184  -0.118643265417   0.435866521508
//  1.0             | 0.896869652944  0.0182725272734 -0.0845900310706 -0.266418670647  0.435866521508
//  ---------------------------------------------------------------------------------------------------
//                4 | 0.896869652944  0.0182725272734 -0.0845900310706 -0.266418670647  0.435866521508
//                3 | 0.776691932910  0.0297472791484 -0.0267440239074  0.220304811849  0.0
namespace griddyn {
namespace paradae {
    Cash_34::Cash_34(Equation* eq, bool variable_step): RungeKutta_SDIRK(eq, variable_step)
    {
        nb_steps = 5;
        order = do_varstep ? 3 : 4;
        this->InitArray();
        rk_A.Clone(DenseMatrix(nb_steps, Real(0.0)));
        rk_b.Resize(nb_steps);
        rk_binf.Resize(nb_steps);
        rk_c.Resize(nb_steps);

        rk_A(0, 0) = 0.435866521508;
        rk_A(1, 0) = -1.13586652150;
        rk_A(2, 0) = 1.08543330679;
        rk_A(3, 0) = 0.416349501547;
        rk_A(4, 0) = 0.896869652944;
        rk_A(1, 1) = 0.435866521508;
        rk_A(2, 1) = -0.721299828287;
        rk_A(3, 1) = 0.190984004184;
        rk_A(4, 1) = 0.0182725272734;
        rk_A(2, 2) = 0.435866521508;
        rk_A(3, 2) = -0.118643265417;
        rk_A(4, 2) = -0.0845900310706;
        rk_A(3, 3) = 0.435866521508;
        rk_A(4, 3) = -0.266418670647;
        rk_A(4, 4) = 0.435866521508;
        rk_b(0) = 0.896869652944;
        rk_b(1) = 0.0182725272734;
        rk_b(2) = -0.0845900310706;
        rk_b(3) = -0.266418670647;
        rk_b(4) = 0.435866521508;
        rk_binf(0) = 0.776691932910;
        rk_binf(1) = 0.0297472791484;
        rk_binf(2) = -0.0267440239074;
        rk_binf(3) = 0.220304811849;
        rk_c(0) = 0.435866521508;
        rk_c(1) = -0.7;
        rk_c(2) = 0.8;
        rk_c(3) = 0.924556761814;
        rk_c(4) = 1.0;
    }
}  // namespace paradae
}  // namespace griddyn
