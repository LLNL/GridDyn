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

#include "../Generator.h"
#include "../Link.h"
#include "contingency.h"
#include "../events/Event.h"
#include "../gridBus.h"
#include "gridDynSimulationFileOps.h"
#include "../gridDynSimulation.h"
#include "../loads/zipLoad.h"
#include "../simulation/diagnostics.h"
#include "utilities/vectorOps.hpp"
#include "utilities/workQueue.h"

namespace griddyn
{
void buildBusContingencies (gridDynSimulation *gds,
                            std::vector<std::shared_ptr<contingency>> &contList,
                            const extraContingencyInfo &info);
void buildLineContingencies (gridDynSimulation *gds,
                             std::vector<std::shared_ptr<contingency>> &contList,
                             const extraContingencyInfo &info);
void buildGenContingencies (gridDynSimulation *gds,
                            std::vector<std::shared_ptr<contingency>> &contList,
                            const extraContingencyInfo &info);
void buildLoadContingencies (gridDynSimulation *gds,
                             std::vector<std::shared_ptr<contingency>> &contList,
                             const extraContingencyInfo &info);

void addContingency (gridDynSimulation *gds,
                     std::vector<std::shared_ptr<contingency>> &contList,
                     std::shared_ptr<Event> &newEvent,
                     const extraContingencyInfo &info);

size_t buildContingencyList (gridDynSimulation *gds,
                             contingency_mode_t cmode,
                             std::vector<std::shared_ptr<contingency>> &contList,
                             const extraContingencyInfo &info)
{
    auto cnt = contList.size ();
    switch (cmode)
    {
    case contingency_mode_t::N_1:  // N-1 contingencies
        buildContingencyList (gds, contingency_mode_t::line, contList, info);
        buildContingencyList (gds, contingency_mode_t::gen, contList, info);
        buildContingencyList (gds, contingency_mode_t::load, contList, info);
        break;
    case contingency_mode_t::N_1_1:  // N-1-1 contingencies
    {
        auto C1 = buildContingencyList (gds, "N-1", info);
        extraContingencyInfo build;
        build.stage = 1;
        contList.reserve (C1.size () * C1.size ());
        for (auto &cont : C1)
        {
            build.baseCont = cont->clone ();
            buildContingencyList (gds, contingency_mode_t::N_1, contList, build);
        }
    }
    break;
    case contingency_mode_t::N_2:  // N-2 contingencies
    {
        auto C1 = buildContingencyList (gds, "n-1", info);
        extraContingencyInfo build;
        build.stage = 0;
        contList.reserve (C1.size () * C1.size ());
        for (auto &cont : C1)
        {
            build.baseCont = cont->clone ();
            buildContingencyList (gds, contingency_mode_t::N_1, contList, build);
        }
    }
    break;
    case contingency_mode_t::bus:  // bus contingencies --disabling each bus for a contingency
    {
        buildBusContingencies (gds, contList, info);
    }
    break;
    case contingency_mode_t::line:  // Disabling each line
    {
        buildLineContingencies (gds, contList, info);
    }
    break;
    case contingency_mode_t::load:  // Disabling each load
    {
        buildLoadContingencies (gds, contList, info);
    }
    break;
    case contingency_mode_t::gen:  // disabling each generator
    {
        buildGenContingencies (gds, contList, info);
    }
    break;
    case contingency_mode_t::custom:
    case contingency_mode_t::unknown:
        break;
    default:
        break;
    }

    return static_cast<index_t> (contList.size () - cnt);
}

std::vector<std::shared_ptr<contingency>>
buildContingencyList (gridDynSimulation *gds, const std::string &contMode, const extraContingencyInfo &info)
{
    contingency_mode_t cmode = getContingencyMode (contMode);
    std::vector<std::shared_ptr<contingency>> contList;
    buildContingencyList (gds, cmode, contList, info);

    return contList;
}

void runContingencyAnalysis (std::vector<std::shared_ptr<contingency>> &contList, const std::string &output)
{
    auto wqI = workQueue::instance ();

    for (auto &cList : contList)
    {
        wqI->addWorkBlock (cList);
    }
    if (output.compare (0, 6, "file:/") == 0)
    {
        saveContingencyOutput (contList, output.substr (7));
    }
    else if (output.compare (0, 10, "database:/") == 0)
    {
        // TODO::something with a database
    }
    else  // assume it is a file output
    {
        saveContingencyOutput (contList, output);
    }
}

void buildBusContingencies (gridDynSimulation *gds,
                            std::vector<std::shared_ptr<contingency>> &contList,
                            const extraContingencyInfo &info)
{
    std::vector<gridBus *> buses;
    gds->getBusVector (buses);
    size_t startSize = contList.size ();
    contList.reserve (startSize + buses.size ());
    for (auto &bus : buses)
    {
        if (bus->isConnected ())
        {
            std::shared_ptr<Event> ge = std::make_shared<Event> ();
            ge->setTarget (bus, "enabled");
            ge->setValue (0.0);
            addContingency (gds, contList, ge, info);
        }
    }
}

void buildLineContingencies (gridDynSimulation *gds,
                             std::vector<std::shared_ptr<contingency>> &contList,
                             const extraContingencyInfo &info)
{
    std::vector<Link *> links;
    gds->getLinkVector (links);
    size_t startSize = contList.size ();
    contList.reserve (startSize + links.size ());
    for (auto &lnk : links)
    {
        if (lnk->isConnected ())
        {
            std::shared_ptr<Event> ge = std::make_shared<Event> ();
            ge->setTarget (lnk, "connected");
            ge->setValue (0.0);
            addContingency (gds, contList, ge, info);
        }
    }
}

void buildLoadContingencies (gridDynSimulation *gds,
                             std::vector<std::shared_ptr<contingency>> &contList,
                             const extraContingencyInfo &info)
{
    std::vector<gridBus *> buses;
    gds->getBusVector (buses);
    size_t startSize = contList.size ();
    contList.reserve (startSize + buses.size ());
    for (auto &bus : buses)
    {
        if (bus->isConnected ())
        {
            if (bus->isConnected ())
            {
                index_t kk = 0;
                auto ld = bus->getLoad (0);
                while (ld != nullptr)
                {
                    auto ge = std::make_shared<Event> ();
                    ge->setTarget (ld, "connected");
                    ge->setValue (0.0);
                    addContingency (gds, contList, ge, info);
                    ++kk;
                    ld = bus->getLoad (kk);
                }
            }
        }
    }
}

void buildGenContingencies (gridDynSimulation *gds,
                            std::vector<std::shared_ptr<contingency>> &contList,
                            const extraContingencyInfo &info)
{
    std::vector<gridBus *> buses;
    gds->getBusVector (buses);
    size_t startSize = contList.size ();
    contList.reserve (startSize + buses.size ());
    for (auto &bus : buses)
    {
        if (bus->isConnected ())
        {
            if (bus->isConnected ())
            {
                index_t kk = 0;
                auto gen = bus->getGen (0);
                while (gen != nullptr)
                {
                    auto ge = std::make_shared<Event> ();
                    ge->setTarget (gen, "connected");
                    ge->setValue (0.0);
                    addContingency (gds, contList, ge, info);
                    ++kk;
                    gen = bus->getGen (kk);
                }
            }
        }
    }
}

void addContingency (gridDynSimulation *gds,
                     std::vector<std::shared_ptr<contingency>> &contList,
                     std::shared_ptr<Event> &newEvent,
                     const extraContingencyInfo &info)
{
    if (info.baseCont)
    {
        auto cont = info.baseCont->clone ();
        cont->add (newEvent, info.stage);
        contList.push_back (std::move (cont));
    }
    else
    {
        if (info.stage == 0)
        {
            contList.push_back (std::make_shared<contingency> (gds, newEvent));
        }
        else
        {
            auto cont = std::make_shared<contingency> (gds);
            cont->add (newEvent, info.stage);
            contList.push_back (std::move (cont));
        }
    }
}

}  // namespace griddyn
