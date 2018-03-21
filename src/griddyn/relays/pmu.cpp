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

#include "pmu.h"
#include "core/coreObjectTemplates.hpp"

#include "../Link.h"
#include "../blocks/delayBlock.h"
#include "../blocks/filteredDerivativeBlock.h"
#include "core/coreExceptions.h"
#include "../events/Event.h"
#include "../gridBus.h"
#include "../measurement/Condition.h"
#include "../measurement/grabberSet.h"
#include "../measurement/gridGrabbers.h"
#include "../comms/controlMessage.h"
#include "../comms/Communicator.h"
#include <cmath>

namespace griddyn
{
namespace relays
{
pmu::pmu (const std::string &objName) : sensor (objName)
{
    outputStrings = {{"voltage"}, {"angle"}, {"frequency"}, {"rocof"}};
}

coreObject *pmu::clone (coreObject *obj) const
{
    auto nobj = cloneBase<pmu, sensor> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }

    nobj->Tv = Tv;
    nobj->Ttheta = Ttheta;
	nobj->Trocof = Trocof;
    nobj->Tcurrent = Tcurrent;
    nobj->transmissionPeriod = transmissionPeriod;
    nobj->sampleRate = sampleRate;
    return nobj;
}

void pmu::setFlag (const std::string &flag, bool val)
{
    if ((flag == "transmit") || (flag == "transmitactive") || (flag == "transmit_active"))
    {
        opFlags.set (transmit_active, val);
    }
    else if ((flag == "three_phase") || (flag == "3phase") || (flag == "three_phase_active"))
    {
        opFlags.set (three_phase_capable, val);
    }
    else if ((flag == "current_active") || (flag == "current"))
    {
        opFlags.set (current_active, val);
    }
    else
    {
        sensor::setFlag (flag, val);
    }
}

void pmu::set (const std::string &param, const std::string &val)
{
    if (param.empty ())
    {
    }
    else
    {
        sensor::set (param, val);
    }
}

using namespace gridUnits;

void pmu::set (const std::string &param, double val, units_t unitType)
{
    if ((param == "tv") || (param == "voltagedelay"))
    {
        Tv = val;
		if (opFlags[dyn_initialized])
		{

		}
    }
    else if ((param == "ttheta") || (param == "tangle") || (param == "angledelay"))
    {
        Ttheta = val;
		if (opFlags[dyn_initialized])
		{

		}
    }
	else if (param == "trocof")
	{
		Trocof = val;
		if (opFlags[dyn_initialized])
		{

		}
	}
	else if ((param == "tcurrent") || (param == "tI") || (param == "currentdelay"))
	{
		Tcurrent=val;
		if (opFlags[dyn_initialized])
		{

		}
	}
    else if ((param == "transmitrate") || (param == "rate"))
    {
		transmissionPeriod = (val >= kMin_Res) ? 1.0 / val : kBigNum;
		
    }
	else if ((param == "transmitperiod") || (param == "period"))
	{
		transmissionPeriod = unitConversionTime(val,unitType,gridUnits::sec);

	}
    else if (param == "samplerate")
    {
        sampleRate = val;
    }
    else
    {
        sensor::set (param, val, unitType);
    }
}

double pmu::get (const std::string &param, gridUnits::units_t unitType) const
{
    if ((param == "tv") || (param == "voltagedelay"))
    {
        return Tv;
    }
    if ((param == "ttheta") || (param == "tangle") || (param == "angledelay"))
    {
        return Ttheta;
    }
    if ((param == "tcurrent") || (param == "tI") || (param == "currentdelay"))
    {
        return Tcurrent;
    }
	if (param == "trocof")
	{
		return Trocof;
	}
    if ((param == "transmitrate") || (param == "rate"))
    {
        return 1.0/transmissionPeriod;
    }
	if (param == "transmitperiod")
	{
		return transmissionPeriod;
	}
    if (param == "samplerate")
    {
        return sampleRate;
    }
    return sensor::get (param, unitType);
}

void pmu::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    if (m_sourceObject == nullptr)
    {
        // we know the parent is most likely an area so find the corresponding bus that matches the user ID
        if (getUserID () != kNullLocation)
        {
            m_sourceObject = getParent ()->getSubObject ("bus", getUserID ());
        }
    }
    // if the first check failed just try to find a bus
    if (m_sourceObject == nullptr)
    {
        m_sourceObject = getParent ()->find ("bus");
    }
    if (m_sourceObject == nullptr)
    {
        LOG_WARNING ("no pmu target specified");
        disable ();
        return;
    }
    // check for 3 phase sensors
    if (dynamic_cast<gridComponent *> (m_sourceObject))
    {
        if (static_cast<gridComponent *> (m_sourceObject)->checkFlag (three_phase_capable))
        {
            if (!opFlags[three_phase_set])
            {
                opFlags[three_phase_active] = true;
            }
        }
        else
        {
            opFlags[three_phase_active] = false;
        }
    }

