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
#include "primary/acBus.h"
#include "loadModels/gridLoad.h"
#include "linkModels/acLine.h"
#include "generators/gridDynGenerator.h"
#include "core/gridDynExceptions.h"
#include "stringOps.h"

#include <fstream>
#include <cstdlib>
#include <iostream>



using namespace gridUnits;


void epcReadBus (gridBus *bus, std::string line, double base, const basicReaderInfo &bri);
void epcReadLoad (gridLoad *ld, std::string line, double base);
void epcReadFixedShunt (gridLoad *ld, std::string line, double base);
void epcReadGen (gridDynGenerator *gen, std::string line, double base);
void epcReadBranch (gridCoreObject *parentObject, std::string line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri);
void epcReadTX (gridCoreObject *parentObject, std::string line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri);

double epcReadSolutionParamters (gridCoreObject *parentObject, std::string line);

bool nextLine (std::ifstream &file, std::string &line)
{
  std::string temp1;
  bool ret = true;
  while (ret)
    {
      if (std::getline (file, line))
        {
          if (line[0] == '#') //ignore comment lines
            {
              continue;
            }
          trimString (line);
          if (line.empty ()) //continue over empty lines
            {
              continue;
            }
          while (line.back () == '/') //get line continuation
            {
              line.pop_back ();
              if (std::getline (file, temp1))
                {
                  line += " " + temp1;
                }
              else
                {
                  ret = false;
                }
            }
        }
      else
        {
          ret = false;
        }
      break;
    }
  return ret;
}

int getSectionCount (std::string &line)
{
  auto bbegin = line.find_first_of ("[");
  int cnt = -1;
  if (bbegin != std::string::npos)
    {
      auto bend = line.find_first_of ("]", bbegin);
      paramRead (line.substr (bbegin + 1, (bend - bbegin - 1)), cnt);
    }
  return cnt;
}

int getLineIndex (std::string &line)
{
  int index = -1;
  trimString (line);
  auto pos = line.find_first_not_of ("0123456789");
  paramRead (line.substr (0, pos), index, -1);
  return index;
}

