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
#include "dimeCollector.h"
#include "gridDynFileInput.h"
#include "readerHelper.h"
#include "loadModels/svd.h"
#include "loadModels/gridLoad.h"
#include "linkModels/acLine.h"
#include "gridBus.h"
#include "generators/gridDynGenerator.h"
#include "core/objectFactoryTemplates.h"
#include "core/coreExceptions.h"
#include "utilities/stringOps.h"
#include "gridDyn.h"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <map>
#include <cassert>
#include <cmath>

using namespace gridUnits;
using namespace stringOps;

int getPSSversion (const std::string &line);
void rawReadBus (gridBus *bus, const std::string &line, basicReaderInfo &opt);
void rawReadLoad (gridLoad *ld, const std::string &line, basicReaderInfo &opt);
void rawReadFixedShunt (gridLoad *ld, const std::string &line, basicReaderInfo &opt);
void rawReadGen (gridDynGenerator *gen,  const std::string &line, basicReaderInfo &opt);
void rawReadBranch (coreObject *parentObject, const std::string &line,std::vector<gridBus *> &busList, basicReaderInfo &opt);
int rawReadTX (coreObject *parentObject, stringVec &txlines,  std::vector<gridBus *> &busList, basicReaderInfo &opt);
void rawReadSwitchedShunt (coreObject *parentObject,  const std::string &line, std::vector<gridBus *> &busList, basicReaderInfo &opt);
void rawReadTXadj (coreObject *parentObject, const std::string &line,  basicReaderInfo &opt);

int rawReadDCLine (coreObject *parentObject,  stringVec &txlines, std::vector<gridBus *> &busList, basicReaderInfo &opt);


enum sections
{
  unknown, bus, branch, load, fixedShunt,generator, tx, switchedShunt, txadj
};

static coreTime cctime;
static std::vector <std::string> sysname;
static std::vector <std::string> sysparam;//###############

//get the basic busFactory
static typeFactory<gridBus> *busfactory = nullptr;

//get the basic load Factory
static typeFactory<gridLoad> *ldfactory = nullptr;
//get the basic Link Factory
static typeFactory<acLine> *linkfactory = nullptr;
//get the basic Generator Factory
static typeFactory<gridDynGenerator> *genfactory = nullptr;

sections findSectionType (const std::string &line);

bool checkNextLine (std::ifstream &file, std::string &nextLine)
{
  if (std::getline (file, nextLine))
    {
      trimString (nextLine);
      if (nextLine[0] == '0')
        {
          return false;
        }
      return true;
    }
  else
    {
      return false;
    }
}

gridBus * findBus (std::vector<gridBus *> &busList, const std::string &line)
{
  auto pos = line.find_first_of (',');
  auto temp1 = line.substr (0, pos);
  trimString (temp1);
  size_t index = std::stoul (temp1);

  if (index > busList.size ())
    {
      std::cerr << "Invalid bus number for load " << index << '\n';
      return nullptr;
    }
  return busList[index - 1];

}

