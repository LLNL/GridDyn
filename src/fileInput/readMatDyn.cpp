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

#include "fileInput.h"
#include "gmlc/utilities/string_viewConversion.h"
#include "griddyn/Governor.h"
#include "griddyn/Link.h"
#include "griddyn/events/Event.h"
#include "griddyn/exciters/ExciterDC1A.h"
#include "griddyn/generators/DynamicGenerator.h"
#include "griddyn/genmodels/GenModel4.h"
#include "griddyn/gridBus.h"
#include "griddyn/gridDynSimulation.h"
#include "griddyn/loads/zipLoad.h"
#include "readerHelper.h"
#include <cstdlib>
#include <iostream>
#include <memory>

namespace griddyn {
using namespace units;
using namespace gmlc::utilities::string_viewOps;
using gmlc::utilities::string_view;
using namespace gmlc::utilities;

void loadGenExcArray(coreObject* parentObject, mArray& excData, std::vector<Generator*>& genList);
void loadGenDynArray(coreObject* parentObject, mArray& genData, std::vector<Generator*>& genList);
void loadGenGovArray(coreObject* parentObject, mArray& govData, std::vector<Generator*>& genList);

void loadMatDyn(coreObject* parentObject,
                const std::string& filetext,
                const basicReaderInfo& /*bri*/)
{
    string_view ftext = filetext;
    mArray M1;

    std::vector<Generator*> genList;
    // read the frequency
    size_t A = ftext.find_first_of('[', 0);
    size_t B = ftext.find_first_of(']', 0);
    auto tstr = ftext.substr(A + 1, B - A - 1);
    auto Tline = split(tstr, "\t ,", delimiter_compression::on);

    size_t D = B;
    size_t C;
    A = ftext.find(Tline[3], D);  // freq
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        C = ftext.find_first_of(";\n", A);
        tstr = ftext.substr(B + 1, C - B - 1);
        double freq = numeric_conversion(tstr, kNullVal);
        parentObject->set("basefreq", freq);
    }
    // get the timestep parameter
    A = ftext.find(Tline[4], D);  // steptime
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        C = ftext.find_first_of(";\n", A);
        tstr = ftext.substr(B + 1, C - B - 1);
        double val = numeric_conversion(tstr, kNullVal);
        parentObject->set("timestep", val);
    }
    // get the stoptime parameter
    A = ftext.find(Tline[5], D);  // stoptime
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        C = ftext.find_first_of(";\n", A);
        tstr = ftext.substr(B + 1, C - B - 1);
        double val = numeric_conversion(tstr, kNullVal);
        parentObject->set("timestop", val);
    }
    A = ftext.find(Tline[0], D);  // gen
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        readMatlabArray(filetext, B + 1, M1);
        loadGenDynArray(parentObject, M1, genList);
    }

    A = ftext.find(Tline[1], D);  // exc
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        readMatlabArray(filetext, B + 1, M1);
        loadGenExcArray(parentObject, M1, genList);
    }

    A = ftext.find(Tline[2], D);  // gov
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        readMatlabArray(filetext, B + 1, M1);
        loadGenGovArray(parentObject, M1, genList);
    }
    A = 1;
    for (auto& ngen : genList) {
        auto gen =
            static_cast<Generator*>(parentObject->findByUserID("gen", static_cast<index_t>(A)));
        A++;
        if (gen == nullptr) {
            std::cout
                << "the number of generators does not match the matdyn file please run with matching "
                   "matpower file first\n";
            return;
        }
        // now we load the existing components of our generator onto the existing one
        coreObject* obj = ngen->getSubObject("exciter", 0);
        if (obj != nullptr) {
            gen->add(obj);
        }

        obj = ngen->getSubObject("genmodel", 0);
        if (obj != nullptr) {
            gen->add(obj);
        }

        obj = ngen->getSubObject("governor", 0);
        if (obj != nullptr) {
            gen->add(obj);
        }
    }
    // now delete the temporary generators the subobjects should have transferred ownership
    for (auto g : genList) {
        delete g;
    }
    // lastly all the loads need to be autoconverted
    index_t b = static_cast<index_t>(parentObject->get("loadcount"));
    for (index_t kk = 1; kk <= b; kk++) {
        auto ld = static_cast<zipLoad*>(parentObject->findByUserID("load", kk));
        ld->set("converttoimpedance", 1);
    }
}