    if (dynamic_cast<gridBus *> (m_sourceObject) != nullptr)
    {
        // no way to get current from a bus
        opFlags[current_active] = false;
    }
    generateOutputNames ();
    createFilterBlocks ();
    return sensor::dynObjectInitializeA (time0, flags);
}

void pmu::generateOutputNames ()
{
    // 4 different scenarios
    if (opFlags[three_phase_active])
    {
        if (opFlags[current_active])
        {
            // three phase voltage and current
            outputStrings = {{"voltageA"},      {"angleA"},        {"voltageB"},      {"angleB"},
                             {"voltageC"},      {"angleC"},        {"current_realA"}, {"current_imagA"},
                             {"current_realB"}, {"current_imagB"}, {"current_realC"}, {"current_imagC"},
                             {"frequency"},     {"rocof"}};
        }
        else
        {
            // three phase voltage
            outputStrings = {{"voltageA"}, {"angleA"}, {"voltageB"},  {"angleB"},
                             {"voltageC"}, {"angleC"}, {"frequency"}, {"rocof"}};
        }
    }
    else
    {
        if (opFlags[current_active])
        {
            // single phase voltage and current
            outputStrings = {{"voltage"}, {"angle"}, {"current_real"}, {"current_imag"}, {"frequency"}, {"rocof"}};
        }
        else
        {
            // single phase voltage
            outputStrings = {{"voltage","v"}, {"angle","ang","theta"}, {"frequency","freq","f"}, {"rocof"}};
        }
    }
}
/** generate the filter blocks and inputs for the sensor object*/
void pmu::createFilterBlocks ()
{
    // 4 different scenarios
    if (opFlags[three_phase_active])
    {
        if (opFlags[current_active])
        {
            // three phase voltage and current
        }
        else
        {
            // three phase voltage
        }
    }
    else
    {
        auto B = new blocks::delayBlock (Tv);
        B->setName ("voltage");
        add (B);
        B = new blocks::delayBlock (Ttheta);
        B->setName ("angle");
        add (B);
        set ("input0", "voltage");
        set ("input1", "angle");
        set ("blockinput0", 0);
        set ("blockinput1", 1);
        setupOutput (0, "block0");
        setupOutput (1, "block1");
        if (opFlags[current_active])
        {
            B = new blocks::delayBlock (Tcurrent);
            B->setName ("current_real");
            add (B);
            B = new blocks::delayBlock (Tcurrent);
            B->setName ("current_reactive");
            add (B);
            set ("input2", "current_real");
            set ("input3", "current_reactive");
            set ("blockinput2", 2);
            set ("blockinput3", 3);
            setupOutput (2, "block2");
            setupOutput (3, "block3");
        }
        auto fblock = new blocks::filteredDerivativeBlock ("freq");
        fblock->set ("t1", Ttheta);
		fblock->set("t2", Trocof);
        add (fblock);
        set ("blockinput" + std::to_string (fblock->locIndex), 1);
        setupOutput (fblock->locIndex, "block" + std::to_string (fblock->locIndex));
        setupOutput (fblock->locIndex + 1, "blockderiv" + std::to_string (fblock->locIndex));
    }
}

void pmu::updateA (coreTime time)
{
	sensor::updateA(time);
    if (time >= nextTransmitTime)
    {
		generateAndTransmitMessage();
		nextTransmitTime = lastTransmitTime + transmissionPeriod;
		if (nextTransmitTime <= time)
		{
			nextTransmitTime = time + transmissionPeriod;
		}
    }

}

coreTime pmu::updateB()
{
	sensor::updateB();
	if (nextUpdateTime > nextTransmitTime)
	{
		nextUpdateTime = nextTransmitTime;
	}
	return nextUpdateTime;
}

void pmu::generateAndTransmitMessage() const
{
	if (opFlags[use_commLink])
	{
		auto &oname = outputNames();

		auto cm = std::make_shared<comms::controlMessage>(comms::controlMessage::GET_RESULT_MULTIPLE);

		auto res = getOutputs(noInputs, emptyStateData, cLocalSolverMode);


		cm->multiFields.resize(res.size());
		cm->multiValues.resize(res.size());
		cm->multiUnits.resize(res.size());
		cm->m_time = prevTime;
		for (index_t ii = 0; ii < static_cast<index_t>(res.size()); ++ii)
		{
			cm->multiFields[ii] = oname[ii][0];
			cm->multiValues[ii] = res[ii];
			cm->multiUnits[ii] = gridUnits::to_string(outputUnits(ii));
		}
		
		cManager.send(std::move(cm));
		
	}

}

}  // namespace relays
}  // namespace griddyn