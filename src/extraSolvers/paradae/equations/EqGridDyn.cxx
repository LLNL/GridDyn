/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "EqGridDyn.h"

#include "../common/Timer.h"
#include "../math/paradaeArrayData.h"
#include "coupling/GhostSwingBus.h"
#include "fileInput/fileInput.h"
#include "griddyn/gridDynSimulation.h"
#include "griddyn/simulation/gridSimulation.h"
#include "runner/gridDynRunner.h"
#include <cmath>

#include <boost/container/small_vector.hpp>
#include <boost/program_options.hpp>

namespace griddyn {
namespace paradae {

    using namespace std;
    using namespace griddyn;

    EquationGridDyn::EquationGridDyn(Real t0_,
                                     Real Tmax_,
                                     int N_unistep_,
                                     gridDynSimulation* gds_,
                                     const Vector& y0_,
                                     solverMode* mode_,
                                     vector<double>& discontinuities,
                                     vector<int>& rootsfound)
    {
        // n=gds_->stateSize(cDaeSolverMode);
        n = gds_->stateSize(*mode_);
        t0 = t0_;
        Tmax = Tmax_;
        N_unistep = N_unistep_;
        y0.Resize(n);
        y0.CopyData(y0_);
        if (gds_ == NULL) {
            cerr << "Calling constructor with noninitialised gridDynSimulation pointer..." << endl;
            abort();
        }
        gds = gds_;
        mode = mode_;
        name = "EquationGridDyn";

        // Old code setting up 100 square pulse discontinuities at t=[0.5, 1.5, 2.5, ...]
        // roots=RootManager(100,1,0,1e-10);
        // roots.n_sactive=100;
        // for (int i=0; i<100; i++)
        //{
        //    roots.is_active(i) = 1;
        //    roots.t_sroot(i) = 0.5 + i;
        //}

        // New code, import discontinuity locations from XML file
        roots = RootManager(discontinuities.size(), rootsfound.size(), 0, 1e-10);
        roots.n_sactive = discontinuities.size();
        roots.n_uactive = rootsfound.size();
        for (int i = 0; i < discontinuities.size(); i++) {
            roots.is_active(i) = 1;
            roots.t_sroot(i) = discontinuities[i];
        }
        for (int i = discontinuities.size(); i < rootsfound.size(); i++) {
            roots.is_active(i) = 1;
        }

        // This will print out all of the variable names
        stringVec stNames;
        gds_->getStateName(stNames, *mode_);
        cout << "\nVariable Names\n";
        for(int ii; ii< stNames.size(); ii++)
        {
          cout << stNames[ii] << "\n";
        }
        cout << "\n\n";

        limits = LimitManager(rootsfound.size());

    }

    void EquationGridDyn::function(const Real t,
                                   const Vector& y,
                                   const Vector& dy,
                                   const Vector& state,
                                   Vector& Fydy)
    {
        nb_calls++;
        // gds->residualFunction(t,y.GetData(),dy.GetData(),Fydy.GetData(),cDaeSolverMode);
        gds->residualFunction(t, y.GetData(), dy.GetData(), Fydy.GetData(), *mode);
    }

    void EquationGridDyn::jacobian_ypcdy(const Real t,
                                         const Vector& y,
                                         const Vector& dy,
                                         const Vector& state,
                                         const Real cj,
                                         Matrix& J)
    {
        // J.dump(std::cout);
        nb_calls_jac++;
        SparseMatrix& pJ = dynamic_cast<SparseMatrix&>(J);
        paradaeArrayData a1(&pJ);
        // gds->jacobianFunction(t,y.GetData(),dy.GetData(),a1,cj,cDaeSolverMode);
        gds->jacobianFunction(t, y.GetData(), dy.GetData(), a1, cj, *mode);
        // static int switcheroo = 0;
        // if(switcheroo == 0)
        //{
        //   std::cout << "\n\n  Time value " << t << "\n\n";
        //   J.dump("new_jac.txt");
        //   switcheroo = 1;
        //}
    }

    void EquationGridDyn::init(const Real t, Vector& y) { y.CopyData(y0); }
    /*
void EquationGridDyn::root_init_state(const Real t, Vector& state)
{
  if (roots.is_active(0))
    if (t>=roots.t_sroot(0))
      state(0)=1;
}
*/


    void EquationGridDyn::root_functions(const Real t,
                                         const Vector& y,
                                         const Vector& dy,
                                         const Vector& state,
                                         Vector& rv)
    {
        //std::cout << "EquationGridDyn::root_functions" << std::endl;
        gds->rootFindingFunction(t, y.GetData(), dy.GetData(), rv.GetData(),
                                 *mode);
    };

    void EquationGridDyn::root_action(const Real troot,
                                      const Vector& yroot,
                                      const Vector& dyroot,
                                      const Vector& iroot)
    {
        //std::cout << "EquationGridDyn::root_action" << std::endl;
        // std::cout << "Scheduled roots " << roots.n_sroots << std::endl;
        // std::cout << "Unscheduled roots " << roots.n_uroots << std::endl;
        std::vector<int> rootMask(iroot.GetData() + roots.n_sroots,
                                  iroot.GetData() + roots.n_sroots + roots.n_uroots);
        gds->rootActionFunction(troot, yroot.GetData(), dyroot.GetData(),
                                rootMask, *mode);
    };


    void EquationGridDyn::limit_functions(const Vector& y,
                                          const Vector& dy,
                                          Vector& flimit)
    {
        // std::cout << "ParaDAE::EquationGridDyn::limit_functions" << std::endl;
        // Need to add time as an input here? for calling stateData sD(time,...) in GridDyn
        Real t = 0.0;
        gds->limitCheckingFunction(t, y.GetData(), dy.GetData(),
                                   flimit.GetData(), *mode);
    };


    void EquationGridDyn::limit_crossings(const Real time,
                                          Vector &y,
                                          Vector &dy,
                                          const Vector& flimit)
    {
        std::vector<int> limitMask(flimit.GetData(),
                                   flimit.GetData() + limits.n_limits);
        gds->limitActionFunction(time, y.GetData(), dy.GetData(), limitMask, *mode);
    }

    // NEED TO CHANGE THIS FUNCTION?
    // Should we pass vectors or gds? Is DynData same as the gds we have stored
    // I dont think so dyndata is the solver interface?
    // can use setState?
    // If pass vectors should this look more like rootfindingfunction?
    // void EquationGridDyn::limit_crossing(const Real time, Vector &y, Vector &dy, Vector& flimit)
    // {
    //     Real t = 0.0;
    //     gds->handleLimitViolation(3, t,
    // }

}  // namespace paradae
}  // namespace griddyn

// need to call root trigger some where?
// does root trigger need to propagate anything back to ParaDAE? It should not
// need to since we'll contine from the last state at the root. So really,
// rootTrigger should just adjust the RHS/Jacobian as needed.

// handleLimitViolation will need to update the state in GridDyn? Maybe not
// since we use getData to pass the raw arrays back into GridDyn.
