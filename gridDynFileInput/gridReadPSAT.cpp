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
#include "loadModels/motorLoad.h"
#include "linkModels/acLine.h"
#include "generators/gridDynGenerator.h"
#include "recorder_events/gridEvent.h"
#include "stringOps.h"

#ifdef OPTIMIZATION_ENABLE
#include "gridDynOpt.h"
#include "models/gridGenOpt.h"
#else
#include "simulation/gridSimulation.h"
#endif

#include "submodels/gridDynExciter.h"
#include "submodels/otherGovernors.h"
#include "submodels/otherGenModels.h"
#include "submodels/gridDynPSS.h"

#include <fstream>
#include <cstdlib>
#include <iostream>

using namespace gridUnits;

void loadPSATBusArray (gridCoreObject *parentObject, double basepower, const mArray &buses, const mArray &SW,
                       const mArray &PV, const mArray &PQ, const stringVec &busnames, std::vector<gridBus *> &busList);
void loadPSATGenArray (gridCoreObject *parentObject, const mArray &buses, const std::vector<gridBus *> &busList);
void loadPSATLinkArray (gridCoreObject *parentObject, const mArray &lnks, const std::vector<gridBus *> &busList);
void loadPSATLinkArrayB (gridCoreObject *parentObject, const mArray &lnks, const std::vector<gridBus *> &busList);
void loadPSATShuntArray (gridCoreObject *parentObject, const mArray &shunts, const std::vector<gridBus *> &busList);
void loadPSATLTCArray (gridCoreObject *parentObject, const mArray &ltc, const std::vector<gridBus *> &busList);
void loadPSATPHSArray (gridCoreObject *parentObject, const mArray &phs, const std::vector<gridBus *> &busList);
void loadPSATSynArray (gridCoreObject *parentObject, const mArray &phs, const std::vector<gridBus *> &busList);
void loadPSATExcArray (gridCoreObject *parentObject, const mArray &exc, const std::vector<gridBus *> &busList);
void loadPSATTgArray (gridCoreObject *parentObject, const mArray &exc, const std::vector<gridBus *> &busList);
void loadPsatFaultArray (gridCoreObject *parentObject, const mArray &fault, const std::vector<gridBus *> &busList);
void loadPsatBreakerArray (gridCoreObject *parentObject, const mArray &brkr, const std::vector<gridBus *> &busList);
void loadPsatSupplyArray (gridCoreObject *parentObject, const mArray &supply, const std::vector<gridBus *> &busList);
void loadPsatMotorArray (gridCoreObject *parentObject, const mArray &mtr, const std::vector<gridBus *> &busList);


void loadOtherObjectData (gridCoreObject *parentObject, std::string filetext, const std::vector<gridBus *> &busList);
static const std::vector<std::pair<std::string, void (*)(gridCoreObject *, const mArray &, const std::vector<gridBus *> &)>>
arrayIdentifiers
{{
   "Shunt.con",loadPSATShuntArray
 },
 {
   "Line.con",loadPSATLinkArray
 },
 {
   "Lines.con",loadPSATLinkArrayB
 },
 {
   "Gen.con",loadPSATGenArray
 },
 {
   "Ltc.con",loadPSATLTCArray
 },
 {
   "Phs.con",loadPSATPHSArray
 },
 {
   "Syn.con",loadPSATSynArray
 },
 {
   "Exc.con",loadPSATExcArray
 },
//{ "Tg.con",loadPSATTgArray },
 {
   "Fault.con",loadPsatFaultArray
 },
 {
   "Breaker.con",loadPsatBreakerArray
 },
//{ "Supply.con",loadPsatSupplyArray },
 {
   "Mot.con",loadPsatMotorArray
 },};