void loadEPC (gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri)
{
  std::ifstream file (filename.c_str (), std::ios::in);
  std::string line;        //line storage
  std::string temp1;        //temporary storage for substrings
  std::string pref2;       // temp storage to 2nd order prefix.
  std::vector<gridBus *> busList;
  gridLoad *ld;
  gridDynGenerator *gen;
  int index;
  double base = 100;
  int cnt, bcount;

  /* Process the first line
  First card in file.
  */


  while (nextLine (file, line))
    {
      auto tokens = splitline (line, " \t");
      trimString (tokens[0]);
      if (tokens[0] == "title")
        {
          std::string title;
          while (std::getline (file, temp1))
            {
              if (temp1[0] == '!')
                {
                  break;
                }
              title += temp1;
            }
          parentObject->set ("name", title);
        }
      else if (tokens[0] == "comments")
        {
          std::string comments;
          while (std::getline (file, temp1))
            {
              if (temp1[0] == '!')
                {
                  break;
                }
              comments += temp1;
            }
          if (!comments.empty ())
            {
              parentObject->set ("description", comments);
            }
        }
      else if (tokens[0] == "bus")
        {
          cnt = getSectionCount (line);

          bcount = 0;
          if (cnt < 0)
            {
              cnt = kBigINT;
            }
          else if (cnt > static_cast<int> (busList.size ()))
            {
              busList.resize (cnt + 2);
            }
          while (bcount < cnt)
            {
              nextLine (file, line);
              index = getLineIndex (line);
              if (index < 0)
                {
                }
              ++bcount;
              if (index > static_cast<int> (busList.size ()))
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
                  busList[index - 1] = new acBus ();
                  busList[index - 1]->set ("basepower", base);
                  epcReadBus (busList[index - 1], line, base, bri);
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
        }
      else if (tokens[0] == "solution")
        {
          nextLine (file, line);
          while (line[0] != '!')
            {
              if (line.substr (0, 5) == "sbase")
                {
                  base = epcReadSolutionParamters (parentObject, line);
                }
              else
                {
                  epcReadSolutionParamters (parentObject, line);
                }
              nextLine (file, line);
            }
        }
      else if (tokens[0] == "branch")
        {
          cnt = getSectionCount (line);

          bcount = 0;
          if (cnt < 0)
            {
              cnt = kBigINT;
            }
          while (bcount < cnt)
            {
              nextLine (file, line);
              index = getLineIndex (line);
              if (index < 0)
                {
                }
              ++bcount;
              epcReadBranch (parentObject, line, base, busList, bri);
            }
        }
      else if (tokens[0] == "transformer")
        {
          cnt = getSectionCount (line);

          bcount = 0;
          if (cnt < 0)
            {
              cnt = kBigINT;
            }
          while (bcount < cnt)
            {
              nextLine (file, line);
              index = getLineIndex (line);
              if (index < 0)
                {
                }
              ++bcount;
              epcReadTX (parentObject, line, base, busList, bri);
            }
        }
      else if (tokens[0] == "generator")
        {
          cnt = getSectionCount (line);

          bcount = 0;
          if (cnt < 0)
            {
              cnt = kBigINT;
            }
          while (bcount < cnt)
            {
              nextLine (file, line);
              index = getLineIndex (line);
              if (index < 0)
                {
                }
              ++bcount;

              if (index > static_cast<int> (busList.size ()))
                {
                  std::cerr << "Invalid bus number for generator " << index << '\n';
                }
              if (busList[index - 1] == nullptr)
                {
                  std::cerr << "Invalid bus number for generator " << index << '\n';

                }
              else
                {
                  gen = new gridDynGenerator ();
                  busList[index - 1]->add (gen);
                  epcReadGen (gen, line, base);

                }

            }
        }
      else if (tokens[0] == "load")
        {
          cnt = getSectionCount (line);

          bcount = 0;
          if (cnt < 0)
            {
              cnt = kBigINT;
            }
          while (bcount < cnt)
            {
              nextLine (file, line);
              index = getLineIndex (line);
              if (index < 0)
                {
                }
              ++bcount;

              if (index > static_cast<int> (busList.size ()))
                {
                  std::cerr << "Invalid bus number for load " << index << '\n';
                }
              if (busList[index - 1] == nullptr)
                {
                  std::cerr << "Invalid bus number for load " << index << '\n';

                }
              else
                {
                  ld = new gridLoad ();
                  busList[index - 1]->add (ld);
                  epcReadLoad (ld, line, base);

                }

            }

        }
      else if (tokens[0] == "shunt")
        {
          cnt = getSectionCount (line);

          bcount = 0;
          if (cnt < 0)
            {
              cnt = kBigINT;
            }
          while (bcount < cnt)
            {
              nextLine (file, line);
              index = getLineIndex (line);
              if (index < 0)
                {
                }
              ++bcount;

              if (index > static_cast<int> (busList.size ()))
                {
                  std::cerr << "Invalid bus number for shunt " << index << '\n';
                }
              if (busList[index - 1] == nullptr)
                {
                  std::cerr << "Invalid bus number for shunt" << index << '\n';

                }
              else
                {
                  ld = new gridLoad ();
                  busList[index - 1]->add (ld);
                  epcReadFixedShunt (ld, line, base);

                }

            }
        }
      else
        {
          //unknown section
        }
    }
  file.close ();
}

/**
tap
<1 or 0>
TCUL adjustment flag
phas
<1 or 0>
Phase shifter adjustment flag
area
<1 or 0>
Area interchange control flag
svd
<1 or 0>
Control shunt adjustment flag
dctap
<1 or 0>
DC converter control flag
gcd
<1 or 0>
GCD control flag
jump
<value>
Jumper threshold impedance, pu
toler
<value>
Newton solution tolerance, MVA
sbase
<value>
System base, MVA
*/

double epcReadSolutionParamters (gridCoreObject *parentObject, std::string line)
{
  auto tokens = splitline (line, " ",delimiter_compression::on);
  double val = doubleRead (tokens[1]);
  if (tokens[0] == "tap")
    {

    }
  else if (tokens[0] == "phas")
    {
    }
  else if (tokens[0] == "area")
    {
    }
  else if (tokens[0] == "svd")
    {
    }
  else if (tokens[0] == "dctap")
    {
    }
  else if (tokens[0] == "gcd")
    {
    }
  else if (tokens[0] == "jump")
    {
    }
  else if (tokens[0] == "toler")
    {
      parentObject->set ("tolerance", val);
    }
  else if (tokens[0] == "sbase")
    {
      parentObject->set ("basepower", val);
    }
  else
    {
      std::cerr << "unknown solution parameter\n";
    }

  return val;
}

void epcReadBus (gridBus *bus, std::string line, double /*base*/, const basicReaderInfo &bri)
{
  std::string temp, temp2;
  double bv;
  double vm;
  double va;
  int type;

  auto strvec = splitlineBracket (line," :",delimiter_compression::on);
  //get the bus name
  temp = strvec[0];
  temp2 = strvec[1];

  if (bri.prefix.empty ())
    {
      if (temp2.empty ())      //12 spaces is default value which would all get trimmed
        {
          temp2 = "BUS_" + temp;
        }
    }
  else
    {
      if (temp2.empty ())      //12 spaces is default value which would all get trimmed
        {
          temp2 = bri.prefix + '_' + temp;
        }
      else
        {
          temp2 = bri.prefix + '_' + temp2;
        }
    }
  bus->setName (temp2);

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
    case -2:
      temp = "PV";
      break;
    case 0:
      temp = "swing";
      break;
    default:
      temp = "PQ";
      break;

    }
  bus->set ("type", temp);
  //skip the load flow area and loss zone for now
  //skip the owner information
  //get the voltage and angle specifications
  paramRead (strvec[4], vm);
  if (vm != 0)
    {
      bus->set ("vtarget", vm);
    }
  paramRead (strvec[5], vm);
  paramRead (strvec[6], va);
  if (va != 0)
    {
      bus->set ("angle", va, deg);
    }
  if (vm != 0)
    {
      bus->set ("voltage", vm);
    }

  paramRead (strvec[9], vm);
  paramRead (strvec[10], va);
  if (va != 0)
    {
      bus->set ("vmin", va);
    }
  if (vm != 0)
    {
      bus->set ("vmax", vm);
    }


}

