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
#ifndef RungeKutta_Implicit_h
#define RungeKutta_Implicit_h

#include "../math/PVector.h"
#include "../math/SMultiVector.h"
#include "RungeKutta.h"
namespace griddyn {
namespace paradae {
    class RungeKutta_Implicit;

    class Solver_App_IRK: public Solver_App_RK {
      protected:
        Real tn, dt;
        PVector x0;
        DenseMatrix rk_A;
        PVector rk_c;
        Equation* equation;

      public:
        Solver_App_IRK(Real rtol,
                       const Vector& atol,
                       Real tn_,
                       Real dt_,
                       const Vector& x0_,
                       RungeKutta_Implicit* rk_);
        virtual ~Solver_App_IRK();
        virtual void
            EvaluateFunAndJac(const Vector& allK, Vector& gx, bool require_jac, bool factorize);
        virtual void dump() const;
    };

    class RungeKutta_Implicit: public RungeKutta {
      protected:
        VirtualMatrix* CurrentJacobian;
        virtual void InitArray();

      public:
        SMultiVector allK_previous;
        RungeKutta_Implicit(Equation* eq, bool varstep);
        virtual ~RungeKutta_Implicit();
        virtual bool SolveInnerSteps(Real t, Real used_dt, const Vector& x0, SMultiVector& allK);
        virtual Solver_App_RK* BuildSolverApp(Real t, Real dt, const Vector& x0);
        virtual VirtualMatrix* GetCurrentJac() { return CurrentJacobian; };
        virtual void SetDenseMatrix(bool dense_mat_ = true);
    };
}  // namespace paradae
}  // namespace griddyn

#endif
