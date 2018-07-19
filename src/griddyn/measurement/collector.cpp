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

#include "collector.h"
#include "Recorder.h"
#include "core/coreExceptions.h"
#include "core/factoryTemplates.hpp"
#include "core/objectInterpreter.h"
#include "../events/Event.h"
#include "gridGrabbers.h"
#include "stateGrabber.h"
#include "utilities/stringOps.h"
#include "utilities/timeSeries.hpp"
#include <cmath>

namespace griddyn
{
static classFactory<collector> collFac ("collector");

static childClassFactory<Recorder, collector>
  grFac (std::vector<std::string>{"recorder", "rec", "file"}, "recorder");

collector::collector (coreTime time0, coreTime period)
    : timePeriod (period), reqPeriod (period), triggerTime (time0)
{
}

collector::collector (const std::string &collectorName)
    : helperObject (collectorName), timePeriod (1.0), reqPeriod (1.0), triggerTime (timeZero)
{
}

std::unique_ptr<collector> collector::clone() const
{
	auto col = std::make_unique<collector>();
	cloneTo(col.get());
	return col;
}

void collector::cloneTo (collector *col) const
{

	col->reqPeriod = reqPeriod;
	col->timePeriod = timePeriod;
	col->setName (getName ());
	col->startTime = startTime;
	col->stopTime = stopTime;
	col->triggerTime = triggerTime;
	col->lastTriggerTime = lastTriggerTime;
    for (size_t kk = 0; kk < points.size (); ++kk)
    {
        if (kk >= col->points.size ())
        {
            auto ggb = (points[kk].dataGrabber) ? points[kk].dataGrabber->clone () : nullptr;
            auto ggbst = (points[kk].dataGrabberSt) ? points[kk].dataGrabberSt->clone () : nullptr;
			col->points.emplace_back (std::move(ggb), std::move(ggbst), points[kk].column);
        }
        else
        {
			if (col->points[kk].dataGrabber)
			{
				if (points[kk].dataGrabber)
				{
					points[kk].dataGrabber->cloneTo(col->points[kk].dataGrabber.get());
				}
			}
			else if (points[kk].dataGrabber)
			{
				col->points[kk].dataGrabber = points[kk].dataGrabber->clone();
			}

			if (col->points[kk].dataGrabberSt)
			{
				if (points[kk].dataGrabberSt)
				{
					points[kk].dataGrabberSt->cloneTo(col->points[kk].dataGrabberSt.get());
				}
			}
			else if (points[kk].dataGrabberSt)
			{
				col->points[kk].dataGrabberSt = points[kk].dataGrabberSt->clone();
			}

			col->points[kk].column = points[kk].column;
        }
    }
	col->data.resize (data.size ());
}

void collector::updateObject (coreObject *gco, object_update_mode mode)
{
    for (auto gg : points)
    {
        if (gg.dataGrabber)
        {
            gg.dataGrabber->updateObject (gco, mode);
            if (gg.dataGrabber->vectorGrab)
            {
                recheck = true;
            }
        }
        else if (gg.dataGrabberSt)
        {
            gg.dataGrabberSt->updateObject (gco, mode);
        }
    }
}

coreObject *collector::getObject () const
{
    if (!points.empty ())
    {
        if (points[0].dataGrabber)
        {
            return points.front ().dataGrabber->getObject ();
        }
        if (points[0].dataGrabberSt)
        {
            return points.front ().dataGrabberSt->getObject ();
        }
    }
    return nullptr;
}

void collector::getObjects (std::vector<coreObject *> &objects) const
{
    for (auto gg : points)
    {
        if (gg.dataGrabber)
        {
            gg.dataGrabber->getObjects (objects);
        }
        else if (gg.dataGrabberSt)
        {
            gg.dataGrabberSt->getObjects (objects);
        }
    }
}

std::vector<std::string> collector::getColumnDescriptions () const
{
    stringVec res;
    res.resize (data.size ());
    for (auto &datapoint : points)
    {
        if (datapoint.dataGrabber->vectorGrab)
        {
            stringVec vdesc;
            datapoint.dataGrabber->getDesc (vdesc);
            for (size_t kk = 0; kk < vdesc.size (); ++kk)
            {
                if ((datapoint.colname.empty ()) || (!vectorName))
                {
                    res[datapoint.column + kk] = vdesc[kk];
                }
                else
                {
                    res[datapoint.column + kk] = datapoint.colname + "[" + std::to_string (kk) + "]";
                }
            }
        }
        else
        {
            if (datapoint.colname.empty ())
            {
                res[datapoint.column] = datapoint.dataGrabber->getDesc ();
            }
            else
            {
                res[datapoint.column] = datapoint.colname;
            }
        }
    }
    return res;
}

void collector::set (const std::string &param, double val)
{
    if (param == "period")
    {
        reqPeriod = val;
        timePeriod = val;
    }
    else if (param == "frequency")
    {
        reqPeriod = 1.0 / val;
        timePeriod = val;
    }
    else if ((param == "triggertime") || (param == "trigger") || (param == "time"))
    {
        triggerTime = val;
    }
    else if ((param == "starttime") || (param == "start"))
    {
        startTime = val;
        triggerTime = startTime;
    }
    else if ((param == "stoptime") || (param == "stop"))
    {
        stopTime = val;
    }
    else if (param == "period_resolution")
    {
        if (val > 0)
        {
            auto per = static_cast<int> (std::round (reqPeriod / val));
            timePeriod = (per == 0) ? val : val * per;
        }
    }
    else
    {
        helperObject::set (param, val);
    }
}

void collector::set (const std::string &param, const std::string &val)
{
    if (param.front () == '#')
    {
    }
    else
    {
        helperObject::set (param, val);
    }
}

void collector::setFlag (const std::string &flag, bool val)
{
    if (flag == "vector_name")
    {
        vectorName = val;
    }
    else
    {
        helperObject::setFlag (flag, val);
    }
}

void collector::setTime (coreTime time)
{
    if (time > triggerTime)
    {
        triggerTime = time;
    }
}

void collector::recheckColumns ()
{
    fsize_t ct = 0;
    std::vector<double> vals;
    // for (size_t kk = 0; kk < points.size(); ++kk)
    for (auto &pt : points)
    {
        if (pt.column == -1)
        {
            pt.column = ct;
        }

        if (pt.dataGrabber->vectorGrab)
        {
            pt.dataGrabber->grabVectorData (vals);
            ct += static_cast<fsize_t> (vals.size ());
        }
        else
        {
            ++ct;
        }
    }
    data.resize (ct);
    recheck = false;
}

count_t collector::grabData(double *outputData, index_t N)
{
	std::vector<double> vals;
	count_t currentCount = 0;
	if (recheck)
	{
		recheckColumns();
	}
	for (auto &datapoint : points)
	{
		if (datapoint.dataGrabber->vectorGrab)
		{
			datapoint.dataGrabber->grabVectorData(vals);
			if (static_cast<index_t>(datapoint.column + vals.size()) < N)
			{
                std::copy (vals.begin (), vals.end (), outputData + datapoint.column);
				currentCount = (std::max)(currentCount, datapoint.column + static_cast<index_t>(vals.size()));
			}
			else if (datapoint.column < N)
			{
                std::copy (vals.begin (), vals.begin () + (N - datapoint.column - 1),
                           outputData + datapoint.column);
				currentCount = N;
			}

		}
		else if (datapoint.column<N)
		{
			data[datapoint.column] = datapoint.dataGrabber->grabData();
			currentCount = (std::max)(currentCount, datapoint.column + 1);
		}
	}
	currentCount = (std::min)(currentCount, N);
	return currentCount;
}


change_code collector::trigger (coreTime time)
{
    std::vector<double> vals;

    if (recheck)
    {
        recheckColumns ();
    }
    for (auto &datapoint : points)
    {
        if (datapoint.dataGrabber->vectorGrab)
        {
            datapoint.dataGrabber->grabVectorData (vals);
            std::copy (vals.begin (), vals.end (), data.begin () + datapoint.column);
        }
        else
        {
            data[datapoint.column] = datapoint.dataGrabber->grabData ();
        }
    }
    lastTriggerTime = time;
    int cnt = 0;
    while (time >= triggerTime)
    {
        triggerTime += timePeriod;
        ++cnt;
        if (cnt > 5)
        {
            triggerTime = time + timePeriod;
        }
    }
    if (triggerTime > stopTime)
    {
        triggerTime = maxTime;
    }
    return change_code::no_change;
}

int collector::getColumn (int requestedColumn)
{
    int retColumn = requestedColumn;
    if (requestedColumn < 0)
    {
        if (recheck)
        {
            retColumn = -1;
        }
        else
        {
            retColumn = static_cast<int> (columns);
        }
    }
    return retColumn;
}

void collector::updateColumns (int requestedColumn)
{
    if (requestedColumn >= static_cast<int> (columns))
    {
        columns = requestedColumn + 1;
    }

    if (!recheck)
    {
        data.resize (columns);
    }
}

// TODO:: a lot of repeated code here try to merge them
void collector::add (std::shared_ptr<gridGrabber> ggb, int requestedColumn)
{
    int newColumn = getColumn (requestedColumn);

    if (ggb->vectorGrab)
    {
        recheck = true;
    }

    updateColumns (newColumn);
    points.emplace_back (ggb, nullptr, newColumn);
    if (!ggb->getDesc ().empty ())
    {
        points.back ().colname = ggb->getDesc ();
    }
    dataPointAdded (points.back ());
    if (!ggb->loaded)
    {
        if (ggb->getObject () != nullptr)
        {
            addWarning ("grabber not loaded invalid field:" + ggb->field);
        }
        else
        {
            addWarning ("grabber object not valid");
        }
    }
}

void collector::add (std::shared_ptr<stateGrabber> sst, int requestedColumn)
{
    int newColumn = getColumn (requestedColumn);
    updateColumns (newColumn);

    points.emplace_back (nullptr, sst, newColumn);

    dataPointAdded (points.back ());
    if (!sst->loaded)
    {
        if (sst->getObject () != nullptr)
        {
            addWarning ("grabber not loaded invalid field:" + sst->field);
        }
        else
        {
            addWarning ("grabber object not valid");
        }
    }
}

void collector::add (std::shared_ptr<gridGrabber> ggb, std::shared_ptr<stateGrabber> sst, int requestedColumn)
{
    int newColumn = getColumn (requestedColumn);
    updateColumns (newColumn);

    points.emplace_back (ggb, sst, newColumn);
    if (!ggb->getDesc ().empty ())
    {
        points.back ().colname = ggb->getDesc ();
    }
    dataPointAdded (points.back ());
    if ((!ggb->loaded) && (!sst->loaded))
    {
        addWarning ("grabber not loaded");
    }
}

// a notification that something was added much more useful in derived classes
void collector::dataPointAdded (const collectorPoint & /*cp*/) {}
void collector::add (const gridGrabberInfo &gdRI, coreObject *obj)
{
    if (gdRI.field.empty ())  // any field specification overrides the offset
    {
        if (gdRI.offset > 0)
        {
            auto ggb = createGrabber (gdRI.offset, obj);
            // auto sst = makeStateGrabbers(gdRI.offset, obj);
            if (ggb)
            {
                ggb->bias = gdRI.bias;
                ggb->gain = gdRI.gain;
                return add (std::shared_ptr<gridGrabber> (std::move (ggb)), gdRI.column);
            }
            throw (addFailureException ());
        }
        obj->log (obj, print_level::warning, "unable to create collector no field or offset specified");
        addWarning ("unable to create collector no field or offset specified");
    }
    else
    {
        if (gdRI.field.find_first_of (",;") != std::string::npos)
        {  // now go into a loop of the comma variables
            // if multiple fields were specified by comma or semicolon separation
            auto v = stringOps::splitlineBracket (gdRI.field, ",;");
            int ccol = gdRI.column;
			auto tempInfo = gdRI;
            for (const auto &fld : v)
            {
				tempInfo.field = fld;
                if (ccol >= 0)
                {
                    /* this wouldn't work if the data was a vector grab, but in that case the recheck flag would be
                     * activated and this information overridden*/
					tempInfo.column = ccol++;  // post increment intended
                }
                add (tempInfo, obj);
            }
        }
        else  // now we get to the interesting bit
        {
            auto fldGrabbers = makeGrabbers (gdRI.field, obj);
            auto stGrabbers = makeStateGrabbers (gdRI.field, obj);
            if (fldGrabbers.size () == 1)
            {
                // merge the gain and bias
                fldGrabbers[0]->gain *= gdRI.gain;
                fldGrabbers[0]->bias *= gdRI.gain;
                fldGrabbers[0]->bias += gdRI.bias;
                if (gdRI.outputUnits != gridUnits::defUnit)
                {
                    fldGrabbers[0]->outputUnits = gdRI.outputUnits;
                }
                // TODO:: PT incorporate state grabbers properly
                add (std::shared_ptr<gridGrabber> (std::move (fldGrabbers[0])), gdRI.column);
            }
            else
            {
                int ccol = gdRI.column;
                for (auto &ggb : fldGrabbers)
                {
                    if (ccol > 0)
                    {
                        add (std::shared_ptr<gridGrabber> (std::move (ggb)), ccol++);
                    }
                    else
                    {
                        add (std::shared_ptr<gridGrabber> (std::move (ggb)));
                    }
                }
            }
            if (fldGrabbers.empty ())
            {
                obj->log (obj, print_level::warning, "no grabbers created from " + gdRI.field);
                addWarning ("no grabbers created from " + gdRI.field);
                throw (addFailureException ());
            }
        }
    }
}

void collector::add (const std::string &field, coreObject *obj)
{
    if (field.find_first_of (",;") != std::string::npos)
    {  // now go into a loop of the comma variables
        // if multiple fields were delimited by comma or semicolon
        auto grabberStrings = stringOps::splitlineBracket (field, ",;");
        for (const auto &fld : grabberStrings)
        {
            add (fld, obj);
        }
    }
    else  // now we get to the interesting bit
    {
        auto fldGrabbers = makeGrabbers (field, obj);
        for (auto &ggb : fldGrabbers)
        {
            add (std::shared_ptr<gridGrabber> (std::move (ggb)));
        }
        if (fldGrabbers.empty ())
        {
            obj->log (obj, print_level::warning, "no grabbers created from " + field);
            addWarning ("no grabbers created from " + field);
            throw (addFailureException ());
        }
    }
}

void collector::reset()
{
	points.clear();
	data.clear();
	warnList.clear();
	warningCount = 0;
	triggerTime = maxTime;
}

void collector::flush () {}
static const std::string emptyString;
const std::string &collector::getSinkName () const { return emptyString; }
std::unique_ptr<collector> makeCollector (const std::string &type, const std::string &name)
{
    if (name.empty ())
    {
        return coreClassFactory<collector>::instance ()->createObject (type);
    }
    return coreClassFactory<collector>::instance ()->createObject (type, name);
}

}  // namespace griddyn
