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
#include "EqGridDyn.h"
#include "../math/paradaeArrayData.h"
#include "../common/Timer.h"
#include "griddyn/gridDynSimulation.h"
#include "fileInput/fileInput.h"
#include "griddyn/simulation/gridSimulation.h"
#include "coupling/GhostSwingBus.h"
#include "runner/gridDynRunner.h"

#include <boost/program_options.hpp>
#include <boost/container/small_vector.hpp>
#include <cmath>

namespace griddyn {
namespace paradae {

using namespace std;
using namespace griddyn;

EquationGridDyn::EquationGridDyn(Real t0_, Real Tmax_, int N_unistep_, gridDynSimulation *gds_, const Vector& y0_, solverMode *mode_)

{
  //n=gds_->stateSize(cDaeSolverMode);
  n=gds_->stateSize(*mode_);
  t0=t0_;
  Tmax=Tmax_;
  N_unistep=N_unistep_;
  y0.Resize(n);
  y0.CopyData(y0_);
  if (gds_==NULL)
    {
      cerr << "Calling constructor with noninitialised gridDynSimulation pointer..." << endl;
      abort();
    }
  gds=gds_;
  mode = mode_;
  name="EquationGridDyn";

  // Matt's original code
  //roots=RootManager(1,1,0,1e-10);
  //roots.is_active(0)=1;
  //roots.n_sactive=1;
  //roots.t_sroot(0)=5;

  // Code for up to 100 square pulse discontinuities
  roots=RootManager(100,1,0,1e-10);
  roots.n_sactive=100;
  for (int i=0; i<100; i++)
  {
      roots.is_active(i) = 1; 
      roots.t_sroot(i) = 0.5 + i;
  }
  
  // This will print out all of the variable names
  //stringVec stNames;
  //gds->getStateName(stNames, cDaeSolverMode);
  //cout << "\nVariable Names\n";
  //for(int ii; ii< stNames.size(); ii++)
  //{
  //   cout << stNames[ii] << "\n";
  //}
  //cout << "\n\n";
}

void EquationGridDyn::function(const Real t, const Vector& y, const Vector& dy, const Vector& state, Vector& Fydy)
{
  nb_calls++;
  //gds->residualFunction(t,y.GetData(),dy.GetData(),Fydy.GetData(),cDaeSolverMode);
  gds->residualFunction(t,y.GetData(),dy.GetData(),Fydy.GetData(), *mode);
}

void EquationGridDyn::jacobian_ypcdy(const Real t, const Vector& y, const Vector& dy, const Vector& state, const Real cj,Matrix& J)
{
  //J.dump(std::cout);
  nb_calls_jac++;
  SparseMatrix& pJ=dynamic_cast<SparseMatrix&>(J);
  paradaeArrayData a1(&pJ);
  //gds->jacobianFunction(t,y.GetData(),dy.GetData(),a1,cj,cDaeSolverMode);
  gds->jacobianFunction(t,y.GetData(),dy.GetData(),a1,cj,*mode);
  //static int switcheroo = 0;
  //if(switcheroo == 0)
  //{
  //   std::cout << "\n\n  Time value " << t << "\n\n";
  //   J.dump("new_jac.txt");
  //   switcheroo = 1;
  //}
}

void EquationGridDyn::init(const Real t,Vector& y)
{
  y.CopyData(y0);
}
/*
void EquationGridDyn::root_init_state(const Real t, Vector& state)
{
  if (roots.is_active(0))
    if (t>=roots.t_sroot(0))
      state(0)=1;
}
*/

} // namespace paradae
} // namespace griddyn
