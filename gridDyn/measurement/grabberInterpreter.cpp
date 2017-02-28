/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

void autoGrabbers (coreObject * obj, std::vector < std::unique_ptr < gridGrabber >> &v);
void allGrabbers (const std::string & mode, coreObject * obj, std::vector < std::unique_ptr < gridGrabber >> &v);

static grabberInterpreter<gridGrabber, opGrabber, functionGrabber> gInterpret ([](const std::string &fld, coreObject *obj){
  return createGrabber (fld, obj);
});

std::vector < std::unique_ptr < gridGrabber >> makeGrabbers (const std::string & command, coreObject * obj)
{
  std::vector < std::unique_ptr < gridGrabber >> v;
  auto gstr = stringOps::splitlineBracket(command);
  stringOps::trim(gstr);
  for (auto &cmd:gstr)
    {
	  auto renameloc = cmd.find(" as ");//spaces are important
	  //extract out a rename
	  std::string rname = "";
	  if (renameloc != std::string::npos)
	  {
		  rname = stringOps::trim(cmd.substr(renameloc + 4));
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
                  v.push_back (std::move(ggb));
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
                  v.push_back (std::move(ggb));
                }
            }


        }
    }
  return v;

}

void autoGrabbers (coreObject *obj, std::vector<std::unique_ptr<gridGrabber> > &v)
{

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus != nullptr)
    {
	  v.reserve(v.size() + 5);
      v.push_back (std::make_unique<objectGrabber<gridBus>>("voltage", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("angle", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("gen", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("load", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("freq", bus));
      return;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld != nullptr)
    {
	  v.reserve(v.size() + 2);
      v.push_back (std::make_unique<objectGrabber<gridLoad>>("p", ld));

      v.push_back (std::make_unique<objectGrabber<gridLoad>>("q", ld));
      return;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen != nullptr)
    {
	  v.reserve(v.size() + 2);
      v.push_back (std::make_unique<objectGrabber<gridDynGenerator>>("p", gen));

      v.push_back (std::make_unique<objectGrabber<gridDynGenerator>>("q", gen));

      return;
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk != nullptr)
    {
	  v.reserve(v.size() + 3);
      v.push_back (std::make_unique<objectGrabber<gridLink>>("p1", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("q1", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("loss", lnk));
      return;
    }

  //get the vector grabs if this is a simulation
  gridSimulation *gds = dynamic_cast<gridSimulation *> (obj);
  if (gds)
    {
	  v.reserve(v.size() + 4);

      v.push_back (std::make_unique<objectGrabber<gridArea>>("voltage", gds));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("angle", gds));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("busgenerationreal", gds));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("busloadreal", gds));
      return;
    }

  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
	  v.reserve(v.size() + 6);

      v.push_back (std::make_unique<objectGrabber<gridArea>>("generationreal", area));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("generationreactive", area));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("loadreal", area));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("loadreactive", area));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("loss", area));

      v.push_back (std::make_unique<objectGrabber<gridArea>>("tieflowreal", area));
      return;
    }


}

void allGrabbers (const std::string &mode, coreObject *obj, std::vector<std::unique_ptr<gridGrabber> > &v)
{

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus)
    {
	  v.reserve(v.size() + 5);

      v.push_back (std::make_unique<objectGrabber<gridBus>>("voltage", bus));
 
      v.push_back (std::make_unique<objectGrabber<gridBus>>("angle", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("gen", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("load", bus));

      v.push_back (std::make_unique<objectGrabber<gridBus>>("freq", bus));
      return;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
	  v.reserve(v.size() + 2);
      v.push_back (std::make_unique<objectGrabber<gridLoad>>("p", ld));

      v.push_back (std::make_unique<objectGrabber<gridLoad>>("q", ld));
      return;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      if ((mode.empty ()) || (mode == "all"))
        {
		  v.reserve(v.size() + 2);
          v.push_back (std::make_unique<objectOffsetGrabber<gridDynGenerator>>("p", gen));

          v.push_back (std::make_unique<objectOffsetGrabber<gridDynGenerator>>("q", gen));
        }
      else if (mode == "all_state")
        {
          auto scount = gen->stateSize (cLocalSolverMode);
		  v.reserve(v.size() + scount);
          for (index_t kk = 0; kk < scount; ++kk)
            {
              v.push_back (std::make_unique<objectOffsetGrabber<gridDynGenerator>>(kk, gen));
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
	  v.reserve(v.size() + 6);

      v.push_back (std::make_unique<objectGrabber<gridLink>>("angle", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("p1", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("p2", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("q1", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("q2", lnk));

      v.push_back (std::make_unique<objectGrabber<gridLink>>("loss", lnk));
      return;


    }



  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      if ((mode.empty ()) || (mode == "all"))
        {
		  v.reserve(v.size() + 6);

          v.push_back (std::make_unique<objectGrabber<gridArea>>("generationreal", area));

          v.push_back (std::make_unique<objectGrabber<gridArea>>("generationreactive", area));
  
          v.push_back (std::make_unique<objectGrabber<gridArea>>("loadreal", area));

          v.push_back (std::make_unique<objectGrabber<gridArea>>("loadreactive", area));
 
          v.push_back (std::make_unique<objectGrabber<gridArea>>("loss", area));

          v.push_back (std::make_unique<objectGrabber<gridArea>>("tieflowreal", area));
		  return;
        }
      else if (mode.compare (0, 8,"all_gen_")==0)
        {
          auto gfield = mode.substr (8);
          auto genCount = static_cast<count_t> (area->get ("gencount"));
          gridDynGenerator *ngen = nullptr;
		  v.reserve(v.size() + genCount);
          for (index_t pp = 0; pp < genCount; ++pp)
            {
              ngen = static_cast<gridDynGenerator *> (area->getSubObject ("gen", pp));
              if (ngen)
                {
                  v.push_back (std::make_unique<objectGrabber<gridDynGenerator>>(gfield, ngen));
                }
            }
		  return;
        }
    }


}

