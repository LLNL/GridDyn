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

#include "grabberInterpreter.hpp"
#include "../Area.h"
#include "../Generator.h"
#include "../Link.h"
#include "../Load.h"
#include "../gridBus.h"
#include "gridGrabbers.h"
#include "objectGrabbers.h"
#include "../simulation/gridSimulation.h"
#include "gmlc/utilities/vectorOps.hpp"

namespace griddyn
{
using namespace units;
using namespace gmlc::utilities;

void autoGrabbers (coreObject *obj, std::vector<std::unique_ptr<gridGrabber>> &v);
void allGrabbers (const std::string &mode, coreObject *obj, std::vector<std::unique_ptr<gridGrabber>> &v);

static grabberInterpreter<gridGrabber, opGrabber, functionGrabber>
gInterpret ([](const std::string &fld, coreObject *obj) { return createGrabber (fld, obj); });


bool isOperatorOutsideBlocks(const std::vector<std::pair<size_t, size_t>> &blocks, size_t loc)
{
	if (loc < blocks[0].first)
	{
		return true;
	}
	if (loc > blocks.back().second)
	{
		return true;
	}
	for (auto blk : blocks)
	{
		if ((loc > blk.first) && (loc < blk.second))
		{
			return false;
		}
	}
	return true;
}
std::vector<std::unique_ptr<gridGrabber>> makeGrabbers (const std::string &command, coreObject *obj)
{
    std::vector<std::unique_ptr<gridGrabber>> v;
    auto gstr = stringOps::splitlineBracket (command);
    stringOps::trim (gstr);
    for (auto &cmd : gstr)
    {
        auto renameloc = cmd.find (" as ");  // spaces are important
        // extract out a rename
        std::string rname;
        if (renameloc != std::string::npos)
        {
            rname = stringOps::trim (cmd.substr (renameloc + 4));
            cmd.erase (renameloc, std::string::npos);
        }
        if (cmd.find_first_of (R"(:(+-/*\^?)") != std::string::npos)
        {
            auto ggb = gInterpret.interpretGrabberBlock (cmd, obj);
            if (ggb)
            {
                if (!rname.empty ())
                {
                    ggb->setDescription (rname);
                }

                if (ggb->loaded)
                {
                    v.push_back (std::move (ggb));
                }
                else if (ggb->field.compare (0, 3, "all") == 0)
                {
                    allGrabbers (cmd, ggb->getObject (), v);
                }
                else if (ggb->field == "auto")
                {
                    autoGrabbers (ggb->getObject (), v);
                }
                else
                {
                    obj->log (obj, print_level::warning, "Unable to load recorder " + command);
                }
            }
        }
        else
        {
            std::string cmdlc = convertToLowerCase (cmd);
            if (cmdlc.compare (0, 3, "all") == 0)
            {
                allGrabbers (cmd, obj, v);
            }
            else if (cmdlc == "auto")
            {
                autoGrabbers (obj, v);
            }
            else
            {  // create a single grabber
                auto ggb = createGrabber (cmdlc, obj);
                if (ggb)
                {
                    if (!rname.empty ())
                    {
                        ggb->setDescription (rname);
                    }
                    v.push_back (std::move (ggb));
                }
            }
        }
    }
    return v;
}

void autoGrabbers (coreObject *obj, std::vector<std::unique_ptr<gridGrabber>> &v)
{
    auto bus = dynamic_cast<gridBus *> (obj);
    if (bus != nullptr)
    {
        v.reserve (v.size () + 5);
        v.push_back (std::make_unique<objectGrabber<gridBus>> ("voltage", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("angle", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("gen", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("load", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("freq", bus));
        return;
    }

    auto ld = dynamic_cast<Load *> (obj);
    if (ld != nullptr)
    {
        v.reserve (v.size () + 2);
        v.push_back (std::make_unique<objectGrabber<Load>> ("p", ld));

        v.push_back (std::make_unique<objectGrabber<Load>> ("q", ld));
        return;
    }

    auto gen = dynamic_cast<Generator *> (obj);
    if (gen != nullptr)
    {
        v.reserve (v.size () + 2);
        v.push_back (std::make_unique<objectGrabber<Generator>> ("p", gen));

        v.push_back (std::make_unique<objectGrabber<Generator>> ("q", gen));

        return;
    }

    auto lnk = dynamic_cast<Link *> (obj);
    if (lnk != nullptr)
    {
        v.reserve (v.size () + 3);
        v.push_back (std::make_unique<objectGrabber<Link>> ("p1", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("q1", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("loss", lnk));
        return;
    }

    // get the vector grabs if this is a simulation
    auto gds = dynamic_cast<gridSimulation *> (obj);
    if (gds != nullptr)
    {
        v.reserve (v.size () + 4);

        v.push_back (std::make_unique<objectGrabber<Area>> ("voltage", gds));

        v.push_back (std::make_unique<objectGrabber<Area>> ("angle", gds));

        v.push_back (std::make_unique<objectGrabber<Area>> ("busgenerationreal", gds));

        v.push_back (std::make_unique<objectGrabber<Area>> ("busloadreal", gds));
        return;
    }

    auto area = dynamic_cast<Area *> (obj);
    if (area != nullptr)
    {
        v.reserve (v.size () + 6);

        v.push_back (std::make_unique<objectGrabber<Area>> ("generationreal", area));

        v.push_back (std::make_unique<objectGrabber<Area>> ("generationreactive", area));

        v.push_back (std::make_unique<objectGrabber<Area>> ("loadreal", area));

        v.push_back (std::make_unique<objectGrabber<Area>> ("loadreactive", area));

        v.push_back (std::make_unique<objectGrabber<Area>> ("loss", area));

        v.push_back (std::make_unique<objectGrabber<Area>> ("tieflowreal", area));
        return;
    }
}

void allGrabbers (const std::string &mode, coreObject *obj, std::vector<std::unique_ptr<gridGrabber>> &v)
{
    auto bus = dynamic_cast<gridBus *> (obj);
    if (bus != nullptr)
    {
        v.reserve (v.size () + 5);

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("voltage", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("angle", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("gen", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("load", bus));

        v.push_back (std::make_unique<objectGrabber<gridBus>> ("freq", bus));
        return;
    }

    auto ld = dynamic_cast<Load *> (obj);
    if (ld != nullptr)
    {
        v.reserve (v.size () + 2);
        v.push_back (std::make_unique<objectGrabber<Load>> ("p", ld));

        v.push_back (std::make_unique<objectGrabber<Load>> ("q", ld));
        return;
    }

    auto gen = dynamic_cast<Generator *> (obj);
    if (gen != nullptr)
    {
        if ((mode.empty ()) || (mode == "all"))
        {
            v.reserve (v.size () + 2);
            v.push_back (std::make_unique<objectOffsetGrabber<Generator>> ("p", gen));

            v.push_back (std::make_unique<objectOffsetGrabber<Generator>> ("q", gen));
        }
        else if (mode == "all_state")
        {
            auto scount = gen->stateSize (cLocalSolverMode);
            v.reserve (v.size () + scount);
            for (index_t kk = 0; kk < scount; ++kk)
            {
                v.push_back (std::make_unique<objectOffsetGrabber<Generator>> (kk, gen));
            }
        }
        else if (mode == "all_model")
        {
            /*size_t scount = gen->stateSize();
            for (size_t kk = 0; kk < scount; ++kk)
            {
            ggb =std::make_shared<gridDynGenGrabber(kk, gen);
            v.push_back(ggb);
            }*/
        }
        else if (mode == "all_gov")
        {
            /*size_t scount = gen->stateSize();
            for (size_t kk = 0; kk < scount; ++kk)
            {
            ggb =std::make_shared<gridDynGenGrabber(kk, gen);
            v.push_back(ggb);
            }*/
        }
        else if (mode == "all_ext")
        {
            /*size_t scount = gen->stateSize();
            for (size_t kk = 0; kk < scount; ++kk)
            {
            ggb =std::make_shared<gridDynGenGrabber(kk, gen);
            v.push_back(ggb);
            }*/
        }
        else if (mode == "all_pss")
        {
            /*size_t scount = gen->stateSize();
            for (size_t kk = 0; kk < scount; ++kk)
            {
            ggb =std::make_shared<gridDynGenGrabber(kk, gen);
            v.push_back(ggb);
            }*/
        }
        return;
    }

    auto lnk = dynamic_cast<Link *> (obj);
    if (lnk != nullptr)
    {
        v.reserve (v.size () + 6);

        v.push_back (std::make_unique<objectGrabber<Link>> ("angle", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("p1", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("p2", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("q1", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("q2", lnk));

        v.push_back (std::make_unique<objectGrabber<Link>> ("loss", lnk));
        return;
    }

    auto area = dynamic_cast<Area *> (obj);
    if (area != nullptr)
    {
        if ((mode.empty ()) || (mode == "all"))
        {
            v.reserve (v.size () + 6);

            v.push_back (std::make_unique<objectGrabber<Area>> ("generationreal", area));

            v.push_back (std::make_unique<objectGrabber<Area>> ("generationreactive", area));

            v.push_back (std::make_unique<objectGrabber<Area>> ("loadreal", area));

            v.push_back (std::make_unique<objectGrabber<Area>> ("loadreactive", area));

            v.push_back (std::make_unique<objectGrabber<Area>> ("loss", area));

            v.push_back (std::make_unique<objectGrabber<Area>> ("tieflowreal", area));
            return;
        }
        if (mode.compare (0, 8, "all_gen_") == 0)
        {
            auto gfield = mode.substr (8);
            auto genCount = static_cast<count_t> (area->get ("gencount"));
            Generator *ngen = nullptr;
            v.reserve (v.size () + genCount);
            for (index_t pp = 0; pp < genCount; ++pp)
            {
                ngen = static_cast<Generator *> (area->getSubObject ("gen", pp));
                if (ngen != nullptr)
                {
                    v.push_back (std::make_unique<objectGrabber<Generator>> (gfield, ngen));
                }
            }
            return;
        }
    }
}

}  // namespace griddyn
