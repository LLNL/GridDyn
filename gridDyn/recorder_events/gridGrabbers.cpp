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
#include "relays/sensor.h"
#include "relays/gridRelay.h"
#include "gridArea.h"
#include "gridBus.h"
#include "simulation/gridSimulation.h"
#include "vectorOps.hpp"
#include "grabberInterpreter.hpp"
#include "functionInterpreter.h"

#include <cmath>
#include <algorithm>

using namespace gridUnits;





std::shared_ptr<gridGrabber> gridGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ggb) const
{
  if (ggb == nullptr)
    {
      ggb = std::make_shared<gridGrabber> ();
    }
  ggb->desc = desc;
  ggb->field = field;
  ggb->fptr = fptr;
  ggb->fptrV = fptrV;
  ggb->fptrN = fptrN;
  ggb->gain = gain;
  ggb->bias = bias;
  ggb->inputUnits = inputUnits;
  ggb->outputUnits = outputUnits;
  ggb->vectorGrab = vectorGrab;
  ggb->loaded = loaded;
  if (nobj)
    {
      ggb->updateObject (nobj);
    }
  else
    {
      ggb->cobj = nobj;
    }
  return ggb;
}

int gridGrabber::setInfo (std::string fld, gridCoreObject* obj)
{
  if (fld == "null")        //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  field = fld;
  if (fld == "nextupdatetime")
    {
      fptr = [ = ](){
          return cobj->getNextUpdateTime ();
        };
    }
  else if (fld == "lastupdateTime")
    {
      fptr = [ = ](){
          return cobj->get ("lastupdatetime");
        };
    }
  else if (fld == "constant")
    {
      fptr = [](){
          return 0.0;
        };
    }
  else
    {
      double testval = obj->get (fld);
      if (testval != kNullVal)
        {

        }
      else
        {
          loaded = false;
          ret = NOT_LOADED;
        }
    }
  if (ret == LOADED)
    {
      cobj = obj;
      loaded = true;
    }
  makeDescription ();
  return ret;
}

void gridGrabber::getDesc (std::vector<std::string > &desc_list) const
{
  if (vectorGrab)
    {
      fptrN (desc_list);
      for (auto &dl : desc_list)
        {
          dl += ':' + field;
        }
    }
  else
    {
      desc_list.resize (1);
      desc_list[0] = desc;
    }
}

double gridGrabber::grabData ()
{
  double val;
  if (loaded)
    {
      if (fptr)
        {
          val = fptr ();
          if (outputUnits != defUnit)
            {
              val = unitConversion (val, inputUnits, outputUnits, cobj->getBasePower (), m_baseVoltage);
            }
        }
      else
        {
          val = cobj->get (field, outputUnits);
        }
      //val = val * gain + bias;
      val = std::fma (val, gain, bias);
    }
  else
    {
      val = kNullVal;
    }

  return val;
}

void gridGrabber::grabData (std::vector<double> &vals)
{
  if (loaded)
    {
      fptrV (vals);
      if (outputUnits != defUnit)
        {
          for (auto &v : vals)
            {
              v = unitConversion (v, inputUnits, outputUnits, cobj->getBasePower (),m_baseVoltage);
            }
        }
    }
  else
    {
      vals.resize (0);
    }
}

void gridGrabber::updateObject (gridCoreObject *obj)
{
  if (obj)
    {
      cobj = obj;
      makeDescription ();
    }
  else
    {
      loaded = false;
    }
}

void gridGrabber::makeDescription ()
{
  desc = cobj->getName () + ':' + field;
  if (outputUnits != defUnit)
    {
      desc + '(' + to_string (outputUnits) + ')';
    }
}

std::shared_ptr<gridGrabber> createGrabber (const std::string &fld, gridCoreObject *obj)
{
  std::shared_ptr<gridGrabber> ggb = nullptr;

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus)
    {
      ggb = std::make_shared<gridBusGrabber> (fld, bus);
      return ggb;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      ggb = std::make_shared<gridLoadGrabber> (fld, ld);
      return ggb;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {

      ggb = std::make_shared<gridDynGenGrabber> (fld, gen);
      return ggb;
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk)
    {
      ggb = std::make_shared<gridLinkGrabber> (fld, lnk);
      return ggb;
    }

  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      ggb = std::make_shared<gridAreaGrabber> (fld, area);
      return ggb;
    }

  gridRelay *rel = dynamic_cast<gridRelay *> (obj);
  if (rel)
    {
      ggb = std::make_shared<gridRelayGrabber> (fld, rel);
      return ggb;
    }

  gridSubModel *sub = dynamic_cast<gridSubModel *> (obj);
  if (sub)
    {
      ggb = std::make_shared<subModelGrabber> (fld, sub);
      return ggb;
    }
  return ggb;

}