void loadPSAT (gridCoreObject *parentObject, const std::string &filetext, const basicReaderInfo &bri)
{
  double basepower = 100;
  // std::string tstr;
  mArray M1,SW,PQ,PV;
  std::vector<gridBus *> busList;
  /*
  A = filetext.find(basename + ".baseMVA") const;
  if (A != std::string::npos)
  {
          B = filetext.find_first_of('=', A);
          C = filetext.find_first_of(";\n", A);
          tstr = filetext.substr(B + 1, C - B - 1);
          paramRead(tstr, basepower);
          parentObject->set("basepower", basepower);
  }
  */
  //get the list of bus names
  bool nmfnd = false;
  gridSimulation::resetObjectCounters ();       //reset all the object counters to 0 to make sure all the numbers match up

  stringVec Vnames;
  auto A = filetext.find ("Varname.bus");
  if (A != std::string::npos)
    {

      size_t B = filetext.find_first_of ('=', A);
      Vnames = readMatlabCellArray (filetext, B + 1);
      nmfnd = true;
    }
  if (nmfnd == 0)
    {
      A = filetext.find ("Bus.names");
      if (A != std::string::npos)
        {

          size_t B = filetext.find_first_of ('=', A);
          Vnames = readMatlabCellArray (filetext, B + 1);
          nmfnd = true;
        }
    }
  if (nmfnd)
    {
      if (!(bri.prefix.empty ()))
        {
          for (auto &vk : Vnames)
            {
              vk = bri.prefix + '_' + vk;
            }
        }
    }
  // now find the bus structure
  A = filetext.find ("Bus.con");
  if (A != std::string::npos)
    {
      size_t B = filetext.find_first_of ('=', A);
      readMatlabArray (filetext, B + 1, M1);
      readMatlabArray ("SW.con", filetext, SW);

      if (Vnames.size () != M1.size ())
        {
          if (Vnames.empty ())
            {
              Vnames.resize (M1.size ());
              for (stringVec::size_type kk = 0; kk < M1.size (); ++kk)
                {
                  if (bri.prefix.empty ())
                    {
                      Vnames[kk] = "Bus-" + std::to_string (M1[kk][0]);
                    }
                  else
                    {
                      Vnames[kk] = bri.prefix + "_Bus-" + std::to_string (M1[kk][0]);
                    }
                }
            }
          else
            {
              std::cout << "WARNING: number of bus names does not match the number of buses listed\n";
            }
        }
      readMatlabArray ("PV.con", filetext, PV);
      readMatlabArray ("PQ.con", filetext, PQ);
      loadPSATBusArray (parentObject, basepower, M1,SW,PV,PQ, Vnames, busList);
    }
  loadOtherObjectData (parentObject, filetext, busList);

}


void loadOtherObjectData (gridCoreObject *parentObject, std::string filetext, const std::vector<gridBus *> &busList)
{
  mArray M1;
  for (auto &namepair : arrayIdentifiers)
    {
      auto A = filetext.find (namepair.first);
      if (A != std::string::npos)
        {
          size_t B = filetext.find_first_of ('=', A);
          readMatlabArray (filetext, B + 1, M1);
          namepair.second (parentObject, M1, busList);
        }
    }
}

void loadPSATBusArray (gridCoreObject *parentObject, double basepower, const mArray &buses, const mArray &SW,
                       const mArray &PV, const mArray &PQ, const stringVec &busnames, std::vector<gridBus *> &busList)
{
  std::vector<double>::size_type kk;
  gridLoad *ld;
  gridBus *bus;
  gridDynGenerator *gen;
  size_t ind1;
  double temp;
  double P, Q;
  for (kk = 0; kk < buses.size (); ++kk)
    {
      ind1 = static_cast<size_t> (buses[kk][0]);
      if (ind1 > busList.size ())
        {
          busList.resize (ind1 * 2 + 1);
        }
      if (busList[ind1 - 1] == nullptr)
        {
          busList[ind1 - 1] = new acBus (busnames[kk]);
          busList[ind1 - 1]->set ("basepower", basepower);
          busList[ind1 - 1]->setUserID (static_cast<int> (ind1));
          parentObject->add (busList[ind1 - 1]);
        }
      bus = busList[ind1 - 1];
      temp = buses[kk][1];

      bus->set ("basevoltage", buses[kk][1]);
      if (buses[kk].size () > 2)
        {
          bus->set ("voltage", buses[kk][2]);
        }
      if (buses[kk].size () > 3)
        {
          bus->set ("angle", buses[kk][3]);
        }

    }
  for (kk = 0; kk < SW.size (); ++kk)
    {
      temp = SW[kk][0];
      ind1 = static_cast<size_t> (temp);
      bus = busList[ind1 - 1];
      bus->set ("type", "swing");
      bus->set ("vtarget", SW[kk][3]);
      bus->set ("atarget", SW[kk][4]);

      gen = new gridDynGenerator ();
      bus->add (gen);
      if (SW[kk].size () >= 7)
        {
          gen->set ("qmax", SW[kk][5]);
          gen->set ("qmin", SW[kk][6]);
        }
      if (SW[kk].size () >= 9)
        {
          bus->set ("vmax", SW[kk][7]);
          bus->set ("vmin", SW[kk][8]);
        }
      if (SW[kk].size () >= 10)
        {
          gen->set ("p", SW[kk][9]);
        }

    }
  for (kk = 0; kk < PV.size (); ++kk)
    {
      temp = PV[kk][0];
      ind1 = static_cast<size_t> (temp);
      bus = busList[ind1 - 1];
      bus->set ("type", "PV");
      bus->set ("vtarget", PV[kk][4]);
      gen = new gridDynGenerator;
      bus->add (gen);
      gen->set ("p", PV[kk][3]);

      if (PV[kk].size () >= 7)
        {
          gen->set ("qmax", PV[kk][5]);
          gen->set ("qmin", PV[kk][6]);
        }
      if (PV[kk].size () >= 9)
        {
          bus->set ("vmax", PV[kk][7]);
          bus->set ("vmin", PV[kk][8]);
        }
      if (PV[kk].size () >= 10)
        {

        }

    }
  for (kk = 0; kk < PQ.size (); ++kk)
    {
      temp = PQ[kk][0];
      ind1 = static_cast<size_t> (temp);
      bus = busList[ind1 - 1];
      P = PQ[kk][3];
      Q = PQ[kk][4];
      if ((P != 0) || (Q != 0))
        {
          ld = new gridLoad (P,Q);
          bus->add (ld);
        }

      if (PQ[kk].size () >= 7)
        {
          bus->set ("vmax", PQ[kk][5]);
          bus->set ("vmin", PQ[kk][6]);
        }

    }
}

