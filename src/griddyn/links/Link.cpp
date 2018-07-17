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

// headers
#include "../Area.h"
#include "../gridBus.h"
#include "../simulation/contingency.h"
#include "acLine.h"
#include "acdcConverter.h"
#include "adjustableTransformer.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include "core/objectInterpreter.h"
#include "dcLink.h"
#include "utilities/matrixDataCompact.hpp"
#include "utilities/stringOps.h"
#include "utilities/vectorOps.hpp"

#include "../measurement/objectGrabbers.h"

#include <cmath>
#include <complex>

namespace griddyn
{
using namespace gridUnits;

// make the object factory types

static typeFactory<Link> blf ("link", stringVec{"trivial", "basic", "transport"});

static childTypeFactory<acLine, Link>
  glf ("link", stringVec{"ac", "line", "phaseshifter", "phase_shifter", "transformer"}, "ac");

namespace links
{
static childTypeFactory<adjustableTransformer, Link>
  gfad ("link", stringVec{"adjust", "adjustable", "adjustabletransformer"});

static childTypeFactory<dcLink, Link> dclnk ("link", stringVec{"dc", "dclink", "dcline"});

static typeFactoryArg<acdcConverter, acdcConverter::mode_t>
  dcrect ("link", stringVec{"rectifier", "rect"}, acdcConverter::mode_t::rectifier);
static typeFactoryArg<acdcConverter, acdcConverter::mode_t>
  dcinv ("link", stringVec{"inverter", "inv"}, acdcConverter::mode_t::inverter);
static childTypeFactory<acdcConverter, Link> acdc ("link", stringVec{"acdc", "acdcconverter", "dcconverter"});
}  // namespace links
std::atomic<count_t> Link::linkCount (0);
// helper defines to have things make more sense
#define DEFAULTPOWERCOMP (this->*(flowCalc[0]))
#define MODEPOWERCOMP (this->*(flowCalc[getLinkApprox (sMode)]))
#define DERIVCOMP (this->*(derivCalc[getLinkApprox (sMode)]))
#define DEFAULTDERIVCOMP (this->*(derivCalc[0]))

Link::Link (const std::string &objName) : gridPrimary (objName)
{
    // default values
    setUserID (++linkCount);
    updateName ();
}

coreObject *Link::clone (coreObject *obj) const
{
    auto lnk = cloneBaseFactory<Link, gridPrimary> (this, obj, &glf);
    if (lnk == nullptr)
    {
        return obj;
    }

    lnk->Pset = Pset;
    lnk->Erating = Erating;
    lnk->ratingB = ratingB;
    lnk->ratingA = ratingA;
    lnk->lossFraction = lossFraction;
    lnk->circuitNum = circuitNum;
    lnk->zone = zone;
    return lnk;
}

Link::~Link () = default;

// timestepP link's buses
void Link::updateBus (gridBus *bus, index_t busNumber)
{
    if (busNumber == 1)
    {
        if (B1 != nullptr)
        {
            B1->remove (this);
        }
        B1 = bus;
        B1->add (this);
    }
    else if (busNumber == 2)
    {
        if (B2 != nullptr)
        {
            B2->remove (this);
        }
        B2 = bus;
        B2->add (this);
    }
    else
    {
        throw (objectAddFailure (this));
    }
}

void Link::followNetwork (int network, std::queue<gridBus *> &stk)
{
    if (isConnected () && opFlags[network_connected])
    {
        if (B1->Network != network)
        {
            stk.push (B1);
        }
        if (B2->Network != network)
        {
            stk.push (B2);
        }
    }
}

void Link::pFlowCheck (std::vector<violation> &Violation_vector)
{
    double mva = std::max (getCurrent (0), getCurrent (1));
    if (mva > ratingA)
    {
        violation V (getName (), MVA_EXCEED_RATING_A);
        V.level = mva;
        V.limit = ratingA;
        V.percentViolation = (mva - ratingA) / ratingA * 100;
        Violation_vector.push_back (V);
    }
    if (mva > ratingB)
    {
        violation V (getName (), MVA_EXCEED_RATING_B);

        V.level = mva;
        V.limit = ratingB;
        V.percentViolation = (mva - ratingB) / ratingB * 100;
        Violation_vector.push_back (V);
    }
    if (mva > Erating)
    {
        violation V (getName (), MVA_EXCEED_ERATING);
        V.level = mva;
        V.limit = Erating;
        V.percentViolation = (mva - Erating) / Erating * 100;
        Violation_vector.push_back (V);
    }
}

double Link::quickupdateP () { return Pset; }
void Link::timestep (const coreTime time, const IOdata & /*inputs*/, const solverMode & /*sMode*/)
{
    if (!isEnabled ())
    {
        return;
    }

    updateLocalCache ();
    prevTime = time;
    /*if (scheduled)
    {
    Psched=sched->timestepP(time);
    }*/
}

static const stringVec locNumStrings{"loss", "switch1", "switch2", "p"};
static const stringVec locStrStrings{"from", "to"};
static const stringVec flagStrings{};
void Link::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
    getParamString<Link, gridPrimary> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

// set properties
void Link::set (const std::string &param, const std::string &val)
{
    if ((param == "bus1") || (param == "from"))
    {
        auto bus = dynamic_cast<gridBus *> (locateObject (val, getParent ()));

        if (bus != nullptr)
        {
            updateBus (bus, 1);
        }
        else
        {
            throw (invalidParameterValue (param));
        }
    }
    else if ((param == "bus2") || (param == "to"))
    {
        auto bus = dynamic_cast<gridBus *> (locateObject (val, getParent ()));
        if (bus != nullptr)
        {
            updateBus (bus, 2);
        }
        else
        {
            throw (invalidParameterValue (param));
        }
    }
    else if (param == "status")
    {
        auto v2 = convertToLowerCase (val);
        if ((v2 == "closed") || (v2 == "connected"))
        {
            reconnect ();
        }
        else if ((v2 == "open") || (v2 == "disconnected"))
        {
            disconnect ();
        }
    }
    else
    {
        gridPrimary::set (param, val);
    }
}

// true is open
// false is closed
void Link::switchMode (index_t num, bool mode)
{
    if (num == 2)
    {
        if (mode == opFlags[switch2_open_flag])
        {
            return;
        }

        opFlags.flip (switch2_open_flag);

        if (opFlags[pFlow_initialized])
        {
            LOG_DEBUG (
              "Switch2 changed||state =" +
              ((opFlags[switch2_open_flag]) ? std::string ("OPEN") : std::string ("CLOSED")) +
              ", link status =" + ((isConnected ()) ? std::string ("CONNECTED") : std::string ("DISCONNECTED")));
            if (isConnected ())
            {
                reconnect ();
            }
            else
            {
                switchChange (2);

                if (!B1->checkCapable ())
                {
                    B1->disconnect ();
                }
                if (!B2->checkCapable ())
                {
                    B2->disconnect ();
                }
                updateLocalCache ();
                alert (this, CONNECTIVITY_CHANGE);
            }
        }
    }
    else
    {
        if (mode == opFlags[switch1_open_flag])
        {
            return;
        }
        opFlags.flip (switch1_open_flag);

        if (opFlags[pFlow_initialized])
        {
            LOG_DEBUG (
              "Switch1 changed||state =" +
              ((opFlags[switch1_open_flag]) ? std::string ("OPEN") : std::string ("CLOSED")) +
              ", link status =" + ((isConnected ()) ? std::string ("CONNECTED") : std::string ("DISCONNECTED")));
            if (isConnected ())
            {
                reconnect ();
            }
            else
            {
                switchChange (1);
                if (!B1->checkCapable ())
                {
                    B1->disconnect ();
                }
                if (!B2->checkCapable ())
                {
                    B2->disconnect ();
                }
                updateLocalCache ();
                alert (this, CONNECTIVITY_CHANGE);
            }
        }
    }
}

void Link::switchChange (int /*switchNum*/) { computePowers (); }
void Link::disconnect ()
{
    if (isConnected ())
    {
        opFlags.set (switch1_open_flag, true);
        opFlags.set (switch2_open_flag, true);
        switchChange (1);
        switchChange (2);
        if (!B1->checkCapable ())
        {
            B1->disconnect ();
        }
        if (!B2->checkCapable ())
        {
            B2->disconnect ();
        }
        updateLocalCache ();
        LOG_DEBUG ("disconnecting line");
        alert (this, CONNECTIVITY_CHANGE);
    }
}

void Link::reconnect ()
{
    if (!isConnected ())
    {
        if (opFlags[switch1_open_flag])
        {
            opFlags.reset (switch1_open_flag);
            switchChange (1);
        }
        if (opFlags[switch2_open_flag])
        {
            opFlags.reset (switch2_open_flag);
            switchChange (2);
        }
        LOG_DEBUG ("reconnecting line");
        updateLocalCache ();
    }

    if (B1->checkFlag (disconnected))
    {
        if (!(B2->checkFlag (disconnected)))
        {
            B1->reconnect (B2);
            updateLocalCache ();
        }
    }
    else if (B2->checkFlag (disconnected))
    {
        B2->reconnect (B1);
        updateLocalCache ();
    }
}

void Link::set (const std::string &param, double val, units_t unitType)
{
    if ((param == "state") || (param == "switch") || (param == "switch1") || (param == "breaker") ||
        (param == "breaker_open") || (param == "breaker1") || (param == "breaker_open1"))
    {
        switchMode (1, (val > 0));
    }
    else if ((param == "switch2") || (param == "breaker2") || (param == "breaker_open2"))
    {
        switchMode (2, (val > 0));
    }
    else if (param == "pset")
    {
        Pset = unitConversion (val, unitType, puMW, systemBasePower);
        opFlags.set (fixed_target_power);
        computePowers ();
    }
    else if ((param == "loss") || (param == "lossfraction"))
    {
        lossFraction = val;
        computePowers ();
    }
    else if ((param == "ratinga") || (param == "rating"))
    {
        ratingA = unitConversion (val, unitType, puMW, systemBasePower);
    }
    else if (param == "ratingb")
    {
        ratingB = unitConversion (val, unitType, puMW, systemBasePower);
    }
    else if ((param == "ratinge") || (param == "emergency_rating") || (param == "erating") || (param == "ratingc"))
    {
        Erating = unitConversion (val, unitType, puMW, systemBasePower);
    }
    else if (param == "circuit")
    {
        circuitNum = static_cast<index_t> (val);
    }
    else
    {
        gridPrimary::set (param, val, unitType);
    }
}

coreObject *Link::getSubObject (const std::string &typeName, index_t num) const
{
    if (typeName == "bus")
    {
        return (num == 1) ? B1 : ((num == 2) ? B2 : nullptr);
    }
    return nullptr;
}

double Link::get (const std::string &param, units_t unitType) const
{
    double val = kNullVal;

    if ((param == "breaker1") || (param == "switch1") || (param == "breaker_open1"))
    {
        val = static_cast<double> (opFlags[switch1_open_flag]);
    }
    else if ((param == "breaker2") || (param == "switch2") || (param == "breaker_open2"))
    {
        val = static_cast<double> (opFlags[switch2_open_flag]);
    }
    else if ((param == "connected") || (param == "breaker"))
    {
        val = static_cast<double> (isConnected ());
    }
    else if ((param == "set") || (param == "pset"))
    {
        val = gridUnits::unitConversion (Pset, puMW, unitType, systemBasePower);
    }
    else if (param == "linkcount")
    {
        val = 1.0;
    }
    else if ((param == "buscount") || (param == "gencount") || (param == "loadcount") || (param == "relaycount"))
    {
        val = 0.0;
    }
    else if ((param == "rating") || (param == "ratinga"))
    {
        val = ratingA;
    }
    else if (param == "ratingb")
    {
        val = ratingB;
    }
    else if (param == "erating")
    {
        val = Erating;
    }
    else if (param == "loss")
    {
        val = unitConversion (getLoss (), puMW, unitType, systemBasePower);
    }
    else if (param == "lossfraction")
    {
        val = lossFraction;
    }
    else if (param == "circuit")
    {
        val = circuitNum;
    }
    else
    {
        auto fptr = getObjectFunction (this, param);
        if (fptr.first)
        {
            coreObject *tobj = const_cast<Link *> (this);
            val = unitConversion (fptr.first (tobj), fptr.second, unitType, systemBasePower);
        }
        else
        {
            val = gridPrimary::get (param, unitType);
        }
    }
    return val;
}

void Link::pFlowObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/)
{
    if (B1 == nullptr)
    {
        opFlags.set (switch1_open_flag);
    }
    if (B2 == nullptr)
    {
        opFlags.set (switch2_open_flag);
    }
}