std::shared_ptr<gridGrabber> createGrabber (int noffset, gridCoreObject *obj)
{
  std::shared_ptr<gridGrabber> ggb = nullptr;

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      if (noffset > 0)
        {
          ggb = std::make_shared<gridDynGenGrabber> (noffset, gen);
        }
      return ggb;
    }

  return ggb;

}


void customGrabber::setGrabberFunction (std::string fld, std::function<double ()> nfptr)
{
  fptr = nfptr;
  loaded = true;
  vectorGrab = false;
  field = fld;
}

gridAreaGrabber::gridAreaGrabber (std::string fld, gridArea *gdA)
{
  setInfo (fld, gdA);
}

std::shared_ptr<gridGrabber> gridAreaGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ggb) const
{
  std::shared_ptr<gridAreaGrabber> agb;
  if (ggb)
    {
    }
  else
    {
      agb = std::make_shared<gridAreaGrabber> (std::string ("null"), nullptr);
    }
  agb->area = area;
  gridGrabber::clone (nobj, agb);
  return ggb;
}

void gridAreaGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridArea *> (obj))
    {
      area = static_cast<gridArea *> (obj);
      gridGrabber::updateObject (obj);
    }
  else
    {
      loaded = false;
    }
}

int gridAreaGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")      //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  field = fld;
  cobj = gdO;
  area = dynamic_cast<gridArea *> (gdO);

  makeLowerCase (field);
  if (field == "voltage")
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getVoltage (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getBusName (N);
        };
      inputUnits = puV;
      vectorGrab = true;
    }
  else if (field == "angle")
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getAngle (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getBusName (N);
        };
      inputUnits = rad;
      vectorGrab = true;
    }
  else if ((field == "busgenerationreal") || (field == "busgen") || (field == "busp")||(field == "busgenreal"))
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getBusGenerationReal (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getBusName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if ((field == "busgenerationreactive") || (field == "busq") || (field == "busgenreactive"))
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getBusGenerationReactive (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getBusName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if ((field == "busloadreal") || (field == "busload"))
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getBusLoadReal (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getBusName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if (field == "busloadreactive")
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getBusLoadReactive (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getBusName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if ((field == "linkreal") || (field == "linkrealpower") || (field == "linkp"))
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getLinkRealPower (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getLinkName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if ((field == "linkreactive") || (field == "linkreactivepower") || (field == "linkq"))
    {

      fptrV = [ = ](std::vector<double> &v){
          area->getLinkReactivePower (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getLinkName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if (field == "linkloss")
    {
      fptrV = [ = ](std::vector<double> &v){
          area->getLinkLoss (v);
        };
      fptrN = [ = ](stringVec &N){
          area->getLinkName (N);
        };
      inputUnits = puMW;
      vectorGrab = true;
    }
  else if ((field == "load") || (field == "loadreal") || (field == "loadp"))
    {
      fptr = [ = ](){
          return area->getLoadReal ();
        };
      inputUnits = puMW;
    }
  else if ((field == "loadreactive") || (field == "loadq"))
    {
      fptr = [ = ](){
          return area->getLoadReactive ();
        };
      inputUnits = puMW;
    }
  else if ((field == "gen") || (field == "generationreal") || (field == "genp"))
    {
      fptr = [ = ](){
          return area->getGenerationReal ();
        };
      inputUnits = puMW;
    }
  else if ((field == "generationreactive") || (field == "genq"))
    {
      fptr = [ = ](){
          return area->getGenerationReactive ();
        };
      inputUnits = puMW;
    }
  else if (field == "loss")
    {
      fptr = [ = ](){
          return area->getLoss ();
        };
      inputUnits = puMW;
    }
  else if (field == "tieflow")
    {
      fptr = [ = ](){
          return area->getTieFlowReal ();
        };
      inputUnits = puMW;
    }
  else
    {
      ret = gridGrabber::setInfo (field, gdO);
    }
  if (ret == LOADED)
    {
      loaded = true;
    }
  makeDescription ();
  return ret;
}

gridBusGrabber::gridBusGrabber (std::string fld, gridBus *gdB)
{
  setInfo (fld, gdB);
}

std::shared_ptr<gridGrabber> gridBusGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ggb) const
{
  std::shared_ptr<gridBusGrabber> bgb;
  if (ggb)
    {
    }
  else
    {
      bgb = std::make_shared<gridBusGrabber> (std::string ("null"), nullptr);
    }
  bgb->bus = bus;
  gridGrabber::clone (nobj, bgb);
  return bgb;
}

void gridBusGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridBus *> (obj))
    {
      bus = static_cast<gridBus *> (obj), gridGrabber::updateObject (obj);
      m_baseVoltage = bus->get ("basevoltage");
    }
  else
    {
      loaded = false;
    }
}

int gridBusGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")        //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  field = fld;

  bus = dynamic_cast<gridBus *> (gdO);
  cobj = bus;
  makeLowerCase (field);
  if ((field == "voltage")||(field == "v"))
    {

      fptr = [ = ](){
          return bus->getVoltage ();
        };
      inputUnits = puV;
    }
  else if ((field == "phase") || (field == "angle")||(field == "a"))
    {

      fptr = [ = ](){
          return bus->getAngle ();
        };
      inputUnits = rad;
    }
  else if ((field == "freq") || (field == "f"))
    {
      //lambda expression
      fptr = [ = ](){
          return bus->getFreq ();
        };
      inputUnits = rps;
    }
  else if ((field == "gen")|| (field == "generation")||(field == "genp")||(field == "genreal"))
    {
      fptr = [ = ](){
          return bus->getGenerationReal ();
        };
      inputUnits = puMW;
    }
  else if ((field == "genq")|| (field == "reactivegen")||(field == "genreactive"))
    {
      fptr = [ = ](){
          return bus->getGenerationReactive ();
        };
      inputUnits = puMW;
    }
  else if ((field == "load") || (field == "loadreal")||(field == "loadp"))
    {
      fptr = [ = ](){
          return bus->getLoadReal ();
        };
      inputUnits = puMW;
    }
  else if ((field == "loadq")||(field == "loadreactive"))
    {
      fptr = [ = ](){
          return bus->getLoadReactive ();
        };
      inputUnits = puMW;
    }
  else if ((field == "link") || (field == "linkp"))
    {

      fptr = [ = ](){
          return bus->getLinkReal ();
        };
      inputUnits = puMW;
    }
  else if (field == "linkq")
    {

      fptr = [ = ](){
          return bus->getLinkReactive ();
        };
      inputUnits = puMW;
    }
  else
    {
      ret = gridGrabber::setInfo (fld, gdO);
    }
  if (ret == LOADED)
    {
      loaded = true;
    }
  makeDescription ();

  return ret;
}

gridDynGenGrabber::gridDynGenGrabber (std::string fld, gridDynGenerator *gdG)
{
  setInfo (fld, gdG);
}

gridDynGenGrabber::gridDynGenGrabber (index_t nOffset, gridDynGenerator *gdG)
{
  setInfo (nOffset, gdG);
}

std::shared_ptr<gridGrabber> gridDynGenGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber>) const
{
  std::shared_ptr<gridDynGenGrabber> ggb = std::make_shared<gridDynGenGrabber> (std::string ("null"), nullptr);
  gridGrabber::clone (nobj, ggb);
  ggb->offset = offset;
  return ggb;
}

void gridDynGenGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridDynGenerator *> (obj))
    {
      gen = static_cast<gridDynGenerator *> (obj), gridGrabber::updateObject (obj);
    }
  else
    {
      loaded = false;
    }
}

int gridDynGenGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")       //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  field = fld;
  gen = dynamic_cast<gridDynGenerator *> (gdO);
  cobj = gdO;
  if (gen == nullptr)
    {
      ret = NOT_LOADED;
      loaded = false;
      return ret;
    }
  makeLowerCase (field);
  if ((field == "power") || (field == "p"))
    {

      fptr = [ = ](){
          return gen->getRealPower ();
        };
      inputUnits = puMW;
    }
  else if ((field == "reactive") || (field == "q"))
    {
      fptr = [ = ](){
          return gen->getReactivePower ();
        };
      inputUnits = puMW;
    }
  else if ((field == "pm")|| (field == "pset"))
    {
      fptr = [ = ](){
          return gen->getPset ();
        };
      inputUnits = puMW;
    }
  else if ((field == "freq") || (field == "omega"))
    {
      fptr = [ = ]() {
          return gen->getFreq (nullptr,cLocalSolverMode);
        };
      inputUnits = puHz;
    }
  else if (field == "angle")
    {
      fptr = [ = ]() {
          return gen->getAngle (nullptr, cLocalSolverMode);
        };
      inputUnits = rad;
    }
  else
    {
      offset = gen->findIndex (field, cLocalbSolverMode);
      if (offset == kInvalidLocation)
        {
          ret = gridGrabber::setInfo (fld, gdO);
        }
      inputUnits = defUnit;
    }
  if (ret == LOADED)
    {
      loaded = true;
    }
  makeDescription ();
  return ret;
}

