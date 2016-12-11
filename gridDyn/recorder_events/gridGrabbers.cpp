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
#include "objectGrabbers.h"
#include "core/helperTemplates.h"
#include "core/gridDynExceptions.h"
#include <cmath>
#include <algorithm>
#include <map>

using namespace gridUnits;
gridGrabber::gridGrabber(const std::string &fld)
{
	gridGrabber::updateField(fld);
}

gridGrabber::gridGrabber(const std::string &fld, coreObject *obj)
{
	gridGrabber::updateObject(obj);
	gridGrabber::updateField(fld);
}

std::shared_ptr<gridGrabber> gridGrabber::clone (std::shared_ptr<gridGrabber> ggb) const
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
  ggb->useVoltage = useVoltage;
  ggb->cobj = cobj;
  return ggb;
}

/* *INDENT-OFF* */
static const std::map<std::string, std::function<double(coreObject *)>> coreFunctions 
{
{ "nextupdatetime", [](coreObject *obj){return obj->getNextUpdateTime(); } },
{ "lastupdatetime", [](coreObject *obj) {return obj->get("lastupdatetime"); } },
{"constant", [](coreObject *) {return 0.0; } },
};

/* *INDENT-OFF* */

int gridGrabber::updateField (std::string fld)
{
  if (fld == "null")        //this is an escape hatch for the clone function
    {
      loaded = false;
      return 0;
    }
  field = fld;
  auto fnd = coreFunctions.find(field);
  if (fnd != coreFunctions.end())
  {
	  fptr = fnd->second;

  }

  loaded = checkIfLoaded();
  if (loaded)
  {
	  makeDescription();
	  return LOADED;
  }
  else
  {
	  if (!customDesc)
	  {
		  desc = "";
	  }
	  
	  return NOT_LOADED;
  }
 
}

std::string gridGrabber::getDesc()
{
	if (desc.empty()&&loaded)
	{
		makeDescription();
	}
	return desc;
}