void loadRAW (coreObject *parentObject,const std::string &filename,const basicReaderInfo &bri)
{

  std::ifstream file (filename.c_str (), std::ios::in);
  std::string line;        //line storage
  std::string temp1;        //temporary storage for substrings
  std::string pref2;       // temp storage to 2nd order prefix.
  std::vector<gridBus *> busList;
  basicReaderInfo opt (bri);
  gridLoad *ld;
  gridDynGenerator *gen;
  gridBus *bus;
  index_t index;
  size_t pos;
  dimeCollector dimesysname;
  /*load up the factories*/
  if (busfactory == nullptr)
    {
      //get the basic busFactory
      busfactory = static_cast<decltype(busfactory)> (coreObjectFactory::instance ()->getFactory ("bus")->getFactory (""));

      //get the basic load Factory
      ldfactory = static_cast<decltype(ldfactory)> (coreObjectFactory::instance ()->getFactory ("load")->getFactory (""));

      //get the basic load Factory
      genfactory = static_cast<decltype(genfactory)> (coreObjectFactory::instance ()->getFactory ("generator")->getFactory (""));
      //get the basic link Factory
      linkfactory = static_cast<decltype(linkfactory)> (coreObjectFactory::instance ()->getFactory ("link")->getFactory (""));
    }
  /* Process the first line
  First card in file.


Columns  2- 9   Date, in format DD/MM/state with leading zeros. If no date
          provided, use 0b/0b/0b where b is blank.
Columns 11-30   Originator's name (A)
Columns 32-37   MVA Base (F*)
Columns 39-42   Year (I)
Column  44      Season (S - Summer, W - Winter)
Column  46-73   Case identification (A) */

  //reset all the object counters
  gridSimulation::resetObjectCounters ();
  //get the base scenario information
  if (std::getline ( file, line ) )
    {
      auto res = sscanf (line.c_str (),"%*d, %lf,%d,%*d,%*d,%lf",&(opt.base),&(opt.version), &(opt.basefreq));

      if (res > 0)
        {
          parentObject->set ("basepower", opt.base);
        }
      //temp1=line.substr(45,27);
      //parentObject->set("name",&temp1);
      if (res > 2)
        {
          if (opt.basefreq != 60.0)
            {
              parentObject->set ("basefreq", opt.basefreq);
            }

        }

      if (opt.version == 0)
        {
          opt.version = getPSSversion (line);
        }
    }
  if (std::getline ( file, line ) )
    {
      pos = line.find_first_of (',');
      temp1 = line.substr (0,pos);
      trimString (temp1);
      parentObject->setName(temp1);
    }
  temp1 = line;
  // get the second comment line and ignore it
  std::getline (file,line);
  temp1 = temp1 + '\n' + line;
  //set the case description
  parentObject->setDescription(temp1);
  //get the bus data section
  //bus data doesn't have a header but it is always first
  int moreData = 1;
  while (moreData)
    {
      if (checkNextLine (file, line))
        {
		  sysparam.push_back(line);
          //get the index
          pos = line.find_first_of (',');
          temp1 = line.substr (0,pos);
          index = std::stoul (temp1);

          if (index > busList.size ())
            {
              if (index < 100000000)
                {
                  busList.resize (2 * index, nullptr);
                }
              else
                {
                  std::cerr << "Bus index overload " << index << '\n';
                }
            }
          if (busList[index - 1] == nullptr)
            {
              busList[index - 1] = busfactory->makeTypeObject ();
              busList[index - 1]->set ("basepower", opt.base);
              busList[index - 1]->setUserID (index);
			 
			  rawReadBus(busList[index - 1], line, opt);
			  try
			  {
				  parentObject->add(busList[index - 1]);
			  }
			  catch (const objectAddFailure &)
			  {
				  addToParentRename(busList[index - 1], parentObject);
			  }
            }
          else
            {
              std::cerr << "Invalid bus code " << index << '\n';
            }

        }
      else
        {
          moreData = 0;
		  dimesysname.send_sysname(sysname);
		  dimesysname.send_sysparam(sysparam);
        }
    }

  stringVec txlines;
  txlines.resize (5);
  int tline = 5;

  bool moreSections = true;

  while (moreSections)
    {
      sections currSection = findSectionType (line);
      moreData = 1;
      switch (currSection)
        {
        case load:
          while (moreData)
            {
              if (checkNextLine (file, line))
                {
                  bus = findBus (busList, line);
                  if (bus)
                    {
                      ld = ldfactory->makeTypeObject ();
                      bus->add (ld);
                      rawReadLoad (ld,line, opt);
                    }
                  else
                    {
                      std::cerr << "Invalid bus number for load " << line.substr (0,30) << '\n';
                    }
                }
              else
                {
                  moreData = 0;
                }
            }
          break;
        case generator:
          while (moreData)
            {
              if (checkNextLine (file, line))
                {
                  bus = findBus (busList, line);
                  if (bus)
                    {
                      gen = genfactory->makeTypeObject ();
                      bus->add (gen);
                      rawReadGen (gen,  line, opt);
                    }
                  else
                    {
                      std::cerr << "Invalid bus number for fixed shunt " << line.substr (0, 30) << '\n';
                    }
                }
              else
                {
                  moreData = 0;
                }
            }
          break;
        case branch:
          while (moreData)
            {
              if (checkNextLine (file, line))
                {
                  rawReadBranch (parentObject,  line,busList, opt);
                }
              else
                {
                  moreData = 0;
                }
            }
          break;
        case fixedShunt:
          while (moreData)
            {
              if (checkNextLine (file, line))
                {
                  bus = findBus (busList, line);
                  if (bus)
                    {
                      ld = ldfactory->makeTypeObject ();
                      bus->add (ld);
                      rawReadFixedShunt (ld, line, opt);
                    }
                  else
                    {
                      std::cerr << "Invalid bus number for fixed shunt " << line.substr (0, 30) << '\n';
                    }
                }
              else
                {
                  moreData = 0;
                }
            }
          break;
        case switchedShunt:
          while (moreData)
            {
              if (checkNextLine (file,line))
                {
                  rawReadSwitchedShunt (parentObject, line,  busList, opt);
                }
              else
                {
                  moreData = 0;
                }
            }
          break;
        case txadj:
          while (moreData)
            {
              if (checkNextLine (file, line))
                {
                  rawReadTXadj (parentObject, line, opt);
                }
              else
                {
                  moreData = 0;
                }
            }
          break;
        case tx:

          while (moreData)
            {
              if (tline == 5)
                {
                  if (checkNextLine (file, line))
                    {
                      txlines[0] = line;
                      std::getline (file, txlines[1]);
                      std::getline (file, txlines[2]);
                      std::getline (file, txlines[3]);
                      std::getline (file, txlines[4]);
                    }
                  else
                    {
                      moreData = 0;
                    }
                }

              else
                {
                  temp1 = txlines[4];
                  trimString (temp1);
                  if (temp1[0] == '0')
                    {
                      moreData = 0;
                      continue;
                    }
                  txlines[0] = temp1;
                  std::getline (file, txlines[1]);
                  std::getline (file, txlines[2]);
                  std::getline (file, txlines[3]);
                  std::getline (file, txlines[4]);
                }
              tline = rawReadTX (parentObject,  txlines, busList, opt);
            }
          break;
        case unknown:
        default:
          while (moreData)
            {
              if (std::getline (file, line))
                {
                  trimString (line);
                  if (line[0] == '0')
                    {
                      moreData = 0;
                      continue;
                    }
                }
              else
                {
                  moreData = 0;
                  moreSections = false;
                }
            }
          break;

        }
    }

  file.close ();
}