void loadPSATGenArray (gridCoreObject * /*parentObject*/, const mArray &gens, const std::vector<gridBus *> &busList)
{
  for (size_t kk = 0; kk < gens.size (); ++kk)
    {
      size_t ind1 = static_cast<size_t> (gens[kk][0]);
      gridBus *bus = busList[ind1 - 1];
      gridDynGenerator *gen = new gridDynGenerator ();
      bus->add (gen);
      if (gens[kk][1] != 0)
        {
          gen->set ("p", gens[kk][1], MW);

        }
      if (gens[kk][2] != 0)
        {
          gen->set ("q", gens[kk][2], MVAR);

        }
      gen->set ("qmax", gens[kk][3], MVAR);
      gen->set ("qmin", gens[kk][4], MVAR);
      bus->set ("vtarget", gens[kk][5]);
      if (gens[kk][6] > 0.0)
        {
          gen->set ("mbase", gens[kk][6], MVAR);
        }
      if (gens[kk][7] <= 0)
        {
          gen->disable ();
        }
      if (gens[kk][8] != 0)
        {
          gen->set ("pmax", gens[kk][8], MW);
        }

      if (gens[kk][9] != 0)
        {
          gen->set ("pmin", gens[kk][9], MW);
        }
    }
}

/*
Column Variable Description Unit
1 - Bus number int
2 Sn Power rating MVA
3 PS0 Forecasted active power pu
4 PSmax Maximum power bid pu
5 PSmin Minimum power bid pu
6 PS Actual active power bid pu
7 CP0 Fixed cost(active power) $ / h
8 CP1 Proportional cost(active power) $ / MWh
9 CP2 Quadratic cost(active power) $ / MW2h
10 CQ0 Fixed cost(reactive power) $ / h
11 CQ1 Proportional cost(reactive power) $ / MVArh
12 CQ2 Quadratic cost(reactive power) $ / MVAr2h
13 u Commitment variable boolean
14 kTB Tie breaking cost $ / MWh
*/

