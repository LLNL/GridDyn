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
#include "Fudziah_45.h"
#include <cmath>

Fudziah_45::Fudziah_45(Equation* eq, bool variable_step):RungeKutta_DIRK(eq,variable_step)
{
  nb_steps=7;
  order=do_varstep?4:5;
  this->InitArray();
  rk_A.Clone(DenseMatrix(nb_steps,Real(0.0)));
  rk_b.Resize(nb_steps);
  rk_binf.Resize(nb_steps);
  rk_c.Resize(nb_steps);

  Real gamma=0.28589;
  for (int i=1;i<nb_steps;i++)
    rk_A(i,i)=gamma;
  rk_A(1,0)=gamma;
  rk_A(2,0)=gamma/2;rk_A(2,1)=(3./2.+sqrt(3))*gamma;
  rk_A(3,0)=0.168035987087646;rk_A(3,1)=-0.04941651019480583; rk_A(3,2)=-0.00450947689284040;
  rk_A(4,0)=0.182315003;rk_A(4,1)=-0.112951603;rk_A(4,2)=-0.027793233;rk_A(4,3)=0.422539833;
  rk_A(5,0)=0.247563917;rk_A(5,1)=-0.425378071;rk_A(5,2)=-0.107036282;rk_A(5,3)=0.395700134;rk_A(5,4)=0.503260302;
  rk_A(6,0)=0.130014275084996;rk_A(6,1)=0.0;rk_A(6,2)=-0.0192901771565916;rk_A(6,3)=0.5353862667089795;rk_A(6,4)=0.2343169293377270;rk_A(6,5)=-0.1663172939751104;
  rk_b(0)=0.130014275084996;rk_b(1)=0.0;rk_b(2)=-0.0192901771565916;rk_b(3)=0.5353862667089795;rk_b(4)=0.2343169293377270;rk_b(5)=-0.1663172939751104;rk_b(6)=gamma;

  rk_binf(0)= 0.153378753793186361936909376615;
  rk_binf(1)= 0.0;
  rk_binf(2)= 0.0214472335406866503858961155707;
  rk_binf(3)= 0.422558039204218496105737783549;
  rk_binf(4)= 0.402615973461908491571456724265;
  rk_binf(5)= 0.0;
  rk_binf(6)= 0.0;

  rk_c(0)=0.0;rk_c(1)=2*gamma;rk_c(2)=3*gamma+gamma*sqrt(3);rk_c(3)=0.4;rk_c(4)=0.75;rk_c(5)=0.9;rk_c(6)=1.0;
}
