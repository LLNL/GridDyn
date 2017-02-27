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
#include "primary/acBus.h"
#include "loadModels/zipLoad.h"
#include "linkModels/acLine.h"
#include "generators/gridDynGenerator.h"
#include "core/coreExceptions.h"
#include "stringConversion.h"
#include "stringOps.h"
#include "string_viewOps.h"
#include "string_viewConversion.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <functional>



using namespace gridUnits;
using namespace utilities::string_viewOps;
using utilities::string_view;

void epcReadBus (gridBus *bus, string_view line, double base, const basicReaderInfo &bri);
void epcReadLoad (zipLoad *ld, string_view line, double base);
void epcReadFixedShunt (zipLoad *ld, string_view line, double base);
void epcReadGen (gridDynGenerator *gen, string_view line, double base);
void epcReadBranch (coreObject *parentObject, string_view line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri);
void epcReadTX (coreObject *parentObject, string_view line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri);

double epcReadSolutionParamters (coreObject *parentObject, string_view line);

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
		  stringOps::trimString(line);
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

int getSectionCount (string_view line)
{
  auto bbegin = line.find_first_of ("[");
  int cnt = -1;
  if (bbegin != string_view::npos)
    {
      auto bend = line.find_first_of ("]", bbegin);
	  cnt = numeric_conversion<int>(line.substr(bbegin + 1, (bend - bbegin - 1)), 0);
    }
  return cnt;
}

int getLineIndex (string_view line)
{
  trimString (line);
  auto pos = line.find_first_not_of ("0123456789");
  int index = numeric_conversion<int>(line.substr(0, pos), -1);
  return index;
}


void ProcessSection(std::string line, std::ifstream &file, std::function<void(string_view)> Func)
{
	int cnt = getSectionCount(line);

	int bcount = 0;
	if (cnt < 0)
	{
		cnt = kBigINT;
	}
	while (bcount < cnt)
	{
		nextLine(file, line);
		int index = getLineIndex(line);
		if (index < 0)
		{
		}
		++bcount;
		Func(line);
	}
}

template <class X>
void ProcessSectionObject(std::string line, std::ifstream &file, const std::string &oname, std::vector<gridBus *> &busList, std::function<void(X*, string_view)> Func)
{
	int cnt = getSectionCount(line);

	int bcount = 0;
	if (cnt < 0)
	{
		cnt = kBigINT;
	}
	while (bcount < cnt)
	{
		nextLine(file, line);
		int index = getLineIndex(line);
		if (index < 0)
		{
		}
		++bcount;

		if (index > static_cast<int> (busList.size()))
		{
			std::cerr << "Invalid bus number for "<<oname<<" " << index << '\n';
		}
		if (busList[index - 1] == nullptr)
		{
			std::cerr << "Invalid bus number for "<<oname<<" " << index << '\n';

		}
		else
		{
			auto obj = new X();
			busList[index - 1]->add(obj);
			Func(obj, line);

		}

	}
}

