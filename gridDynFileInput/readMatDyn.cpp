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


#include "gridDynFileInput.h"
#include "readerHelper.h"
#include "events/gridEvent.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynGovernor.h"
#include "submodels/otherGenModels.h"
#include "generators/gridDynGenerator.h"
#include "linkModels/gridLink.h"
#include "loadModels/zipLoad.h"
#include "gridBus.h"
#include "gridDyn.h"
#include "utilities/stringOps.h"

#include <cstdlib>
#include <iostream>
#include <memory>

using namespace gridUnits;
using namespace stringOps;


void loadGenExcArray (coreObject *parentObject, mArray &Exc, std::vector<gridDynGenerator *> &genList);
void loadGenDynArray (coreObject *parentObject, mArray &Gen, std::vector<gridDynGenerator *> &genList);
void loadGenGovArray (coreObject *parentObject, mArray &Gov, std::vector<gridDynGenerator *> &genList);


void loadMatDyn (coreObject *parentObject, const std::string &filetext, const basicReaderInfo &)
{
  mArray M1;

  std::vector<gridDynGenerator *> genList;
  gridDynGenerator *gen;
  //read the frequency
  size_t A = filetext.find_first_of ('[',0);
  size_t B = filetext.find_first_of (']', 0);
  std::string tstr = filetext.substr (A + 1, B - A - 1);
  auto Tline = splitline (tstr, "\t ,",delimiter_compression::on);

  size_t D = B;
  size_t C;
  A = filetext.find (Tline[3],D);       //freq
  if (A != std::string::npos)
    {
      B = filetext.find_first_of ('=', A);
      C = filetext.find_first_of (";\n", A);
      tstr = filetext.substr (B + 1, C - B - 1);
      double freq = numeric_conversion (tstr,kNullVal);
      parentObject->set ("basefreq", freq);
    }
  //get the timestep parameter
  A = filetext.find (Tline[4],D);      //steptime
  if (A != std::string::npos)
    {
      B = filetext.find_first_of ('=', A);
      C = filetext.find_first_of (";\n", A);
      tstr = filetext.substr (B + 1, C - B - 1);
      double val = numeric_conversion (tstr,kNullVal);
      parentObject->set ("timestep", val);
    }
  //get the stoptime parameter
  A = filetext.find (Tline[5],D);     //stoptime
  if (A != std::string::npos)
    {
      B = filetext.find_first_of ('=', A);
      C = filetext.find_first_of (";\n", A);
      tstr = filetext.substr (B + 1, C - B - 1);
      double val = numeric_conversion (tstr,kNullVal);
      parentObject->set ("timestop", val);
    }
  A = filetext.find (Tline[0],D);     //gen
  if (A != std::string::npos)
    {
      B = filetext.find_first_of ('=', A);
      readMatlabArray (filetext, B + 1, M1);
      loadGenDynArray (parentObject,  M1, genList);
    }

  A = filetext.find (Tline[1],D);     //exc
  if (A != std::string::npos)
    {
      B = filetext.find_first_of ('=', A);
      readMatlabArray (filetext, B + 1, M1);
      loadGenExcArray (parentObject, M1, genList);
    }

  A = filetext.find (Tline[2],D);     //gov
  if (A != std::string::npos)
    {
      B = filetext.find_first_of ('=', A);
      readMatlabArray (filetext, B + 1, M1);
      loadGenGovArray (parentObject, M1, genList);
    }
  A = 1;
  for (auto &ngen : genList)
    {
      gen = static_cast<gridDynGenerator *> (parentObject->findByUserID ("gen",static_cast<index_t> (A)));
      A++;
      if (!gen)
        {
          std::cout << "the number of generators does not match the matdyn file please run with matching matpower file first\n";
          return;
        }
      //now we load the existing components of our generator onto the existing one
      coreObject *obj = ngen->getSubObject ("exciter", 0);
      if (obj)
        {
          gen->add (obj);
        }

      obj = ngen->getSubObject ("genmodel", 0);
      if (obj)
        {
          gen->add (obj);
        }

      obj = ngen->getSubObject ("governor", 0);
      if (obj)
        {
          gen->add (obj);
        }

    }
  //now delete the temporary generators the subobjects should have transfered ownership
  for (auto g : genList)
    {
      delete g;
    }
  //lastly all the loads need to be autoconverted
  index_t b = static_cast<index_t> (parentObject->get ("loadcount"));
  for (index_t kk = 1; kk <= b; kk++)
    {
      auto ld = static_cast<zipLoad *> (parentObject->findByUserID ("load", kk));
      ld->set ("converttoimpedance", 1);//TODO:: change this so it doesn't do a dynamic allocation
    }

}

