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

#include "../common/def.h"
#include "../math/SVector.h"
#include "../math/VirtualMatrix.h"
#include <cmath>
namespace griddyn {
namespace paradae {
    class Solver_App {
      public:
        bool update_jacobian;
        Solver_App() { CurrentJacobian = NULL; };
        virtual ~Solver_App(){};
        virtual void
            EvaluateFunAndJac(const Vector& x, Vector& fx, bool require_jac, bool factorize) = 0;
        virtual Real XNorm(const Vector& dx, const Vector& x) const { return dx.Norm2() / tol; };
        virtual Real FxNorm(const Vector& fx) const { return XNorm(fx, SVector(fx.GetM(), 0)); };
        virtual Real XNorm(const Vector& dx, const Vector& x, Real tol_) const
        {
            return dx.Norm2() / tol_;
        };
        virtual Real FxNorm(const Vector& fx, Real tol_) const
        {
            return XNorm(fx, SVector(fx.GetM(), 0), tol_);
        };
        virtual void SetTol(Real tol_) { tol = tol_; };
        virtual void dump() const {};
        VirtualMatrix* GetCurrentJacobian() { return CurrentJacobian; };
        void ForceUpdateJacobian() { update_jacobian = true; };

      protected:
        Real tol;
        bool dense_mat;
        VirtualMatrix* CurrentJacobian;

        // In case no Jacobian provided
        void ApproxJacobian(const Vector& x, bool do_facto);
        void ApproxJacobian(const Vector& x, const Vector& fx, bool do_facto);
    };

    class Solver {
      protected:
        int max_iter;

      public:
        virtual ~Solver(){};
        virtual int Solve(Solver_App* app, Vector& x) = 0;
    };
}  // namespace paradae
}  // namespace griddyn
