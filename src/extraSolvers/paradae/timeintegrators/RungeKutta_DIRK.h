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
#ifndef RungeKutta_DIRK_h
#define RungeKutta_DIRK_h

#include "RungeKutta_Implicit.h"
namespace griddyn {
namespace paradae {
    class Solver_App_DIRK: public Solver_App_IRK {
      public:
        Solver_App_DIRK(Real rtol,
                        const Vector& atol,
                        Real t_,
                        Real dt_,
                        const Vector& x0_,
                        RungeKutta_Implicit* rk_);
        virtual ~Solver_App_DIRK(){};
        virtual void
            EvaluateFunAndJac(const Vector& allK, Vector& gx, bool require_jac, bool factorize);
    };

    class RungeKutta_DIRK: public RungeKutta_Implicit {
      protected:
        virtual void InitArray();

      public:
        RungeKutta_DIRK(Equation* eq, bool varstep = false);
        virtual void SetDenseMatrix(bool dense_mat_ = true);
        virtual Solver_App_RK* BuildSolverApp(Real t, Real dt, const Vector& x0);
        virtual ~RungeKutta_DIRK(){};
    };
}  // namespace paradae
}  // namespace griddyn

#endif