int getPSSversion (const std::string &line)
{
  int ver = 29;
  auto slp = line.find_first_of ('/');
  if (slp == std::string::npos)
    {
      return ver;
    }
  else
    {
      auto sloc = line.find ("PSS", slp);
      if (sloc != std::string::npos)
        {
          auto dloc = line.find_first_of ('-', sloc + 3);
          auto sploc = line.find_first_of (' ', dloc);
          ver = std::stoi (line.substr (dloc + 1, sploc - dloc - 2));
        }
      else
        {
          sloc = line.find ("VER", slp);
          if (sloc != std::string::npos)
            {
              ver = std::stoi (line.substr (sloc + 3, 4));
              return ver;
            }
          sloc = line.find ("version", slp);
          if (sloc != std::string::npos)
            {
              ver = std::stoi (line.substr (sloc + 7, 4));
              return ver;
            }
        }
    }
  return ver;
}

/* *INDENT-OFF* */
static const std::map<std::string, sections> sectionNames
{
    {"BEGIN FIXED SHUNT",fixedShunt},
    { "BEGIN SWITCHED SHUNT DATA",switchedShunt},
    { "BEGIN AREA INTERCHANGE DATA",unknown},
    { "BEGIN TWO-TERMINAL DC LINE DATA",unknown},
    { "BEGIN TRANSFORMER IMPEDNCE CORRECTION DATA",unknown},
    { "BEGIN MULTI-TERMINAL DC LINE DATA", unknown},
    { "BEGIN MULTI-SECTION LINE GROUP DATA", unknown},
    { "BEGIN ZONE DATA", unknown},
    { "BEGIN INTER-AREA TRANSFER DATA", unknown},
    { "BEGIN OWNER DATA", unknown},
    { "BEGIN FACTS CONTROL DEVICE DATA", unknown},
    { "BEGIN LOAD DATA", load},
    { "BEGIN GENERATOR DATA", generator},
    { "BEGIN BRANCH DATA", branch},
    { "BEGIN TRANSFORMER ADJUSTMENT DATA", txadj},
    { "BEGIN TRANSFORMER DATA", tx},
};

/* *INDENT-ON* */

sections findSectionType (const std::string &line)
{
  size_t ts;
  for (auto &sname:sectionNames)
    {
      ts = line.find (sname.first);
      if (ts != std::string::npos)
        {
          return sname.second;
        }
    }
  return unknown;
}

void rawReadBus (gridBus *bus, const std::string &line, basicReaderInfo &opt)
{
  double bv;
  double vm;
  double va;
  int type;

  auto strvec = splitline (line);
  //get the bus name
  auto temp = trim(strvec[0]);
  auto temp2 = trim(removeQuotes(strvec[1]));
  sysname.push_back(temp2);
  if (opt.prefix.empty ())
    {
      if (temp2.empty ())         //12 spaces is default value which would all get trimmed
        {
          temp2 =  "BUS_" + temp;
        }
    }
  else
    {
      if (temp2.empty ())         //12 spaces is default value which would all get trimmed
        {
          temp2 = opt.prefix + "_BUS_" + temp;
        }
      else
        {
          temp2 = opt.prefix + '_' + temp2;
        }
    }
  bus->setName(temp2);

  //get the baseVoltage
  bv = std::stod (strvec[2]);
  if (bv > 0.0)
    {
      bus->set ("basevoltage", bv);
    }


  //get the bus type
  if (strvec[3].empty ())
    {
      type = 1;
    }
  else
    {
      type = std::stoi (strvec[3]);
    }

  switch (type)
    {
    case 1:
      temp = "PQ";
      break;
    case 2:
      temp = "PV";
      break;
    case 3:
      temp = "swing";
      break;
    case 4:
      bus->disable();
      temp = "PQ";
	  break;
	default:
		bus->disable();
    }
  bus->set ("type",temp);

  if (opt.version >= 31)
    {
      //skip the load flow area and loss zone for now
      //skip the owner information
      //get the voltage and angle specifications
       vm = numeric_conversion<double>(strvec[7],0.0);
       va = numeric_conversion<double>(strvec[8],0.0);
      if (strvec.size () > 10)
        {
           bv = numeric_conversion<double>(strvec[9],0.0);
          bus->set ("vmax", bv);
           bv = numeric_conversion<double>(strvec[10],0.0);
          bus->set ("vmin", bv);
        }
    }
  else
    {
      //get the zone information
       vm = numeric_conversion<double>(strvec[7],0.0);
      bus->set ("zone",vm);

       vm = numeric_conversion<double>(strvec[8],0.0);
       va = numeric_conversion<double>(strvec[9],0.0);
      //load the fixed shunt data
      double p, q;

       p = numeric_conversion<double>(strvec[4],0.0);
       q = numeric_conversion<double>(strvec[5],0.0);
      if ((p != 0) || (q != 0))
        {
          auto ld = ldfactory->makeTypeObject ();
          bus->add (ld);
          if (p != 0.0)
            {
              ld->set ("yp", p, MW);
            }
          if (q != 0.0)
            {
              ld->set ("yq", -q, MVAR);
            }

        }
    }


  if (va != 0)
    {
      bus->set ("angle", va,deg);
    }
  if (vm != 0)
    {
      bus->set ("voltage", vm);
    }
}

