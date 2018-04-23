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
#include "FE_ExpTrap_12.h"

FE_ExpTrap_12::FE_ExpTrap_12(Equation* eq, bool variable_step):RungeKutta_Explicit(eq,variable_step)
{
  nb_steps=2;
  order=do_varstep?1:2;

  rk_A.Clone(DenseMatrix(nb_steps,Real(0.0)));
  rk_b.Resize(nb_steps);
  rk_binf.Resize(nb_steps);
  rk_c.Resize(nb_steps);

  rk_A(1,0)=Real(1.0);
  rk_b(0)=Real(1./2.);rk_b(1)=Real(1./2.);
  rk_binf(0)=Real(1.);rk_binf(1)=Real(0.);
  rk_c(1)=Real(1.);
}