void loadEPC (coreObject *parentObject, const std::string &filename, const basicReaderInfo &bri)
{
  std::ifstream file (filename.c_str (), std::ios::in);
  
  std::string temp1;        //temporary storage for substrings
  std::string pref2;       // temp storage to 2nd order prefix.
  std::vector<gridBus *> busList;
  int index;
  double base = 100;
  int cnt, bcount;

  /* Process the first line
  First card in file.
  */

  std::string line;        //line storage
  while (nextLine (file, line))
    {
      auto tokens = split (line, " \t");
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
              if (line.compare (0, 5,"sbase")==0)
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
		  ProcessSection(line, file, [&](string_view config) {epcReadBranch(parentObject, config, base, busList, bri); });
        }
      else if (tokens[0] == "transformer")
        {
		  ProcessSection(line, file, [&](string_view config) {epcReadTX(parentObject, config, base, busList, bri); });
        }
      else if (tokens[0] == "generator")
        {
		  ProcessSectionObject<gridDynGenerator>(line, file, "generator", busList, [base](gridDynGenerator *gen, string_view config) {epcReadGen(gen, config, base); });
        }
      else if (tokens[0] == "load")
        {
		  ProcessSectionObject<zipLoad>(line, file, "load", busList, [base](zipLoad *ld, string_view config) {epcReadLoad(ld, config, base); });
        }
      else if (tokens[0] == "shunt")
        {
		  ProcessSectionObject<zipLoad>(line, file, "shunt", busList, [base](zipLoad *ld, string_view config) {epcReadFixedShunt(ld, config, base); });
          
        }
      else
        {
		  std::cerr << "unrecognized token" << tokens[0] << '\n';
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

double epcReadSolutionParamters (coreObject *parentObject, string_view line)
{
  auto tokens = split (line, " ",delimiter_compression::on);
  double val = numeric_conversion (tokens[1],0.0);
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

void epcReadBus (gridBus *bus, string_view line, double /*base*/, const basicReaderInfo &bri)
{

  auto strvec = splitlineBracket (line," :",default_bracket_chars,delimiter_compression::on);
  //get the bus name
  auto temp = strvec[0];
  std::string temp2 = strvec[1].to_string();

  if (bri.prefix.empty ())
    {
      if (temp2.empty ())      //12 spaces is default value which would all get trimmed
        {
          temp2 = "BUS_" + temp.to_string();
        }
    }
  else
    {
      if (temp2.empty ())      //12 spaces is default value which would all get trimmed
        {
          temp2 = bri.prefix + '_' + temp.to_string();
        }
      else
        {
          temp2 = bri.prefix + '_' + temp2;
        }
    }
  bus->setName (temp2);

  //get the baseVoltage
  double bv = numeric_conversion<double>(strvec[2],-1.0);
  if (bv > 0.0)
    {
      bus->set ("basevoltage", bv);
    }


   int type = numeric_conversion<int> (strvec[3],1);

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
  bus->set ("type", temp.to_string());
  //skip the load flow area and loss zone for now
  //skip the owner information
  //get the voltage and angle specifications
  double vm = numeric_conversion<double>(strvec[4],0.0);
  if (vm != 0)
    {
      bus->set ("vtarget", vm);
    }
   vm = numeric_conversion<double>(strvec[5],0.0);
  double va = numeric_conversion<double>(strvec[6],0.0);
  if (va != 0)
    {
      bus->set ("angle", va, deg);
    }
  if (vm != 0)
    {
      bus->set ("voltage", vm);
    }

   vm = numeric_conversion<double>(strvec[9],0.0);
   va = numeric_conversion<double>(strvec[10],0.0);
  if (va != 0)
    {
      bus->set ("vmin", va);
    }
  if (vm != 0)
    {
      bus->set ("vmax", vm);
    }


}

void epcReadLoad (zipLoad *ld, string_view line, double /*base*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int status;


  auto strvec = splitlineBracket (line, " :", default_bracket_chars,delimiter_compression::on);

  //get the load index and name
  prefix = ld->getParent ()->getName () + "_Load";
  if (!strvec[3].empty ())
    {
      prefix += '_' + strvec[3].to_string();
    }
  if (!strvec[4].empty ())
    {
      ld->setName (strvec[4].to_string());
    }
  else
    {
      ld->setName (prefix);
    }
  //get the status
  status = toIntSimple(strvec[5]);
  if (status == 0)
    {
      ld->disable();
    }
  //skip the area and zone information for now

  //get the constant power part of the load
   p = numeric_conversion<double>(strvec[6],0.0);
   q = numeric_conversion<double>(strvec[7],0.0);
  if (p != 0.0)
    {
      ld->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      ld->set ("q", q, MVAR);
    }
  //get the constant current part of the load
   p = numeric_conversion<double>(strvec[8],0.0);
   q = numeric_conversion<double>(strvec[9],0.0);
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
   p = numeric_conversion<double>(strvec[10],0.0);
   q = numeric_conversion<double>(strvec[11],0.0);
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

void epcReadFixedShunt (zipLoad *ld, string_view line, double /*base*/)
{
  std::string temp;
  std::string prefix;
  double p;
  double q;
  int status;


  auto strvec = splitlineBracket (line, " :",default_bracket_chars, delimiter_compression::on);

  //get the load index and name
  prefix = ld->getParent ()->getName () + "_Shunt";
  if (!strvec[7].empty ())
    {
      prefix += '_' + trim(strvec[7]).to_string();
    }
  if (!strvec[9].empty ())
    {
      ld->setName (trim(strvec[9]).to_string());
    }
  else
    {
      ld->setName (prefix);
    }


  //get the status
  status = toIntSimple (strvec[10]);
  if (status == 0)
    {
      ld->disable();
    }
  //skip the area and zone information for now

  //get the constant power part of the load
   p = numeric_conversion<double>(strvec[13],0.0);
   q = numeric_conversion<double>(strvec[14],0.0);
  if (p != 0.0)
    {
      ld->set ("yp", p, puMW);
    }
  if (q != 0.0)
    {
      ld->set ("yq", -q, puMW);
    }


}


void epcReadGen (gridDynGenerator *gen, string_view	 line, double /*base*/)
{

  auto strvec = splitlineBracket (line, " :",default_bracket_chars, delimiter_compression::on);

  //get the gen index and name
  std::string prefix = gen->getParent ()->getName () + "_Gen";
  if (!strvec[3].empty ())
    {
      prefix += '_' + strvec[3].to_string();
    }
  if (!strvec[4].empty ())
    {
      gen->setName (trim(strvec[5]).to_string());
    }
  else
    {
      gen->setName (prefix);
    }


  //get the status
  int status = toIntSimple (strvec[5]);
  if (status == 0)
    {
      gen->disable();
    }

  //get the power generation
   double p = numeric_conversion<double>(strvec[13],0.0);
   double q = numeric_conversion<double>(strvec[16],0.0);
  if (p != 0.0)
    {
      gen->set ("p", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("q", q, MVAR);
    }
  //get the Pmax and Pmin
   p = numeric_conversion<double>(strvec[14],0.0);
   q = numeric_conversion<double>(strvec[15],0.0);
  if (p != 0.0)
    {
      gen->set ("pmax", p, MW);
    }
  if (q != 0.0)
    {
      gen->set ("pmin", q, MW);
    }
  //get the Qmax and Qmin
   p = numeric_conversion<double>(strvec[17],0.0);
   q = numeric_conversion<double>(strvec[18],0.0);
  if (p != 0.0)
    {
      gen->set ("qmax", p, MVAR);
    }
  if (q != 0.0)
    {
      gen->set ("qmin", q, MVAR);
    }
  //get the machine base
   double mb = numeric_conversion<double>(strvec[19],0.0);
  gen->set ("mbase", mb);

   mb = numeric_conversion<double>(strvec[22],0.0);
  gen->set ("rs", mb);

   mb = numeric_conversion<double>(strvec[23],0.0);
  gen->set ("xs", mb);


   int rbus = numeric_conversion<int>(strvec[6],0);

  if (rbus != 0)
    {
      //TODO something tricky as it is a remote controlled bus
    }
  //TODO  get the impedance fields and other data

}


void epcReadBranch (coreObject *parentObject, string_view line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri)
{
  std::string temp2;
  gridBus *bus1, *bus2;
  gridLink *lnk;
  double val;
  int status;

  auto strvec = splitlineBracket (line, " :",default_bracket_chars,delimiter_compression::on);

  std::string temp = trim(strvec[0]).to_string();
  
  int ind1 = std::stoi (temp);
  if (bri.prefix.empty ())
    {
      temp2 = temp + "_to_";
    }
  else
    {
      temp2 = bri.prefix + '_' + temp + "_to_";
    }

  temp = trim(strvec[3]).to_string();
  int ind2 = std::stoi (temp);

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];

  if (!strvec[8].empty ())
    {
      lnk = new acLine (strvec[8].to_string());
    }
  else
    {
      lnk = new acLine (temp2);
    }


  //set the base power to that used this model
  lnk->set ("basepower", base);
  lnk->updateBus (bus1, 1);
  lnk->updateBus (bus2, 2);

  parentObject->add (lnk);
  //get the branch parameters
  status =toIntSimple (strvec[9]);
  if (status == 0)
    {
      lnk->disable();
    }

   double R = numeric_conversion<double>(strvec[10],0.0);
   double X = numeric_conversion<double>(strvec[11],0.0);

  lnk->set ("r", R);
  lnk->set ("x", X);



  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance


  //get line capacitance
   val = numeric_conversion<double>(strvec[12],0.0);
  if (val != 0)
    {
      lnk->set ("b", val);
    }
   val = numeric_conversion<double>(strvec[13],0.0);
  if (val != 0)
    {
      lnk->set ("ratinga", val);
    }
   val = numeric_conversion<double>(strvec[14],0.0);
  if (val != 0)
    {
      lnk->set ("ratingb", val);
    }
   val = numeric_conversion<double>(strvec[15],0.0);
  if (val != 0)
    {
      lnk->set ("erating", val);
    }

   val = numeric_conversion<double>(strvec[18],0.0);
  if (val != 0)
    {
      lnk->set ("length", val,km);
    }

}

void epcReadTX (coreObject *parentObject, string_view line, double base, std::vector<gridBus *> &busList, const basicReaderInfo &bri)
{
  std::string temp2;
  gridBus *bus1, *bus2;
  gridLink *lnk;
  int code;
  int ind1, ind2;
  double R, X;
  double val;
  int status;
  int cbus;

  auto strvec = splitlineBracket (line, " :", default_bracket_chars, delimiter_compression::on);

  auto temp = trim(strvec[0]).to_string();
  ind1 = std::stoi (temp);
  if (bri.prefix.empty ())
    {
      temp2 = "tx_" + temp + "_to_";
    }
  else
    {
      temp2 = bri.prefix + "_tx_" + temp + "_to_";
    }

  temp = trim(strvec[3]).to_string();
  ind2 = std::stoi (temp);

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];

  code = numeric_conversion<int> (strvec[9],1);
  switch (code)
    {
    case 1:
    case 11:
      code = 1;
      lnk = new acLine ();
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
      lnk->setName (trim(strvec[8]).to_string());
    }
  else
    {
      lnk->setName (temp2);
    }


  parentObject->add (lnk);
  //get the branch parameters
  status = toIntSimple (strvec[9]);
  if (status == 0)
    {
      lnk->disable();
    }

  double tbase = base;
   tbase = numeric_conversion<double>(strvec[22],0.0);
  //primary and secondary winding resistance
   R = numeric_conversion<double>(strvec[23],0.0);
   X = numeric_conversion<double>(strvec[24],0.0);

  lnk->set ("r", R * tbase / base);
  lnk->set ("x", X * tbase / base);



  //skip the load flow area and loss zone and circuit for now

  //get the branch impedance


  //get line capacitance

   val = numeric_conversion<double>(strvec[35],0.0);
  if (val != 0)
    {
      lnk->set ("ratinga", val);
    }
   val = numeric_conversion<double>(strvec[36],0.0);
  if (val != 0)
    {
      lnk->set ("ratingb", val);
    }
   val = numeric_conversion<double>(strvec[37],0.0);
  if (val != 0)
    {
      lnk->set ("erating", val);
    }

   val = numeric_conversion<double>(strvec[45],0.0);
  if (val != 0)
    {
      lnk->set ("tap", val);
    }
   val = numeric_conversion<double>(strvec[32],0.0);
  if (val != 0)
    {
      lnk->set ("tapangle", val, deg);
    }
  //now get the stuff for the adjustable transformers
  if (code > 1)
    {
	  cbus = numeric_conversion<int>(strvec[10], 0);
      if (cbus != 0)
        {
          static_cast<adjustableTransformer *> (lnk)->setControlBus (busList[cbus - 1]);
        }
       R = numeric_conversion<double>(strvec[40],0.0);
       X = numeric_conversion<double>(strvec[41],0.0);
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
       R = numeric_conversion<double>(strvec[42],0.0);
       X = numeric_conversion<double>(strvec[43],0.0);
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
       R = numeric_conversion<double>(strvec[44],0.0);
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
