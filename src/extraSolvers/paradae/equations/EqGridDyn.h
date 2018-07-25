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
#pragma once

#include "Equation_DAE_full.h"
#include "../common/MapParam.h"
#include <memory>
#include <vector>
namespace griddyn
{
    class gridDynSimulation;
    class solverMode;
}

using namespace std;
namespace griddyn {
namespace paradae {
/** @brief class that connects ParaDAE and GridDyn 
 */
class EquationGridDyn : public Equation_DAE_full {
  griddyn::gridDynSimulation *gds;
public:
  // EquationGridDyn specific
  EquationGridDyn(Real t0_, Real Tmax_, int N_unistep_, griddyn::gridDynSimulation *gds_, const Vector& y0_, griddyn::solverMode *mode_, 
                  vector<double> &discontinuities);
  static EquationGridDyn Default(const MapParam& param);
  griddyn::solverMode *mode;  //!< to the solverMode

  // Redefinition of inherited virtual methods
  virtual void function(const Real t, const Vector& y, const Vector& dy, const Vector& state, Vector& Fydy);
  virtual void jacobian_ypcdy(const Real t, const Vector& y, const Vector& dy, const Vector& state, const Real cj, Matrix& J);
  virtual void init(const Real t,Vector& y);
  virtual ~EquationGridDyn(){};
};

} // namespace paradae
} // namespace griddyn