bool Link::isConnected () const { return (!(opFlags[switch1_open_flag] || opFlags[switch2_open_flag])); }
int Link::fixRealPower (double power, id_type_t measureTerminal, id_type_t /*fixedTerminal*/, units_t unitType)
{
    if (measureTerminal == 1)
    {
        Pset = unitConversion (power, unitType, puMW, systemBasePower);
    }
    else
    {
        Pset = unitConversion (power, unitType, puMW, systemBasePower) / (1.0 - lossFraction);
    }
    opFlags.set (fixed_target_power);
    return 1;
}

static IOlocs aLoc{0, 1};

int Link::fixPower (double rPower,
                    double /*qPower*/,
                    id_type_t measureTerminal,
                    id_type_t fixedTerminal,
                    gridUnits::units_t unitType)
{
    return fixRealPower (rPower, measureTerminal, fixedTerminal, unitType);
}

void Link::dynObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/)
{
    if ((B1 == nullptr) || (B2 == nullptr))
    {
        disable ();
    }
    else if ((!B1->isEnabled ()) || (!B2->isEnabled ()))
    {
        disable ();
    }
}

void Link::computePowers ()
{
    if (isConnected ())
    {
        linkFlows.P1 = Pset;
        linkFlows.P2 = Pset - std::abs (Pset) * lossFraction;
    }
    else
    {
        linkFlows.P1 = 0;
        linkFlows.P2 = 0;
    }
}

