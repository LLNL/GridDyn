/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "extraSolvers.h"
#include "core/factoryTemplates.hpp"
#include "griddyn/solvers/solverInterface.h"
#ifdef ENABLE_BRAID
#include "braid/braidInterface.h"
#endif
#include <memory>

namespace griddyn
{
static std::vector<std::shared_ptr<classFactory<SolverInterface>>> extraFactories;

void loadExtraSolvers (const std::string &subset)
{
    if ((subset.empty()) || (subset == "braid"))
    {
#ifdef ENABLE_BRAID
        auto bfact=std::make_shared<childClassFactory<braid::braidSolver, SolverInterface>>(stringVec{ "braid"});
        extraFactories.push_back(bfact);
#endif
    }
    //  auto b = std::make_shared<childTypeFactory<extra::txThermalModel, Relay>> ("relay", stringVec{"thermaltx"});
    //extraFactories.push_back (b);

    //auto c = std::make_shared<childTypeFactory<extra::txLifeSpan, Relay>> ("relay", stringVec{"txaging", "txage"});
    //extraFactories.push_back (c);
}
}//namespace griddyn