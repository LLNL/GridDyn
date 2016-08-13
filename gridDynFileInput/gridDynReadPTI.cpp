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


#include "gridDynFileInput.h"
#include "readerHelper.h"
#include "gridBus.h"
#include "loadModels/gridLoad.h"
#include "linkModels/acLine.h"
#include "generators/gridDynGenerator.h"
#include "objectFactoryTemplates.h"
#include "stringOps.h"

#include <fstream>
#include <cstdlib>
#include <iostream>



using namespace gridUnits;


void ptiReadBus (gridBus *bus, const std::string &line, basicReaderInfo &opt);
void ptiReadLoad (gridLoad *ld, const std::string &line, basicReaderInfo &opt);
void ptiReadFixedShunt (gridLoad *ld, const std::string &line, basicReaderInfo &opt);
void ptiReadGen (gridDynGenerator *gen, const std::string &line, basicReaderInfo &opt);
void ptiReadBranch (gridCoreObject *parentObject, const std::string &line, std::vector<gridBus *> &busList, basicReaderInfo &opt);
int ptiReadTX (gridCoreObject *parentObject, stringVec &txlines, std::vector<gridBus *> &busList, basicReaderInfo &opt);

//static variables with the factories
//get the basic busFactory
static typeFactory<gridBus> *busfactory = nullptr;

//get the basic load Factory
static typeFactory<gridLoad> *ldfactory = nullptr;
//get the basic Link Factory
static typeFactory<gridLink> *linkfactory = nullptr;
//get the basic Generator Factory
static typeFactory<gridDynGenerator> *genfactory = nullptr;


void loadPTI (gridCoreObject *parentObject, const std::string &filename,const basicReaderInfo &bri)
{
  std::ifstream file (filename.c_str (), std::ios::in);
  std::string line;        //line storage
  std::string temp1;        //temporary storage for substrings
  std::string pref2;       // temp storage to 2nd order prefix.
  std::vector<gridBus *> busList;
  gridLoad *ld;
  gridDynGenerator *gen;
  index_t index;
  size_t pos;
  basicReaderInfo opt (&bri);

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


  if (std::getline (file, line))
    {
      auto res = sscanf (line.c_str (), "%*d, %lf,%*d,%*d,%*d,%lf", &(opt.base), &(opt.basefreq));
      if (res > 0)
        {
          parentObject->set ("systemBasePower", opt.base);
        }
      //temp1=line.substr(45,27);
      //parentObject->set("name",&temp1);
    }
  if (std::getline (file, line))
    {
      pos = line.find_first_of (',');
      temp1 = line.substr (0, pos);
      trimString (temp1);
      parentObject->set ("name", temp1);
    }
  // get the second comment line and ignore it
  std::getline (file, line);
  //get the bus data section

  int moreData = 1;
  while (moreData)
    {
      if (std::getline (file, line))
        {

          if (line[0] == '0')
            {
              moreData = 0;
              continue;
            }
          //get the index
          pos = line.find_first_of (",");
          temp1 = line.substr (0, pos);
          trimString (temp1);
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
              busList[index - 1]->setUserID (index);
              ptiReadBus (busList[index - 1], line, opt);
              parentObject->add (busList[index - 1]);
              if (busList[index - 1]->getParent () != parentObject)
                {

                  busList[index - 1]->setName (busList[index - 1]->getName () + "BUS-" + std::to_string (index));
                  parentObject->add (busList[index - 1]);
                  if (busList[index - 1]->getParent () != parentObject)
                    {
                      std::cerr << "Unable to add bus " << index << '\n';
                    }
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
        }
    }
  moreData = 1;
  //get the load data section data
  while (moreData)
    {
      if (std::getline (file, line))
        {

          if (line[0] == '0')
            {
              moreData = 0;
              continue;
            }
          //get the bus index
          pos = line.find_first_of (",");
          temp1 = line.substr (0, pos);
          trimString (temp1);
          index = std::stoul (temp1);

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
              ld = ldfactory->makeTypeObject ();
              busList[index - 1]->add (ld);
              ptiReadLoad (ld, line, opt);

            }

        }
      else
        {
          moreData = 0;
        }
    }
  //get the Fixed Shunt data
  moreData = 1;
  while (moreData)
    {
      if (std::getline (file, line))
        {
          if (line[0] == '0')
            {
              moreData = 0;
              continue;
            }
          //get the bus index
          pos = line.find_first_of (',');
          temp1 = line.substr (0, pos);
          trimString (temp1);
          index = std::stoul (temp1);

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
              ld = ldfactory->makeTypeObject ();
              busList[index - 1]->add (ld);
              ptiReadFixedShunt (ld, line, opt);

            }

        }
      else
        {
          moreData = 0;
        }
    }
  //get the generator Data
  moreData = 1;
  while (moreData)
    {
      if (std::getline (file, line))
        {
          if (line[0] == '0')
            {
              moreData = 0;
              continue;
            }
          //get the bus index
          pos = line.find_first_of (",");
          temp1 = line.substr (0, pos);
          trimString (temp1);
          index = std::stoul (temp1);

          if (index > busList.size ())
            {
              std::cerr << "Invalid bus number for generator " << index << '\n';
            }
          if (busList[index - 1] == nullptr)
            {
              std::cerr << "Invalid bus number for generator " << index << '\n';

            }
          else
            {
              gen = genfactory->makeTypeObject ();
              busList[index - 1]->add (gen);
              ptiReadGen (gen, line, opt);

            }

        }
      else
        {
          moreData = 0;
        }
    }
  //get the transmission line data
  moreData = 1;
  while (moreData)
    {
      if (std::getline (file, line))
        {
          if (line[0] == '0')
            {
              moreData = 0;
              continue;
            }
          ptiReadBranch (parentObject, line, busList, opt);
        }
      else
        {
          moreData = 0;
        }
    }
  //read the transformer data
  moreData = 1;
  stringVec txlines;
  txlines.resize (5);
  int tline = 5;
  while (moreData)
    {
      if (std::getline (file, line))
        {
          if (line[0] == '0')
            {
              moreData = 0;
              continue;
            }
          if (tline == 5)
            {
              txlines[0] = line;
              std::getline (file, txlines[1]);
              std::getline (file, txlines[2]);
              std::getline (file, txlines[3]);
              std::getline (file, txlines[4]);
            }
          else
            {
              if (txlines[4][0] == '0')
                {
                  moreData = 0;
                  continue;
                }
              txlines[0] = txlines[4];
              txlines[1] = line;
              std::getline (file, txlines[2]);
              std::getline (file, txlines[3]);
              std::getline (file, txlines[4]);
            }
          tline = ptiReadTX (parentObject, txlines, busList, opt);
        }
      else
        {
          moreData = 0;
        }
    }
  file.close ();
}