void loadPsatSupplyArray (gridCoreObject *parentObject, const mArray &genCost, const std::vector<gridBus *> &busList)
{
#ifndef OPTIMIZATION_ENABLE
  return;
#else
  gridGenOpt *go;
  gridOptObject *oo;
//  gridCoreObject *obj;
  gridBus *bus;
  gridDynGenerator *gen;
  gridDynOptimization *gdo = dynamic_cast<gridDynOptimization *> (parentObject->find ("root"));
  if (!(gdo))
    {
      return;
    }
  for (auto &genLine : genCost)
    {
      bus = busList[static_cast<size_t> (genLine[0]) - 1];
      gen = bus->getGen ();
      if (!(gen))
        {
          continue;
        }
      go = new gridGenOpt (gen);
      oo = gdo->makeOptObjectPath (bus);
      oo->add (go);

      if (genLine[2] != 0)
        {
          go->set ("forecast", genLine[2], MW);
        }
      if (genLine[3] != 0)
        {
          go->set ("pmax", genLine[3], MW);
        }
      if (genLine[4] != 0)
        {
          go->set ("pmin", genLine[4], MW);
        }

      if (genLine[6] > 0)
        {
          go->set ("constantp",genLine[6],Cph);
        }
      if (genLine[9] > 0)
        {
          go->set ("constantq", genLine[9], Cph);
        }
      if (genLine[7] > 0)
        {
          go->set ("linearp", genLine[7], CpMWh);
        }
      if (genLine[10] > 0)
        {
          go->set ("linearq", genLine[10], CpMVARh);
        }
      if (genLine[8] != 0)
        {
          go->set ("quadraticp", genLine[8], CpMW2h);
        }
      if (genLine[11] != 0)
        {
          go->set ("quadraticq", genLine[11], CpMVAR2h);
        }
      if (genLine[13] != 0)
        {
          go->set ("penalty", genLine[13], CpMWh);
        }
    }
#endif
}

/* Branch data
Column Variable Description Unit
1 k From Bus int
2 m To Bus int
3 Sn Power rating MVA
4 Vn Voltage rating kV
5 fn Frequency rating Hz
6 - not used -
7 kT Primary and secondary voltage ratio kV/kV
8 r Resistance pu
9 x Reactance pu
10 - not used -
y 11 a Fixed tap ratio pu/pu
y 12  Fixed phase shift deg
y 13 Imax Current limit pu
y 14 Pmax Active power limit pu
y 15 Smax Apparent power limit pu
*/

void loadPSATLinkArray (gridCoreObject *parentObject, const mArray &lnks, const std::vector<gridBus *> &busList)
{
  std::vector<double>::size_type kk;
  gridLink *lnk;
  gridBus *bus1, *bus2;
  size_t ind1, ind2;
  double temp;
  bool tx = false;
  double len;
  for (kk = 0; kk < lnks.size (); ++kk)
    {
      temp = lnks[kk][0];
      ind1 = static_cast<size_t> (temp);
      bus1 = busList[ind1 - 1];
      temp = lnks[kk][1];
      ind2 = static_cast<size_t> (temp);
      bus2 = busList[ind2 - 1];
      lnk = new gridLink ();

      lnk->updateBus (bus1, 1);
      lnk->updateBus (bus2, 2);
      parentObject->add (lnk);
      if (lnks[kk][6] != 0)
        {
          tx = true;
        }
      if (tx)
        {
          lnk->set ("r", lnks[kk][7]);
          lnk->set ("x", lnks[kk][8]);
        }
      else
        {
          len = lnks[kk][5];
          if (len > 0)
            {
              lnk->set ("r", lnks[kk][7] * len);
              lnk->set ("x", lnks[kk][8] * len);
              lnk->set ("b", lnks[kk][9] * len);
            }
          else
            {
              lnk->set ("r", lnks[kk][7]);
              lnk->set ("x", lnks[kk][8]);
              lnk->set ("b", lnks[kk][9]);
            }

        }



      if (lnks[kk][5] != 0)
        {
          lnk->set ("ratinga", lnks[kk][2],MVAR);
        }

      if (lnks[kk].size () >= 11)
        {

          if (lnks[kk][10] > 0.05)              //just make sure list a tap
            {
              lnk->set ("tap", lnks[kk][10]);
            }
        }
      if (lnks[kk].size () >= 12)
        {

          if (lnks[kk][11] != 0)
            {
              lnk->set ("tapangle", lnks[kk][11], deg);
            }
        }

    }
}