void epcReadLoad (gridLoad *ld, std::string line, double /*base*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int status;


  auto strvec = splitlineBracket (line, " :", delimiter_compression::on);

  //get the load index and name
  prefix = ld->getParent ()->getName () + "_Load";
  if (!strvec[3].empty ())
    {
      prefix += '_' + strvec[3];
    }
  if (!strvec[4].empty ())
    {
      ld->setName (strvec[4]);
    }
  else
    {
      ld->setName (prefix);
    }
  //get the status
  status = std::stoi (strvec[5]);
  if (status == 0)
    {
      ld->enabled = false;
    }
  //skip the area and zone information for now

  //get the constant power part of the load
  paramRead (strvec[6], p);
  paramRead (strvec[7], q);
  if (p != 0.0)
    {
      ld->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("q", q, MVAR);
    }
  //get the constant current part of the load
  paramRead (strvec[8], p);
  paramRead (strvec[9], q);
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
  paramRead (strvec[10], p);
  paramRead (strvec[11], q);
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

void epcReadFixedShunt (gridLoad *ld, std::string line, double /*base*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int status;


  auto strvec = splitlineBracket (line, " :", delimiter_compression::on);

  //get the load index and name
  prefix = ld->getParent ()->getName () + "_Shunt";
  if (!strvec[7].empty ())
    {
      prefix += '_' + strvec[7];
    }
  if (!strvec[9].empty ())
    {
      ld->setName (strvec[9]);
    }
  else
    {
      ld->setName (prefix);
    }


  //get the status
  status = std::stoi (strvec[10]);
  if (status == 0)
    {
      ld->enabled = false;
    }
  //skip the area and zone information for now

  //get the constant power part of the load
  paramRead (strvec[13], p);
  paramRead (strvec[14], q);
  if (p != 0.0)
    {
      ld->set ("yp", p, puMW);
    }
  if (q != 0.0)
    {
      ld->set ("yq", -q, puMW);
    }


}


void epcReadGen (gridDynGenerator *gen, std::string line, double /*base*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int rbus;
  int status;

  auto strvec = splitlineBracket (line, " :", delimiter_compression::on);

  //get the gen index and name
  prefix = gen->getParent ()->getName () + "_Gen";
  if (!strvec[3].empty ())
    {
      prefix += '_' + strvec[3];
    }
  if (!strvec[4].empty ())
    {
      gen->setName (strvec[5]);
    }
  else
    {
      gen->setName (prefix);
    }


  //get the status
  status = std::stoi (strvec[5]);
  if (status == 0)
    {
      gen->enabled = false;
    }

  //get the power generation
  paramRead (strvec[13], p);
  paramRead (strvec[16], q);
  if (p != 0.0)
    {
      gen->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("q", q, MVAR);
    }
  //get the Pmax and Pmin
  paramRead (strvec[14], p);
  paramRead (strvec[15], q);
  if (p != 0.0)
    {
      gen->set ("pmax", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("pmin", q, MW);
    }
  //get the Qmax and Qmin
  paramRead (strvec[17], p);
  paramRead (strvec[18], q);
  if (p != 0.0)
    {
      gen->set ("qmax", p, MVAR);
    }
  if (q != 0.0)
    {
      gen->set ("qmin", q, MVAR);
    }
  //get the machine base
  double mb;
  paramRead (strvec[19], mb);
  gen->set ("mbase", mb);

  paramRead (strvec[22], mb);
  gen->set ("rs", mb);

  paramRead (strvec[23], mb);
  gen->set ("xs", mb);


  paramRead (strvec[6], rbus);

  if (rbus != 0)
    {
      //TODO something tricky as it is a remote controlled bus
    }
  //TODO  get the impedance fields and other data

}


void epcReadBranch (gridCoreObject *parentObject, std::string line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri)
{
  std::string temp2;
  gridBus *bus1, *bus2;
  gridLink *lnk;
  int ind1, ind2;
  double R, X;
  double val;
  int status;

  auto strvec = splitlineBracket (line, " :",delimiter_compression::on);

  std::string temp = strvec[0];
  ind1 = std::stoi (temp);
  if (bri.prefix.empty ())
    {
      temp2 = temp + "_to_";
    }
  else
    {
      temp2 = bri.prefix + '_' + temp + "_to_";
    }

  temp = strvec[3];
  ind2 = std::stoi (temp);

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];

  if (!strvec[8].empty ())
    {
      lnk = new gridLink (strvec[8]);
    }
  else
    {
      lnk = new gridLink (temp2);
    }


  //set the base power to that used this model
  lnk->set ("basepower", base);
  lnk->updateBus (bus1, 1);
  lnk->updateBus (bus2, 2);

  parentObject->add (lnk);
  //get the branch parameters
  status = std::stoi (strvec[9]);
  if (status == 0)
    {
      lnk->enabled = false;
    }

  paramRead (strvec[10], R);
  paramRead (strvec[11], X);

  lnk->set ("r", R);
  lnk->set ("x", X);



  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance


  //get line capacitance
  paramRead (strvec[12], val);
  if (val != 0)
    {
      lnk->set ("b", val);
    }
  paramRead (strvec[13], val);
  if (val != 0)
    {
      lnk->set ("ratinga", val);
    }
  paramRead (strvec[14], val);
  if (val != 0)
    {
      lnk->set ("ratingb", val);
    }
  paramRead (strvec[15], val);
  if (val != 0)
    {
      lnk->set ("erating", val);
    }

  paramRead (strvec[18], val);
  if (val != 0)
    {
      lnk->set ("length", val,km);
    }

}

void epcReadTX (gridCoreObject *parentObject, std::string line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri)
{
  std::string temp, temp2;
  gridBus *bus1, *bus2;
  gridLink *lnk;
  int code;
  int ind1, ind2;
  double R, X;
  double val;
  int status;
  int cbus;

  auto strvec = splitlineBracket (line, " :",delimiter_compression::on);

  temp = strvec[0];
  ind1 = std::stoi (temp);
  if (bri.prefix.empty ())
    {
      temp2 = "tx_" + temp + "_to_";
    }
  else
    {
      temp2 = bri.prefix + "_tx_" + temp + "_to_";
    }

  temp = strvec[3];
  ind2 = std::stoi (temp);

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];

  code = std::stoi (strvec[9]);
  switch (code)
    {
    case 1:
    case 11:
      code = 1;
      lnk = new gridLink ();
      // lnk->set ("type", "transformer");
      break;
    case 2:
    case 12:
      code = 2;
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "voltage");
      break;
    case 3:
    case 13:
      code = 3;
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "mvar");
      break;
    case 4:
    case 14:
      code = 4;
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "mw");
      break;
    default:
      std::cerr << "unrecognized transformer code\n";
      return;
    }
  //set the base power to that used this model
  lnk->set ("basepower", base);
  lnk->updateBus (bus1, 1);
  lnk->updateBus (bus2, 2);

  if (!strvec[7].empty ())
    {
      lnk->setName (strvec[8]);
    }
  else
    {
      lnk->setName (temp2);
    }


  parentObject->add (lnk);
  //get the branch parameters
  status = std::stoi (strvec[9]);
  if (status == 0)
    {
      lnk->enabled = false;
    }

  double tbase = base;
  paramRead (strvec[22], tbase);
  //primary and secondary winding resistance
  paramRead (strvec[23], R);
  paramRead (strvec[24], X);

  lnk->set ("r", R * tbase / base);
  lnk->set ("x", X * tbase / base);



  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance


  //get line capacitance

  paramRead (strvec[35], val);
  if (val != 0)
    {
      lnk->set ("ratinga", val);
    }
  paramRead (strvec[36], val);
  if (val != 0)
    {
      lnk->set ("ratingb", val);
    }
  paramRead (strvec[37], val);
  if (val != 0)
    {
      lnk->set ("erating", val);
    }

  paramRead (strvec[45], val);
  if (val != 0)
    {
      lnk->set ("tap", val);
    }
  paramRead (strvec[32], val);
  if (val != 0)
    {
      lnk->set ("tapangle", val, deg);
    }
  //now get the stuff for the adjustable transformers
  if (code > 1)
    {
      paramRead (strvec[10], cbus);
      if (cbus != 0)
        {
          static_cast<adjustableTransformer *> (lnk)->setControlBus (busList[cbus - 1]);
        }
      paramRead (strvec[40], R);
      paramRead (strvec[41], X);
      if (code == 4)
        {
          lnk->set ("maxtapangle", R, deg);
          lnk->set ("mintapangle", X, deg);
        }
      else
        {
          lnk->set ("maxtap", R);
          lnk->set ("mintap", X);
        }
      paramRead (strvec[42], R);
      paramRead (strvec[43], X);
      if (code == 4)
        {
          lnk->set ("pmax", R, MW);
          lnk->set ("pmin", X, MW);
        }
      else if (code == 3)
        {
          lnk->set ("qmax", R, MVAR);
          lnk->set ("qmin", X, MVAR);
        }
      else
        {
          lnk->set ("vmax", R);
          lnk->set ("vmin", X);
        }
      paramRead (strvec[44], R);
      if (code == 4)
        {
          lnk->set ("stepsize", R,deg);
        }
      else
        {
          lnk->set ("stepsize", R);
        }
    }
}