void ptiReadBus (gridBus *bus, const std::string &line, basicReaderInfo &opt)
{
  std::string temp, temp2;
  double bv;
  double vm;
  double va;
  int type;

  auto strvec = splitline (line);
  //get the bus name
  temp = strvec[0];
  trimString (temp);
  temp2 = strvec[1];
  //check for quotes on the name
  removeQuotes (temp2);
  if (opt.prefix.empty ())
    {
      if (temp2.empty ())         //12 spaces is default value which would all get trimmed
        {
          temp2 = "BUS_" + temp;
        }
    }
  else
    {
      if (temp2.empty ())         //12 spaces is default value which would all get trimmed
        {
          temp2 = opt.prefix + '_' + temp;
        }
      else
        {
          temp2 = opt.prefix + '_' + temp2;
        }
    }
  bus->set ("name", temp2);

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
      bus->enabled = false;
      temp = "PQ";
    }
  bus->set ("type", temp);
  //skip the load flow area and loss zone for now
  //skip the owner information
  //get the voltage and angle specifications
  paramRead (strvec[7], vm);
  paramRead (strvec[8], va);
  if (va != 0)
    {
      bus->set ("angle", va / 180 * kPI);
    }
  if (vm != 0)
    {
      bus->set ("voltage", vm);
    }
}