void loadPSATLinkArrayB (gridCoreObject *parentObject, const mArray &lnks, const std::vector<gridBus *> &busList)
{
  std::vector<double>::size_type kk;
  gridLink *lnk;
  gridBus *bus1, *bus2;
  size_t ind1, ind2;
  double temp;
  bool tx = false;
  double len;
  for (kk = 0; kk < lnks.size (); ++kk)
    {
      temp = lnks[kk][0];
      ind1 = static_cast<size_t> (temp);
      bus1 = busList[ind1 - 1];
      temp = lnks[kk][1];
      ind2 = static_cast<size_t> (temp);
      bus2 = busList[ind2 - 1];
      lnk = new gridLink ();
      lnk->updateBus (bus1, 1);
      lnk->updateBus (bus2, 2);
      parentObject->add (lnk);
      if (lnks[kk][6] != 0)
        {
          tx = true;
        }
      if (tx)
        {
          lnk->set ("r", lnks[kk][7]);
          lnk->set ("x", lnks[kk][8]);
        }
      else
        {
          len = lnks[kk][5];
          lnk->set ("r", lnks[kk][7] * len);
          lnk->set ("x", lnks[kk][8] * len);
          lnk->set ("b", lnks[kk][9] * len);
        }



      if (lnks[kk][5] != 0)
        {
          lnk->set ("ratinga", lnks[kk][2], MVAR);
        }

      if (lnks[kk].size () >= 11)
        {

          if (lnks[kk][10] > 0.05)              //just make sure list a tap
            {
              lnk->set ("tap", lnks[kk][10]);
            }
        }
      if (lnks[kk].size () >= 12)
        {

          if (lnks[kk][11] != 0)
            {
              lnk->set ("tapangle", lnks[kk][11], deg);
            }
        }

    }
}

