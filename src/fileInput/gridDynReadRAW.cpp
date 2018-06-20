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

#include "core/coreExceptions.h"
#include "core/objectFactoryTemplates.hpp"
#include "griddyn/Generator.h"
#include "griddyn/gridBus.h"
#include "griddyn/gridDynSimulation.h"
#include "fileInput.h"
#include "griddyn/links/acLine.h"
#include "griddyn/links/adjustableTransformer.h"
#include "griddyn/Load.h"
#include "griddyn/loads/svd.h"
#include "readerHelper.h"
#include "utilities/stringOps.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>

namespace griddyn
{
using namespace gridUnits;
using namespace stringOps;

int getPSSversion (const std::string &line);
void rawReadBus (gridBus *bus, const std::string &line, basicReaderInfo &opt);
void rawReadLoad (Load *ld, const std::string &line, basicReaderInfo &opt);
void rawReadFixedShunt (Load *ld, const std::string &line, basicReaderInfo &opt);
void rawReadGen (Generator *gen, const std::string &line, basicReaderInfo &opt);
void rawReadBranch (coreObject *parentObject,
                    const std::string &line,
                    std::vector<gridBus *> &busList,
                    basicReaderInfo &opt);
int rawReadTX (coreObject *parentObject,
               stringVec &txlines,
               std::vector<gridBus *> &busList,
               basicReaderInfo &opt);
void rawReadSwitchedShunt (coreObject *parentObject,
                           const std::string &line,
                           std::vector<gridBus *> &busList,
                           basicReaderInfo &opt);
void rawReadTXadj (coreObject *parentObject,
                   const std::string &line,
                   std::vector<gridBus *> &busList,
                   basicReaderInfo &opt);

int rawReadDCLine (coreObject *parentObject,
                   stringVec &txlines,
                   std::vector<gridBus *> &busList,
                   basicReaderInfo &opt);

enum sections
{
    unknown,
    bus,
    branch,
    load,
    fixedShunt,
    generator,
    tx,
    switchedShunt,
    txadj
};

// get the basic busFactory
static typeFactory<gridBus> *busfactory = nullptr;

// get the basic load Factory
static typeFactory<Load> *ldfactory = nullptr;
// get the basic Link Factory
static typeFactory<acLine> *linkfactory = nullptr;
// get the basic Generator Factory
static typeFactory<Generator> *genfactory = nullptr;

sections findSectionType (const std::string &line);

bool checkNextLine (std::ifstream &file, std::string &nextLine)
{
    if (std::getline (file, nextLine))
    {
        trimString (nextLine);
        return (nextLine[0] != '0');
    }
    return false;
}

gridBus *findBus (std::vector<gridBus *> &busList, const std::string &line)
{
    auto pos = line.find_first_of (',');
    auto temp1 = trim (line.substr (0, pos));

    auto index = std::stoul (temp1);

    if (index >= busList.size ())
    {
        std::cerr << "Invalid bus number" << index << '\n';
        return nullptr;
    }
    return busList[index];
}

void loadRAW (coreObject *parentObject, const std::string &fileName, const basicReaderInfo &bri)
{
    std::ifstream file (fileName.c_str (), std::ios::in);
    std::string line;  // line storage
    std::string temp1;  // temporary storage for substrings
    std::string pref2;  // temp storage to 2nd order prefix.
    std::vector<gridBus *> busList;
    basicReaderInfo opt (bri);
    Load *ld;
    Generator *gen;
    gridBus *bus;
    index_t index;
    size_t pos;

    /*load up the factories*/
    if (busfactory == nullptr)
    {
        // get the basic busFactory
        busfactory =
          static_cast<decltype (busfactory)> (coreObjectFactory::instance ()->getFactory ("bus")->getFactory (""));

        // get the basic load Factory
        ldfactory =
          static_cast<decltype (ldfactory)> (coreObjectFactory::instance ()->getFactory ("load")->getFactory (""));

        // get the basic load Factory
        genfactory = static_cast<decltype (genfactory)> (
          coreObjectFactory::instance ()->getFactory ("generator")->getFactory (""));
        // get the basic link Factory
        linkfactory = static_cast<decltype (linkfactory)> (
          coreObjectFactory::instance ()->getFactory ("link")->getFactory (""));
    }
    /* Process the first line
    First card in file.


    Columns  2- 9   Date, in format DD/MM/state with leading zeros. If no date
            provided, use 0b/0b/0b where b is blank.
    Columns 11-30   Originator's name (A)
    Columns 32-37   MVA Base (F*)
    Columns 39-42   Year (I)
    Column  44      Season (S - Summer, W - Winter)
    Column  46-73   Case identification (A) */

    // reset all the object counters
    gridSimulation::resetObjectCounters ();
    // get the base scenario information
    if (std::getline (file, line))
    {
        auto res = sscanf (line.c_str (), "%*d, %lf,%d,%*d,%*d,%lf", &(opt.base), &(opt.version), &(opt.basefreq));

        if (res > 0)
        {
            parentObject->set ("basepower", opt.base);
        }
        // temp1=line.substr(45,27);
        // parentObject->set("name",&temp1);
        if (res > 2)
        {
            if (opt.basefreq != 60.0)
            {
                parentObject->set ("basefreq", opt.basefreq);
            }
        }

        if (opt.version == 0)
        {
            opt.version = getPSSversion (line);
        }
    }
    if (std::getline (file, line))
    {
        pos = line.find_first_of (',');
        temp1 = line.substr (0, pos);
        trimString (temp1);
        parentObject->setName (temp1);
    }
    temp1 = line;
    // get the second comment line and ignore it
    std::getline (file, line);
    temp1 = temp1 + '\n' + line;
    // set the case description
    parentObject->setDescription (temp1);
    // get the bus data section
    // bus data doesn't have a header but it is always first
    bool moreData = true;
    while (moreData)
    {
        if (checkNextLine (file, line))
        {
            // get the index
            pos = line.find_first_of (',');
            temp1 = line.substr (0, pos);
            index = numeric_conversion<index_t> (temp1, 0);

            if (index >= static_cast<index_t> (busList.size ()))
            {
                if (index < 100000000)
                {
                    busList.resize (2 * index + 1, nullptr);
                }
                else
                {
                    std::cerr << "Bus index overload " << index << '\n';
                }
            }
            if (busList[index] == nullptr)
            {
                busList[index] = busfactory->makeTypeObject ();
                busList[index]->set ("basepower", opt.base);
                busList[index]->setUserID (index);

                rawReadBus (busList[index], line, opt);
                auto tobj = parentObject->find(busList[index]->getName());
                if (tobj==nullptr)
                {
                    parentObject->add (busList[index]);
                }
				            else
                {
                    auto prevName = busList[index]->getName ();
                    busList[index]->setName (prevName + '_' +
                                             std::to_string (busList[index]->getInt ("basevoltage")));
                    try
                    {
                        parentObject->add (busList[index]);
                    }
                    catch (const objectAddFailure &)
                    {
                        busList[index]->setName (prevName);
                        addToParentRename (busList[index], parentObject);
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
            moreData = false;
        }
    }

    stringVec txlines;
    txlines.resize (5);
    int tline = 5;

    bool moreSections = true;

    while (moreSections)
    {
        sections currSection = findSectionType (line);
        moreData = true;
        switch (currSection)
        {
        case load:
            while (moreData)
            {
                if (checkNextLine (file, line))
                {
                    bus = findBus (busList, line);
                    if (bus != nullptr)
                    {
                        ld = ldfactory->makeTypeObject ();
                        bus->add (ld);
                        rawReadLoad (ld, line, opt);
                    }
                    else
                    {
                        std::cerr << "Invalid bus number for load " << line.substr (0, 30) << '\n';
                    }
                }
                else
                {
                    moreData = false;
                }
            }
            break;
        case generator:
            while (moreData)
            {
                if (checkNextLine (file, line))
                {
                    bus = findBus (busList, line);
                    if (bus != nullptr)
                    {
                        gen = genfactory->makeTypeObject ();
                        bus->add (gen);
                        rawReadGen (gen, line, opt);
                    }
                    else
                    {
                        std::cerr << "Invalid bus number for fixed shunt " << line.substr (0, 30) << '\n';
                    }
                }
                else
                {
                    moreData = false;
                }
            }
            break;
        case branch:
            while (moreData)
            {
                if (checkNextLine (file, line))
                {
                    rawReadBranch (parentObject, line, busList, opt);
                }
                else
                {
                    moreData = false;
                }
            }
            break;
        case fixedShunt:
            while (moreData)
            {
                if (checkNextLine (file, line))
                {
                    bus = findBus (busList, line);
                    if (bus != nullptr)
                    {
                        ld = ldfactory->makeTypeObject ();
                        bus->add (ld);
                        rawReadFixedShunt (ld, line, opt);
                    }
                    else
                    {
                        std::cerr << "Invalid bus number for fixed shunt " << line.substr (0, 30) << '\n';
                    }
                }
                else
                {
                    moreData = false;
                }
            }
            break;
        case switchedShunt:
            while (moreData)
            {
                if (checkNextLine (file, line))
                {
                    rawReadSwitchedShunt (parentObject, line, busList, opt);
                }
                else
                {
                    moreData = false;
                }
            }
            break;
        case txadj:
            while (moreData)
            {
                if (checkNextLine (file, line))
                {
                    rawReadTXadj (parentObject, line, busList, opt);
                }
                else
                {
                    moreData = false;
                }
            }
            break;
        case tx:

            while (moreData)
            {
                if (tline == 5)
                {
                    if (checkNextLine (file, line))
                    {
                        txlines[0] = line;
                        std::getline (file, txlines[1]);
                        std::getline (file, txlines[2]);
                        std::getline (file, txlines[3]);
                        std::getline (file, txlines[4]);
                    }
                    else
                    {
                        moreData = false;
                    }
                }

                else
                {
                    temp1 = txlines[4];
                    trimString (temp1);
                    if (temp1[0] == '0')
                    {
                        moreData = false;
                        continue;
                    }
                    txlines[0] = temp1;
                    std::getline (file, txlines[1]);
                    std::getline (file, txlines[2]);
                    std::getline (file, txlines[3]);
                    std::getline (file, txlines[4]);
                }
                tline = rawReadTX (parentObject, txlines, busList, opt);
            }
            break;
        case unknown:
        default:
            while (moreData)
            {
                if (std::getline (file, line))
                {
                    trimString (line);
                    if (line[0] == '0')
                    {
                        moreData = false;
                        continue;
                    }
                }
                else
                {
                    moreData = false;
                    moreSections = false;
                }
            }
            break;
        }
    }

    file.close ();
}

int getPSSversion (const std::string &line)
{
    int ver = 29;
    auto slp = line.find_first_of ('/');
    if (slp == std::string::npos)
    {
        return ver;
    }

    auto sloc = line.find ("PSS", slp);
    if (sloc != std::string::npos)
    {
        auto dloc = line.find_first_of ('-', sloc + 3);
        auto sploc = line.find_first_of (' ', dloc);
        ver = std::stoi (line.substr (dloc + 1, sploc - dloc - 2));
    }
    else
    {
        sloc = line.find ("VER", slp);
        if (sloc != std::string::npos)
        {
            ver = std::stoi (line.substr (sloc + 3, 4));
            return ver;
        }
        sloc = line.find ("version", slp);
        if (sloc != std::string::npos)
        {
            ver = std::stoi (line.substr (sloc + 7, 4));
            return ver;
        }
    }
    return ver;
}

static const std::map<std::string, sections> sectionNames{
  {"BEGIN FIXED SHUNT", fixedShunt},
  {"BEGIN SWITCHED SHUNT DATA", switchedShunt},
  {"BEGIN AREA INTERCHANGE DATA", unknown},
  {"BEGIN TWO-TERMINAL DC LINE DATA", unknown},
  {"BEGIN TRANSFORMER IMPEDANCE CORRECTION DATA", unknown},
  { "BEGIN IMPEDANCE CORRECTION DATA", unknown },
  {"BEGIN MULTI-TERMINAL DC LINE DATA", unknown},
  {"BEGIN MULTI-SECTION LINE GROUP DATA", unknown},
  {"BEGIN ZONE DATA", unknown},
  {"BEGIN INTER-AREA TRANSFER DATA", unknown},
  {"BEGIN OWNER DATA", unknown},
  {"BEGIN FACTS CONTROL DEVICE DATA", unknown},
  {"BEGIN LOAD DATA", load},
  {"BEGIN GENERATOR DATA", generator},
  {"BEGIN BRANCH DATA", branch},
  {"BEGIN TRANSFORMER ADJUSTMENT DATA", txadj},
  {"BEGIN TRANSFORMER DATA", tx},
};


sections findSectionType (const std::string &line)
{
    for (auto &sname : sectionNames)
    {
        auto ts = line.find (sname.first);
        if (ts != std::string::npos)
        {
            return sname.second;
        }
		auto l2 = convertToUpperCase(line);
		ts = l2.find(sname.first);
		if (ts != std::string::npos)
		{
			return sname.second;
		}
    }
    return unknown;
}

void rawReadBus (gridBus *bus, const std::string &line, basicReaderInfo &opt)
{
    double bv;
    double vm;
    double va;
    int type;

    auto strvec = splitline (line);
    // get the bus name
    auto temp = trim (strvec[0]);
    auto temp2 = trim (removeQuotes (strvec[1]));

    if (opt.prefix.empty ())
    {
        if (temp2.empty ())  // 12 spaces is default value which would all get trimmed
        {
            temp2 = "BUS_" + temp;
        }
    }
    else
    {
        if (temp2.empty ())  // 12 spaces is default value which would all get trimmed
        {
            temp2 = opt.prefix + "_BUS_" + temp;
        }
        else
        {
            temp2 = opt.prefix + '_' + temp2;
        }
    }
    bus->setName (temp2);

    // get the localBaseVoltage
    bv = std::stod (strvec[2]);
    if (bv > 0.0)
    {
        bus->set ("basevoltage", bv);
    }

    // get the bus type
    if (strvec[3].empty ())
    {
        type = 1;
    }
    else
    {
        type = std::stoi (strvec[3]);
    }

    switch (type)
    {
    case 1:
        temp = "PQ";
        break;
    case 2:
        temp = "PV";
        break;
    case 3:
        temp = "swing";
        break;
    case 4:
        bus->disable ();
        temp = "PQ";
        break;
    default:
        bus->disable ();
    }
    bus->set ("type", temp);
    if (opt.version >= 31)
    {
        // skip the load flow area and loss zone for now
        // skip the owner information
        // get the voltage and angle specifications
        vm = numeric_conversion<double> (strvec[7], 0.0);
        va = numeric_conversion<double> (strvec[8], 0.0);
        if (strvec.size () > 10)
        {
            bv = numeric_conversion<double> (strvec[9], 0.0);
            bus->set ("vmax", bv);
            bv = numeric_conversion<double> (strvec[10], 0.0);
            bus->set ("vmin", bv);
        }
    }
    else
    {
        // get the zone information
        vm = numeric_conversion<double> (strvec[7], 0.0);
        bus->set ("zone", vm);

        vm = numeric_conversion<double> (strvec[8], 0.0);
        va = numeric_conversion<double> (strvec[9], 0.0);
        // load the fixed shunt data
        double p, q;

        p = numeric_conversion<double> (strvec[4], 0.0);
        q = numeric_conversion<double> (strvec[5], 0.0);
        if ((p != 0) || (q != 0))
        {
            auto ld = ldfactory->makeTypeObject ();
            bus->add (ld);
            if (p != 0.0)
            {
                ld->set ("yp", p, MW);
            }
            if (q != 0.0)
            {
                ld->set ("yq", -q, MVAR);
            }
        }
    }

    if (va != 0)
    {
        bus->set ("angle", va, deg);
    }
    if (vm != 0)
    {
        bus->set ("voltage", vm);
    }
}

void rawReadLoad (Load *ld, const std::string &line, basicReaderInfo & /*bri*/)
{
    auto strvec = splitline (line);

    // get the load index and name
    auto temp = trim (removeQuotes (strvec[1]));

    auto prefix = ld->getParent ()->getName () + "_load_" + temp;
    ld->setName (prefix);

    // get the status
    auto status = std::stoi (strvec[2]);
    if (status == 0)
    {
        ld->disable ();
    }
    // skip the area and zone information for now

    // get the constant power part of the load
    auto p = numeric_conversion<double> (strvec[5], 0.0);
    auto q = numeric_conversion<double> (strvec[6], 0.0);
    if (p != 0.0)
    {
        ld->set ("p", p, MW);
    }
    if (q != 0.0)
    {
        ld->set ("q", q, MVAR);
    }
    // get the constant current part of the load
    p = numeric_conversion<double> (strvec[7], 0.0);
    q = numeric_conversion<double> (strvec[8], 0.0);
    if (p != 0.0)
    {
        ld->set ("ip", p, MW);
    }
    if (q != 0.0)
    {
        ld->set ("iq", q, MVAR);
    }
    // get the impedance part of the load
    p = numeric_conversion<double> (strvec[9], 0.0);
    q = numeric_conversion<double> (strvec[10], 0.0);
    if (p != 0.0)
    {
        ld->set ("yp", p, MW);
    }
    if (q != 0.0)
    {
        ld->set ("yq", -q, MVAR);
    }
    // ignore the owner field
}

void rawReadFixedShunt (Load *ld, const std::string &line, basicReaderInfo & /*bri*/)
{
    auto strvec = splitline (line);

    // get the load index and name
    auto temp = trim (removeQuotes (strvec[1]));
    auto name = ld->getParent ()->getName () + "_shunt_" + temp;
    ld->setName (name);

    // get the status
    auto status = std::stoi (strvec[2]);
    if (status == 0)
    {
        ld->disable ();
    }
    // skip the area and zone information for now

    // get the constant power part of the load
    auto p = numeric_conversion<double> (strvec[3], 0.0);
    auto q = numeric_conversion<double> (strvec[4], 0.0);
    if (p != 0.0)
    {
        ld->set ("yp", p, MW);
    }
    if (q != 0.0)
    {
        ld->set ("yq", -q, MVAR);
    }
}

void rawReadGen (Generator *gen, const std::string &line, basicReaderInfo &opt)
{
    auto strvec = splitline (line);

    // get the load index and name
    auto temp = trim (removeQuotes (strvec[1]));

    auto prefix = gen->getParent ()->getName () + "_Gen_" + temp;
    gen->setName (prefix);
    // get the status
    auto status = std::stoi (strvec[14]);
    if (status == 0)
    {
        gen->disable ();
    }

    // get the power generation
    auto p = numeric_conversion<double> (strvec[2], 0.0);
    auto q = numeric_conversion<double> (strvec[3], 0.0);
    if (p != 0.0)
    {
        gen->set ("p", p, MW);
    }
    if (q != 0.0)
    {
        gen->set ("q", q, MVAR);
    }
    // get the Qmax and Qmin
    auto qmax = numeric_conversion<double> (strvec[4], 0.0);
    auto qmin = numeric_conversion<double> (strvec[5], 0.0);
    if (qmax != 0.0)
    {
        gen->set ("qmax", qmax, MW);
    }
    if (qmin != 0.0)
    {
        gen->set ("qmin", qmin, MVAR);
    }
    auto V = numeric_conversion<double> (strvec[6], 0.0);
    if (V > 0)
    {
        double vp = gen->getParent ()->get ("vtarget");
        if (std::abs (vp - V) > 0.0001)
        {
            gen->set ("vtarget", V);
            // for raw files the bus doesn't necessarily set a control point it comes from the generator, so we
            // have to set it here.
			if (!opt.checkFlag(no_generator_bus_voltage_reset))
			{
				gen->getParent()->set("vtarget", V);
				gen->getParent()->set("voltage", V);
			}
        }
        else
        {
            gen->set ("vtarget", vp);
        }
    }
    auto rbus = numeric_conversion<int> (strvec[7], 0);

    if (rbus != 0)
    {
        gridBus *remoteBus = static_cast<gridBus *> (gen->getParent ()->getParent ()->findByUserID ("bus", rbus));
        gen->add (remoteBus);
    }

    auto mb = numeric_conversion<double> (strvec[8], 0.0);
    gen->set ("mbase", mb);

    auto r = numeric_conversion<double> (strvec[9], 0.0);
    gen->set ("rs", r);

    auto x = numeric_conversion<double> (strvec[10], 0.0);
    gen->set ("xs", x);

    if (!opt.checkFlag(ignore_step_up_transformer))
    {
        r = numeric_conversion<double> (strvec[11], 0.0);
        x = numeric_conversion<double> (strvec[12], 0.0);
        if ((r != 0) || (x != 0))  // need to add a step up transformer
        {
            auto oBus = static_cast<gridBus *> (gen->find("bus"));
            gridBus *nBus = busfactory->makeTypeObject ();
            auto lnk =
              new acLine (r * opt.base / mb,
                          x * opt.base /
                            mb);  // we need to adjust to the simulation base as opposed to the machine base

            oBus->remove (gen);
            nBus->add (gen);

            lnk->updateBus (oBus, 1);
            lnk->updateBus (nBus, 2);

            if (!gen->isEnabled ())
            {
                nBus->disable ();
            }
            if (!oBus->isEnabled ())
            {
                nBus->disable ();
            }
            oBus->getParent ()->add (nBus);
            oBus->getParent ()->add (lnk);
            if (gen->getName ().compare (0, oBus->getName ().length (), oBus->getName ()) == 0)
            {
                lnk->setName (gen->getName () + "_TX");
                nBus->setName (gen->getName () + "_TXBUS");
            }
            else
            {
                lnk->setName (oBus->getName () + '_' + gen->getName () + "_TX");
                nBus->setName (oBus->getName () + '_' + gen->getName () + "_TXBUS");
            }
            // get the tap ratio
            r = numeric_conversion<double> (strvec[13], 0.0);
            lnk->set ("tap", r);
            // match the voltage and angle of the other bus
            nBus->setVoltageAngle (oBus->getVoltage () * r, oBus->getAngle ());
            gen->add (oBus);
            // get the power again for the generator
            p = numeric_conversion<double> (strvec[2], 0.0);
            q = numeric_conversion<double> (strvec[3], 0.0);
            // now adjust the newBus angle and Voltage to match the power flows
            lnk->fixPower (-p, -q, 1, 1, MVAR);
            if (!gen->isEnabled ())
            {
                nBus->disable ();
            }
        }
    }
}

auto generateBranchName (const stringVector &strvec,
                         const std::vector<gridBus *> &busList,
                         const std::string &prefix,
                         int cctIndex = -1)
{
    int ind1 = std::stoi (trim (strvec[0]));

    int ind2 = std::stoi (trim (strvec[1]));

    // negative bus number indicates direction of measurement in PSS/E this is irrelevent in GridDyn since it can
    // do both directions
    if (ind2 < 0)
    {
        // int tmp=ind1;
        ind2 = abs (ind2);
        // ind2 = tmp;
    }

    if ((ind1 < 0) || (ind2 < 0) || (ind1 >= static_cast<int> (busList.size ())) ||
        (ind2 >= static_cast<int> (busList.size ())))
    {
        std::cerr << "invalid link buses\n";
        assert (false);
    }
    // create the bus name
    std::string name;
    if (prefix.empty ())
    {
        name = busList[ind1]->getName () + "_to_" + busList[ind2]->getName ();
    }
    else
    {
        name = prefix + '_' + busList[ind1]->getName () + "_to_" + busList[ind2]->getName ();
    }
    if (cctIndex >= 0)
    {
        auto temp = removeQuotes (strvec[cctIndex]);
        trimString (temp);
        if (temp != "1")
        {
            name.push_back ('_');
            name += temp;
        }
    }

    return std::make_tuple (name, ind1, ind2);
}

void rawReadBranch (coreObject *parentObject,
                    const std::string &line,
                    std::vector<gridBus *> &busList,
                    basicReaderInfo &opt)
{
    auto strvec = splitline (line);

    std::string name;
    int ind1, ind2;
    std::tie (name, ind1, ind2) = generateBranchName (strvec, busList, opt.prefix, 2);

    acLine *lnk = linkfactory->makeTypeObject (name);
    // set the base power to that used this model
    lnk->set ("basepower", opt.base);

    lnk->updateBus (busList[ind1], 1);
    lnk->updateBus (busList[ind2], 2);

    // check for circuit identifier

    try
    {
        parentObject->add (lnk);
    }
    catch (const objectAddFailure &)
    {
        // must be a parallel branch
        std::string sub = lnk->getName ();
        char m = 'a';
        while (lnk->isRoot ())
        {
            lnk->setName (sub + '_' + m);
            m = m + 1;
            try
            {
                parentObject->add (lnk);
            }
            catch (const objectAddFailure &e)
            {
                if (m > 'z')
                {
                    throw (e);
                }
            }
        }
    }

    auto R = numeric_conversion<double> (strvec[3], 0.0);
    auto X = numeric_conversion<double> (strvec[4], 0.0);
    // get line impedances and resistance
    lnk->set ("r", R);
    lnk->set ("x", X);
    // get line capacitance
    auto val = numeric_conversion<double> (strvec[5], 0.0);
    lnk->set ("b", val);

    int status;
    if (opt.version >= 29)
    {
        status = std::stoi (strvec[13]);
        if (status == 0)
        {
            lnk->disable ();
        }
    }
    else
    {
        status = std::stoi (strvec[15]);
        if (status == 0)
        {
            lnk->disable ();
        }
    }
    if (opt.version <= 26)  // transformers described in this section and in TX adj section
    {
        val = numeric_conversion<double> (strvec[9], 0.0);
        if (val != 0.0)
        {
            lnk->set ("tap", val);
            val = numeric_conversion<double> (strvec[10], 0.0);
            if (val != 0)
            {
                lnk->set ("tapAngle", val, deg);
            }
        }
    }

    // skip the load flow area and loss zone and circuit for now

    // get the branch impedance

    // TODO get the other parameters (not critical for power flow)
}

void rawReadTXadj (coreObject *parentObject,
                   const std::string &line,
                   std::vector<gridBus *> &busList,
                   basicReaderInfo &opt)
{
    // int status;

    auto strvec = splitline (line);

    std::string name;
    int ind1, ind2;
    std::tie (name, ind1, ind2) =
      generateBranchName (strvec, busList, (opt.prefix.empty ()) ? "tx_" : opt.prefix + "_tx_");

    auto lnk = static_cast<acLine *> (parentObject->find (name));

    if (lnk == nullptr)
    {
        parentObject->log (parentObject, print_level::error, "unable to locate link " + name);
        return;
    }

    auto adjTX = new links::adjustableTransformer ();
    lnk->clone (adjTX);
    lnk->addOwningReference ();
    parentObject->remove (lnk);
    adjTX->updateBus (lnk->getBus (1), 1);
    adjTX->updateBus (lnk->getBus (2), 2);
    lnk->updateBus (nullptr, 1);
    lnk->updateBus (nullptr, 2);
    removeReference (lnk);
    parentObject->add (adjTX);
    auto Ta = adjTX->getTapAngle ();
    int code;
    if (Ta != 0)
    {
        adjTX->set ("mode", "mw");
        adjTX->set ("stepmode", "continuous");
        code = 3;
    }
    else
    {
        adjTX->set ("mode", "voltage");
        code = 1;
    }
    // get the control bus
    if (code != 3)
    {
        auto cind = numeric_conversion<int> (strvec[3], 0);
        if (cind > 0)
        {
            if (cind == ind1)
            {
                adjTX->setControlBus (1);
            }
            else if (cind == ind2)
            {
                adjTX->setControlBus (2);
            }
            else
            {
                adjTX->setControlBus (static_cast<gridBus *> (adjTX->getParent ()->findByUserID ("bus", cind)));
            }
        }
        else
        {
            if (-cind == ind1)
            {
                adjTX->setControlBus (1);
            }
            else if (-cind == ind2)
            {
                adjTX->setControlBus (2);
            }
            else
            {
                adjTX->setControlBus (static_cast<gridBus *> (adjTX->getParent ()->findByUserID ("bus", -cind)));
                adjTX->set ("direction", -1);
            }
        }
    }
    //
    auto mx = numeric_conversion<double> (strvec[4], 0.0);
    auto mn = numeric_conversion<double> (strvec[5], 0.0);
    if ((mx - mn > 1.0) && (code != 3))
    {
        adjTX->set ("mode", "mw");
        adjTX->set ("stepmode", "continuous");
        code = 3;
    }
    if (code == 3)
    {
        // not sure why I need this but
        Ta = Ta * 180 / kPI;
        if (Ta > mx)
        {
            mx = Ta;
        }
        if (Ta < mn)
        {
            mn = Ta;
        }
        adjTX->set ("maxtapangle", mx, deg);
        adjTX->set ("mintapangle", mn, deg);
    }
    else
    {
        if (mx < mn)
        {
            std::swap (mx, mn);
        }
        adjTX->set ("maxtap", mx);
        adjTX->set ("mintap", mn);
    }
    mx = numeric_conversion<double> (strvec[6], 0.0);
    mn = numeric_conversion<double> (strvec[7], 0.0);
    if ((mx - mn > 1.0) && (code == 1))
    {
        adjTX->set ("mode", "mvar");
        code = 2;
    }
    if (code == 1)
    {
        if (mx - mn > 0.00001)
        {
            adjTX->set ("vmax", mx);
            adjTX->set ("vmin", mn);
        }
    }
    else if (code == 3)
    {
        if (mx - mn > 0.00001)
        {
            adjTX->set ("pmax", mx, MW);
            adjTX->set ("pmin", mn, MW);
        }
    }
    else
    {
        if (mx - mn > 0.00001)
        {
            adjTX->set ("qmax", mx, MVAR);
            adjTX->set ("qmin", mn, MVAR);
        }
    }
    if (code != 3)  // get the stepsize
    {
        auto val = numeric_conversion<double> (strvec[8], 0.0);
        if (val != 0)
        {
            // abs required since for some reason the file can have negative step sizes
            // I think just to do reverse indexing which I don't do.
            adjTX->set ("step", std::abs (val));
        }
        else
        {
            adjTX->set ("stepmode", "continuous");
        }
    }
    auto cind = numeric_conversion<int> (strvec[9], 0);
    if (cind != 0)
    {
        parentObject->log (parentObject, print_level::warning,
                           "transformer impedance tables not implemented yet ");
    }
    cind = numeric_conversion<int> (strvec[10], 0);
    {
        if (cind == 0)
        {
            adjTX->set ("no_pflow_adjustments", 1);
        }
    }
    mx = numeric_conversion<double> (strvec[11], 0.0);
    mn = numeric_conversion<double> (strvec[12], 0.0);
    if ((mx != 0) || (mn != 0))
    {
        parentObject->log (parentObject, print_level::warning, "load drop compensation not implemented yet ");
    }
}

int rawReadTX (coreObject *parentObject, stringVec &txlines, std::vector<gridBus *> &busList, basicReaderInfo &opt)
{
    // gridBus *bus3;
    acLine *lnk = nullptr;

    stringVec strvec5;
    auto strvec = splitline (txlines[0]);

    auto strvec2 = splitline (txlines[1]);
    auto strvec3 = splitline (txlines[2]);
    auto strvec4 = splitline (txlines[3]);

    std::string name;
    int ind1, ind2;
    std::tie (name, ind1, ind2) =
      generateBranchName (strvec, busList, (opt.prefix.empty ()) ? "tx_" : opt.prefix + "_tx_",3);

    int ind3 = std::stoi (strvec[2]);
    int tline = 4;
    if (ind3 != 0)
    {
        tline = 5;
        strvec5 = splitline (txlines[4]);
        // TODO handle 3 way transformers(complicated)
        std::cout << "3 winding transformers not supported at this time\n";
        return tline;
    }

    auto bus1 = busList[ind1];
    auto bus2 = busList[ind2];
    int code = std::stoi (strvec3[6]);
    switch (abs (code))
    {
    case 0:
    default:
        lnk = linkfactory->makeTypeObject (name);
        break;
    case 1:
        lnk = new links::adjustableTransformer (name);
        lnk->set ("mode", "voltage");
        break;
    case 2:
        lnk = new links::adjustableTransformer (name);
        lnk->set ("mode", "mvar");
        break;
    case 3:
        lnk = new links::adjustableTransformer (name);
        lnk->set ("mode", "mw");
        break;
    }
    if (code < 0)  // account for negative code values
    {
        lnk->set ("mode", "manual");
    }
    lnk->set ("basepower", opt.base);
    lnk->updateBus (bus1, 1);
    lnk->updateBus (bus2, 2);

    try
    {
        parentObject->add (lnk);
    }
    catch (const objectAddFailure &)
    {
        // must be a parallel branch
        auto &sub = lnk->getName ();
        char m = 'a';
        while (lnk->isRoot ())
        {
            lnk->setName (sub + '_' + m);
            m = m + 1;
            try
            {
                parentObject->add (lnk);
            }
            catch (const objectAddFailure &e)
            {
                if (m > 'z')
                {
                    throw (e);
                }
            }
        }
    }

    // skip the load flow area and loss zone and circuit for now

    // get the branch impedance

    auto R = numeric_conversion<double> (strvec2[0], 0.0);
    auto X = numeric_conversion<double> (strvec2[1], 0.0);

    lnk->set ("r", R);
    lnk->set ("x", X);
    // get line capacitance

    auto status = std::stoi (strvec[11]);
    if (status == 0)
    {
        lnk->disable ();
    }
    else if (status > 1)
    {
        // TODO:  other conditions for 3 way transformers
    }

    // TODO:PT get the other parameters (not critical for power flow)

    auto val = numeric_conversion<double> (strvec3[0], 0.0);
    if (val != 0)
    {
        lnk->set ("tap", val);
    }
    val = numeric_conversion<double> (strvec3[2], 0.0);
    if (val != 0)
    {
        lnk->set ("tapangle", val, deg);
    }
    // now get the stuff for the adjustable transformers
    // SGS set this for adjustable transformers....is this correct?
    if (abs (code) > 0)
    {
        auto cbus = numeric_conversion<int> (strvec3[7], 0);
        if (cbus != 0)
        {
            static_cast<links::adjustableTransformer *> (lnk)->setControlBus (busList[cbus]);
        }
        R = numeric_conversion<double> (strvec3[8], 0.0);
        X = numeric_conversion<double> (strvec3[9], 0.0);
        if (code == 3)
        {
            lnk->set ("maxtapangle", R, deg);
            lnk->set ("mintapangle", X, deg);
        }
        else
        {
            lnk->set ("maxtap", R);
            lnk->set ("mintap", X);
        }
        R = numeric_conversion<double> (strvec3[10], 0.0);
        X = numeric_conversion<double> (strvec3[11], 0.0);
        if (code == 3)
        {
            lnk->set ("pmax", R, MW);
            lnk->set ("pmin", X, MW);
        }
        else if (code == 2)
        {
            lnk->set ("qmax", R, MVAR);
            lnk->set ("qmin", X, MVAR);
        }
        else
        {
            lnk->set ("vmax", R);
            lnk->set ("vmin", X);
        }
        R = numeric_conversion<double> (strvec3[12], 0.0);
        if (code != 3)
        {
            lnk->set ("nsteps", R);
        }
    }
    return tline;
}

int rawReadDCLine (coreObject * /*parentObject*/,
                   stringVec & /*txlines*/,
                   std::vector<gridBus *> & /*busList*/,
                   basicReaderInfo & /*bri*/)
{
    return 0;
}

void rawReadSwitchedShunt (coreObject *parentObject,
                           const std::string &line,
                           std::vector<gridBus *> &busList,
                           basicReaderInfo &opt)
{
    auto strvec = splitline (line);

    auto index = std::stoul (strvec[0]);
    gridBus *rbus = nullptr;
    loads::svd *ld = nullptr;
    double temp;
    if (index > busList.size ())
    {
        std::cerr << "Invalid bus number for load " << index << '\n';
    }
    if (busList[index] == nullptr)
    {
        std::cerr << "Invalid bus number for load " << index << '\n';
    }
    else
    {
        ld = new loads::svd ();
        busList[index]->add (ld);
    }

    auto mode = numeric_conversion<int> (strvec[1], 0);
    int shift = 0;
    if (opt.version > 32)
    {
		shift = 2;
    }
    auto high = numeric_conversion<double> (strvec[2+shift], 0.0);
    auto low = numeric_conversion<double> (strvec[3+shift], 0.0);
    // get the controlled bus
    auto cbus = numeric_conversion<int> (strvec[4+shift], -1);

    if (cbus < 0)
    {
        trimString (strvec[4+shift]);
        if (strvec[4+shift] == "I")
        {
            cbus = index;
        }
        else if (strvec[4+shift].empty ())
        {
            cbus = index;
        }
        else
        {
            rbus = static_cast<gridBus *> (parentObject->find (strvec[4+shift]));
            if (rbus != nullptr)
            {
                cbus = rbus->getUserID ();
            }
        }
    }
    else if (cbus == 0)
    {
        cbus = index;
    }
    else
    {
        rbus = busList[cbus];
    }

    switch (mode)
    {
    case 0:
        ld->set ("mode", "manual");
        break;
    case 1:
        ld->set ("mode", "stepped");
        ld->set ("vmax", high);
        ld->set ("vmin", low);
        if (cbus != static_cast<int> (index))
        {
            ld->setControlBus (rbus);
        }

        temp = numeric_conversion<double> (strvec[5+shift], 0.0);
        if (temp > 0)
        {
            ld->set ("participation", temp / 100.0);
        }
        break;
    case 2:
        ld->set ("mode", "cont");
        ld->set ("vmax", high);
        ld->set ("vmin", low);
        if (cbus != static_cast<int> (index))
        {
            ld->setControlBus (rbus);
        }
        temp = numeric_conversion<double> (strvec[5+shift], 0.0);
        if (temp > 0)
        {
            ld->set ("participation", temp / 100.0);
        }
        break;
    case 3:
        ld->set ("mode", "stepped");
        ld->set ("control", "reactive");
        ld->set ("qmax", high);
        ld->set ("qmin", low);
        if (cbus != static_cast<int> (index))
        {
            ld->setControlBus (rbus);
        }
        break;
    case 4:
        ld->set ("mode", "stepped");
        ld->set ("control", "reactive");
        ld->set ("qmax", high);
        ld->set ("qmin", low);
        if (cbus != static_cast<int> (index))
        {
            ld->setControlBus (rbus);
        }
        // TODO: PT load target object note:unusual condition
        break;
    case 5:
        ld->set ("mode", "stepped");
        ld->set ("control", "reactive");
        ld->set ("qmax", high);
        ld->set ("qmin", low);
        if (cbus != static_cast<int> (index))
        {
            ld->setControlBus (rbus);
        }
        break;
    case 6:
        ld->set ("mode", "stepped");
        ld->set ("control", "reactive");
        ld->set ("qmax", high);
        ld->set ("qmin", low);
        if (cbus != static_cast<int> (index))
        {
            ld->setControlBus (rbus);
        }
        // TODO: PT load target object note:unusual condition
        break;
    default:
        ld->set ("mode", "manual");
        break;
    }
    // load the switched shunt blocks
    int start = 7;
    if (opt.version <= 27)
    {
        start = 5;
    }
    else if (opt.version > 32)
    {
        start = 9;
    }
    size_t ksize = strvec.size () - 1;
    for (size_t kk = start + 1; kk < ksize; kk += 2)
    {
        auto cnt = numeric_conversion<int> (strvec[kk], 0);
        auto block = numeric_conversion<double> (strvec[kk + 1], 0.0);
        if ((cnt > 0) && (block != 0.0))
        {
            ld->addBlock (cnt, -block, MVAR);
        }
        else
        {
            break;
        }
    }
    // set the initial value
    auto initVal = numeric_conversion<double> (strvec[start], 0.0);

    ld->set ("yq", -initVal, MVAR);
}

}  // namespace griddyn
