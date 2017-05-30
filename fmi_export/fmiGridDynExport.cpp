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

#include "fmi/FMI2/fmi2Functions.h"
#include "fmiRunner.h"
#include "griddyn.h"
#include <memory>
#include <mutex>
#include <vector>

const char *fmi2GetTypesPlatform (void) { return "default"; }
const char *fmi2GetVersion (void) { return "2.0"; }

using runner_t = std::unique_ptr<fmiRunner>;
static std::mutex fmiLock;  //!< lock for allowing multi-threaded access


static std::vector<runner_t> fmiRunnerInstances (1);  // vector of fmiRunner Instances

using compID = std::pair<unsigned int, std::uint64_t>;

fmi2Status fmi2SetDebugLogging (fmi2Component comp, fmi2Boolean, size_t, const fmi2String[]) { return fmi2OK; }


/* Creation and destruction of FMU instances and setting debug status */
fmi2Component fmi2Instantiate (fmi2String instanceName,
                               fmi2Type fmuType,
                               fmi2String fmuGUID,
                               fmi2String fmuResourceLocation,
                               const fmi2CallbackFunctions *functions,
                               fmi2Boolean visible,
                               fmi2Boolean loggingOn)
{
    if (fmuType == fmi2Type::fmi2CoSimulation)
    {
        auto locString = std::string (fmuResourceLocation);
        if (locString.compare (0, 6, "file:/") == 0)
        {
            if (locString[6] == '/')
            {
                if (locString[7] == '/')
                {
                    locString.erase (0, 8);
                }
                else
                {
                    locString.erase (0, 7);
                }
            }
            else
            {
                locString.erase (0, 6);
            }
        }
        try
        {
            auto fmiM = std::make_unique<fmiRunner> (instanceName, locString, functions);
            auto rv = new compID;
            rv->second = fmiM->GetID ();
            std::lock_guard<std::mutex> arrayLock (fmiLock);
            fmiRunnerInstances.push_back (std::move (fmiM));
            rv->first = static_cast<unsigned int>(fmiRunnerInstances.size ()) - 1;
            return (fmi2Component) (rv);
        }
        catch (const std::invalid_argument &)
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

void fmi2FreeInstance (fmi2Component comp)
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return;
    }
    fmiRunnerInstances[p->first] = nullptr;
}

/* Enter and exit initialization mode, terminate and reset */
fmi2Status fmi2SetupExperiment (fmi2Component comp,
                                fmi2Boolean toleranceDefined,
                                fmi2Real tolerance,
                                fmi2Real startTime,
                                fmi2Boolean stopTimeDefined,
                                fmi2Real stopTime)
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    runner->getSim ()->set ("starttime", startTime);
    if (stopTimeDefined)
    {
        runner->getSim ()->set ("stoptime", stopTime);
    }
    return fmi2OK;
}
fmi2Status fmi2EnterInitializationMode (fmi2Component comp)
{
    // in GridDyn there is no difference between startup and initialization
    return fmi2OK;
}
fmi2Status fmi2ExitInitializationMode (fmi2Component comp)
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    runner->simInitialize ();
    runner->UpdateOutputs ();
    return fmi2OK;
}
fmi2Status fmi2Terminate (fmi2Component comp)
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    runner->Finalize ();
    fmiRunnerInstances[p->first] = nullptr;  // actually freeing the instance

    delete p;
    return fmi2OK;
}
fmi2Status fmi2Reset (fmi2Component comp) { return fmi2OK; }

/* Getting and setting variable values */
fmi2Status fmi2GetReal (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        value[ii] = runner->Get (vr[ii]);
        if (value[ii] < -1e45)
        {
            // send a log message
            ret = fmi2Warning;
        }
    }
    return ret;
}
fmi2Status fmi2GetInteger (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        auto res = runner->Get (vr[ii]);
        if (res < -1e45)
        {
            ret = fmi2Warning;
        }
        value[ii] = static_cast<int> (res);
    }
    return ret;
}
fmi2Status fmi2GetBoolean (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        auto res = runner->Get (vr[ii]);
        if (res < -1e45)
        {
            ret = fmi2Warning;
        }
        value[ii] = static_cast<bool> (res);
    }
    return ret;
}
fmi2Status fmi2GetString (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, fmi2String value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        if (vr[ii] == 0)
        {
            auto val = runner->getSim ()->getString ("recorddirectory");
            // memcpy(value,val.c_str(), val.end (), value);
        }
        else
        {

            ret = fmi2Warning;
        }
    }
    return ret;
}

