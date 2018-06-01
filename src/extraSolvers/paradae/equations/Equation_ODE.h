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
#ifndef Equation_ODE_h
#define Equation_ODE_h

#include "Equation.h"

namespace griddyn {
namespace paradae {

class Equation_ODE : public Equation {
public:
  // Equation_ODE specific
  virtual void function_ode(const Real t, const Vector& y, const Vector& state, Vector& dy) = 0;
  virtual void jacobian_ode(const Real t, const Vector& y, const Vector& state, Matrix& Jy, bool add=false);
  void approx_jacobian_ode(const Real t, const Vector& y, const Vector& state, Matrix& J, bool add=false);
  void approx_jacobian_ode(const Real t, const Vector& y, const Vector& fy, const Vector& state, Matrix& J, bool add=false);

  // Redefinition of inherited virtual methods
  virtual ~Equation_ODE(){};
  virtual void function(const Real t, const Vector& y, const Vector& dy, const Vector& state, Vector& Fydy);
  virtual void jacobian_y(const Real t, const Vector& y, const Vector& dy, const Vector& state, Matrix& J, bool add=false);
  virtual void jacobian_dy(const Real t, const Vector& y, const Vector& dy, const Vector& state, Matrix& J, bool add=false);
  virtual void Get_dy_from_y(const Real t, const Vector& y, const Vector& state, Vector& dy){function_ode(t,y,state,dy);};
  virtual type_Equation GetTypeEq(){return ODE;};
};
} // namespace paradae
} // namespace griddyn

#endif