void ptiReadLoad (gridLoad *ld, const std::string &line, basicReaderInfo & /*opt*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int status;

  auto strvec = splitline (line);

  //get the load index and name
  temp = strvec[1];
  trimString (temp);
  prefix = ld->getParent ()->getName () + "_load_" + temp;
  ld->setName (prefix);

  //get the status
  status = std::stoi (strvec[2]);
  if (status == 0)
    {
      ld->enabled = false;
    }
  //skip the area and zone information for now

  //get the constant power part of the load
  paramRead (strvec[5], p);
  paramRead (strvec[6], q);
  if (p != 0.0)
    {
      ld->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("q", q, MVAR);
    }
  //get the constant current part of the load
  paramRead (strvec[7], p);
  paramRead (strvec[8], q);
  if (p != 0.0)
    {
      ld->set ("ip", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("iq", q, MVAR);
    }
  //get the impedance part of the load
  //note:: in PU power units, need to convert to Pu resistance
  paramRead (strvec[9], p);
  paramRead (strvec[10], q);
  if (p != 0.0)
    {
      ld->set ("r", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("x", q, MVAR);
    }
  //ignore the owner field

}

void ptiReadFixedShunt (gridLoad *ld, const std::string &line, basicReaderInfo & /*opt*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int status;

  auto strvec = splitline (line);

  //get the load index and name
  temp = strvec[1];
  trimString (temp);
  prefix = ld->getParent ()->getName () + "_shunt_" + temp;
  ld->setName ( prefix);

  //get the status
  status = std::stoi (strvec[2]);
  if (status == 0)
    {
      ld->enabled = false;
    }
  //skip the area and zone information for now

  //get the constant power part of the load
  paramRead (strvec[3], p);
  paramRead (strvec[4], q);
  if (p != 0.0)
    {
      ld->set ("yp", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("yq", -q, MVAR);
    }


}


void ptiReadGen (gridDynGenerator *gen, const std::string &line, basicReaderInfo & /*opt*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  double V;
  int rbus;
  int status;

  auto strvec = splitline (line);

  //get the load index and name
  temp = strvec[1];
  prefix = gen->getParent ()->getName () + "_Gen_" + temp;
  gen->set ("name", prefix);
  //get the status
  status = std::stoi (strvec[14]);
  if (status == 0)
    {
      gen->enabled = false;
    }

  //get the power generation
  paramRead (strvec[2], p);
  paramRead (strvec[3], q);
  if (p != 0.0)
    {
      gen->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("q", q, MVAR);
    }
  //get the Qmax and Qmin
  paramRead (strvec[4], p);
  paramRead (strvec[5], q);
  if (p != 0.0)
    {
      gen->set ("qmax", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("qmin", q, MVAR);
    }
  paramRead (strvec[6], V);
  if (V > 0)
    {
      gen->set ("vset", V);
    }
  paramRead (strvec[7], rbus);

  if (rbus != 0)
    {
      //TODO something tricky as it is a remote controlled bus
    }
  //TODO  get the impedance fields and other data

}


void ptiReadBranch (gridCoreObject *parentObject, const std::string &line, std::vector<gridBus *> &busList, basicReaderInfo &opt)
{
  std::string temp, temp2;
  gridBus *bus1, *bus2;
  gridLink *lnk;
  int ind1, ind2;
  double R, X;
  double val;
  int status;

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

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];

  lnk = linkfactory->makeTypeObject ();
  lnk->updateBus (bus1, 1);
  lnk->updateBus (bus2, 2);
  lnk->setName (temp2);

  parentObject->add (lnk);

  status = std::stoi (strvec[13]);
  if (status == 0)
    {
      lnk->enabled = false;
    }

  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance

  paramRead (strvec[3], R);
  paramRead (strvec[4], X);

  lnk->set ("r", R);
  lnk->set ("x", X);
  //get line capacitance
  paramRead (strvec[5], val);
  lnk->set ("b", val);

  //TODO get the other parameters (not critical for power flow)
}

int ptiReadTX (gridCoreObject *parentObject, stringVec &txlines, std::vector<gridBus *> &busList, basicReaderInfo &opt)
{
  int tline = 4;
  std::string temp, temp2;
  gridBus *bus1, *bus2;
  //gridBus *bus3;
  gridLink *lnk;
  int code;
  int ind1, ind2, ind3;
  double R, X;
  double val;
  int status;

  stringVec strvec, strvec2, strvec3, strvec4, strvec5;
  strvec = splitline (txlines[0]);

  strvec2 = splitline (txlines[1]);
  strvec3 = splitline (txlines[2]);
  strvec4 = splitline (txlines[3]);

  temp = strvec[0];
  ind1 = std::stoi (temp);
  temp = strvec[1];
  ind2 = std::stoi (temp);

  temp = strvec[2];
  ind3 = std::stoi (temp);
  if (ind3 != 0)
    {
      tline = 5;
      strvec5 = splitline (txlines[4] );
      //TODO handle 3 way transformers(complicated)
      std::cout << "3 winding transformers not supported at this time\n";
      return tline;
    }
  else
    {
      if (opt.prefix.empty ())
        {
          temp2 = "tx_" + temp + "_to_";
        }
      else
        {
          temp2 = opt.prefix + "_tx_" + temp + "_to_";
        }
    }
  temp = strvec[1];
  ind2 = std::stoi (temp);

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];
  code = std::stoi (strvec3[6]);
  switch (code)
    {
    case 0:
      lnk = linkfactory->makeTypeObject ();
      lnk->set ("type", "transformer");
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
    default:
      parentObject->log (parentObject,GD_WARNING_PRINT,"Unrecognized link code assuming transformer" + std::to_string (code));
      lnk = linkfactory->makeTypeObject ();
      lnk->set ("type", "transformer");
      break;
    }

  lnk->updateBus (bus1, 1);
  lnk->updateBus (bus2, 2);
  lnk->setName (temp2);

  parentObject->add (lnk);

  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance

  paramRead (strvec2[0], R);
  paramRead (strvec2[1], X);

  lnk->set ("r", R);
  lnk->set ("x", X);
  //get line capacitance
  paramRead (strvec[5], val);
  lnk->set ("b", val);


  status = std::stoi (strvec[11]);
  if (status == 0)
    {
      lnk->enabled = false;
    }
  else if (status > 1)
    {
      //TODO:  other conditions for 3 way transformers
    }


  //TODO get the other parameters (not critical for power flow)

  paramRead (strvec3[0], val);
  if (val != 0)
    {
      lnk->set ("tap", val);
    }
  paramRead (strvec3[2], val);
  if (val != 0)
    {
      lnk->set ("tapangle", val);
    }
  //now get the stuff for the adjustable transformers
  if (code > 0)
    {

    }
  return tline;
}
