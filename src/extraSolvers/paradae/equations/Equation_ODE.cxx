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
#include "Equation_ODE.h"

void Equation_ODE::function(const Real t, const Vector& y, const Vector& dy, const Vector& state, Vector& Fydy)
{
  function_ode(t,y,state,Fydy);
  Fydy.AXPBY(-1.0,1.0,dy);
}

void Equation_ODE::jacobian_dy(const Real t, const Vector& y, const Vector& dy, const Vector& state, Matrix& J, bool add)
{
  if (add)
    {
      for (int i=0;i<y.GetM();i++)
	J(i,i)+=Real(-1.0);
    }
  else
    {
      J.Fill(0.0);
      for (int i=0;i<y.GetM();i++)
	J(i,i)=Real(-1.0);
    }
}

void Equation_ODE::jacobian_y(const Real t, const Vector& y, const Vector& dy, const Vector& state, Matrix& J, bool add)
{
  jacobian_ode(t,y,state,J,add);
}

void Equation_ODE::jacobian_ode(const Real t, const Vector& y, const Vector& state, Matrix& Jy, bool add)
{
  approx_jacobian_ode(t,y,state,Jy,add);
}


void Equation_ODE::approx_jacobian_ode(const Real t, const Vector& y, const Vector& state, Matrix& J, bool add)
{
  SVector fy(y.GetM(),0.0);
  function_ode(t,y,state,fy);
  approx_jacobian_ode(t,y,fy,state,J,add);
}

void Equation_ODE::approx_jacobian_ode(const Real t, const Vector& y, const Vector& fy, const Vector& state, Matrix& J, bool add)
{
  Real h=y.Norm2()/(1000.*n);
  SVector yh(y);
  SVector fyh(n,0.0);

  if (h<1e-7)
    h=1e-7;
  for (int j=0;j<n;j++)
    {
      yh(j)+=h;
      function_ode(t,yh,state,fyh);
      fyh.AXPBY(-1.0/h,1.0/h,fy);

      for (int i=0;i<n;i++)
	{
	  if (add)
	    J(i,j)+=fyh(i);
	  else
	    J(i,j)=fyh(i);
	}
      yh(j)-=h;
    }
}
