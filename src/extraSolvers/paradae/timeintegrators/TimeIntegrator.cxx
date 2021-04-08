/*
 * Copyright (c) 2018-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "TimeIntegrator.h"

#include <cmath>
namespace griddyn {
namespace paradae {
    using namespace std;

    DATA_Struct::DATA_Struct(int nx, int nb, int ng, int ns, int nl):
        tprev(nb, 1), xprev(nb, nx), dxprev(nx), gprev(ng), sprev(ns), xroot(ng > 0 ? nx : 0),
        dxroot(ng > 0 ? nx : 0), groot(ng), sroot(ng > 0 ? ns : 0), xnext(nx), dxnext(nx),
        gnext(ng), snext(ns), flimit(nl), troot(0.0)
    {
    }

    void DATA_Struct::Rotate(RCODE rc)
    {
        t += used_dt;
        if (rc == OK_ROOT || rc == WARN_ROOT) {
            xprev.Resize(1, dxprev.GetM());
            tprev.Resize(1, 1);
        }
        xprev.PushAndPop(xnext);
        tprev.PushAndPop(t);
        dxprev.CopyData(dxnext);
        gprev.CopyData(gnext);
        sprev.CopyData(snext);
    }

    void DATA_Struct::RollBack()
    {
        next_dt = used_dt;
        PVector tmp;
        xprev.GetPVector(0, tmp);
        xnext.CopyData(tmp);
        dxnext.CopyData(dxprev);
        gnext.CopyData(gprev);
        snext.CopyData(sprev);
    }

    void DATA_Struct::SetNextAtRoot()
    {
        used_dt = troot - t;
        xnext.CopyData(xroot);
        dxnext.CopyData(dxroot);
        gnext.CopyData(groot);
        snext.CopyData(sroot);
    }

    bool TimeIntegrator::CheckLimits(Vector& y, Vector& dy, Vector& flimit)
    {
        bool limit_exceeded = false;

        Real tol = 1e-10;  // HACK: Should be using the root tolerance?

        if (equation->HasLimits()) {
            /* Call limit function to check if limits are exceeded*/
            equation->limit_functions(y, dy, flimit);
            for (int i = 0; i < equation->GetNLimits(); i++) {
                if (flimit(i) <= tol) limit_exceeded = true;
            }
        }

        /* Treat exceeding limits */
        if (limit_exceeded) {
            equation->limit_crossings(y, dy, flimit);
        }

        return limit_exceeded;
    }

    bool TimeIntegrator::CheckRoots(DATA_Struct& val)
    {
        cout << "Looking for roots in [ "
             << val.t << " , " << val.t + val.used_dt << " ]" << endl;

        bool root_crossed = false;
        if (equation->HasEvents()) {
            Real tlo = val.t;
            Real thi = val.t + val.used_dt;

            int ns = val.tprev.GetSSize();
            int nx = size_x;
            SMultiVector t_y(ns + 1, 1);
            PMultiVector pt_y(t_y);
            SMultiVector v_y(ns + 1, nx);
            PMultiVector pv_y(v_y);
            SMultiVector t_dy(2, 1);
            PMultiVector pt_dy(t_dy);
            SMultiVector v_dy(2, nx);
            PMultiVector pv_dy(v_dy);

            PVector xtmp1, xtmp2;
            for (int i = 0; i < ns; i++) {
                t_y(i) = val.tprev(i);
                val.xprev.GetPVector(i, xtmp1);
                v_y.GetPVector(i, xtmp2);
                xtmp2.CopyData(xtmp1);
            }
            t_y(ns) = thi;
            v_y.GetPVector(ns, xtmp2);
            xtmp2.CopyData(val.xnext);

            t_dy(0) = tlo;
            t_dy(1) = thi;
            v_dy.GetPVector(0, xtmp2);
            xtmp2.CopyData(val.dxprev);
            v_dy.GetPVector(1, xtmp2);
            xtmp2.CopyData(val.dxnext);

            IPoly P(pt_y, pv_y, pt_dy, pv_dy);
            SVector glo;
            val.snext.CopyData(val.sprev);
            val.sroot.CopyData(val.sprev);
            if (equation->HasUEvents()) {
                glo = val.gprev;
                equation->root_functions(thi, val.xnext, val.dxnext, val.snext, val.gnext);
                val.groot.CopyData(val.gnext);
            }
            root_crossed = equation->CheckAllRoots(P, tlo, glo, thi, val.groot, val.sroot);

            /* If troot == t: no root!! */ // DJG: NO! previous_troot == troot
            //if (thi - val.t < equation->GetRoots().tol)
            if (root_crossed &&
                abs(thi - val.troot) < equation->GetRoots().tol)
            {
                cout << std::setprecision(10);
                cout << "too close!" << endl;
                cout << "thi - val.t = "
                     << thi << " - " << val.t << " = "
                     << thi - val.t
                     << " < " << equation->GetRoots().tol << endl;
                root_crossed = false;
            }

            if (root_crossed) {
                val.troot = thi;
                val.snext.CopyData(val.sroot);
                P.GetValueY(thi, val.xroot);
                P.GetValueDY(thi, val.dxroot);

                cout << "gprev = " << val.gprev << " gnext = " << val.gnext << endl;
                cout << "troot = " << std::setprecision(10) << val.troot << endl;

                // DJG comment out root_crossing in Equation::CheckAllRoots and
                // add this new function to take action if a root is found which
                // passes the solution at the root in addition
                auto roots = equation->GetRoots();
                equation->root_action(val.troot, val.xroot, val.dxroot,
                                      roots.iroot);
            }
        }
        //cout << "TimeIntegrator::CheckRoots -- was a root found? " << root_crossed << endl;
        return root_crossed;
    }

    RCODE TimeIntegrator::PostStep(DATA_Struct& val,
                                   bool success_solver,
                                   bool found_root,
                                   bool success_error_test)
    {
        // JBS HACK
        if (val.used_dt < 1e-5) success_error_test = 1;

        if (!success_solver) {
            /* Newton solver failed, we reset the *next values to *prev, and set an arbitrary
             * rfactor */
            val.RollBack();
            val.next_dt = val.used_dt / max_rfactor;  // TODO : which rfactor should we use?
            return NONLIN_FAIL;
        }
        if (!success_error_test) {
            if (found_root) {
                // The error test failed but there was a root. Set the next_dt to get to the root
                val.next_dt = val.troot - val.t;
                // JBS COMMENT OUT val.next_dt=val.troot-val.t;
                // DJG COMMENT OUT val.next_dt = (val.troot - val.t) / max_rfactor;  // JBS Comment in

                // If serial: Redo the step with new step size.
                // Else: keep information and let braid deal with the root
                if (!do_braid) val.RollBack();

                return WARN_ROOT;
            }
            // TODO if a root was found, use that information...
            /* Solution failed the error estimate test. We still keep it, rfactor should already be
             * set */
            return ERRTEST_FAIL;
        }
        if (!found_root) {
            /* No root. Everything went fine... */
            return OK;
        }
        if (abs(val.t + val.used_dt - val.troot) < 1e-13) {
            /* We landed on a root. It's ok but we still need to SetNextAtRoot */
            val.troot = val.t + val.used_dt;
            val.SetNextAtRoot();
            return OK;
        }
        /* Now, everything is ok, but there was a root on the way... */
        if (do_braid) {
            /* Let Braid deal with the root. Keep next_dt from adaptive error measure */
            return WARN_ROOT;
        } else {
            /* Serial: either go to the root adaptively, or step over the root ignoring it */
            if (do_varstep) {
                /* We integrate until the root only, used_dt was modified. Just keep going. */
                val.SetNextAtRoot();
                return OK_ROOT;
            } else {
                /* We passed a root taking the step, going too far. We keep troot in next_dx for future
                 * use */
                val.next_dt = val.troot - val.t;
                val.snext.CopyData(val.sroot);
                return WARN_ROOT;
            }
        }
    }

}  // namespace paradae
}  // namespace griddyn