void loadGenDynArray (coreObject * /*parentObject*/, mArray &Gen, std::vector<gridDynGenerator *> &genList)
{
  gridDynGenerator *gen = nullptr;
  gridDynExciter *exc;
  gridDynGovernor *gov;
  gridDynGenModel *gm;

  /*[genmodel excmodel govmodel H D xd xq xd_tr xq_tr Td_tr Tq_tr]*/
  for (const auto &genLine : Gen)
    {
      gen = new gridDynGenerator ();
      switch (static_cast<int> (genLine[0]))
        {
        case 1:        //classical model
          gm = new gridDynGenModelClassical ();
          gm->set ("h", genLine[3]);
          gm->set ("d", genLine[4]);
          gm->set ("x", genLine[5]);
          gm->set ("xp", genLine[6]);
          gen->add (gm);
          break;
        case 2:         //fourth order model
          gm = new gridDynGenModel4 ();
          gm->set ("h", genLine[3]);
          gm->set ("d", genLine[4]);
          gm->set ("xd", genLine[5]);
          gm->set ("xq", genLine[6]);
          gm->set ("xdp", genLine[7]);
          gm->set ("xqp", genLine[8]);
          gm->set ("tdp", genLine[9]);
          gm->set ("tqp", genLine[10]);
          gen->add (gm);
          break;
        default:
          std::cout << "unknown genmodel code in gen matrix\n";
          break;
        }
      //now run through the different exciter models
      switch (static_cast<int> (genLine[1]))          //switch on exciter model
        {
        case 1:        //constant excitation means no exciter
          break;
        case 2:          //DC1A eciter
          exc = new gridDynExciterDC1A ();
          gen->add (exc);
          break;
        case 3:        //AC4A exciter  NOT IMPLEMENTED YET
        default:
          std::cout << "unknown exciter code in gen matrix\n";
          break;


        }
      //now run through the different governor models
      switch (static_cast<int> (genLine[2]))          //switch on exciter model
        {
        case 1:        //constant mechanical power
          break;
        case 2:
          gov = new gridDynGovernor ();
          gen->add (gov);
          break;
        default:
          std::cout << "unknown governor code in gen matrix\n";
          break;

        }
      genList.push_back (gen);
    }

}

/*
1 gen, number of the generator
2 Ka, amplifier gain
3 Ta, amplifier time constant
4 Ke, exciter gain
5 Te, exciter time constant
6 Kf , stabilizer gain
7 Tf , stabilizer time constant
8 Aex, parameter saturation function
9 Bex, parameter saturation function
10 Urmin, lower voltage limit
11 Urmax, upper voltage limit
*/
void loadGenExcArray (coreObject * /*parentObject*/, mArray &excData, std::vector<gridDynGenerator *> &genList)
{

  /*[genmodel excmodel govmodel H D xd xq xd_tr xq_tr Td_tr Tq_tr]*/
  for (const auto &excLine : excData)
    {
	  gridDynExciter *exc = nullptr;
      index_t ind1 = static_cast<index_t> (excLine[0]);
      if (ind1 <= genList.size ())
        {
		  gridDynGenerator *gen = genList[ind1 - 1];
          exc = static_cast<gridDynExciter *> (gen->getSubObject ("exciter", 0));
        }
      if (!exc)
        {
          continue;
        }
      //this means it is the DC1A exciter
      exc->set ("ka", excLine[1]);
      exc->set ("ta", excLine[2]);
      exc->set ("ke", excLine[3]);
      exc->set ("te", excLine[4]);
      exc->set ("kf", excLine[5]);
      exc->set ("tf", excLine[6]);
      exc->set ("aex", excLine[7]);
      exc->set ("bex", excLine[8]);
      exc->set ("urmin", excLine[9]);
      exc->set ("urmax", excLine[10]);

    }
}
/*
1 gen, number of the generator
2 K, droop
3 T1, time constant
4 T2, time constant
5 T3, servo motor time constant
6 Pup, upper ramp limit
7 Pdown, lower ramp limit
8 Pmax, maximal turbine output
9 Pmin, minimal turbine output
*/
void loadGenGovArray(coreObject * /*parentObject*/, mArray &govData, std::vector<gridDynGenerator *> &genList)
{

	gridDynGenerator *gen = nullptr;

	/*[genmodel excmodel govmodel H D xd xq xd_tr xq_tr Td_tr Tq_tr]*/
	for (const auto &govLine:govData)
	{
		gridDynGovernor *gov = nullptr;
		index_t ind1 = static_cast<index_t>(govLine[0]);
		if (ind1 <= genList.size())
		{
			gen = genList[ind1 - 1];
			gov = static_cast<gridDynGovernor *>(gen->getSubObject("governor", 0));
		}
		if (!gov)
		{
			continue;
		}
		
		gov->set("k", govLine[1]);
		gov->set("t1", govLine[2]);
		gov->set("t2", govLine[3]);
		gov->set("t3", govLine[4]);
		gov->set("pup", govLine[5]);
		gov->set("pdown", govLine[6]);
		//these next two should be set at the generator, they then filter down to the governor
		gen->set("pmax", govLine[7]);
		gen->set("pmin", govLine[8]);

	}
}