int gridDynGenGrabber::setInfo (index_t nOffset, gridCoreObject *gdO)
{
  int ret = LOADED;
  gen = dynamic_cast<gridDynGenerator *> (gdO);
  cobj = gdO;
  if (gen == nullptr)
    {
      ret = NOT_LOADED;
      loaded = false;
      return ret;
    }
  if (offset < gen->stateSize (cLocalSolverMode))
    {
      offset = nOffset;
    }
  else
    {
      loaded = false;
      ret = NOT_LOADED;
    }
  if (ret == LOADED)
    {
      loaded = true;
    }
  desc = gen->getName () + ':' + std::to_string (nOffset);
  return ret;
}

double gridDynGenGrabber::grabData ()
{
  double val;
  if (loaded)
    {
      if (offset != kInvalidLocation)
        {
          if (offset == kNullLocation)
            {
              offset = gen->findIndex (field,cLocalbSolverMode);
            }
          if (offset != kNullLocation)
            {
              val = gen->getState (offset);
            }
          else
            {
              val = kNullVal;
            }
          val = val * gain + bias;
        }
      else
        {
          val = gridGrabber::grabData ();
        }
    }
  else
    {
      val = kNullVal;
    }
  return val;
}

subModelGrabber::subModelGrabber (std::string fld, gridSubModel*gdG)
{
  setInfo (fld, gdG);
}

subModelGrabber::subModelGrabber (index_t nOffset, gridSubModel *gdG)
{
  setInfo (nOffset, gdG);
}

std::shared_ptr<gridGrabber> subModelGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber>) const
{
  std::shared_ptr<subModelGrabber> ggb = std::make_shared<subModelGrabber> (std::string ("null"), nullptr);
  gridGrabber::clone (nobj, ggb);
  ggb->offset = offset;
  return ggb;
}

void subModelGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridSubModel *> (obj))
    {
      sub = static_cast<gridSubModel *> (obj), gridGrabber::updateObject (obj);
    }
  else
    {
      loaded = false;
    }
}

int subModelGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")             //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  sub = dynamic_cast<gridSubModel *> (gdO);
  cobj = gdO;
  if (sub == nullptr)
    {
      ret = NOT_LOADED;
      loaded = false;
      return ret;
    }

  field = convertToLowerCase (fld);
  offset = sub->findIndex (field, cLocalbSolverMode);
  if (offset == kInvalidLocation)
    {
      ret = gridGrabber::setInfo (fld, gdO);
    }
  inputUnits = defUnit;
  if (ret == LOADED)
    {
      loaded = true;
    }
  makeDescription ();
  return ret;
}

int subModelGrabber::setInfo (index_t nOffset, gridCoreObject *gdO)
{
  int ret = LOADED;
  sub = dynamic_cast<gridSubModel *> (gdO);
  cobj = gdO;
  if (sub == nullptr)
    {
      ret = NOT_LOADED;
      loaded = false;
      return ret;
    }
  if (offset < sub->stateSize (cLocalSolverMode))
    {
      offset = nOffset;
    }
  else
    {
      loaded = false;
      ret = NOT_LOADED;
    }
  if (ret == LOADED)
    {
      loaded = true;
    }
  desc = sub->getName () + ':' + std::to_string (nOffset);
  return ret;
}

double subModelGrabber::grabData ()
{
  double val;
  if (loaded)
    {
      if (offset != kInvalidLocation)
        {
          if (offset == kNullLocation)
            {
              offset = sub->findIndex (field, cLocalbSolverMode);
            }
          if (offset != kNullLocation)
            {
              val = sub->getState (offset);
            }
          else
            {
              val = kNullVal;
            }
          //val = val * gain + bias;
          val = std::fma (val, gain, bias);
        }
      else
        {
          val = gridGrabber::grabData ();
        }


    }
  else
    {
      val = kNullVal;
    }
  return val;
}

gridLoadGrabber::gridLoadGrabber (std::string fld, gridLoad *gdL)
{
  setInfo (fld, gdL);
}