void gridGrabber::getDesc (std::vector<std::string > &desc_list) const
{
  if (vectorGrab)
    {
      fptrN (cobj,desc_list);
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
	if (!loaded)
	{
		return kNullVal;
	}
	double val;
	if (fptr)
	{
		val = fptr(cobj);
		if (outputUnits != defUnit)
		{
			val = unitConversion(val, inputUnits, outputUnits, cobj->get("basepower"), m_baseVoltage);
		}
	}
	else
	{
		val = cobj->get(field, outputUnits);
	}
	//val = val * gain + bias;
	val = std::fma(val, gain, bias);
	return val;
}

void gridGrabber::grabData (std::vector<double> &vals)
{
  if (loaded)
    {
      fptrV (cobj,vals);
      if (outputUnits != defUnit)
        {
          for (auto &v : vals)
            {
              v = unitConversion (v, inputUnits, outputUnits, cobj->get("basepower"),m_baseVoltage);
            }
        }
    }
  else
    {
      vals.resize (0);
    }
}

void gridGrabber::updateObject (coreObject *obj,object_update_mode mode)
{
  if (obj)
    {
	  if (mode == object_update_mode::direct)
	  {
		  cobj = obj;
	  }
	  else
	  {
		  cobj = findMatchingObject(cobj, obj);
		  if (!cobj)
		  {
			  throw(objectUpdateFailException());
		  }
	  }
	  if (cobj)
	  {
		  if (useVoltage)
		  {
			  m_baseVoltage = cobj->get("basevoltage");
			  makeDescription();
		  }
	  }
    }
  else
  {
	  cobj = obj;
  }
  loaded = checkIfLoaded();
  if (loaded)
  {
	  makeDescription();
  }
  else if (!customDesc)
  {
	  desc = "";
  }
}

void gridGrabber::makeDescription ()
{
	if (!customDesc)
	{
		desc = (cobj) ? (cobj->getName() + ':' + field) : field;

		if (outputUnits != defUnit)
		{
			desc += '(' + to_string(outputUnits) + ')';
		}
	}
  
}

coreObject * gridGrabber::getObject() const
{
	return cobj;
}

void gridGrabber::getObjects(std::vector<coreObject *> &objects) const
{
	objects.push_back(getObject());
}

bool gridGrabber::checkIfLoaded()
{
	if (cobj)
	{
		if ((fptr) || (fptrV))
		{
			return true;
		}
		else if (!field.empty())
		{
			try
			{
				double testval = cobj->get(field);
				if (testval != kNullVal)
				{
					return true;
				}
			}
			catch (const unrecognizedParameter &)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
			
	}
	else if (field == "constant")
	{
		return true;
	}
	return false;
}

std::shared_ptr<gridGrabber> createGrabber (const std::string &fld, coreObject *obj)
{
  std::shared_ptr<gridGrabber> ggb = nullptr;

  gridBus *bus = dynamic_cast<gridBus *> (obj);
  if (bus)
    {
      ggb = std::make_shared<objectGrabber<gridBus>> (fld, bus);
      return ggb;
    }

  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      ggb = std::make_shared<objectOffsetGrabber<gridLoad>> (fld, ld);
      return ggb;
    }

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {

      ggb = std::make_shared<objectOffsetGrabber<gridDynGenerator>> (fld, gen);
      return ggb;
    }

  gridLink *lnk = dynamic_cast<gridLink *> (obj);
  if (lnk)
    {
      ggb = std::make_shared<objectGrabber<gridLink>> (fld, lnk);
      return ggb;
    }

  gridArea *area = dynamic_cast<gridArea *> (obj);
  if (area)
    {
      ggb = std::make_shared<objectGrabber<gridArea>> (fld, area);
      return ggb;
    }

  gridRelay *rel = dynamic_cast<gridRelay *> (obj);
  if (rel)
    {
      ggb = std::make_shared<objectGrabber<gridRelay>> (fld, rel);
      return ggb;
    }

  gridSubModel *sub = dynamic_cast<gridSubModel *> (obj);
  if (sub)
    {
      ggb = std::make_shared<objectOffsetGrabber<gridSubModel>> (fld, sub);
      return ggb;
    }
  return ggb;

}

std::shared_ptr<gridGrabber> createGrabber (int noffset, coreObject *obj)
{
  std::shared_ptr<gridGrabber> ggb = nullptr;

  gridDynGenerator *gen = dynamic_cast<gridDynGenerator *> (obj);
  if (gen)
    {
      ggb = std::make_shared<objectOffsetGrabber<gridDynGenerator>> (noffset, gen);
      return ggb;
    }
  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
  {
	  ggb = std::make_shared<objectOffsetGrabber<gridLoad>>(noffset, ld);
	  return ggb;
  }
  return ggb;

}


void customGrabber::setGrabberFunction (std::string fld, std::function<double (coreObject *)> nfptr)
{
  fptr = nfptr;
  loaded = true;
  vectorGrab = false;
  field = fld;
}

void customGrabber::setGrabberFunction(std::function<void(coreObject *, std::vector<double> &)> nVptr)
{
	vectorGrab = true;
	fptrV = nVptr;
	loaded = true;
}

bool customGrabber::checkIfLoaded()
{
	if ((fptr) || (fptrV))
	{
		return true;
	}
	else
	{
		return false;
	}
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


int functionGrabber::updateField (std::string fld)
{
  function_name = fld;

  if (isFunctionName (function_name, function_type::arg))
    {
      opptr = get1ArgFunction (function_name);
      vectorGrab = bgrabber->vectorGrab;
    }
  else if (isFunctionName (function_name, function_type::vect_arg))
    {
      opptrV = getArrayFunction (function_name);
      vectorGrab = false;
    }
  loaded = checkIfLoaded();
  if (loaded)
  {
	  makeDescription();
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

std::shared_ptr<gridGrabber> functionGrabber::clone (std::shared_ptr<gridGrabber> ggb) const
{

	std::shared_ptr<functionGrabber> fgb = cloneBase<functionGrabber, gridGrabber>(this, ggb);
  if (!fgb)
    {
	  return ggb;
    }
  fgb->bgrabber = bgrabber->clone ();
  fgb->function_name = function_name;
  fgb->opptr = opptr;
  fgb->opptrV = opptrV;
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



void functionGrabber::updateObject (coreObject *obj, object_update_mode mode)
{
  if (bgrabber)
    {
      bgrabber->updateObject (obj,mode);
    }
  loaded = checkIfLoaded();
  if (loaded)
  {
	  makeDescription();
  }
}

bool functionGrabber::checkIfLoaded()
{
	if (bgrabber->loaded)
	{
		return true;
	}
	else
	{
		return false;
	}
}

coreObject *functionGrabber::getObject () const
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


void functionGrabber::getObjects(std::vector<coreObject *> &objects) const
{
	if (bgrabber)
	{
		bgrabber->getObjects(objects);
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
     
    }
  loaded = opGrabber::checkIfLoaded();
  if (loaded)
  {
	  opGrabber::makeDescription();
  }
}


int opGrabber::updateField (std::string fld)
{
  op_name = fld;

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

bool opGrabber::checkIfLoaded()
{
	if ((bgrabber1->loaded) && (bgrabber2->loaded))
	{
		return true;
	}

	return false;
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

std::shared_ptr<gridGrabber> opGrabber::clone (std::shared_ptr<gridGrabber> ggb) const
{

	std::shared_ptr<opGrabber> ogb = cloneBase<opGrabber, gridGrabber>(this, ggb);
  if (!ogb)
    {
	  return ggb;
    }
  
  ogb->bgrabber1 = bgrabber1->clone (nullptr);
  ogb->bgrabber2 = bgrabber2->clone (nullptr);
  ogb->op_name = op_name;
  ogb->opptr = opptr;
  ogb->opptrV = opptrV;
  return ogb;
}

double opGrabber::grabData ()
{
  double val;
  if (bgrabber1->vectorGrab)
    {
      bgrabber1->grabData (tempArray1);
      bgrabber2->grabData (tempArray2);
      val = opptrV (tempArray1,tempArray2);
    }
  else
    {
      double temp1 = bgrabber1->grabData ();
      double temp2 = bgrabber2->grabData ();
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


void opGrabber::updateObject (coreObject *obj, object_update_mode mode)
{
  if (bgrabber1)
    {
      bgrabber1->updateObject (obj,mode);
    }
  if (bgrabber2)
    {
      bgrabber2->updateObject (obj,mode);
    }
}

void opGrabber::updateObject (coreObject *obj,int num)
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

coreObject *opGrabber::getObject () const
{
  if (bgrabber1)
    {
      return bgrabber1->getObject ();
    }
    return nullptr;
}


void opGrabber::getObjects(std::vector<coreObject *> &objects) const
{
	if (bgrabber1)
	{
		bgrabber1->getObjects(objects);
	}
	if (bgrabber2)
	{
		bgrabber2->getObjects(objects);
	}
}