fmi2Status fmi2SetReal (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        auto res = runner->Set (vr[ii], value[ii]);
        if (!res)
        {
            ret = fmi2Warning;
        }
    }
    return ret;
}
fmi2Status
fmi2SetInteger (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        auto res = runner->Set (vr[ii], static_cast<double> (value[ii]));
        if (!res)
        {
            ret = fmi2Warning;
        }
    }
    return ret;
}
fmi2Status
fmi2SetBoolean (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[])
{
    return fmi2Warning;
}
fmi2Status fmi2SetString (fmi2Component comp, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[])
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    auto ret = fmi2OK;
    for (index_t ii = 0; ii < nvr; ++ii)
    {
        if (vr[ii] == 0)
        {
            runner->getSim ()->set ("recorddirectory", value[ii]);
        }
        else
        {
            auto res = runner->SetString (vr[ii], value[ii]);
            if (!res)
            {
                ret = fmi2Warning;
            }
        }
    }
    return ret;
}

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate (fmi2Component comp, fmi2FMUstate *FMUstate) { return fmi2Discard; }
fmi2Status fmi2SetFMUstate (fmi2Component comp, fmi2FMUstate FMUstate) { return fmi2Discard; }
fmi2Status fmi2FreeFMUstate (fmi2Component comp, fmi2FMUstate *FMUstate)
{
    if (*FMUstate == NULL)
    {
    }
    else
    {
    }
    return fmi2OK;
}
fmi2Status fmi2SerializedFMUstateSize (fmi2Component comp, fmi2FMUstate FMUstate, size_t *size)
{
    return fmi2Discard;
}
fmi2Status
fmi2SerializeFMUstate (fmi2Component comp, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size)
{
    return fmi2Discard;
}
fmi2Status
fmi2DeSerializeFMUstate (fmi2Component comp, const fmi2Byte serializedState[], size_t size, fmi2FMUstate *FMUstate)
{
    return fmi2Discard;
}

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative (fmi2Component comp,
                                         const fmi2ValueReference vUnknownRef[],
                                         size_t nUnknown,
                                         const fmi2ValueReference vKnownRef[],
                                         size_t nKnown,
                                         const fmi2Real dvKnown[],
                                         fmi2Real dvUnknown[])
{
    return fmi2Discard;
}

/***************************************************
Types for Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives (fmi2Component comp,
                                        const fmi2ValueReference vr[],
                                        size_t nvr,
                                        const fmi2Integer order[],
                                        const fmi2Real value[])
{
    return fmi2Discard;
}
fmi2Status fmi2GetRealOutputDerivatives (fmi2Component comp,
                                         const fmi2ValueReference vr[],
                                         size_t nvr,
                                         const fmi2Integer order[],
                                         fmi2Real value[])
{
    return fmi2Discard;
}

fmi2Status fmi2DoStep (fmi2Component comp,
                       fmi2Real currentCommunicationPoint,
                       fmi2Real communicationStepSize,
                       fmi2Boolean noSetPriorPoint)
{
    auto p = static_cast<compID *> (comp);
    runner_t &runner =
      (p->first < fmiRunnerInstances.size ()) ? fmiRunnerInstances[p->first] : fmiRunnerInstances[0];
    if ((!runner) || (runner->GetID () != p->second))
    {
        return fmi2Error;
    }
    runner->Step (currentCommunicationPoint + communicationStepSize);
    return fmi2OK;
}
fmi2Status fmi2CancelStep (fmi2Component comp) { return fmi2Discard; }

/* Inquire slave status */
fmi2Status fmi2GetStatus (fmi2Component comp, const fmi2StatusKind s, fmi2Status *status) { return fmi2Discard; }
fmi2Status fmi2GetRealStatus (fmi2Component comp, const fmi2StatusKind s, fmi2Real *status) { return fmi2Discard; }
fmi2Status fmi2GetIntegerStatus (fmi2Component comp, const fmi2StatusKind s, fmi2Integer *status)
{
    return fmi2Discard;
}
fmi2Status fmi2GetBooleanStatus (fmi2Component comp, const fmi2StatusKind s, fmi2Boolean *status)
{
    return fmi2Discard;
}
fmi2Status fmi2GetStringStatus (fmi2Component comp, const fmi2StatusKind s, fmi2String *status)
{
    return fmi2Discard;
}