void rawReadLoad (gridLoad *ld, const std::string &line, basicReaderInfo &)
{

  double p;
  double q;
  int status;

  auto strvec = splitline (line);

  //get the load index and name
  auto temp = trim(removeQuotes(strvec[1]));

  auto prefix = ld->getParent ()->getName () + "_load_" + temp;
  ld->setName (prefix);

  //get the status
  status = std::stoi (strvec[2]);
  if (status == 0)
    {
      ld->disable();
    }
  //skip the area and zone information for now

  //get the constant power part of the load
  p = numeric_conversion<double>(strvec[5],0.0);
  q = numeric_conversion<double>(strvec[6],0.0);
  if (p != 0.0)
    {
      ld->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("q", q, MVAR);
    }
  //get the constant current part of the load
  p = numeric_conversion<double>(strvec[7],0.0);
  q = numeric_conversion<double>(strvec[8],0.0);
  if (p != 0.0)
    {
      ld->set ("ip", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("iq", q, MVAR);
    }
  //get the impedance part of the load
  p = numeric_conversion<double>(strvec[9],0.0);
  q = numeric_conversion<double>(strvec[10],0.0);
  if (p != 0.0)
    {
      ld->set ("yp", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("yq", -q, MVAR);
    }
  //ignore the owner field

}

void rawReadFixedShunt (gridLoad *ld, const std::string &line, basicReaderInfo &)
{
  std::string prefix;
  double p;
  double q;
  int status;

  auto strvec = splitline (line);

  //get the load index and name
  auto temp = trim(removeQuotes(strvec[1]));
  prefix = ld->getParent ()->getName () + "_shunt_" + temp;
  ld->setName ( prefix);

  //get the status
  status = std::stoi (strvec[2]);
  if (status == 0)
    {
      ld->disable();
    }
  //skip the area and zone information for now

  //get the constant power part of the load
  p = numeric_conversion<double>(strvec[3],0.0);
  q = numeric_conversion<double>(strvec[4],0.0);
  if (p != 0.0)
    {
      ld->set ("yp", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("yq", -q, MVAR);
    }


}


void rawReadGen (gridDynGenerator *gen, const std::string &line, basicReaderInfo &opt)
{

  double p;
  double q;
  double V;
  double mb;
  int rbus;
  int status;

  auto strvec = splitline (line);

  //get the load index and name
  auto temp = trim(removeQuotes(strvec[1]));

  auto prefix = gen->getParent ()->getName () + "_Gen_" + temp;
  gen->setName (prefix);
  //get the status
  status = std::stoi (strvec[14]);
  if (status == 0)
    {
      gen->disable();
    }

  //get the power generation
  p = numeric_conversion<double>(strvec[2],0.0);
  q = numeric_conversion<double>(strvec[3],0.0);
  if (p != 0.0)
    {
      gen->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("q", q, MVAR);
    }
  //get the Qmax and Qmin
  p = numeric_conversion<double>(strvec[4],0.0);
  q = numeric_conversion<double>(strvec[5],0.0);
  if (p != 0.0)
    {
      gen->set ("qmax", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("qmin", q, MVAR);
    }
  V = numeric_conversion<double>(strvec[6],0.0);
  if (V > 0)
    {
      double vp = gen->getParent ()->get ("vtarget");
      if (std::abs (vp - V) > 0.0001)
        {
          gen->set ("vtarget", V);
          //for raw files the bus doesn't necessarily set a control point it comes from the generator, so we
          //have to set it here.
          gen->getParent ()->set ("vtarget", V);
          gen->getParent ()->set ("v", V);
        }
      else
        {
          gen->set ("vtarget", vp);
        }
    }
   rbus = numeric_conversion<int>(strvec[7],0);

  if (rbus != 0)
    {
      gridBus *remoteBus = static_cast<gridBus *> (gen->getParent ()->getParent ()->findByUserID ("bus",rbus));
      gen->add (remoteBus);
    }

   mb = numeric_conversion<double>(strvec[8],0.0);
  gen->set ("mbase", mb);

  double r, x;
   r = numeric_conversion<double>(strvec[9],0.0);
  gen->set ("rs", r);

   x = numeric_conversion<double>(strvec[10],0.0);
  gen->set ("xs", x);

  if (!CHECK_CONTROLFLAG (opt.flags,ignore_step_up_transformer))
    {
      r = numeric_conversion<double>(strvec[11],0.0);
       x = numeric_conversion<double>(strvec[12],0.0);
      if ((r != 0)||(x != 0)) //need to add a step up transformer
        {
          gridBus *oBus = static_cast<gridBus *> (gen->getParent ());
          gridBus *nBus = busfactory->makeTypeObject ();
          acLine *lnk = new acLine (r * opt.base / mb, x * opt.base / mb); //we need to adjust to the simulation base as opposed to the machine base

          oBus->remove (gen);
          nBus->add (gen);

          lnk->updateBus (oBus,1);
          lnk->updateBus (nBus,2);



          if (!gen->isEnabled())
            {
              nBus->disable ();
            }
          if (!oBus->isEnabled())
            {
              nBus->disable ();
            }
          oBus->getParent ()->add (nBus);
          oBus->getParent ()->add (lnk);
          if (gen->getName ().compare (0,oBus->getName ().length (),oBus->getName ()) == 0)
            {
              lnk->setName (gen->getName () + "_TX");
              nBus->setName (gen->getName () + "_TXBUS");
            }
          else
            {
              lnk->setName (oBus->getName () + '_' + gen->getName () + "_TX");
              nBus->setName ( oBus->getName () + '_' + gen->getName () + "_TXBUS");
            }
          //get the tap ratio
          r = numeric_conversion<double>(strvec[13],0.0);
          lnk->set ("tap",r);
          //match the voltage and angle of the other bus
          nBus->setVoltageAngle (oBus->getVoltage () * r, oBus->getAngle ());
          gen->add (oBus);
          //get the power again for the generator
           p = numeric_conversion<double>(strvec[2],0.0);
           q = numeric_conversion<double>(strvec[3],0.0);
          //now adjust the newBus angle and Voltage to match the power flows
          lnk->fixPower (-p,-q, 1,1, MVAR);
          if (!gen->isEnabled())
            {
              nBus->disable ();
            }
        }
    }
}


void rawReadBranch (coreObject *parentObject, const std::string &line, std::vector<gridBus *> &busList, basicReaderInfo &opt)
{
  auto strvec = splitline (line);

  std::string temp = trim (strvec[0]);
  int ind1 = std::stoi (temp);
  std::string temp2;
  if (opt.prefix.empty ())
    {
      temp2 = temp + "_to_";
    }
  else
    {
      temp2 = opt.prefix + '_' + temp + "_to_";
    }

  temp = trim (strvec[1]);
  int ind2 = std::stoi (temp);

  // SGS 2015/02/25
  // SGS check with Philip.
  // Swap to/from buses if toBus is negative.  toBus is the metered end.
  // Or should this just take ind2 = abs(ind2).  Min email suggested swapping the end points.
  if (ind2 < 0)
    {
      // int tmp=ind1;
      ind2 = abs (ind2);
      // ind2 = tmp;
      temp.erase (temp.begin ());
    }

  if ((ind1 < 1) || (ind2 < 1)||(ind1 > static_cast<int> (busList.size ()))||(ind2 > static_cast<int> (busList.size ())))
    {
      std::cerr << "invalid link buses\n";
      assert (false);
    }


  temp2 = temp2 + temp;


  temp = strvec[2];
  removeQuotes (temp);
  trimString (temp);
  if (temp != "1")
    {
      temp2 = temp2 + '_' + temp;
    }

  acLine *lnk = linkfactory->makeTypeObject (temp2);
  //set the base power to that used this model
  lnk->set ("basepower", opt.base);

  lnk->updateBus (busList[ind1 - 1],1);
  lnk->updateBus (busList[ind2 - 1],2);

  //check for circuit identifier

  try
  {
	  parentObject->add(lnk);
  }
  catch (const objectAddFailure &)
  {
	  //must be a parallel branch
	  std::string sub = lnk->getName();
	  char m = 'a';
	  while (lnk->isRoot())
	  {
		  lnk->setName(sub + '_' + m);
		  m = m + 1;
		  try
		  {
			  parentObject->add(lnk);
		  }
		  catch (const objectAddFailure &e)
		  {
			  if (m > 'z')
			  {
				  throw(e);
			  }
		  }
	  }
  }
  

  double R = numeric_conversion (strvec[3],0.0);
  double X = numeric_conversion (strvec[4],0.0);
  //get line impedances and resistance
  lnk->set ("r", R);
  lnk->set ("x", X);
  //get line capacitance
  double val = numeric_conversion (strvec[5],0.0);
  lnk->set ("b", val);

  int status;
  if (opt.version >= 29)
    {
      status = std::stoi (strvec[13]);
      if (status == 0)
        {
          lnk->disable();
        }
    }
  else
    {
      status = std::stoi (strvec[15]);
      if (status == 0)
        {
          lnk->disable();
        }
    }
  if (opt.version <= 26)  //transformers described in this section and in TX adj section
    {
      val = numeric_conversion (strvec[9],0.0);
      if (val != 0.0)
        {
          lnk->set ("tap",val);
          val = numeric_conversion<double>(strvec[10],0.0);
          if (val != 0)
            {
              lnk->set ("tapAngle",val,deg);
            }
        }
    }

  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance



  //TODO get the other parameters (not critical for power flow)
}

void rawReadTXadj (coreObject *parentObject, const std::string &line, basicReaderInfo &opt)
{
  std::string temp, temp2;
  acLine *lnk;
  int code = 1;
  int ind1, ind2,cind;
  double mx, mn;
  double Ta;
  double val;
  //int status;

  auto strvec = splitline (line);

  temp = strvec[0];
  ind1 = std::stoi (temp);
  if (opt.prefix.empty ())
    {
      temp2 = temp + "_to_";
    }
  else
    {
      temp2 = opt.prefix + '_' + temp + "_to_";
    }

  temp = strvec[1];
  ind2 = std::stoi (temp);

  if (ind2 < 0)
    {
      // int tmp=ind1;
      //negate = true;
      ind2 = abs (ind2);
      temp.erase (temp.begin ());
      // ind2 = tmp;
    }
  temp2 = temp2 + temp;
  //get the circuit identifier
  temp = strvec[2];
  removeQuotes (temp);
  if (temp != "1")
    {
      temp2 = temp2 + '_' + temp;
    }

  lnk = static_cast<acLine *> (parentObject->find (temp2));

  if (!lnk)
    {
      parentObject->log (parentObject,print_level::error,"unable to locate link " + temp2);
      return;
    }

  adjustableTransformer *adjTX = new adjustableTransformer ();
  lnk->clone (adjTX);
  lnk->addOwningReference();
  parentObject->remove (lnk);
  adjTX->updateBus (lnk->getBus (1),1);
  adjTX->updateBus (lnk->getBus (2),2);
  lnk->updateBus (nullptr,1);
  lnk->updateBus (nullptr,2);
  removeReference(lnk);
  parentObject->add (adjTX);
  Ta = adjTX->getTapAngle ();
  if (Ta != 0)
    {
      adjTX->set ("mode","mw");
      adjTX->set ("stepmode","continuous");
      code = 3;
    }
  else
    {
      adjTX->set ("mode","voltage");
      code = 1;
    }
  //get the control bus
  if (code != 3)
    {
       cind = numeric_conversion<int>(strvec[3],0);
      if (cind > 0)
        {
          if (cind == ind1)
            {
              adjTX->setControlBus (1);
            }
          else if (cind == ind2)
            {
              adjTX->setControlBus (2);
            }
          else
            {
              adjTX->setControlBus (static_cast<gridBus *> (adjTX->getParent ()->findByUserID ("bus",cind)));
            }
        }
      else
        {
          if (-cind == ind1)
            {
              adjTX->setControlBus (1);
            }
          else if (-cind == ind2)
            {
              adjTX->setControlBus (2);
            }
          else
            {
              adjTX->setControlBus (static_cast<gridBus *> (adjTX->getParent ()->findByUserID ("bus", -cind)));
              adjTX->set ("direction", -1);
            }

        }
    }
  //
  mx = numeric_conversion<double>(strvec[4],0.0);
  mn = numeric_conversion<double>(strvec[5],0.0);
  if ((mx - mn > 1.0)&&(code != 3))
    {
      adjTX->set ("mode", "mw");
      adjTX->set ("stepmode", "continuous");
      code = 3;
    }
  if (code == 3)
    {
      //not sure why I need this but
      Ta = Ta * 180 / kPI;
      if (Ta > mx)
        {
          mx = Ta;
        }
      if (Ta < mn)
        {
          mn = Ta;
        }
      adjTX->set ("maxtapangle", mx, deg);
      adjTX->set ("mintapangle", mn, deg);
    }
  else
    {
      if (mx < mn)
        {
          std::swap (mx,mn);
        }
      adjTX->set ("maxtap", mx);
      adjTX->set ("mintap", mn);
    }
   mx = numeric_conversion<double>(strvec[6],0.0);
   mn = numeric_conversion<double>(strvec[7],0.0);
  if ((mx - mn > 1.0) && (code == 1))
    {
      adjTX->set ("mode", "mvar");
      code = 2;
    }
  if (code == 1)
    {
      if (mx - mn > 0.00001)
        {
          adjTX->set ("vmax", mx);
          adjTX->set ("vmin", mn);
        }
    }
  else if (code == 3)
    {
      if (mx - mn > 0.00001)
        {
          adjTX->set ("pmax", mx, MW);
          adjTX->set ("pmin", mn, MW);
        }
    }
  else
    {
      if (mx - mn > 0.00001)
        {
          adjTX->set ("qmax", mx,MVAR);
          adjTX->set ("qmin", mn,MVAR);
        }
    }
  if (code != 3) //get the stepsize
    {
       val = numeric_conversion<double>(strvec[8],0.0);
      if (val != 0)
        {
          //abs required since for some reason the file can have negative step sizes
          //I think just to do reverse indexing which I don't do.
          adjTX->set ("step",std::abs (val));
        }
      else
        {
          adjTX->set ("stepmode","continuous");
        }
    }
  cind = numeric_conversion<int>(strvec[9], 0);
  if (cind != 0)
    {
      parentObject->log (parentObject, print_level::warning, "transformer impedance tables not implemented yet ");
    }
  cind = numeric_conversion<int>(strvec[10], 0);
  {
    if (cind == 0)
      {
        adjTX->set ("no_pflow_adjustments",1);
      }
  }
   mx = numeric_conversion<double>(strvec[11],0.0);
   mn = numeric_conversion<double>(strvec[12],0.0);
  if ((mx != 0)||(mn != 0))
    {
      parentObject->log (parentObject, print_level::warning, "load drop compensation not implemented yet ");
    }

}

int rawReadTX (coreObject *parentObject, stringVec &txlines, std::vector<gridBus *> &busList, basicReaderInfo &opt)
{
  int tline = 4;
  std::string temp2;
  gridBus *bus1, *bus2;
  //gridBus *bus3;
  acLine *lnk = nullptr;
  int status;

  stringVec strvec,strvec2,strvec3,strvec4,strvec5;
  strvec = splitline (txlines[0]);

  strvec2 = splitline (txlines[1]);
  strvec3 = splitline (txlines[2]);
  strvec4 = splitline (txlines[3]);

  auto temp = strvec[0];

  int ind1 = std::stoi (temp);
  temp = strvec[1];
  int ind2 = std::stoi (temp);

  temp = strvec[2];
  int ind3 = std::stoi (temp);
  if (ind3 != 0)
    {
      tline = 5;
      strvec5 = splitline (txlines[4]);
      //TODO handle 3 way transformers(complicated)
      std::cout << "3 winding transformers not supported at this time\n";
      return tline;
    }
  else
    {
      trimString (strvec[0]);
      if (opt.prefix.empty ())
        {
          temp2 = "tx_" + strvec[0] + "_to_";
        }
      else
        {
          temp2 = opt.prefix + "_tx_" + strvec[0] + "_to_";
        }
    }

  temp2 = temp2 + trim(strvec[1]);
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];
  int code = std::stoi (strvec3[6]);
  // SGS FIXME!!!!
  // Code is negative in PJM version 30 raw file....hack to make it in valid range.
  switch (abs (code))
    {
    case 0:
      lnk = linkfactory->makeTypeObject ();
      break;
    case 1:
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "voltage");
      break;
    case 2:
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "mvar");
      break;
    case 3:
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "mw");
      break;
    }
  if (code < 0) //account for negative code values
    {
      lnk->set ("mode", "manual");
    }
  lnk->set ("basepower", opt.base);
  lnk->updateBus (bus1, 1);
  lnk->updateBus (bus2, 2);
  lnk->setName (temp2);
  try
  {
	  parentObject->add(lnk);
  }
  catch (const objectAddFailure &)
  {
	  //must be a parallel branch
	  std::string sub = lnk->getName();
	  char m = 'a';
	  while (lnk->isRoot())
	  {
		  lnk->setName(sub + '_' + m);
		  m = m + 1;
		  try
		  {
			  parentObject->add(lnk);
		  }
		  catch (const objectAddFailure &e)
		  {
			  if (m > 'z')
			  {
				  throw(e);
			  }
		  }
	  }
  }

  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance

  double R = numeric_conversion<double>(strvec2[0],0.0);
  double X = numeric_conversion<double>(strvec2[1],0.0);

  lnk->set ("r", R);
  lnk->set ("x", X);
  //get line capacitance


  status = std::stoi (strvec[11]);
  if (status == 0)
    {
      lnk->disable();
    }
  else if (status > 1)
    {
      //TODO:  other conditions for 3 way transformers
    }


  //TODO:PT get the other parameters (not critical for power flow)

  double val = numeric_conversion<double>(strvec3[0],0.0);
  if (val != 0)
    {
      lnk->set ("tap", val);
    }
  val = numeric_conversion<double>(strvec3[2],0.0);
  if (val != 0)
    {
      lnk->set ("tapangle", val,deg);
    }
  //now get the stuff for the adjustable transformers
  // SGS set this for adjustable transformers....is this correct?
  if (abs (code) > 0)
    {
	  int cbus = numeric_conversion<int>(strvec3[7], 0);
      if (cbus != 0)
        {
          static_cast<adjustableTransformer *> (lnk)->setControlBus (busList[cbus - 1]);
        }
       R = numeric_conversion<double>(strvec3[8],0.0);
       X = numeric_conversion<double>(strvec3[9],0.0);
      if (code == 3)
        {
          lnk->set ("maxtapangle", R,deg);
          lnk->set ("mintapangle", X,deg);
        }
      else
        {
          lnk->set ("maxtap", R);
          lnk->set ("mintap", X);
        }
       R = numeric_conversion<double>(strvec3[10],0.0);
       X = numeric_conversion<double>(strvec3[11],0.0);
      if (code == 3)
        {
          lnk->set ("pmax", R, MW);
          lnk->set ("pmin", X, MW);
        }
      else if (code == 2)
        {
          lnk->set ("qmax", R,MVAR);
          lnk->set ("qmin", X,MVAR);
        }
      else
        {
          lnk->set ("vmax", R);
          lnk->set ("vmin", X);
        }
       R = numeric_conversion<double>(strvec3[12],0.0);
      if (code != 3)
        {
          lnk->set ("nsteps", R);
        }
    }
  return tline;
}

