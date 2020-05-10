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

#include "../testHelper.h"
#include "griddyn/Area.h"
#include "griddyn/Block.h"
#include "griddyn/Source.h"
#include "griddyn/events/Event.h"
#include "griddyn/generators/DynamicGenerator.h"
#include "griddyn/gridSubModel.h"
#include "griddyn/links/acLine.h"
#include "griddyn/loads/zipLoad.h"
#include "griddyn/primary/acBus.h"
#include "griddyn/simulation/diagnostics.h"
#include "griddyn/simulation/gridDynSimulationFileOps.h"
#include "griddyn/solvers/solverInterface.h"
#include <iostream>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/floating_point_comparison.hpp>
using namespace griddyn;

BOOST_AUTO_TEST_SUITE(size_report)

BOOST_AUTO_TEST_CASE(objectSizeReport)
{
    std::cout << "solverOffset size=" << sizeof(solverOffsets) << '\n';
    std::cout << "offsetTableSize=" << sizeof(offsetTable) << '\n';
    std::cout << "solverModeSize=" << sizeof(solverMode) << '\n';
    std::cout << "coreTime size = " << sizeof(coreTime) << '\n';

    auto coreSize = sizeof(coreObject);
    std::cout << "core object size=" << coreSize << '\n';

    auto compSize = sizeof(gridComponent);
    std::cout << "gridComponent size=" << compSize << " adds " << compSize - coreSize << '\n';

    auto primSize = sizeof(gridPrimary);
    std::cout << "gridPrimary size=" << primSize << " adds " << primSize - compSize << '\n';

    auto secSize = sizeof(gridSecondary);
    std::cout << "gridSecondary size=" << secSize << " adds " << secSize - compSize << '\n';

    //Common Buses
    std::cout << "bus size=" << sizeof(gridBus) << " adds " << sizeof(gridBus) - primSize << '\n';
    std::cout << "acbus size=" << sizeof(acBus) << " adds " << sizeof(acBus) - sizeof(gridBus)
              << '\n';

    //Common Loads
    std::cout << "load size=" << sizeof(Load) << " adds " << sizeof(Load) - secSize << '\n';

    std::cout << "zipload size=" << sizeof(zipLoad) << " adds " << sizeof(zipLoad) - sizeof(Load)
              << '\n';

    //Common Generators
    std::cout << "Generator size=" << sizeof(Generator) << " adds " << sizeof(Generator) - secSize
              << '\n';
    std::cout << "dynamic Generator size=" << sizeof(DynamicGenerator) << " adds "
              << sizeof(DynamicGenerator) - sizeof(Generator) << '\n';

    //Common Links
    std::cout << "Link size=" << sizeof(Link) << " adds " << sizeof(Link) - primSize << '\n';
    std::cout << "ac Link size=" << sizeof(acLine) << " adds " << sizeof(acLine) - sizeof(Link)
              << '\n';

    //subModel Sizes
    std::cout << "submodel size" << sizeof(gridSubModel) << "adds "
              << sizeof(gridSubModel) - compSize << '\n';

    //Source Size
    std::cout << "Source size=" << sizeof(Source) << " adds "
              << sizeof(Source) - sizeof(gridSubModel) << '\n';

    //Source Size
    std::cout << "Block size=" << sizeof(Block) << " adds " << sizeof(Block) - sizeof(gridSubModel)
              << '\n';
}

BOOST_AUTO_TEST_SUITE_END()