void loadPSATShuntArray (gridCoreObject * /*parentObject*/, const mArray &shunts, const std::vector<gridBus *> &busList)
{
  std::vector<double>::size_type kk;
  gridLoad *ld;
  gridBus *bus1;
  size_t ind1;
  double g,b,temp;

  for (kk = 0; kk < shunts.size (); ++kk)
    {
      temp = shunts[kk][0];
      ind1 = static_cast<size_t> (temp);
      bus1 = busList[ind1 - 1];

      ld = bus1->getLoad ();
      if (ld == nullptr)
        {
          ld = new gridLoad ();
          bus1->add (ld);
        }

      g = shunts[kk][4];
      b = shunts[kk][5];
      if (g != 0)
        {
          ld->set ("yp", g);
        }
      if (b != 0)
        {
          ld->set ("yq", b);
        }
      if (shunts[kk].size () > 6)
        {
          if (shunts[kk][6] == 0)
            {
              ld->disable ();
            }
        }

    }
}
/*
Column Variable Description Unit
1 k Bus number(from) int
2 m Bus number(to) int
3 Sn Power rating MVA
4 Vn Voltage rating kV
5 fn Frequency rating Hz
6 kT Nominal tap ratio kV / kV
7 H Integral deviation pu
8 K Inverse time constant 1 / s
9 mmax Max tap ratio pu / pu
10 mmin Min tap ratio pu / pu
11 m Tap ratio step pu / pu
12 Vref(Qref) Reference voltage(power) pu
13 xT Transformer reactance pu
14 rT Transformer resistance pu
15 r Remote control bus number int
16 - Control
1 Secondary voltage Vm
2 Reactive power Qm
3 Remote voltage Vr
int
17 u Connection status{ 0, 1 }
*/
void loadPSATLTCArray (gridCoreObject *parentObject, const mArray &ltc, const std::vector<gridBus *> &busList)
{
  for (size_t kk = 0; kk < ltc.size (); ++kk)
    {
      size_t ind1 = static_cast<size_t> (ltc[kk][0]);
      gridBus *bus1 = busList[ind1 - 1];
      size_t ind2 = static_cast<size_t> (ltc[kk][1]);
      gridBus *bus2 = busList[ind2 - 1];
      gridLink *lnk = new adjustableTransformer ();

      lnk->updateBus (bus1, 1);
      lnk->updateBus (bus2, 2);
      parentObject->add (lnk);
      lnk->set ("r", ltc[kk][13]);
      lnk->set ("x", ltc[kk][12]);
      lnk->set ("mintap", ltc[kk][9]);
      lnk->set ("maxtap", ltc[kk][8]);
      //	lnk->set("tap", ltc[kk][10]);
      lnk->set ("stepsize", ltc[kk][11]);
      switch (static_cast<int> (ltc[kk][15]))
        {
        case 1:         //secondary voltage
          lnk->set ("mode", "v");
          lnk->set ("vtarget", ltc[kk][11]);
          break;
        case 2:        //reactive power
          lnk->set ("mode", "mvar");
          lnk->set ("qtarget", ltc[kk][11]);
          break;
        case 3:         //remote control voltage bus
          lnk->set ("mode", "v");
          lnk->set ("vtarget", ltc[kk][11]);
          ind1 = static_cast<size_t> (ltc[kk][14]);
          bus1 = busList[ind1 - 1];
          static_cast<adjustableTransformer *> (lnk)->setControlBus (bus1);
          break;
        }
      //check if lnk is enabled
      if (ltc[kk].size () == 18)
        {
          if (ltc[kk][17] < 0.1)               //lnk is disabled
            {
              lnk->disconnect ();
            }
        }
      else
        {
          if (ltc[kk][16] < 0.1)               //lnk is disabled
            {
              lnk->disconnect ();
            }
        }

    }
}
/*
1 k Bus number(from) int
2 m Bus number(to) int
3 Sn Power rating MVA
4 Vn1 Primary voltage rating kV
5 Vn2 Secondary voltage rating kV
6 fn Frequency rating Hz
7 Tm Measurement time constant s
8 Kp Proportional gain -
9 Ki Integral gain -
10 Pref Reference power pu
11 rT Transformer resistance pu
12 xT Transformer reactance pu
13 αmax Maximum phase angle rad
14 αmin Minimum phase angle rad
15 m Transformer fixed tap ratio pu / pu
16 u Connection status{ 0, 1 }
*/
void loadPSATPHSArray (gridCoreObject *parentObject, const mArray &phs, const std::vector<gridBus *> &busList)
{
  gridLink *lnk;
  gridBus *bus1, *bus2;
  for (size_t kk = 0; kk < phs.size (); ++kk)
    {
      size_t ind1 = static_cast<size_t> (phs[kk][0]);
      bus1 = busList[ind1 - 1];
      size_t ind2 = static_cast<size_t> (phs[kk][1]);
      bus2 = busList[ind2 - 1];
      lnk = new adjustableTransformer ();

      lnk->updateBus (bus1, 1);
      lnk->updateBus (bus2, 2);
      parentObject->add (lnk);
      lnk->set ("r", phs[kk][10]);
      lnk->set ("x", phs[kk][11]);
      lnk->set ("mintapangle",phs[kk][13]);
      lnk->set ("maxtapangle", phs[kk][12]);
      lnk->set ("tap", phs[kk][14]);

      lnk->set ("mode", "mw");
      lnk->set ("ptarget", phs[kk][9]);
      lnk->set ("change", "continuous");
      //check if lnk is enabled
      if (phs[kk][15] < 0.1)           //lnk is disabled
        {
          lnk->disable ();
        }
    }
}
/*
1 - Bus number int all
2 Sn Power rating MVA all
3 Vn Voltage rating kV all
4 fn Frequency rating Hz all
5 - Machine model - all
6 xl Leakage reactance pu all
7 ra Armature resistance pu all
8 xd d-axis synchronous reactance pu III, IV, V.1, V.2, V.3, VI, VIII
9 x′
d d-axis transient reactance pu II, III, IV, V.1, V.2, V.3, VI, VIII
10 x′′
d d-axis subtransient reactance pu V.2, VI, VIII
11 T′
d0 d-axis open circuit transient time constant s III, IV, V.1, V.2, V.3, VI, VIII
12 T′′
d0 d-axis open circuit subtransient time constant s V.2, VI, VIII
13 xq q-axis synchronous reactance pu III, IV, V.1, V.2, V.3, VI, VIII
14 x′
q q-axis transient reactance pu IV, V.1, VI, VIII
15 x′′
q q-axis subtransient reactance pu V.2, VI, VIII
16 T′
q0 q-axis open circuit transient time constant s IV, V.1, VI, VIII
17 T′′
q0 q-axis open circuit subtransient time constant s V.1, V.2, VI, VIII
18 M = 2H Mechanical starting time (2 × inertia constant) kWs/kVA all
19 D Damping coefficient − all
† 20 Kω Speed feedback gain gain III, IV, V.1, V.2, VI
† 21 KP Active power feedback gain gain III, IV, V.1, V.2, VI
† 22 γP Active power ratio at node [0,1] all
† 23 γQ Reactive power ratio at node [0,1] all
† 24 TAA d-axis additional leakage time constant s V.2, VI, VIII
† 25 S(1.0) First saturation factor - III, IV, V.1, V.2, VI, VIII
† 26 S(1.2) Second saturation factor - III, IV, V.1, V.2, VI, VIII
† 27 nCOI Center of inertia number int all
† 28 u Connection status {0, 1} all
*/

