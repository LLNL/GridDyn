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
#include "objectGrabbers.h"

using namespace gridUnits;

void autoGrabbers (gridCoreObject * obj, std::vector < std::shared_ptr < gridGrabber >> &v);
void allGrabbers (const std::string & mode, gridCoreObject * obj, std::vector < std::shared_ptr < gridGrabber >> &v);

static grabberInterpreter<gridGrabber, opGrabber, functionGrabber> gInterpret ([](const std::string &fld, gridCoreObject *obj){
  return createGrabber (fld, obj);
});

std::vector < std::shared_ptr < gridGrabber >> makeGrabbers (const std::string & command, gridCoreObject * obj)
{
  std::vector < std::shared_ptr < gridGrabber >> v;
  auto gstr = splitlineBracketTrim (command);
  for (auto &cmd:gstr)
    {
	  auto renameloc = cmd.find(" as ");//spaces are important
	  //extract out a rename
	  std::string rname = "";
	  if (renameloc != std::string::npos)
	  {
		  rname = trim(cmd.substr(renameloc + 4));
		  cmd.erase(renameloc, std::string::npos);
	  }
      if (cmd.find_first_of (":(+-/*\\^?") != std::string::npos)
        {
          auto ggb = gInterpret.interpretGrabberBlock (cmd, obj);
          if (ggb)
            {
			  if (!rname.empty())
			  {
				  ggb->setDescription(rname);
			  }
			  
              if (ggb->loaded)
                {
                  v.push_back (ggb);
                }
              else if (ggb->field.compare (0, 3,"all")==0)
                {
                  allGrabbers (cmd, ggb->getObject (), v);
                }
              else if (ggb->field == "auto")
                {
                  autoGrabbers (ggb->getObject (), v);
                }
              else
                {
                  obj->log (obj,print_level::warning,"Unable to load recorder " + command);
                }
            }

        }
      else
        {
		  std::string cmdlc = convertToLowerCase(cmd);
          if (cmdlc.compare(0, 3,"all") == 0)
            {
              allGrabbers (cmd, obj, v);
            }
          else if (cmdlc == "auto")
            {
              autoGrabbers (obj, v);
            }
          else
            {     //create a single grabber
              auto ggb = createGrabber (cmdlc, obj);
              if (ggb)
                {
				  if (!rname.empty())
				  {
					  ggb->setDescription(rname);
				  }
                  v.push_back (ggb);
                }
            }


        }
    }
  return v;

}

void autoGrabbers (gridCoreObject *obj, std::vector<std::shared_ptr<gridGrabber> > &v)
{
  std::shared_ptr<gridGrabber> ggb;

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus != nullptr)
    {
      ggb = std::make_shared<objectGrabber<gridBus>> ("voltage", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("angle", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("gen", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("load", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("freq", bus);
      v.push_back (ggb);
      return;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld != nullptr)
    {
      ggb = std::make_shared<objectGrabber<gridLoad>> ("p", ld);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLoad>> ("q", ld);
      v.push_back (ggb);
      return;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen != nullptr)
    {
      ggb = std::make_shared<objectGrabber<gridDynGenerator>> ("p", gen);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridDynGenerator>> ("q", gen);
      v.push_back (ggb);

      return;
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk != nullptr)
    {
      ggb = std::make_shared<objectGrabber<gridLink>> ("p1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("q1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("loss", lnk);
      v.push_back (ggb);
      return;
    }

  //get the vector grabs if this is a simulation
  gridSimulation *gds = dynamic_cast<gridSimulation *> (obj);
  if (gds)
    {
      ggb = std::make_shared<objectGrabber<gridArea>> ("voltage", gds);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("angle", gds);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("busgenerationreal", gds);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("busloadreal", gds);
      v.push_back (ggb);
      return;
    }

  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      ggb = std::make_shared<objectGrabber<gridArea>> ("generationreal", area);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("generationreactive", area);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("loadreal", area);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("loadreactive", area);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("loss", area);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridArea>> ("tieflowreal", area);
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
      ggb = std::make_shared<objectGrabber<gridBus>> ("voltage", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("angle", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("gen", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("load", bus);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridBus>> ("freq", bus);
      v.push_back (ggb);
      return;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      ggb = std::make_shared<objectGrabber<gridLoad>> ("p", ld);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLoad>> ("q", ld);
      v.push_back (ggb);
      return;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      if ((mode.empty ()) || (mode == "all"))
        {
          ggb = std::make_shared<objectOffsetGrabber<gridDynGenerator>> ("p", gen);
          v.push_back (ggb);
          ggb = std::make_shared<objectOffsetGrabber<gridDynGenerator>> ("q", gen);
          v.push_back (ggb);
        }
      else if (mode == "all_state")
        {
          auto scount = gen->stateSize (cLocalSolverMode);
          for (index_t kk = 0; kk < scount; ++kk)
            {
              ggb = std::make_shared<objectOffsetGrabber<gridDynGenerator>> (kk, gen);
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

      ggb = std::make_shared<objectGrabber<gridLink>> ("angle", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("p1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("p2", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("q1", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("q2", lnk);
      v.push_back (ggb);
      ggb = std::make_shared<objectGrabber<gridLink>> ("loss", lnk);
      v.push_back (ggb);
      return;


    }



  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      if ((mode.empty ()) || (mode == "all"))
        {
          ggb = std::make_shared<objectGrabber<gridArea>> ("generationreal", area);
          v.push_back (ggb);
          ggb = std::make_shared<objectGrabber<gridArea>> ("generationreactive", area);
          v.push_back (ggb);
          ggb = std::make_shared<objectGrabber<gridArea>> ("loadreal", area);
          v.push_back (ggb);
          ggb = std::make_shared<objectGrabber<gridArea>> ("loadreactive", area);
          v.push_back (ggb);
          ggb = std::make_shared<objectGrabber<gridArea>> ("loss", area);
          v.push_back (ggb);
          ggb = std::make_shared<objectGrabber<gridArea>> ("tieflowreal", area);
          v.push_back (ggb);
		  return;
        }
      else if (mode.substr (0, 8) == "all_gen_")
        {
          auto gfield = mode.substr (8);
          auto genCount = static_cast<count_t> (area->get ("gencount"));
          gridDynGenerator *ngen = nullptr;
          for (index_t pp = 0; pp < genCount; pp++)
            {
              ngen = static_cast<gridDynGenerator *> (area->findByUserID ("gen", pp + 1));
              if (ngen)
                {
                  ggb = std::make_shared<objectGrabber<gridDynGenerator>> (gfield, ngen);
                  v.push_back (ggb);
                }
            }
		  return;
        }
    }


}

