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
#include "stringOps.h"
#include <fstream>
#include <cstdlib>
#include <iostream>

using namespace gridUnits;

void cdfReadBusLine (gridBus *bus,std::string line,double base, const basicReaderInfo &bri);
void cdfReadBranch (gridCoreObject *parentObject,std::string line,double base,std::vector<gridBus *> busList, const basicReaderInfo &bri);

void loadCDF (gridCoreObject *parentObject,const std::string &filename, const basicReaderInfo &bri)
{
  std::ifstream file (filename.c_str (), std::ios::in);
  std::string line;        //line storage
  std::string temp1;        //temporary storage for substrings

  if (!(file.is_open ()))
    {
      std::cerr << "Unable to open file " << filename << '\n';
      //	return;
    }
  std::vector<gridBus *> busList;
  int index;
  double base = 100;

  /* Process the first line
  First card in file.

Columns  2- 9   Date, in format DD/MM/YY with leading zeros. If no date
          provided, use 0b/0b/0b where b is blank.
Columns 11-30   Originator's name (A)
Columns 32-37   MVA Base (F*)
Columns 39-42   Year (I)
Column  44      Season (S - Summer, W - Winter)
Column  46-73   Case identification (A) */


  if (std::getline ( file, line ) )
    {
      while (line.size () < 36)
        {
          if (!(std::getline (file, line)))
            {
              return;
            }
        }
      //std::cout<<line<<'\n';

      temp1 = line.substr (31,5);        //get the base power
      base = std::stod (temp1);
      parentObject->set ("basepower",base);
      temp1 = line.substr (45,27);
      trimString (temp1);
      if (bri.prefix.empty ())
        {
          parentObject->set ("name",temp1);
        }
    }
  //loop over the sections
  while (std::getline ( file, line ) )
    {
      if (line.compare (0,3,"BUS") == 0)
        {

          int morebus = 1;
          while (morebus)
            {
              if (std::getline ( file, line ))
                {

                  temp1 = line.substr (0,4);
                  if (temp1 == "-999")
                    {
                      morebus = 0;
                      continue;
                    }
                  if (temp1.length () < 4)
                    {
                      continue;
                    }
                  index = std::stoi (temp1);
                  if (static_cast<size_t> (index) > busList.size ())
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
                      busList[index - 1]->set ("basepower", base);                          //set the basepower for the bus
                      busList[index - 1]->setUserID (index);
                      cdfReadBusLine (busList[index - 1],line,base,bri);
                      parentObject->add (busList[index - 1]);
                      if (busList[index - 1]->getParent () != parentObject)
                        {

                          busList[index - 1]->setName (busList[index - 1]->getName () + "_BUS-" + std::to_string (index));

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
                  morebus = 0;
                }
            }
        }
      else if (line.compare (0,6,"BRANCH") == 0)
        {
          int morebranch = 1;
          while (morebranch)
            {
              if (std::getline ( file, line ))
                {

                  temp1 = line.substr (0,4);
                  if (temp1 == "-999")
                    {
                      morebranch = 0;
                      continue;
                    }
                  if (temp1.length () < 4)
                    {
                      continue;
                    }
                  cdfReadBranch (parentObject,line,base,busList,bri);
                }
              else
                {
                  morebranch = 0;
                }
            }
        }
    }
  file.close ();
}

/**********************************************************
Read a line from a CDF file corresponding to a bus specification
**********************************************************/

/*
Columns  1- 4   Bus number (I) *
Columns  7-17   Name (A) (left justify) *
Columns 19-20   Load flow area number (I) Don't use zero! *
Columns 21-23   Loss zone number (I)
Columns 25-26   Type (I) *
0 - Unregulated (load, PQ)
1 - Hold MVAR generation within voltage limits, (PQ)
2 - Hold voltage within VAR limits (gen, PV)
3 - Hold voltage and angle (swing, V-Theta) (must always
have one)
Columns 28-33   Final voltage, p.u. (F) *
Columns 34-40   Final angle, degrees (F) *
Columns 41-49   Load MW (F) *
Columns 50-58   Load MVAR (F) *
Columns 59-67   Generation MW (F) *
Columns 68-75   Generation MVAR (F) *
Columns 77-83   Base KV (F)
Columns 85-90   Desired volts (pu) (F) (This is desired remote voltage if
this bus is controlling another bus.
Columns 91-98   Maximum MVAR or voltage limit (F)
Columns 99-106  Minimum MVAR or voltage limit (F)
Columns 107-114 Shunt conductance G (per unit) (F) *
Columns 115-122 Shunt susceptance B (per unit) (F) *
Columns 124-127 Remote controlled bus number
*/

void cdfReadBusLine (gridBus *bus,std::string line,double base,const basicReaderInfo &bri)
{
  gridLoad *ld = nullptr;
  gridDynGenerator *gen = nullptr;

  std::string temp = trim (line.substr (5,12));
  std::string temp2 = (temp.length () >= 11) ? trim (temp.substr (9, 3)) : "";

  removeQuotes (temp);
  if (!(bri.prefix.empty ()))
    {
      if (temp.empty ())
        {
          temp = bri.prefix + "_BUS_" + std::to_string (bus->getUserID ());
        }
      else
        {
          temp = bri.prefix + '_' + temp;
        }
    }
  else if (temp.empty ())
    {
      temp = "BUS_" + std::to_string (bus->getUserID ());
    }

  bus->set ("name",temp);      //set the name
  //skip the load flow area and loss zone for now
  //get the baseVoltage
  temp = line.substr (76,6);
  double val = std::stod (temp);
  if (val > 0.0)
    {
      bus->set ("basevoltage", val);
    }
  else if (!temp2.empty ())
    {
      val = doubleRead (temp2,0.0);
      if (val > 0)
        {
          bus->set ("basevoltage", val);
        }
      else
        {
          val = convertBV (temp2);
          if (val > 0)
            {
              bus->set ("basevoltage", val);
            }
        }
    }
  //voltage and angle common to all bus types
  //get the actual voltage
  temp = line.substr (27, 6);
  val = std::stod (temp);
  bus->set ("voltage", val);
  //get the angle
  temp = line.substr (33, 7);
  val = std::stod (temp);
  bus->set ("angle", val / 180 * kPI);

  //get the bus type
  temp = line.substr (24,2);
  double P, Q;
  int code = std::stoi (temp);
  switch (code)
    {
    case 0:             //PQ
      bus->set ("type","pq");

      break;
    case 1:              //PQ Voltage limits
      bus->set ("type", "pq");
      //get the Vmax and Vmin
      temp = line.substr (90,7);
      P = std::stod (temp);
      temp = line.substr (98,7);
      Q = std::stod (temp);
      bus->set ("vmax",P);
      bus->set ("vmin",Q);
      //get the desired voltage
      temp = line.substr (27, 6);
      val = std::stod (temp);
      bus->set ("voltage", val);
      break;
    case 2:             //pv bus
      bus->set ("type","pv");
      //get the Qmax and Qmin
      temp = line.substr (90,7);
      P = std::stod (temp);
      temp = line.substr (98,7);
      Q = std::stod (temp);
      if (P > 0)
        {
          bus->set ("qmax", P / base);
        }
      if (Q < 0)
        {
          bus->set ("qmin", Q / base);
        }
      //get the desired voltage
      temp = line.substr (84,6);
      val = std::stod (temp);
      bus->set ("vtarget",val);
      break;
    case 3:             //swing bus
      bus->set ("type","slk");
      //get the desired voltage
      temp = line.substr (84,6);
      val = std::stod (temp);
      bus->set ("vtarget",val);
      temp = line.substr (33, 7);
      val = std::stod (temp);
      bus->set ("atarget", val, deg);
      break;
    }
  P = 0,Q = 0;
  temp = line.substr (40,9);
  P = std::stod (temp);
  temp = line.substr (49,9);
  Q = std::stod (temp);

  if ((P != 0)||(Q != 0))
    {
      ld = new gridLoad (P / base,Q / base);
      bus->add (ld);
    }
  //get the shunt impedance
  P = std::stod (line.substr (106, 8));
  Q = std::stod (line.substr (114, 8));
  if ((P != 0) || (Q != 0))
    {
      if (ld == nullptr)
        {
          ld = new gridLoad ();
          bus->add (ld);
        }
      if (P != 0)
        {
          ld->set ("yp", P);
        }
      if (Q != 0)
        {
          ld->set ("yq", -Q);
        }
    }
  //get the generation
  P = std::stod (line.substr (58, 9));
  Q = std::stod (line.substr (67, 8));

  if ((P != 0) || (Q != 0) || (code == 3))
    {
      gen = new gridDynGenerator ();
      bus->add (gen);
      gen->set ("p",P / base);
      gen->set ("q",Q / base);
      //get the Qmax and Qmin
      temp = line.substr (90, 7);
      P = std::stod (temp);
      temp = line.substr (98, 7);
      Q = std::stod (temp);
      if (P != 0)
        {
          gen->set ("qmax", P / base);
        }
      if (Q != 0)
        {
          gen->set ("qmin", Q / base);
        }

    }
  else if (bus->getType () != gridBus::busType::PQ)
    {
      temp = line.substr (90, 7);
      P = std::stod (temp);
      temp = line.substr (98, 7);
      Q = std::stod (temp);
      if ((P != 0) || (Q != 0))
        {
          gen = new gridDynGenerator ();
          bus->add (gen);
          if (P != 0)
            {
              gen->set ("qmax", P / base);
            }
          if (Q != 0)
            {
              gen->set ("qmin", Q / base);
            }
        }
    }


}
/*
Columns  1- 4   Tap bus number (I) *
                 For transformers or phase shifters, the side of the model
                 the non-unity tap is on
Columns  6- 9   Z bus number (I) *
                 For transformers and phase shifters, the side of the model
                 the device impedance is on.
Columns 11-12   Load flow area (I)
Columns 14-15   Loss zone (I)
Column  17      Circuit (I) * (Use 1 for single lines)
Column  19      Type (I) *
                 0 - Transmission line
                 1 - Fixed tap
                 2 - Variable tap for voltage control (TCUL, LTC)
                 3 - Variable tap (turns ratio) for MVAR control
                 4 - Variable phase angle for MW control (phase shifter)
Columns 20-29   Branch resistance R, per unit (F) *
Columns 30-39   Branch reactance X, per unit (F) * No zero impedance lines
Columns 40-50   Line charging B, per unit (F) * (total line charging, +B)
Columns 51-55   Line MVA rating No 1 (I) Left justify!
Columns 57-61   Line MVA rating No 2 (I) Left justify!
Columns 63-67   Line MVA rating No 3 (I) Left justify!
Columns 69-72   Control bus number
Column  74      Side (I)
                 0 - Controlled bus is one of the terminals
                 1 - Controlled bus is near the tap side
                 2 - Controlled bus is near the impedance side (Z bus)
Columns 77-82   Transformer final turns ratio (F)
Columns 84-90   Transformer (phase shifter) final angle (F)
Columns 91-97   Minimum tap or phase shift (F)
Columns 98-104  Maximum tap or phase shift (F)
Columns 106-111 Step size (F)
Columns 113-119 Minimum voltage, MVAR or MW limit (F)
Columns 120-126 Maximum voltage, MVAR or MW limit (F)
*/

void cdfReadBranch (gridCoreObject *parentObject,std::string line,double base,std::vector<gridBus *> busList,const basicReaderInfo &bri)
{
  gridBus *bus1, *bus2;
  gridLink *lnk = nullptr;
  int code;
  int cntrl = 0;
  int cbus = 0;
  int ind1,ind2;
  double R,X;
  double val;
  std::string temp = trim (line.substr (0,4));
  ind1 = std::stoi (temp);
  std::string temp2 = temp + "_to_";
  if (!bri.prefix.empty ())
    {
      temp2 = bri.prefix + '_' + temp2;
    }

  temp = line.substr (5, 4);
  ind2 = intRead (temp);

  temp2 = temp2 + temp;
  bus1 = busList[ind1 - 1];
  bus2 = busList[ind2 - 1];

  //get the branch type
  code = intRead (line.substr (18, 1));
  switch (code)
    {
    case 0:     //ac line
      lnk = new acLine ();
//      lnk->set ("type", "ac");
      lnk->set ("basepower", base);
      break;
    case 1:      //transformer
      lnk = new acLine ();
      //   lnk->set ("type", "transformer");
      lnk->set ("basepower", base);
      break;
    case 2:
      lnk = new adjustableTransformer ();
      lnk->set ("mode", "voltage");
      lnk->set ("basepower", base);
      cbus = intRead (line.substr (68, 4));
      if (cbus > 0)
        {
          dynamic_cast<adjustableTransformer *> (lnk)->setControlBus (busList[cbus - 1]);
        }
      temp = line.substr (73, 1);
      if (temp[0] != ' ')
        {
          cntrl = intRead (temp, 0);
          if (cntrl != 0)
            {
              if (cntrl == 1)
                {
                  lnk->set ("direction", -1);
                }
              else
                {
                  lnk->set ("direction", 1);
                }
            }
        }

      //get Vmin and Vmax
      R = doubleRead (line.substr (112, 7), 0);
      X = doubleRead (line.substr (119, 7), 0);

      lnk->set ("vmin", R);
      lnk->set ("vmax", X);
      //get tapMin and tapMax
      R = doubleRead (line.substr (90, 7), 0);
      X = doubleRead (line.substr (97, 7),0);
      lnk->set ("mintap", R);
      lnk->set ("maxtap", X);
      break;
    case 3:
      lnk = new adjustableTransformer ();
      lnk->set ("basepower", base);
      lnk->set ("mode", "mvar");
      R = doubleRead (line.substr (112, 7), 0);
      X = doubleRead (line.substr (119, 7), 0);
      lnk->set ("qmin", R,MW);
      lnk->set ("qmax", X,MW);
      //get tapMin and tapMax
      R = doubleRead (line.substr (90, 7), 0);
      X = doubleRead (line.substr (97, 7), 0);
      lnk->set ("mintap", R);
      lnk->set ("maxtap", X);
      break;
    case 4:
      lnk = new adjustableTransformer ();
      lnk->set ("basepower", base);
      lnk->set ("mode", "mw");
      R = doubleRead (line.substr (112, 7), 0);
      X = doubleRead (line.substr (119, 7), 0);
      lnk->set ("pmin", R, MW);
      lnk->set ("pmax", X, MW);
      //get tapAngleMin and tapAngleMax
      R = doubleRead (line.substr (90, 7), 0);
      X = doubleRead (line.substr (97, 7), 0);
      lnk->set ("mintapangle", R,deg);
      lnk->set ("maxtapangle", X,deg);
      break;
    default:
      printf ("unrecognized line code %d\n",code);
      return;
    }


  lnk->updateBus (bus1,1);
  lnk->updateBus (bus2,2);
  lnk->set ("name",temp2);

  parentObject->add (lnk);
  if (lnk->getParent () == nullptr)
    {
      //must be a parallel branch
      std::string sub = lnk->getName ();
      char m = 'a';
      while (lnk->getParent () == nullptr)
        {
          lnk->setName (sub + '_' + m);
          m = m + 1;
          parentObject->add (lnk);
        }
    }
  //skip the load flow area and loss zone and circuit for now TODO:: PT Fix this when area controls are put in place


  //get the branch impedance
  R = doubleRead (line.substr (19, 10), 0);
  X = doubleRead (line.substr (29, 10), 0);

  lnk->set ("r",R);
  lnk->set ("x",X);
  //get line capacitance
  temp = line.substr (40,11);
  trimString (temp);
  val = std::stod (temp);
  lnk->set ("b",val);

  //turns ratio
  temp = line.substr (76,6);
  trimString (temp);
  val = std::stod (temp);
  if (val > 0)
    {
      lnk->set ("tap",val);
    }
  //tapStepSize
  temp = line.substr (105,6);
  trimString (temp);
  val = std::stod (temp);
  if (val != 0)
    {
      if (code == 4)
        {
          lnk->set ("tapchange", val * kPI / 180.0);
        }
      else if (code >= 2)
        {
          lnk->set ("tapchange", val);
        }

    }

  //tapAngle
  temp = line.substr (83,7);
  trimString (temp);
  val = std::stod (temp);
  if (val != 0)
    {
      lnk->set ("tapangle", val,deg);
    }

}

double convertBV (std::string &bv)
{
  double val = 0.0;
  trimString (bv);
  if (bv == "V1")
    {
      val = 345;
    }
  else if (bv == "V2")
    {
      val = 138;
    }
  else if (bv == "HV")
    {
      val = 345;
    }
  else if (bv == "LV")
    {
      val = 138;
    }
  else if (bv == "ZV")
    {
      val = 1;
    }
  else if (bv == "TV")
    {
      val = 33;
    }
  else if (bv == "V3")
    {
      val = 161;
    }
  else if (bv == "V4")
    {
      val = 33;
    }
  else if (bv == "V5")
    {
      val = 14;
    }
  else if (bv == "V6")
    {
      val = 11;
    }
  else if (bv == "V7")
    {
      val = 1;
    }
  return val;
}