std::shared_ptr<gridGrabber> gridLoadGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ) const
{
  std::shared_ptr<gridLoadGrabber> ggb = std::make_shared<gridLoadGrabber> (std::string ("null"), nullptr);
  gridGrabber::clone (nobj, ggb);
  return ggb;
}

void gridLoadGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridLoad *> (obj))
    {
      load = static_cast<gridLoad *> (obj), gridGrabber::updateObject (obj);
    }
  else
    {
      loaded = false;
    }
}

double gridLoadGrabber::grabData ()
{
  double val;
  if (loaded)
    {
      if (offset != kInvalidLocation)
        {
          if (offset == kNullLocation)
            {
              offset = load->findIndex (field, cLocalSolverMode);
            }
          if (offset != kNullLocation)
            {
              val = load->getState (offset);
            }
          else
            {
              val = kNullVal;
            }
          val = std::fma (val, gain, bias);
        }
      else
        {
          val = gridGrabber::grabData ();
        }

    }
  else
    {
      val = kNullVal;
    }
  return val;
}


int gridLoadGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")        //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  field = fld;
  load = dynamic_cast<gridLoad *> (gdO);
  cobj = gdO;
  makeLowerCase (fld);
  if ((field == "power")|| (field == "p"))
    {
      //dgptr = &gridLoad::getRealPower;
      fptr = [ = ](){
          return load->getRealPower ();
        };
      inputUnits = puMW;
    }
  else if ((field == "q") || (field == "mvar"))
    {
      //dgptr = &gridLoad::getReactivePower;
      fptr = [ = ](){
          return load->getReactivePower ();
        };
      inputUnits = puMW;
    }
  else
    {
      offset = load->findIndex (field, cLocalSolverMode);
      if (offset == kInvalidLocation)
        {
          ret = gridGrabber::setInfo (fld, gdO);
        }
      inputUnits = defUnit;
    }

  if (ret == LOADED)
    {
      loaded = true;
    }
  makeDescription ();
  return ret;
}


gridLinkGrabber::gridLinkGrabber (std::string fld, gridLink *gdL)
{
  setInfo (fld, gdL);
}

std::shared_ptr<gridGrabber> gridLinkGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ) const
{
  std::shared_ptr<gridLinkGrabber> ggb = std::make_shared<gridLinkGrabber> (std::string ("null"), nullptr);
  gridGrabber::clone (nobj, ggb);
  return ggb;
}

void gridLinkGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridLink *> (obj))
    {
      link = static_cast<gridLink *> (obj), gridGrabber::updateObject (obj);
    }
  else
    {
      loaded = false;
    }
}

int gridLinkGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")        //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  int ret = LOADED;
  field = fld;
  link = dynamic_cast<gridLink *> (gdO);
  cobj = gdO;
  makeLowerCase (fld);
  int num = trailingStringInt (fld, field,1);
  if (field == "angle")
    {
      //dgptr = &gridLink::getAngle;
      fptr = [ = ](){
          return link->getAngle ();
        };
      inputUnits = rad;
    }
  else if ((field == "power") || (field == "p")||(field == "realpower"))
    {
      //dgptr = &gridLink::getAngle;

      fptr = [ = ](){
          return link->getRealPower (num);
        };

      inputUnits = puMW;
    }
  else if ((field == "q")||(field == "reactivepower"))
    {
      //dgptr = &gridLink::getAngle;

      fptr = [ = ](){
          return link->getReactivePower (num);
        };
      inputUnits = puMW;
    }
  else if ((field == "impedance") ||(field == "z"))
    {
      fptr = [ = ](){
          return link->getTotalImpedance (num);
        };
      inputUnits = puOhm;
    }
  else if ((field == "admittance") || (field == "y"))
    {
      fptr = [ = ](){
          return 1.0 / link->getTotalImpedance (num);
        };
      inputUnits = puMW;
    }
  else if ((field == "switch") || (field == "breaker"))
    {

      fptr = [ = ]() {
          return static_cast<double> (link->switchTest (num));
        };

    }
  else if (field == "connected")
    {

      fptr = [ = ]() {
          return static_cast<double> (link->isConnected ());
        };

    }
  else if (field == "attached")
    {

      fptr = [&]() {
          return static_cast<double> (((!link->checkFlag (gridLink::switch1_open_flag))|| (!link->checkFlag (gridLink::switch2_open_flag)))&&(link->enabled));
        };

    }
  else if ((field == "realimpedance") || (field == "r"))
    {
      fptr = [ = ](){
          return link->getRealImpedance (num);
        };
      inputUnits = puOhm;
    }
  else if ((field == "imagimpedance") || (field == "x"))
    {

      fptr = [ = ](){
          return link->getImagImpedance (num);
        };
      inputUnits = puOhm;
    }
  else if ((field == "current") || (field == "i"))
    {
      fptr = [ = ](){
          return link->getCurrent (num);
        };

      inputUnits = puA;
    }
  else if (field == "realcurrent")
    {

      fptr = [ = ](){
          return link->getRealCurrent (num);
        };
      inputUnits = puA;
    }
  else if (field == "voltage")
    {

      fptr = [ = ](){
          return link->getVoltage (num);
        };
      inputUnits = puA;
    }
  else if (field == "imagcurrent")
    {

      fptr = [ = ](){
          return link->getImagCurrent (num);
        };

      inputUnits = puA;
    }
  else if ((field == "loss") || (field == "lossreal"))
    {
      //dgptr = &gridLink::getLoss;
      fptr = [ = ](){
          return link->getLoss ();
        };
      inputUnits = puMW;
    }
  else if (field == "lossreactive")
    {
      //dgptr = &gridLink::getLoss;
      fptr = [ = ](){
          return link->getReactiveLoss ();
        };
      inputUnits = puMW;
    }
  else
    {
      ret = gridGrabber::setInfo (fld, gdO);
    }

  if (ret == LOADED)
    {
      loaded = true;
    }
  makeDescription ();
  return ret;
}