void loadGenDynArray(coreObject* /*parentObject*/,
                     mArray& genData,
                     std::vector<Generator*>& genList)
{
    Exciter* exc;
    Governor* gov;
    GenModel* gm;

    /*[genmodel excmodel govmodel H D xd xq xd_tr xq_tr Td_tr Tq_tr]*/
    for (const auto& genLine : genData) {
        auto gen = new DynamicGenerator();
        switch (static_cast<int>(genLine[0])) {
            case 1:  // classical model
                gm = new genmodels::GenModelClassical();
                gm->set("h", genLine[3]);
                gm->set("d", genLine[4]);
                gm->set("x", genLine[5]);
                gm->set("xp", genLine[6]);
                gen->add(gm);
                break;
            case 2:  // fourth order model
                gm = new genmodels::GenModel4();
                gm->set("h", genLine[3]);
                gm->set("d", genLine[4]);
                gm->set("xd", genLine[5]);
                gm->set("xq", genLine[6]);
                gm->set("xdp", genLine[7]);
                gm->set("xqp", genLine[8]);
                gm->set("tdp", genLine[9]);
                gm->set("tqp", genLine[10]);
                gen->add(gm);
                break;
            default:
                std::cout << "unknown genmodel code in gen matrix\n";
                break;
        }
        // now run through the different exciter models
        switch (static_cast<int>(genLine[1]))  // switch on exciter model
        {
            case 1:  // constant excitation means no exciter
                break;
            case 2:  // DC1A exciter
                exc = new exciters::ExciterDC1A();
                gen->add(exc);
                break;
            case 3:  // AC4A exciter  NOT IMPLEMENTED YET
            default:
                std::cout << "unknown exciter code in gen matrix\n";
                break;
        }
        // now run through the different governor models
        switch (static_cast<int>(genLine[2]))  // switch on exciter model
        {
            case 1:  // constant mechanical power
                break;
            case 2:
                gov = new Governor();
                gen->add(gov);
                break;
            default:
                std::cout << "unknown governor code in gen matrix\n";
                break;
        }
        genList.push_back(gen);
    }
}

/*
1 gen, number of the generator
2 Ka, amplifier gain
3 Ta, amplifier time constant
4 Ke, exciter gain
5 Te, exciter time constant
6 Kf , stabilizer gain
7 Tf , stabilizer time constant
8 Aex, parameter saturation function
9 Bex, parameter saturation function
10 Urmin, lower voltage limit
11 Urmax, upper voltage limit
*/
void loadGenExcArray(coreObject* /*parentObject*/,
                     mArray& excData,
                     std::vector<Generator*>& genList)
{
    /*[genmodel excmodel govmodel H D xd xq xd_tr xq_tr Td_tr Tq_tr]*/
    for (const auto& excLine : excData) {
        Exciter* exc = nullptr;
        auto ind1 = static_cast<index_t>(excLine[0]);

        if (isValidIndex(ind1, genList)) {
            Generator* gen = genList[ind1 - 1];  // zero based in C vs 1 based in matlab
            exc = static_cast<Exciter*>(gen->getSubObject("exciter", 0));
        }
        if (exc == nullptr) {
            continue;
        }
        // this means it is the DC1A exciter
        exc->set("ka", excLine[1]);
        exc->set("ta", excLine[2]);
        exc->set("ke", excLine[3]);
        exc->set("te", excLine[4]);
        exc->set("kf", excLine[5]);
        exc->set("tf", excLine[6]);
        exc->set("aex", excLine[7]);
        exc->set("bex", excLine[8]);
        exc->set("urmin", excLine[9]);
        exc->set("urmax", excLine[10]);
    }
}
/*
1 gen, number of the generator
2 K, droop
3 T1, time constant
4 T2, time constant
5 T3, servo motor time constant
6 Pup, upper ramp limit
7 Pdown, lower ramp limit
8 Pmax, maximal turbine output
9 Pmin, minimal turbine output
*/
void loadGenGovArray(coreObject* /*parentObject*/,
                     mArray& govData,
                     std::vector<Generator*>& genList)
{
    /*[genmodel excmodel govmodel H D xd xq xd_tr xq_tr Td_tr Tq_tr]*/
    for (const auto& govLine : govData) {
        Governor* gov = nullptr;
        Generator* gen = nullptr;
        auto ind1 = static_cast<index_t>(govLine[0]);
        if (isValidIndex(ind1, genList)) {
            gen = genList[ind1 - 1];  // zero based in C vs 1 based in matlab
            if (gen == nullptr) {
                // probably throw some sort of error here
                continue;
            }
            gov = static_cast<Governor*>(gen->getSubObject("governor", 0));
        }
        if (gov == nullptr) {
            continue;
        }

        gov->set("k", govLine[1]);
        gov->set("t1", govLine[2]);
        gov->set("t2", govLine[3]);
        gov->set("t3", govLine[4]);
        gov->set("pup", govLine[5]);
        gov->set("pdown", govLine[6]);
        // these next two should be set at the generator, they then filter down to the governor
        gen->set("pmax", govLine[7]);
        gen->set("pmin", govLine[8]);
    }
}