void Link::ioPartialDerivatives (id_type_t /*busId*/,
                                 const stateData & /*sD*/,
                                 matrixData<double> & /*md*/,
                                 const IOlocs & /*inputLocs*/,
                                 const solverMode & /*sMode*/)
{
}

void Link::outputPartialDerivatives (id_type_t /*busId*/,
                                     const stateData & /*sD*/,
                                     matrixData<double> & /*md*/,
                                     const solverMode & /*sMode*/)
{
}

count_t Link::outputDependencyCount (index_t /*num*/, const solverMode & /*sMode*/) const { return 0; }
IOdata Link::getOutputs (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode) const
{
    return getOutputs (1, sD, sMode);
}

bool isBus2 (id_type_t busId, gridBus *bus) { return ((busId == 2) || (isSameObject (busId, bus))); }

IOdata Link::getOutputs (id_type_t busId, const stateData & /*sD*/, const solverMode & /*sMode*/) const
{
    // set from/to buses
    IOdata out{0.0, 0.0};

    if (isBus2 (busId, B2))
    {
        out[PoutLocation] = Pset;
    }
    else
    {
        out[PoutLocation] = Pset - std::abs (Pset) * lossFraction;
    }
    return out;
}

void Link::disable ()
{
    if (!isEnabled ())
    {
        return;
    }
    if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
    {
        alert (this, STATE_COUNT_CHANGE);
    }
    else
    {
        alert (this, JAC_COUNT_CHANGE);
    }
    coreObject::disable ();
    if ((B1 != nullptr) && (B1->isEnabled ()))
    {
        if (!(B1->checkCapable ()))
        {
            B1->disable ();
        }
    }
    if ((B2 != nullptr) && (B2->isEnabled ()))
    {
        if (!(B2->checkCapable ()))
        {
            B2->disable ();
        }
    }
}