gridRelayGrabber::gridRelayGrabber (std::string fld, gridRelay *gdR)
{
  setInfo (fld, gdR);
}

std::shared_ptr<gridGrabber> gridRelayGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber>) const
{
  std::shared_ptr<gridRelayGrabber> ggb = std::make_shared<gridRelayGrabber> (std::string ("null"), nullptr);
  gridGrabber::clone (nobj, ggb);
  return ggb;
}

void gridRelayGrabber::updateObject (gridCoreObject *obj)
{
  if (dynamic_cast<gridLink *> (obj))
    {
      rel = static_cast<gridRelay *> (obj), gridGrabber::updateObject (obj);
    }
  else
    {
      loaded = false;
    }
}

int gridRelayGrabber::setInfo (std::string fld, gridCoreObject *gdO)
{
  if (fld == "null")       //this is an escape hatch for the clone function
    {
      loaded = false;
      return FUNCTION_EXECUTION_SUCCESS;
    }
  int ret = FUNCTION_EXECUTION_SUCCESS;
  field = fld;
  rel = dynamic_cast<gridRelay *> (gdO);
  if (!(rel))
    {
      return FUNCTION_EXECUTION_FAILURE;
    }
  cobj = gdO;
  makeLowerCase (fld);
  int num = trailingStringInt (fld, field,0);
  if ((fld == "cv")||(fld == "currentvalue")||(fld == "value")||(fld == "output"))
    {
      //dgptr = &gridLink::getAngle;
      fptr = [ = ](){
          return rel->getOutput (nullptr,cLocalSolverMode,0);
        };
    }
  else if (field == "status")
    {
      fptr = [ = ](){
          return static_cast<double> (rel->getConditionStatus (num - 1));
        };
    }
  else if ((field == "output")||(field == "o"))
    {
      fptr = [ = ](){
          return rel->getOutput (nullptr, cLocalSolverMode,num);
        };
    }
  else if ((field == "block")||(field == "b"))
    {
      if (dynamic_cast<sensor *> (rel))
        {
          fptr = [ = ](){
              return static_cast<sensor *> (rel)->getBlockOutput (nullptr,cLocalSolverMode,num);
            };
        }
      else
        {
          ret = gridGrabber::setInfo (fld, gdO);
        }
    }
  else if ((field == "condition") || (field == "c"))
    {
      fptr = [ = ]() {
          return rel->getConditionValue (num);
        };
    }
  else if ((field == "input")||(field == "i"))
    {
      if (dynamic_cast<sensor *> (rel))
        {
          fptr = [ = ](){
              return static_cast<sensor *> (rel)->getInput (nullptr, cLocalSolverMode, num);
            };
        }
      else
        {
          ret = gridGrabber::setInfo (fld, gdO);
        }
    }
  else
    {
      if (dynamic_cast<sensor *> (rel))
        {
          //try to lookup named output for sensors
          index_t outIndex = static_cast<sensor *> (rel)->lookupOutput (fld);
          if (outIndex != kNullLocation)
            {
              fptr = [ = ]() {
                  return rel->getOutput (nullptr, cLocalSolverMode, outIndex);
                };
            }
          else
            {
              ret = gridGrabber::setInfo (fld, gdO);
            }
        }
      else
        {
          ret = gridGrabber::setInfo (fld, gdO);
        }

    }

  if (ret == FUNCTION_EXECUTION_SUCCESS)
    {
      loaded = true;
    }
  makeDescription ();
  return ret;
}