//read matdyn Event files
void loadMatDynEvent(coreObject *parentObject, const std::string &filetext, const basicReaderInfo &)
{
	
	mArray::size_type kk;
	std::shared_ptr<gridEvent> evnt;
	gridBus *bus;
	gridLoad *ld;
	gridLink *lnk;
	int ind;
	mArray event1,M1;
	gridSimulation *gds = dynamic_cast<gridSimulation *>(parentObject->getRoot());
	if (gds == nullptr)
	{ //cant make events if we don't have access to the simulation
		return;
	}
	//read the frequency
	size_t A = filetext.find_first_of('[', 0);
	size_t B = filetext.find_first_of(']', 0);
	std::string tstr = filetext.substr(A + 1, B - A - 1);
	auto Tline=splitline(tstr,  "\t ,");
	size_t C = B;
	A = filetext.find(Tline[0],C);//event
	if (A != std::string::npos)
	{
		B = filetext.find_first_of('=', A);
		readMatlabArray(filetext, B + 1, event1);
		//loadGenDynArray(parentObject, M1, genList);
	}

	A = filetext.find(Tline[1],C);//buschange
	if (A != std::string::npos)
	{
		B = filetext.find_first_of('=', A);
		readMatlabArray(filetext, B + 1, M1);
		for (kk = 0; kk < M1.size(); ++kk)
		{
			evnt = std::make_unique<gridEvent>(M1[kk][0]);
			ind = static_cast<int>(M1[kk][1]);
			bus = static_cast<gridBus *>(parentObject->findByUserID("bus",ind));
			ld = bus->getLoad();
			if (!(ld))
			{
				ld = new zipLoad();
				bus->add(ld);
			}
			switch (static_cast<int>(M1[kk][2]))
			{
			case 3://P
				evnt->setTarget(ld, "p");
				evnt->setValue(M1[kk][3], MW);
				break;
			case 4: //Q
				evnt->setTarget(ld, "q");
				evnt->setValue(M1[kk][3], MVAR);
				break;
			case 5: //GS
				evnt->setTarget(ld, "yp");
				evnt->setValue(M1[kk][3], MW);
				break;
			case 6: //BS
				evnt->setTarget(ld, "yq");
				evnt->setValue(-M1[kk][3], MW);
				break;
			default:
				break;
			}
			gds->add(std::move(evnt));
		}
	//	loadGenExcArray(parentObject, M1, genList);
	}

	A = filetext.find(Tline[2],C);//linechange
	if (A != std::string::npos)
	{
		B = filetext.find_first_of('=', A);
		readMatlabArray(filetext, B + 1, M1);
		for (const auto &lc:M1)
		{
			evnt = std::make_unique<gridEvent>(lc[0]);
			
			ind = static_cast<int>(lc[1]);
			lnk = static_cast<gridLink *>(parentObject->findByUserID("link",ind));
			switch (static_cast<int>(lc[2]))
			{
			case 3://r
				evnt->setTarget(lnk, "r");
				evnt->setValue(lc[3]);
				break;
			case 4: //X
				evnt->setTarget(lnk, "x");
				evnt->setValue(lc[3]);
				break;
			case 5: //B
				evnt->setTarget(lnk, "b");
				evnt->setValue(lc[3],MW);
				break;
			case 9: //tap
				evnt->setTarget(lnk, "tap");
				evnt->setValue(lc[3]);
				break;
			case 10: //BS
				evnt->setTarget(lnk, "tapangle");
				evnt->setValue(lc[3], deg);
				break;
			case 11: //BS
				evnt->setTarget(lnk, "enable");
				evnt->setValue(lc[3]);
				break;
			default:
				break;
			}
			gds->add(std::move(evnt));
		}
	}
}