double Link::getMaxTransfer () const
{
    if (!isConnected ())
    {
        return 0;
    }
    if (Erating > 0)
    {
        return Erating;
    }
    if (ratingB > 0)
    {
        return ratingB;
    }
    if (ratingA > 0)
    {
        return ratingA;
    }

    return (kBigNum);
}

double Link::getBusAngle (id_type_t busId) const
{
    if (busId < 500_ind)
    {
        auto B = getBus (busId);
        if (B != nullptr)
        {
            return B->getAngle ();
        }
    }
    // these are special cases for getting opposite angles as called by the attached buses
    if (isSameObject (busId, B2))
    {
        return (B1 != nullptr) ? B1->getAngle () : kNullVal;
    }
    if (isSameObject (busId, B1))
    {
        return (B2 != nullptr) ? B2->getAngle () : kNullVal;
    }
    // now just default to the original behavior
    auto B = getBus (busId);
    if (B != nullptr)
    {
        return B->getAngle ();
    }
    return kNullVal;
}

double Link::getBusAngle (const stateData &sD, const solverMode &sMode, id_type_t busId) const
{
    if (busId < 500_ind)
    {
        auto B = getBus (busId);
        if (B != nullptr)
        {
            return B->getAngle ();
        }
    }
    // these are special cases for getting opposite angles as called by the attached buses
    if (isSameObject (busId, B2))
    {
        return (B1 != nullptr) ? B1->getAngle (sD, sMode) : kNullVal;
    }
    if (isSameObject (busId, B1))
    {
        return (B2 != nullptr) ? B2->getAngle (sD, sMode) : kNullVal;
    }
    // now just default to the original behavior
    auto B = getBus (busId);
    if (B != nullptr)
    {
        return B->getAngle (sD, sMode);
    }
    return kNullVal;
}