void loadPSATSynArray (gridCoreObject * /*parentObject*/, const mArray &syn, const std::vector<gridBus *> &busList)
{
  gridDynGenerator *gen;
  gridDynGenModel *gm = nullptr;
  gridBus *bus1;
  size_t ind1;
  double mode;
  double temp;
  int index = 1;
  for (auto genData : syn)
    {
      temp = genData[0];
      ind1 = static_cast<size_t> (temp);
      bus1 = busList[ind1 - 1];
      gen = bus1->getGen (0);
      if (!(gen))
        {
          continue;
        }
      gen->setUserID (index);
      ++index;
      mode = genData[4];
      if (mode < 2.1)          //second order classical model
        {
          gm = new gridDynGenModel ();
        }
      else if (mode < 3.1)          //3rd order model
        {
          gm = new gridDynGenModel3 ();
        }
      else if (mode < 4.1)          //4th order model
        {
          gm = new gridDynGenModel4 ();
        }
      else if (mode < 5.15)           //5th order model type 1
        {
          gm = new gridDynGenModel5 ();
        }
      else if (mode < 5.25)           //5th order model type 2
        {
          gm = new gridDynGenModel5type2 ();
        }
      else if (mode < 5.35)           //5th order model type 3
        {
          gm = new gridDynGenModel5type3 ();
        }
      else if (mode < 6.05)           //6th order model
        {
          gm = new gridDynGenModel6type2 ();
        }
      else if (mode < 8.05)           //8th order model
        {
          gm = new gridDynGenModel8 ();
        }
      if (gm == nullptr)
        {
          std::cout << "genModel " << mode << " not implmented yet\n";
          continue;
        }
      gm->set ("rating", genData[1], MW);
      gen->set ("basevoltage", genData[2], kV);
      double xl = genData[5];
      gm->set ("xl", genData[5]);
      gm->set ("r", genData[6]);
      gm->set ("xdp", genData[8] - xl);
      gm->set ("h", genData[17] / 2.0);
      gm->set ("d", genData[18],puHz);           //the damping coefficient in PSAT is in puHz
      if (mode > 2.1)          //deal with the voltage speed adjustment
        {
          if (genData.size () >= 21)
            {
              gm->set ("kw", genData[19]);
              gm->set ("kp", genData[20]);
            }
          gm->set ("xd", genData[7] - xl);
          gm->set ("tdop", genData[10]);
          gm->set ("xq", genData[12] - xl);
        }
      if (mode >= 3.1)
        {
          gm->set ("xqp", genData[13] - xl);
          gm->set ("tqop", genData[15]);
        }
      if (mode > 4.9)
        {
          gm->set ("tqopp", genData[16]);
        }
      if ((mode == 5.2) || (mode >= 6))
        {
          if (genData.size () >= 24)
            {
              gm->set ("taa", genData[23]);
            }
          gm->set ("xdpp", genData[9] - xl);
          gm->set ("tdopp", genData[11]);
          gm->set ("xqpp", genData[14] - xl);
        }



    }
}

/*
Table 16.1: Turbine Governor Type I Data Format (Tg.con)
Column Variable Description Unit
1 - Generator number int
2 1 Turbine governor type int
3 !ref Reference speed pu
4 R Droop pu/pu
5 Tmax Maximum turbine output pu
6 Tmin Minimum turbine output pu
7 Ts Governor time constant s
8 Tc Servo time constant s
9 T3 Transient gain time constant s
10 T4 Power fraction time constant s
11 T5 PSfrag replacements Reheat time constant s
*/

void loadPSATTgArray (gridCoreObject *parentObject, const mArray &tg, const std::vector<gridBus *> & /*busList*/)
{
  gridDynGenerator *gen;
  gridDynGovernor *gm = nullptr;
  //gridBus *bus1;
  index_t ind1;
  double mode;
  double temp;
  for (auto govData : tg)
    {
      temp = govData[0];
      ind1 = static_cast<index_t> (temp);

      gen = static_cast<gridDynGenerator *> (parentObject->findByUserID ("gen", ind1));
      if (!(gen))
        {
          continue;
        }
      mode = govData[1];
      if (mode < 1.1)          //second order classical model
        {
          gm = new gridDynGovernor ();
        }
      else if (mode < 2.1)          //3rd order model
        {
          gm = new gridDynGovernorIeeeSimple ();
        }

      if (gm == nullptr)
        {
          std::cout << "governor " << mode << " not implmented yet\n";
          continue;
        }
      gm->set ("r", govData[3]);
      gm->set ("pmax", govData[4]);
      gm->set ("pmin", govData[5]);

      gm->set ("t1", govData[6]);
      gm->set ("t2", govData[7]);
      gm->set ("t3", govData[8]);
      gm->set ("t4", govData[9]);
    }
}


