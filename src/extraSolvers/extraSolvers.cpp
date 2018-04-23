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

#include "extraModels.h"
#include "core/objectFactoryTemplates.hpp"
#include "txLifeSpan.h"
#include "txThermalModel.h"

#include <memory>

namespace griddyn
{
static std::vector<std::shared_ptr<objectFactory>> extraFactories;

void loadExtraModels (const std::string & /*subset*/)
{
    auto b = std::make_shared<childTypeFactory<extra::txThermalModel, Relay>> ("relay", stringVec{"thermaltx"});
    extraFactories.push_back (b);

    auto c = std::make_shared<childTypeFactory<extra::txLifeSpan, Relay>> ("relay", stringVec{"txaging", "txage"});
    extraFactories.push_back (c);
}
}//namespace griddyn