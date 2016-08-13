/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "gridGrabbers.h"
#include "generators/gridDynGenerator.h"
#include "loadModels/gridLoad.h"
#include "linkModels/gridLink.h"
#include "gridArea.h"
#include "gridBus.h"
#include "simulation/gridSimulation.h"
#include "vectorOps.hpp"
#include "grabberInterpreter.hpp"


#include <cmath>
#include <algorithm>
#include <unordered_set>

using namespace gridUnits;

void autoGrabbers (gridCoreObject * obj, std::vector < std::shared_ptr < gridGrabber >> &v);
void allGrabbers (const std::string & mode, gridCoreObject * obj, std::vector < std::shared_ptr < gridGrabber >> &v);

static grabberInterpreter<gridGrabber, opGrabber, functionGrabber> gInterpret ([](const std::string &fld, gridCoreObject *obj){
  return createGrabber (fld, obj);
});

std::vector < std::shared_ptr < gridGrabber >> makeGrabbers (const std::string & command, gridCoreObject * obj)
{
  std::vector < std::shared_ptr < gridGrabber >> v;
  size_t rlc;
  std::shared_ptr<gridGrabber> ggb;
  auto gstr = splitlineBracketTrim (command);
  for (const auto &cmd:gstr)
    {
      if ((rlc = cmd.find_first_of (":(+-/*\\^")) != std::string::npos)
        {
          ggb = gInterpret.interpretGrabberBlock (cmd, obj);
          if (ggb)
            {
              if (ggb->loaded)
                {
                  v.push_back (ggb);
                }
              else if (ggb->field.substr (0, 3) == "all")
                {
                  allGrabbers (cmd, ggb->getObject (), v);
                }
              else if (ggb->field == "auto")
                {
                  autoGrabbers (ggb->getObject (), v);
                }
              else
                {
                  obj->log (obj,GD_WARNING_PRINT,"Unable to load recorder " + command);
                }
            }

        }
      else
        {
          if (cmd.substr (0, 3) == "all")
            {
              allGrabbers (cmd, obj, v);
            }
          else if (command == "auto")
            {
              autoGrabbers (obj, v);
            }
          else
            {     //create a single grabber
              ggb = createGrabber (cmd, obj);
              if (ggb)
                {
                  v.push_back (ggb);
                }
            }


        }
    }
  return v;

}

void autoGrabbers (gridCoreObject *obj, std::vector<std::shared_ptr<gridGrabber> > &v)
{
  std::shared_ptr<gridGrabber> ggb = nullptr;

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus != nullptr)
    {
      ggb = std::make_shared<gridBusGrabber> ("voltage", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("angle", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("gen", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("load", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("freq", bus);
      v.push_back (ggb);
      return;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld != nullptr)
    {
      ggb = std::make_shared<gridLoadGrabber> ("p", ld);
      v.push_back (ggb);
      ggb = std::make_shared<gridLoadGrabber> ("q", ld);
      v.push_back (ggb);
      return;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen != nullptr)
    {
      ggb = std::make_shared<gridDynGenGrabber> ("p", gen);
      v.push_back (ggb);
      ggb = std::make_shared<gridDynGenGrabber> ("q", gen);
      v.push_back (ggb);

      return;
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk != nullptr)
    {
      ggb = std::make_shared<gridLinkGrabber> ("p1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("q1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("loss", lnk);
      v.push_back (ggb);
      return;
    }

  //get the vector grabs if this is a simulation
  gridSimulation *gds = dynamic_cast<gridSimulation *> (obj);
  if (gds)
    {
      ggb = std::make_shared<gridAreaGrabber> ("voltage", gds);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("angle", gds);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("busgenerationreal", gds);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("busloadreal", gds);
      v.push_back (ggb);
      return;
    }

  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      ggb = std::make_shared<gridAreaGrabber> ("generationreal", area);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("generationreactive", area);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("loadreal", area);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("loadreactive", area);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("loss", area);
      v.push_back (ggb);
      ggb = std::make_shared<gridAreaGrabber> ("tieflowreal", area);
      v.push_back (ggb);
      return;
    }


}

void allGrabbers (const std::string &mode, gridCoreObject *obj, std::vector<std::shared_ptr<gridGrabber> > &v)
{
  std::shared_ptr<gridGrabber> ggb = nullptr;

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus)
    {
      ggb = std::make_shared<gridBusGrabber> ("voltage", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("angle", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("gen", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("load", bus);
      v.push_back (ggb);
      ggb = std::make_shared<gridBusGrabber> ("freq", bus);
      v.push_back (ggb);
      return;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      ggb = std::make_shared<gridLoadGrabber> ("p", ld);
      v.push_back (ggb);
      ggb = std::make_shared<gridLoadGrabber> ("q", ld);
      v.push_back (ggb);
      return;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      if ((mode.empty ()) || (mode == "all"))
        {
          ggb = std::make_shared<gridDynGenGrabber> ("p", gen);
          v.push_back (ggb);
          ggb = std::make_shared<gridDynGenGrabber> ("q", gen);
          v.push_back (ggb);
        }
      else if (mode == "all_state")
        {
          auto scount = gen->stateSize (cLocalSolverMode);
          for (index_t kk = 0; kk < scount; ++kk)
            {
              ggb = std::make_shared<gridDynGenGrabber> (kk, gen);
              v.push_back (ggb);
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

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk)
    {

      ggb = std::make_shared<gridLinkGrabber> ("angle", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("p1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("p2", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("q1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("q2", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<gridLinkGrabber> ("loss", lnk);
      v.push_back (ggb);
      return;


    }



  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      if ((mode.empty ()) || (mode == "all"))
        {
          ggb = std::make_shared<gridAreaGrabber> ("generationreal", area);
          v.push_back (ggb);
          ggb = std::make_shared<gridAreaGrabber> ("generationreactive", area);
          v.push_back (ggb);
          ggb = std::make_shared<gridAreaGrabber> ("loadreal", area);
          v.push_back (ggb);
          ggb = std::make_shared<gridAreaGrabber> ("loadreactive", area);
          v.push_back (ggb);
          ggb = std::make_shared<gridAreaGrabber> ("loss", area);
          v.push_back (ggb);
          ggb = std::make_shared<gridAreaGrabber> ("tieflowreal", area);
          v.push_back (ggb);
          return;
        }
      else if (mode.substr (0, 8) == "all_gen_")
        {
          std::string gfield = mode.substr (8);
          count_t genCount = static_cast<count_t> (area->get ("gencount"));
          gridDynGenerator *ngen = nullptr;
          for (index_t pp = 0; pp < genCount; pp++)
            {
              ngen = static_cast<gridDynGenerator *> (area->findByUserID ("gen", pp + 1));
              if (ngen)
                {
                  ggb = std::make_shared<gridDynGenGrabber> (gfield, ngen);
                  v.push_back (ggb);
                }
            }
          return;
        }
    }


}