void loadPSATExcArray (gridCoreObject *parentObject, const mArray &excData, const std::vector<gridBus *> & /*busList*/)
{
  gridDynGenerator *gen;
  gridDynExciter *gm = nullptr;
  index_t ind1;
  double mode;
  for (auto eData : excData)
    {
      ind1 = static_cast<index_t> (eData[0]);

      gen = static_cast<gridDynGenerator *> (parentObject->findByUserID ("gen", ind1));
      if (!(gen))
        {
          continue;
        }

      mode = eData[1];
      if (mode < 1.1)          //second order classical model
        {
          gm = new gridDynExciter ();
        }
      else if (mode < 2.1)          //3rd order model
        {
          gm = new gridDynExciter ();
        }

      if (gm == nullptr)
        {
          std::cout << "exciter " << mode << " not implmented yet\n";
          continue;
        }
      gm->set ("r", eData[3]);
      gm->set ("pmax", eData[4]);
      gm->set ("pmin", eData[5]);

      gm->set ("t1", eData[6]);
      gm->set ("t2", eData[7]);
      gm->set ("t3", eData[8]);
      gm->set ("t4", eData[9]);
    }
}


void loadPsatFaultArray (gridCoreObject *parentObject, const mArray &fault, const std::vector<gridBus *> &busList)
{
  gridBus *bus;
  std::shared_ptr<gridEvent> evnt1, evnt2;
  gridSimulation *gds = dynamic_cast<gridSimulation *> (parentObject->find ("root"));
  if (gds == nullptr)
    {     //cant make faults if we don't have access to the simulation
      return;
    }
  gridLoad *ld;
  int ind;
  for (auto &flt : fault)
    {
      ind = static_cast<int> (flt[0]);
      bus = busList[ind - 1];

      ld = new gridLoad ();
      bus->add (ld);

      if (flt[6] != 0)
        {
          evnt1 = std::make_shared<gridEvent> (flt[4]);
          evnt2 = std::make_shared<gridEvent> (flt[5]);
          evnt1->setTarget (ld, "r");
          evnt1->value = flt[6];
          evnt2->setTarget (ld, "r");
          evnt2->value = 0;
          gds->add (evnt1);
          gds->add (evnt2);
        }

      if (flt[6] != 0)
        {
          evnt1 = std::make_shared<gridEvent> (flt[4]);
          evnt2 = std::make_shared<gridEvent> (flt[5]);
          evnt1->setTarget (ld, "x");
          evnt1->value = flt[7];
          evnt2->setTarget (ld, "x");
          evnt2->value = 0;
          gds->add (evnt1);
          gds->add (evnt2);
        }


    }
}

void loadPsatBreakerArray (gridCoreObject *parentObject, const mArray &brkr, const std::vector<gridBus *> & /*busList*/)
{
  double status = 1.0;
  gridLink *lnk;
  gridSimulation *gds = dynamic_cast<gridSimulation *> (parentObject->find ("root"));
  if (gds == nullptr)
    {     //cant make faults if we don't have access to the simulation
      return;
    }
  std::shared_ptr<gridEvent> evnt1, evnt2;
  index_t ind;
  for (auto &brk : brkr)
    {
      ind = static_cast<index_t> (brk[0]);
      lnk = static_cast<gridLink *> (parentObject->findByUserID ("link", ind));

      if (brk[5] < 0.1)
        {
          lnk->disable ();
          status = 0.0;
        }
      evnt1 = std::make_shared<gridEvent> (brk[6]);
      evnt2 = std::make_shared<gridEvent> (brk[7]);
      evnt1->setTarget (lnk, "enabled");
      evnt1->value = (status < 0.1) ? 1.0 : 0.0;
      evnt2->setTarget (lnk, "enabled");
      evnt2->value = status;
      gds->add (evnt1);
      gds->add (evnt2);

    }
}

void loadPsatMotorArray (gridCoreObject * /*parentObject*/, const mArray &mtr, const std::vector<gridBus *> &busList)
{

  for (size_t kk = 0; kk < mtr.size (); ++kk)
    {
      size_t ind1 = static_cast<size_t> (mtr[kk][0]);
      gridBus *bus1 = busList[ind1 - 1];

      motorLoad *motor = new motorLoad ();
      bus1->add (motor);
      //TODO:PT add parameters
    }
}
