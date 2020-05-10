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
#ifndef RungeKutta_hxx
#define RungeKutta_hxx

#include "../equations/Equation.h"
#include "../math/DenseMatrix.h"
#include "../math/SMultiVector.h"
#include "../math/SVector.h"
#include "../math/Vector.h"
#include "../solvers/Solver.h"
#include "../timeintegrators/TimeIntegrator.h"
#include <list>
namespace griddyn {
namespace paradae {
    class RungeKutta;

    class Solver_App_RK: public Solver_App {
      protected:
        Real rtol;
        SMultiVector atol;
        PVector pstate;

      public:
        Solver_App_RK(Real rtol_, const Vector& atol_, const Vector& x0_, RungeKutta* rk_);
        virtual ~Solver_App_RK(){};
        virtual Real XNorm(const Vector& dx, const Vector& x) const;
        virtual Real FxNorm(const Vector& fx) const;
        virtual Real XNorm(const Vector& dx, const Vector& x, Real tol_) const;
        virtual Real FxNorm(const Vector& fx, Real tol_) const;
        virtual void
            EvaluateFunAndJac(const Vector& allK, Vector& gx, bool require_jac, bool factorize)
        {
            abort();
        };
    };

    class RungeKutta: public TimeIntegrator {
      protected:
        DenseMatrix rk_A;
        SVector rk_b;
        SVector rk_binf;
        SVector rk_c;
        Solver_App_RK* app;

      public:
        RungeKutta();
        ~RungeKutta(){};
        RungeKutta(Equation* eq, bool varstep = false);
        RCODE AdvanceStep(DATA_Struct& val, int iter_ref = 0);
        bool EstimateNextStepSize(const Vector& x0,
                                  const Vector& x1,
                                  const SMultiVector& allK,
                                  const Real& used_dt,
                                  Real& refinement);

        virtual bool
            SolveInnerSteps(Real t, Real used_dt, const Vector& x0, SMultiVector& allK) = 0;
        virtual Solver_App_RK* BuildSolverApp(Real t, Real dt, const Vector& x0);
        virtual void show();
        virtual TI_type GetType() { return RK; };

        // Accessors
        inline DenseMatrix& GetA() { return rk_A; };
        inline Vector& GetB() { return rk_b; };
        inline Vector& GetBinf() { return rk_binf; };
        inline Vector& GetC() { return rk_c; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