functionGrabber::functionGrabber ()
{
}

functionGrabber::functionGrabber (std::shared_ptr<gridGrabber> ggb, std::string func)
{
  function_name = func;
  if (ggb)
    {
      bgrabber = ggb;
    }
  if (isFunctionName (func,function_type::arg))
    {
      opptr = get1ArgFunction (func);
      vectorGrab = bgrabber->vectorGrab;
      if (bgrabber->loaded)
        {
          loaded = true;
        }
    }
  else if (isFunctionName (func, function_type::vect_arg))
    {
      opptrV = getArrayFunction (func);
      vectorGrab = false;
      if (bgrabber->loaded)
        {
          loaded = true;
        }
    }
}


int functionGrabber::setInfo (std::string fld, gridCoreObject* obj)
{
  function_name = fld;

  if (obj)
    {
      bgrabber->updateObject (obj);
    }
  if (isFunctionName (function_name, function_type::arg))
    {
      opptr = get1ArgFunction (function_name);
      vectorGrab = bgrabber->vectorGrab;
      if (bgrabber->loaded)
        {
          loaded = true;
        }
    }
  else if (isFunctionName (function_name, function_type::vect_arg))
    {
      opptrV = getArrayFunction (function_name);
      vectorGrab = false;
      if (bgrabber->loaded)
        {
          loaded = true;
        }
    }
  return FUNCTION_EXECUTION_SUCCESS;
}

void functionGrabber::getDesc (std::vector<std::string > &desc_list) const
{
  if (vectorGrab)
    {
      stringVec dA1;
      bgrabber->getDesc (dA1);
      desc_list.resize (dA1.size ());
      for (size_t kk = 0; kk < dA1.size (); ++kk)
        {
          desc_list[kk] = function_name + '(' + dA1[kk] + ')';
        }
    }
  else
    {
      stringVec dA1, dA2;
      bgrabber->getDesc (dA1);
      desc_list.resize (dA1.size ());
      desc_list[0] = function_name + '(' + dA1[0] + ')';
    }
}

std::shared_ptr<gridGrabber> functionGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ggb) const
{

  std::shared_ptr<functionGrabber> fgb;
  if (ggb == nullptr)
    {
      fgb = std::make_shared<functionGrabber> ();
    }
  else
    {
      if (std::dynamic_pointer_cast<functionGrabber> (ggb))
        {
          fgb = std::dynamic_pointer_cast<functionGrabber> (ggb);
        }
      else
        {
          return gridGrabber::clone (nobj, ggb);
        }
    }
  fgb->bgrabber = bgrabber->clone (nobj, nullptr);
  fgb->function_name = function_name;
  fgb->opptr = opptr;
  fgb->opptrV = opptrV;
  gridGrabber::clone (nobj, fgb);
  return fgb;
}

double functionGrabber::grabData ()
{
  double val;
  double temp;
  if (bgrabber->vectorGrab)
    {
      bgrabber->grabData (tempArray);
      val = opptrV (tempArray);
    }
  else
    {
      temp = bgrabber->grabData ();
      val = opptr (temp);
    }

  val = std::fma (val, gain, bias);
  return val;
}

void functionGrabber::grabData (std::vector<double> &vdata)
{
  if (bgrabber->vectorGrab)
    {
      bgrabber->grabData (tempArray);
    }
  std::transform (tempArray.begin (),tempArray.end (),vdata.begin (),opptr);
}



void functionGrabber::updateObject (gridCoreObject *obj)
{
  if (bgrabber)
    {
      bgrabber->updateObject (obj);
    }
}

gridCoreObject *functionGrabber::getObject () const
{
  if (bgrabber)
    {
      return bgrabber->getObject ();
    }
  else
    {
      return nullptr;
    }
}

//operatorGrabber
opGrabber::opGrabber ()
{
}