double Link::getVoltage (id_type_t busId) const
{
    if (isBus2 (busId, B2))
    {
        return B2->getVoltage ();
    }
    return B1->getVoltage ();
}

void Link::setState (coreTime time,
                     const double /*state*/[],
                     const double /*dstate_dt*/[],
                     const solverMode & /*sMode*/)
{
    prevTime = time;
}

void Link::updateLocalCache (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode)
{
    if (!isEnabled ())
    {
        return;
    }
    if ((linkInfo.seqID == sD.seqID) && (sD.seqID != 0))
    {
        return;  // already computed
    }
    linkInfo.v1 = B1->getVoltage (sD, sMode);
    double t1 = B1->getAngle (sD, sMode);
    linkInfo.v2 = B2->getVoltage (sD, sMode);
    double t2 = B2->getAngle (sD, sMode);

    linkInfo.theta1 = t1 - t2;
    linkInfo.theta2 = t2 - t1;
    linkInfo.seqID = sD.seqID;
}

void Link::updateLocalCache ()
{
    if (!isEnabled ())
    {
        return;
    }
    linkInfo.v1 = B1->getVoltage ();
    double t1 = B1->getAngle ();
    linkInfo.v2 = B2->getVoltage ();
    double t2 = B2->getAngle ();

    linkInfo.theta1 = t1 - t2;
    linkInfo.theta2 = t2 - t1;
}

gridBus *Link::getBus (index_t busInd) const
{
    // for Links it is customary to refer to the buses as 1 and 2, but for indexing schemes they sometimes atart at
    // 0
    // so this function will return Bus 1 for indices <=1 and Bus if the index is 2.
    return ((busInd <= 1) || (busInd == B1->getID ())) ?
             B1 :
             (((busInd == 2) || (busInd == B2->getID ())) ? B2 : nullptr);
}

double Link::getRealPower (id_type_t busId) const { return (isBus2 (busId, B2)) ? linkFlows.P2 : linkFlows.P1; }
double Link::getReactivePower (id_type_t busId) const
{
    return (isBus2 (busId, B2)) ? linkFlows.Q2 : linkFlows.Q1;
}

double Link::remainingCapacity () const { return getMaxTransfer () - std::abs (linkFlows.P1); }
double Link::getAngle (const double state[], const solverMode &sMode) const
{
    double t1 = B1->getAngle (state, sMode);
    double t2 = B2->getAngle (state, sMode);
    return t1 - t2;
}

double Link::getAngle () const { return (linkInfo.theta1); }
double Link::getLoss () const { return std::abs (linkFlows.P1 + linkFlows.P2); }
double Link::getReactiveLoss () const { return std::abs (linkFlows.Q1 + linkFlows.Q2); }
double Link::getRealImpedance (id_type_t busId) const
{
    if (isBus2 (busId, B2))  // from bus
    {
        std::complex<double> Z = (linkInfo.v2 * linkInfo.v2) / std::complex<double> (linkFlows.P2, linkFlows.Q2);
        return std::isnormal (Z.real ()) ? Z.real () : kBigNum;
    }
    std::complex<double> Z = (linkInfo.v1 * linkInfo.v1) / std::complex<double> (linkFlows.P1, linkFlows.Q1);
    return std::isnormal (Z.real ()) ? Z.real () : kBigNum;
}
double Link::getImagImpedance (id_type_t busId) const
{
    if (isBus2 (busId, B2))  // from bus
    {
        std::complex<double> Z = (linkInfo.v2 * linkInfo.v2) / std::complex<double> (linkFlows.P2, linkFlows.Q2);
        return std::isnormal (Z.imag ()) ? Z.imag () : kBigNum;
    }
    std::complex<double> Z = (linkInfo.v1 * linkInfo.v1) / std::complex<double> (linkFlows.P1, linkFlows.Q1);
    return std::isnormal (Z.imag ()) ? Z.imag () : kBigNum;
}

