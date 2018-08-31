/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "gridGrabbers.h"
#include "../Area.h"
#include "../Generator.h"
#include "../Link.h"
#include "../Load.h"
#include "../Relay.h"
#include "core/coreExceptions.h"

#include "../gridBus.h"
#include "../gridSubModel.h"
#include "../relays/sensor.h"
#include "../simulation/gridSimulation.h"
#include "grabberInterpreter.hpp"
#include "objectGrabbers.h"
#include "utilities/functionInterpreter.h"
#include "utilities/vectorOps.hpp"
#include <algorithm>
#include <cmath>
#include <map>

namespace griddyn
{
using namespace gridUnits;
gridGrabber::gridGrabber (const std::string &fld) { gridGrabber::updateField (fld); }
gridGrabber::gridGrabber (const std::string &fld, coreObject *obj)
{
    gridGrabber::updateObject (obj);
    gridGrabber::updateField (fld);
}

std::unique_ptr<gridGrabber> gridGrabber::clone () const
{
    auto ggb = std::make_unique<gridGrabber> ();
    gridGrabber::cloneTo (ggb.get ());
    return ggb;
}

void gridGrabber::cloneTo (gridGrabber *ggb) const
{
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
    ggb->cobj = cobj;
}

static const std::map<std::string, std::function<double(coreObject *)>> coreFunctions{
  {"nextupdatetime", [](coreObject *obj) { return obj->getNextUpdateTime (); }},
  {"lastupdatetime", [](coreObject *obj) { return obj->get ("lastupdatetime"); }},
  {"currenttime", [](coreObject *obj) { return obj->currentTime (); }},
  {"constant", [](coreObject * /*obj*/) { return 0.0; }},
};

void gridGrabber::updateField (const std::string &fld)
{
    if (fld == "null")  // this is an escape hatch for the clone function
    {
        loaded = false;
        return;
    }
    field = fld;
    auto fnd = coreFunctions.find (field);
    if (fnd != coreFunctions.end ())
    {
        fptr = fnd->second;
    }

    loaded = checkIfLoaded ();
}

const std::string &gridGrabber::getDesc () const
{
    if (desc.empty () && loaded)
    {
        makeDescription ();
    }
    return desc;
}

void gridGrabber::getDesc (std::vector<std::string> &desc_list) const
{
    if (vectorGrab)
    {
        fptrN (cobj, desc_list);
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
        val = fptr (cobj);
        if (outputUnits != defUnit)
        {
            val =
              unitConversion (val, inputUnits, outputUnits, cobj->get ("basepower"), cobj->get ("basevoltage"));
        }
    }
    else
    {
        val = cobj->get (field, outputUnits);
    }
    // val = val * gain + bias;
    val = std::fma (val, gain, bias);
    return val;
}

void gridGrabber::grabVectorData (std::vector<double> &vdata)
{
    if ((loaded) && (vectorGrab))
    {
        fptrV (cobj, vdata);
        if (outputUnits != defUnit)
        {
            auto localBasePower = cobj->get ("basepower");
            auto localBaseVoltage = cobj->get ("basevoltage");
            for (auto &v : vdata)
            {
                v = unitConversion (v, inputUnits, outputUnits, localBasePower, localBaseVoltage);
            }
        }
    }
    else
    {
        vdata.resize (0);
    }
}

coreTime gridGrabber::getTime () const
{
    if (cobj != nullptr)
    {
        return cobj->currentTime ();
    }
    return negTime;
}

void gridGrabber::updateObject (coreObject *obj, object_update_mode mode)
{
    if (obj != nullptr)
    {
        if (mode == object_update_mode::direct)
        {
            cobj = obj;
        }
        else
        {
            cobj = findMatchingObject (cobj, obj);
            if (cobj == nullptr)
            {
                throw (objectUpdateFailException ());
            }
        }
    }
    else
    {
        cobj = obj;
    }
    loaded = checkIfLoaded ();
}

void gridGrabber::makeDescription () const
{
    if (!customDesc)
    {
        desc = (cobj != nullptr) ? (cobj->getName () + ':' + field) : field;

        if (outputUnits != defUnit)
        {
            desc += '(' + to_string (outputUnits) + ')';
        }
    }
}

coreObject *gridGrabber::getObject () const { return cobj; }
void gridGrabber::getObjects (std::vector<coreObject *> &objects) const { objects.push_back (getObject ()); }
bool gridGrabber::checkIfLoaded ()
{
    if (cobj != nullptr)
    {
        if ((fptr) || (fptrV))
        {
            return true;
        }
        if (!field.empty ())
        {
            try
            {
                double testval = cobj->get (field);
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

std::unique_ptr<gridGrabber> createGrabber (const std::string &fld, coreObject *obj)
{
    std::unique_ptr<gridGrabber> ggb = nullptr;

    auto bus = dynamic_cast<gridBus *> (obj);
    if (bus != nullptr)
    {
        ggb = std::make_unique<objectGrabber<gridBus>> (fld, bus);
        return ggb;
    }

    auto ld = dynamic_cast<Load *> (obj);
    if (ld != nullptr)
    {
        ggb = std::make_unique<objectOffsetGrabber<Load>> (fld, ld);
        return ggb;
    }

    auto gen = dynamic_cast<Generator *> (obj);
    if (gen != nullptr)
    {
        ggb = std::make_unique<objectOffsetGrabber<Generator>> (fld, gen);
        return ggb;
    }

    auto lnk = dynamic_cast<Link *> (obj);
    if (lnk != nullptr)
    {
        ggb = std::make_unique<objectGrabber<Link>> (fld, lnk);
        return ggb;
    }

    auto area = dynamic_cast<Area *> (obj);
    if (area != nullptr)
    {
        ggb = std::make_unique<objectGrabber<Area>> (fld, area);
        return ggb;
    }

    auto rel = dynamic_cast<Relay *> (obj);
    if (rel != nullptr)
    {
        ggb = std::make_unique<objectGrabber<Relay>> (fld, rel);
        return ggb;
    }

    auto sub = dynamic_cast<gridSubModel *> (obj);
    if (sub != nullptr)
    {
        ggb = std::make_unique<objectOffsetGrabber<gridSubModel>> (fld, sub);
        return ggb;
    }
    return ggb;
}

std::unique_ptr<gridGrabber> createGrabber (int noffset, coreObject *obj)
{
    std::unique_ptr<gridGrabber> ggb = nullptr;

    auto gen = dynamic_cast<Generator *> (obj);
    if (gen != nullptr)
    {
        ggb = std::make_unique<objectOffsetGrabber<Generator>> (noffset, gen);
        return ggb;
    }
    auto ld = dynamic_cast<Load *> (obj);
    if (ld != nullptr)
    {
        ggb = std::make_unique<objectOffsetGrabber<Load>> (noffset, ld);
        return ggb;
    }
    return ggb;
}

void customGrabber::setGrabberFunction (const std::string &fld, std::function<double(coreObject *)> nfptr)
{
    fptr = std::move (nfptr);
    loaded = true;
    vectorGrab = false;
    field = fld;
}

void customGrabber::setGrabberFunction (std::function<void(coreObject *, std::vector<double> &)> nVptr)
{
    vectorGrab = true;
    fptrV = std::move (nVptr);
    loaded = true;
}

bool customGrabber::checkIfLoaded () { return ((fptr) || (fptrV)); }

functionGrabber::functionGrabber (std::shared_ptr<gridGrabber> ggb, std::string func) : bgrabber (std::move (ggb))
{
    function_name = std::move (func);

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
}

void functionGrabber::updateField (const std::string &fld)
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
    loaded = checkIfLoaded ();
}

void functionGrabber::getDesc (std::vector<std::string> &desc_list) const
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

std::unique_ptr<gridGrabber> functionGrabber::clone () const
{
    std::unique_ptr<gridGrabber> fgrab = std::make_unique<functionGrabber> ();
    functionGrabber::cloneTo (fgrab.get ());
    return fgrab;
}

void functionGrabber::cloneTo (gridGrabber *ggb) const
{
    gridGrabber::cloneTo (ggb);
    auto fgb = dynamic_cast<functionGrabber *> (ggb);

    if (fgb == nullptr)
    {
        return;
    }
    fgb->bgrabber = bgrabber->clone ();
    fgb->function_name = function_name;
    fgb->opptr = opptr;
    fgb->opptrV = opptrV;
}

double functionGrabber::grabData ()
{
    double val;
    if (bgrabber->vectorGrab)
    {
        bgrabber->grabVectorData (tempArray);
        val = opptrV (tempArray);
    }
    else
    {
        double temp = bgrabber->grabData ();
        val = opptr (temp);
    }

    val = std::fma (val, gain, bias);
    return val;
}

void functionGrabber::grabVectorData (std::vector<double> &vdata)
{
    if (bgrabber->vectorGrab)
    {
        bgrabber->grabVectorData (tempArray);
    }
    std::transform (tempArray.begin (), tempArray.end (), vdata.begin (), opptr);
}

coreTime functionGrabber::getTime () const
{
    if (bgrabber)
    {
        return bgrabber->getTime ();
    }
    return negTime;
}

void functionGrabber::updateObject (coreObject *obj, object_update_mode mode)
{
    if (bgrabber)
    {
        bgrabber->updateObject (obj, mode);
    }
    loaded = checkIfLoaded ();
}

bool functionGrabber::checkIfLoaded () { return (bgrabber->loaded); }

coreObject *functionGrabber::getObject () const
{
    if (bgrabber)
    {
        return bgrabber->getObject ();
    }
    return nullptr;
}

void functionGrabber::getObjects (std::vector<coreObject *> &objects) const
{
    if (bgrabber)
    {
        bgrabber->getObjects (objects);
    }
}

// operatorGrabber
opGrabber::opGrabber (std::shared_ptr<gridGrabber> ggb1, std::shared_ptr<gridGrabber> ggb2, std::string op)
    : op_name (std::move (op))
{
    if (ggb1)
    {
        bgrabber1 = std::move (ggb1);
    }
    if (ggb2)
    {
        bgrabber2 = std::move (ggb2);
    }
    if (isFunctionName (op_name, function_type::arg2))
    {
        opptr = get2ArgFunction (op_name);
        vectorGrab = (bgrabber1) ? bgrabber1->vectorGrab : false;
        loaded = opGrabber::checkIfLoaded ();
    }
    else if (isFunctionName (op_name, function_type::vect_arg2))
    {
        opptrV = get2ArrayFunction (op_name);
        vectorGrab = false;
    }
    loaded = opGrabber::checkIfLoaded ();
}

void opGrabber::updateField (const std::string &fld)
{
    op_name = fld;

    if (isFunctionName (op_name, function_type::arg2))
    {
        opptr = get2ArgFunction (op_name);
        vectorGrab = (bgrabber1) ? bgrabber1->vectorGrab : false;
        loaded = opGrabber::checkIfLoaded ();
    }
    else if (isFunctionName (op_name, function_type::vect_arg2))
    {
        opptrV = get2ArrayFunction (op_name);
        vectorGrab = false;
        loaded = opGrabber::checkIfLoaded ();
    }
}

bool opGrabber::checkIfLoaded ()
{
    return (((bgrabber1) && (bgrabber1->loaded)) && ((bgrabber2) && (bgrabber2->loaded)));
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

std::unique_ptr<gridGrabber> opGrabber::clone () const
{
    std::unique_ptr<gridGrabber> ograb = std::make_unique<opGrabber> ();
    opGrabber::cloneTo (ograb.get ());
    return ograb;
}

void opGrabber::cloneTo (gridGrabber *ggb) const
{
    gridGrabber::cloneTo (ggb);
    auto ogb = dynamic_cast<opGrabber *> (ggb);

    if (ogb == nullptr)
    {
        return;
    }
    if (bgrabber1)
    {
        ogb->bgrabber1 = bgrabber1->clone ();
    }
    if (bgrabber2)
    {
        ogb->bgrabber2 = bgrabber2->clone ();
    }
    ogb->op_name = op_name;
    ogb->opptr = opptr;
    ogb->opptrV = opptrV;
}

double opGrabber::grabData ()
{
    double val;
    if (bgrabber1->vectorGrab)
    {
        bgrabber1->grabVectorData (tempArray1);
        bgrabber2->grabVectorData (tempArray2);
        val = opptrV (tempArray1, tempArray2);
    }
    else
    {
        double temp1 = bgrabber1->grabData ();
        double temp2 = bgrabber2->grabData ();
        val = opptr (temp1, temp2);
    }
    val = std::fma (val, gain, bias);
    return val;
}

void opGrabber::grabVectorData (std::vector<double> &vdata)
{
    if (bgrabber1->vectorGrab)
    {
        vdata.resize (tempArray1.size ());
        bgrabber1->grabVectorData (tempArray1);
        bgrabber2->grabVectorData (tempArray2);
        std::transform (tempArray1.begin (), tempArray1.end (), tempArray2.begin (), vdata.begin (), opptr);
    }
}

void opGrabber::updateObject (coreObject *obj, object_update_mode mode)
{
    if (bgrabber1)
    {
        bgrabber1->updateObject (obj, mode);
    }
    if (bgrabber2)
    {
        bgrabber2->updateObject (obj, mode);
    }
}

void opGrabber::updateObject (coreObject *obj, int num)
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

coreTime opGrabber::getTime () const
{
    if (bgrabber1)
    {
        return bgrabber1->getTime ();
    }
    if (bgrabber2)
    {
        return bgrabber2->getTime ();
    }

    return negTime;
}
coreObject *opGrabber::getObject () const
{
    if (bgrabber1)
    {
        return bgrabber1->getObject ();
    }
    return nullptr;
}

void opGrabber::getObjects (std::vector<coreObject *> &objects) const
{
    if (bgrabber1)
    {
        bgrabber1->getObjects (objects);
    }
    if (bgrabber2)
    {
        bgrabber2->getObjects (objects);
    }
}

}  // namespace griddyn