opGrabber::opGrabber (std::shared_ptr<gridGrabber> ggb1, std::shared_ptr<gridGrabber> ggb2, std::string op)
{
  op_name = op;
  if (ggb1)
    {
      bgrabber1 = ggb1;
    }
  if (ggb2)
    {
      bgrabber2 = ggb2;
    }
  if (isFunctionName (op, function_type::arg2))
    {
      opptr = get2ArgFunction (op);
      vectorGrab = bgrabber1->vectorGrab;
      if ((bgrabber1->loaded)&&(bgrabber2->loaded))
        {
          loaded = true;
        }
    }
  else if (isFunctionName (op, function_type::vect_arg2))
    {
      opptrV = get2ArrayFunction (op);
      vectorGrab = false;
      if ((bgrabber1->loaded) && (bgrabber2->loaded))
        {
          loaded = true;
        }
    }
}


int opGrabber::setInfo (std::string fld, gridCoreObject* obj)
{
  op_name = fld;

  if (obj)
    {
      bgrabber1->updateObject (obj);
      bgrabber2->updateObject (obj);
    }
  if (isFunctionName (op_name, function_type::arg2))
    {
      opptr = get2ArgFunction (op_name);
      vectorGrab = bgrabber1->vectorGrab;
      if ((bgrabber1->loaded) && (bgrabber2->loaded))
        {
          loaded = true;
        }
    }
  else if (isFunctionName (op_name, function_type::vect_arg2))
    {
      opptrV = get2ArrayFunction (op_name);
      vectorGrab = false;
      if ((bgrabber1->loaded) && (bgrabber2->loaded))
        {
          loaded = true;
        }
    }
  return LOADED;
}

void opGrabber::getDesc (stringVec &desc_list) const
{
  if (vectorGrab)
    {
      stringVec dA1, dA2;
      bgrabber1->getDesc (dA1);
      bgrabber2->getDesc (dA2);
      desc_list.resize (dA1.size ());
      for (size_t kk = 0; kk < dA1.size (); ++kk)
        {
          desc_list[kk] = dA1[kk] + op_name + dA2[kk];
        }
    }
  else
    {
      stringVec dA1, dA2;
      bgrabber1->getDesc (dA1);
      bgrabber2->getDesc (dA2);
      desc_list.resize (dA1.size ());
      desc_list[0] = dA1[0] + op_name + dA2[0];
    }
}

std::shared_ptr<gridGrabber> opGrabber::clone (gridCoreObject *nobj, std::shared_ptr<gridGrabber> ggb) const
{

  std::shared_ptr<opGrabber> ogb;
  if (ggb == nullptr)
    {
      ogb = std::make_shared<opGrabber> ();
    }
  else
    {
      if (std::dynamic_pointer_cast<opGrabber> (ggb))
        {
          ogb = std::dynamic_pointer_cast<opGrabber> (ggb);
        }
      else
        {
          return gridGrabber::clone (nobj,ggb);
        }
    }
  ogb->bgrabber1 = bgrabber1->clone (nobj,nullptr);
  ogb->bgrabber2 = bgrabber2->clone (nobj,nullptr);
  ogb->op_name = op_name;
  ogb->opptr = opptr;
  ogb->opptrV = opptrV;
  gridGrabber::clone (nobj,ogb);
  return ogb;
}

double opGrabber::grabData ()
{
  double val;
  double temp1,temp2;
  if (bgrabber1->vectorGrab)
    {
      bgrabber1->grabData (tempArray1);
      bgrabber2->grabData (tempArray2);
      val = opptrV (tempArray1,tempArray2);
    }
  else
    {
      temp1 = bgrabber1->grabData ();
      temp2 = bgrabber2->grabData ();
      val = opptr (temp1,temp2);
    }
  val = std::fma (val, gain, bias);
  return val;
}

void opGrabber::grabData (std::vector<double> &vdata)
{
  if (bgrabber1->vectorGrab)
    {
      vdata.resize (tempArray1.size ());
      bgrabber1->grabData (tempArray1);
      bgrabber2->grabData (tempArray2);
      std::transform (tempArray1.begin (),tempArray1.end (),tempArray2.begin (),vdata.begin (),opptr);
    }

}


void opGrabber::updateObject (gridCoreObject *obj)
{
  if (bgrabber1)
    {
      bgrabber1->updateObject (obj);
    }
  if (bgrabber2)
    {
      bgrabber2->updateObject (obj);
    }
}

void opGrabber::updateObject (gridCoreObject *obj,int num)
{
  if (num == 1)
    {
      if (bgrabber1)
        {
          bgrabber1->updateObject (obj);
        }
    }
  else if (num == 2)
    {
      if (bgrabber2)
        {
          bgrabber2->updateObject (obj);
        }
    }

}

gridCoreObject *opGrabber::getObject () const
{
  if (bgrabber1)
    {
      return bgrabber1->getObject ();
    }
  else
    {
      return nullptr;
    }
}