int rawReadDCLine (coreObject * /*parentObject*/, stringVec & /*txlines*/, std::vector<gridBus *> & /*busList*/, basicReaderInfo &)
{
  return 0;
}

void rawReadSwitchedShunt (coreObject *parentObject, const std::string &line, std::vector<gridBus *> &busList, basicReaderInfo &opt)
{
  auto strvec = splitline (line);

  unsigned int index;
  index = std::stoul (strvec[0]);
  gridBus *rbus = nullptr;
  svd *ld = nullptr;
  double temp;
  if (index > busList.size ())
    {
      std::cerr << "Invalid bus number for load " << index << '\n';
    }
  if (busList[index - 1] == nullptr)
    {
      std::cerr << "Invalid bus number for load " << index << '\n';

    }
  else
    {
      ld = new svd ();
      busList[index - 1]->add (ld);
    }

  int mode = numeric_conversion<int>(strvec[1],0);
  double high = numeric_conversion<double>(strvec[2],0.0);
  double low = numeric_conversion<double>(strvec[3],0.0);
  //get the controlled bus
  int cbus = numeric_conversion<int>(strvec[4], -1);

  if (cbus < 0)
    {
      trimString (strvec[4]);
      if (strvec[4] == "I")
        {
          cbus = index;
        }
      else if (strvec[4].empty ())
        {
          cbus = index;
        }
      else
        {
          rbus = static_cast<gridBus *> (parentObject->find (strvec[4]));
          if (rbus)
            {
              cbus = rbus->getUserID ();
            }
        }
    }
  else if (cbus == 0)
    {
      cbus = index;
    }
  else
    {
      rbus = busList[cbus - 1];
    }

  switch (mode)
    {
    case 0:
      ld->set ("mode","manual");
      break;
    case 1:
      ld->set ("mode", "stepped");
      ld->set ("vmax",high);
      ld->set ("vmin",low);
      if (cbus != static_cast<int> (index))
        {
          ld->setControlBus (rbus);
        }

	  temp = numeric_conversion<double>(strvec[5], 0.0);
      if (temp > 0)
        {
          ld->set ("participation",temp / 100.0);
        }
      break;
    case 2:
      ld->set ("mode", "cont");
      ld->set ("vmax", high);
      ld->set ("vmin", low);
      if (cbus != static_cast<int> (index))
        {
          ld->setControlBus (rbus);
        }
	  temp = numeric_conversion<double>(strvec[5], 0.0);
      if (temp > 0)
        {
          ld->set ("participation", temp / 100.0);
        }
      break;
    case 3:
      ld->set ("mode", "stepped");
      ld->set ("control","reactive");
      ld->set ("qmax", high);
      ld->set ("qmin", low);
      if (cbus != static_cast<int> (index))
        {
          ld->setControlBus (rbus);
        }
      break;
    case 4:
      ld->set ("mode", "stepped");
      ld->set ("control", "reactive");
      ld->set ("qmax", high);
      ld->set ("qmin", low);
      if (cbus != static_cast<int> (index))
        {
          ld->setControlBus (rbus);
        }
      //TODO: PT load target object note:unusual condition
      break;
    case 5:
      ld->set ("mode", "stepped");
      ld->set ("control", "reactive");
      ld->set ("qmax", high);
      ld->set ("qmin", low);
      if (cbus != static_cast<int> (index))
        {
          ld->setControlBus (rbus);
        }
      break;
    case 6:
      ld->set ("mode", "stepped");
      ld->set ("control", "reactive");
      ld->set ("qmax", high);
      ld->set ("qmin", low);
      if (cbus != static_cast<int> (index))
        {
          ld->setControlBus (rbus);
        }
      //TODO: PT load target object note:unusual condition
      break;
    default:
      ld->set ("mode", "manual");
      break;


    }
  //load the switched shunt blocks
  int start = 7;
  if (opt.version <= 27)
    {
      start = 5;
    }

  int cnt = 0;
  double block = 0.0;
  size_t ksize = strvec.size () - 1;
  for (size_t kk = start + 1; kk < ksize; kk += 2)
    {
      cnt = numeric_conversion<int>(strvec[kk],0);
      block = numeric_conversion<double>(strvec[kk + 1],0.0);
      if ((cnt > 0)&&(block != 0))
        {
          ld->addBlock (cnt,-block,MVAR);
        }
      else
        {
          break;
        }
    }
  //set the initial value
  temp = numeric_conversion<double>(strvec[start], 0.0);

  ld->set ("yq",-temp,MVAR);

}