// read matdyn Event files
void loadMatDynEvent(coreObject* parentObject,
                     const std::string& filetext,
                     const basicReaderInfo& /*bri*/)
{
    string_view ftext = filetext;
    mArray event1, M1;
    auto gds = dynamic_cast<gridSimulation*>(parentObject->getRoot());
    if (gds == nullptr) {  // can't make events if we don't have access to the simulation
        return;
    }
    // read the frequency
    size_t A = ftext.find_first_of('[', 0);
    size_t B = ftext.find_first_of(']', 0);
    auto tstr = ftext.substr(A + 1, B - A - 1);
    auto Tline = split(tstr, "\t ,");
    size_t C = B;
    A = ftext.find(Tline[0], C);  // event
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        readMatlabArray(filetext, B + 1, event1);
        // loadGenDynArray(parentObject, M1, genList);
    }

    A = ftext.find(Tline[1], C);  // buschange
    if (A != string_view::npos) {
        B = ftext.find_first_of('=', A);
        readMatlabArray(filetext, B + 1, M1);
        for (auto& eventSpec : M1) {
            auto evnt = std::make_shared<Event>(eventSpec[0]);
            auto ind = static_cast<index_t>(eventSpec[1]);
            auto bus = static_cast<gridBus*>(parentObject->findByUserID("bus", ind));
            auto ld = bus->getLoad();
            if (ld == nullptr) {
                ld = new zipLoad();
                bus->add(ld);
            }
            switch (static_cast<int>(eventSpec[2])) {
                case 3:  // P
                    evnt->setTarget(ld, "p");
                    evnt->setValue(eventSpec[3], MW);
                    break;
                case 4:  // Q
                    evnt->setTarget(ld, "q");
                    evnt->setValue(eventSpec[3], MVAR);
                    break;
                case 5:  // GS
                    evnt->setTarget(ld, "yp");
                    evnt->setValue(eventSpec[3], MW);
                    break;
                case 6:  // BS
                    evnt->setTarget(ld, "yq");
                    evnt->setValue(-eventSpec[3], MW);
                    break;
                default:
                    break;
            }
            gds->add(std::move(evnt));
        }
        //    loadGenExcArray(parentObject, M1, genList);
    }

    A = ftext.find(Tline[2], C);  // linechange
    if (A != std::string::npos) {
        B = ftext.find_first_of('=', A);
        readMatlabArray(filetext, B + 1, M1);
        for (const auto& lc : M1) {
            auto evnt = std::make_shared<Event>(lc[0]);

            auto ind = static_cast<index_t>(lc[1]);
            auto lnk = static_cast<Link*>(parentObject->findByUserID("link", ind));
            switch (static_cast<int>(lc[2])) {
                case 3:  // r
                    evnt->setTarget(lnk, "r");
                    evnt->setValue(lc[3]);
                    break;
                case 4:  // X
                    evnt->setTarget(lnk, "x");
                    evnt->setValue(lc[3]);
                    break;
                case 5:  // B
                    evnt->setTarget(lnk, "b");
                    evnt->setValue(lc[3], MW);
                    break;
                case 9:  // tap
                    evnt->setTarget(lnk, "tap");
                    evnt->setValue(lc[3]);
                    break;
                case 10:  // BS
                    evnt->setTarget(lnk, "tapangle");
                    evnt->setValue(lc[3], deg);
                    break;
                case 11:  // BS
                    evnt->setTarget(lnk, "enable");
                    evnt->setValue(lc[3]);
                    break;
                default:
                    break;
            }
            gds->add(std::move(evnt));
        }
    }
}

}  // namespace griddyn
