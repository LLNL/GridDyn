/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "basicSolver.h"
#include "core/helperTemplates.hpp"
#include "griddyn.h"
#include "utilities/stringOps.h"
#include "utilities/vectorOps.hpp"

namespace griddyn
{
namespace solvers
{
basicSolver::basicSolver (mode_t alg) : algorithm (alg) { mode.algebraic = true; }
basicSolver::basicSolver (const std::string &objName, mode_t alg) : solverInterface (objName), algorithm (alg)
{
    mode.algebraic = true;
}

basicSolver::basicSolver (gridDynSimulation *gds, const solverMode &sMode)
    : solverInterface (gds, sMode), algorithm (mode_t::gauss)
{
    mode.algebraic = true;
}

std::shared_ptr<solverInterface> basicSolver::clone (std::shared_ptr<solverInterface> si, bool fullCopy) const
{
    auto rp = cloneBase<basicSolver, solverInterface> (this, si, fullCopy);
    if (!rp)
    {
        return si;
    }
    rp->algorithm = algorithm;
    rp->alpha = alpha;
    return rp;
}
double *basicSolver::state_data () noexcept { return state.data (); }
double *basicSolver::deriv_data () noexcept { return nullptr; }
double *basicSolver::type_data () noexcept { return type.data (); }
const double *basicSolver::state_data () const noexcept { return state.data (); }
const double *basicSolver::deriv_data () const noexcept { return nullptr; }
const double *basicSolver::type_data () const noexcept { return type.data (); }
void basicSolver::allocate (count_t stateCount, count_t numRoots)
{
    // load the vectors
    if (stateCount != svsize)
    {
        state.resize (stateCount);
        tempState1.resize (stateCount);
        tempState2.resize (stateCount);
        svsize = stateCount;

        flags.reset (initialized_flag);
        flags.set (allocated_flag);
        rootsfound.resize (numRoots);
    }
}

void basicSolver::initialize (coreTime /*time0*/)
{
    if (!flags[allocated_flag])
    {
        throw (InvalidSolverOperation (-2));
    }
    flags.set (initialized_flag);
    solverCallCount = 0;
}

double basicSolver::get (const std::string &param) const
{
    if (param == "alpha")
    {
        return alpha;
    }
    if (param == "iterations")
    {
        return static_cast<double> (iterations);
    }
    return solverInterface::get (param);
}
void basicSolver::set (const std::string &param, const std::string &val)
{
    if (param == "algorithm")
    {
        auto lcs = convertToLowerCase (val);
        if (lcs == "gauss")
        {
            algorithm = mode_t::gauss;
            mode.approx[force_recalc] = false;
        }
        else if (lcs == "gauss-seidel")
        {
            algorithm = mode_t::gauss_seidel;
            mode.approx[force_recalc] = true;
        }
    }
    else
    {
        solverInterface::set (param, val);
    }
}
void basicSolver::set (const std::string &param, double val)
{
    if (param == "alpha")
    {
        alpha = val;
    }
    else
    {
        solverInterface::set (param, val);
    }
}

void cleanOscillations (const std::vector<double> &s1,
                        const std::vector<double> &s2,
                        std::vector<double> &s3,
                        double conv);

int basicSolver::solve (coreTime tStop, coreTime & /*tReturn*/, step_mode /*stepMode*/)
{
    double md = 1.0;
    iterations = 0;
    if (algorithm == mode_t::gauss)
    {
        while (md > tolerance)
        {
            m_gds->algUpdateFunction (tStop, state.data (), tempState1.data (), mode, alpha);
            ++iterations;
            md = absMaxDiff (state, tempState1);
            // printf("Iteration %d max change=%f\n", iterations, md);
            if (md <= tolerance)
            {
                std::swap (state, tempState1);
                break;
            }
            m_gds->algUpdateFunction (tStop, tempState1.data (), tempState2.data (), mode, alpha);
            ++iterations;
            md = absMaxDiff (tempState1, tempState2);
            // printf("Iteration %d max change=%f\n", iterations, md);
            if (md <= tolerance)
            {
                std::swap (state, tempState2);
                break;
            }
            cleanOscillations (state, tempState1, tempState2, 0.15);
            m_gds->algUpdateFunction (tStop, tempState2.data (), state.data (), mode, alpha);
            ++iterations;
            md = absMaxDiff (tempState2, state);

            if (iterations > max_iterations)
            {
                break;
            }
        }
        printf ("Iteration %d max change=%f\n", iterations, md);
    }
    else if (algorithm == mode_t::gauss_seidel)
    {
        alpha = 1.2;
        while (md > tolerance)
        {
            tempState1 = state;
            m_gds->algUpdateFunction (tStop, tempState1.data (), tempState1.data (), mode, alpha);
            ++iterations;
            md = absMaxDiff (state, tempState1);
            // printf("Iteration %d max change=%f\n", iterations, md);
            if (md <= tolerance)
            {
                std::swap (state, tempState1);
                break;
            }
            tempState2 = tempState1;
            m_gds->algUpdateFunction (tStop, tempState2.data (), tempState2.data (), mode, alpha);
            ++iterations;
            md = absMaxDiff (tempState1, tempState2);
            // printf("Iteration %d max change=%f\n", iterations, md);
            if (md <= tolerance)
            {
                std::swap (state, tempState2);
                break;
            }
            cleanOscillations (state, tempState1, tempState2, 0.15);
            state = tempState2;
            m_gds->algUpdateFunction (tStop, state.data (), state.data (), mode, alpha);
            ++iterations;
            md = absMaxDiff (tempState2, state);
            if (iterations > max_iterations)
            {
                break;
            }
        }
        printf ("Iteration %d max change=%f\n", iterations, md);
    }
    if (iterations < max_iterations)
    {
        return FUNCTION_EXECUTION_SUCCESS;
    }
    return SOLVER_CONVERGENCE_ERROR;
}

void cleanOscillations (const std::vector<double> &s1,
                        const std::vector<double> &s2,
                        std::vector<double> &s3,
                        double conv)
{
    auto term = s1.size ();
    for (size_t kk = 0; kk < term; ++kk)
    {
        double roc = std::abs (s3[kk] - s1[kk]) / (std::abs (s2[kk] - s3[kk]) + std::abs (s1[kk] - s2[kk]));
        if (roc < conv)
        {
            s3[kk] = 0.5 * (s3[kk] * (1.0 + roc) + s2[kk] * (1.0 - roc));
        }
    }
}

}  // namespace solvers
}  // namespace griddyn