double Link::getTotalImpedance (id_type_t busId) const
{
    if (isBus2 (busId, B2))  // from bus
    {
        //  printf("id2 impedance=%f\n", signn(linkFlows.P2 + linkFlows.Q2)*(linkInfo.v2*linkInfo.v2) /
        //  std::hypot(linkFlows.P2, linkFlows.Q2));
        double val = signn (linkFlows.P2 + linkFlows.Q2) * (linkInfo.v2 * linkInfo.v2) /
                     std::hypot (linkFlows.P2, linkFlows.Q2);
        return (std::isnormal (val) ? val : kBigNum);
    }
    // printf("id1 impedance=%f\n", signn(linkFlows.P1 + linkFlows.Q1)*(linkInfo.v1*linkInfo.v1) /
    // std::hypot(linkFlows.P1, linkFlows.Q1));
    double val =
      signn (linkFlows.P1 + linkFlows.Q1) * (linkInfo.v1 * linkInfo.v1) / std::hypot (linkFlows.P1, linkFlows.Q1);
    return (std::isnormal (val) ? val : kBigNum);
}

double Link::getCurrent (id_type_t busId) const
{
    double val;
    if (isBus2 (busId, B2))  // from bus
    {
        val = std::hypot (linkFlows.P2, linkFlows.Q2) / (linkInfo.v2);
    }
    else
    {
        val = std::hypot (linkFlows.P1, linkFlows.Q1) / (linkInfo.v1);
    }
    return (std::isnormal (val) ? val : 0);
}

double Link::getRealCurrent (id_type_t busId) const
{
    double val;
    if (isBus2 (busId, B2))  // from bus
    {
        val = linkFlows.P2 / (linkInfo.v2);
    }
    else
    {
        val = linkFlows.P1 / (linkInfo.v1);
    }
    return (std::isnormal (val) ? val : 0);
}
double Link::getImagCurrent (id_type_t busId) const
{
    double val;
    if (isBus2 (busId, B2))  // from bus
    {
        val = linkFlows.Q2 / (linkInfo.v2);
    }
    else
    {
        val = linkFlows.Q1 / (linkInfo.v1);
    }
    return (std::isnormal (val) ? val : 0);
}

Link *getMatchingLink (Link *lnk, gridPrimary *src, gridPrimary *sec)
{
    Link *L2 = nullptr;
    if (lnk->isRoot ())
    {
        return nullptr;
    }
    if (isSameObject (lnk->getParent (), src))  // if this is true then things are easy
    {
        L2 = sec->getLink (lnk->locIndex);
    }
    else
    {
        std::vector<int> lkind;
        auto par = dynamic_cast<gridPrimary *> (lnk->getParent ());
        if (par == nullptr)
        {
            return nullptr;
        }
        lkind.push_back (lnk->locIndex);
        while (par->getID () != src->getID ())
        {
            lkind.push_back (par->locIndex);
            par = dynamic_cast<gridPrimary *> (par->getParent ());
            if (par == nullptr)
            {
                return nullptr;
            }
        }
        // now work our way backwards through the secondary
        par = sec;
        for (size_t kk = lkind.size () - 1; kk > 0; --kk)
        {
            par = dynamic_cast<gridPrimary *> (par->getArea (lkind[kk]));
        }
        L2 = par->getLink (lkind[0]);
    }
    return L2;
}

bool compareLink (Link *lnk1, Link *lnk2, bool cmpBus, bool printDiff)
{
    if (cmpBus)
    {
        bool ret = compareBus (lnk1->getBus (1), lnk2->getBus (1), printDiff);
        ret = ret && compareBus (lnk1->getBus (2), lnk2->getBus (2), printDiff);
        return ret;
    }
    if (typeid (lnk1) != typeid (lnk2))
    {
        if (printDiff)
        {
            printf ("Links are of different types\n");
        }
        return false;
    }
    if ((dynamic_cast<acLine *> (lnk1) != nullptr) && (dynamic_cast<acLine *> (lnk2) != nullptr))
    {
        if (std::abs (lnk1->get ("r") - lnk2->get ("r")) > 0.0001)
        {
            if (printDiff)
            {
                printf ("Links have different r\n");
            }
            return false;
        }
        if (std::abs (lnk1->get ("x") - lnk2->get ("x")) > 0.0001)
        {
            if (printDiff)
            {
                printf ("Links have different x\n");
            }
            return false;
        }
        if (std::abs (lnk1->get ("b") - lnk2->get ("b")) > 0.0001)
        {
            if (printDiff)
            {
                printf ("Links have different b\n");
            }
            return false;
        }
    }
    return true;
}

}  // namespace griddyn