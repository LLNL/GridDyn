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

#include "fmuBuilder.h"
#include "fileInput/fileInput.h"
#include "fmiCollector.h"
#include "fmiCoordinator.h"
#include "fmiEvent.h"
#include "griddyn/gridDynSimulation.h"
#include "loadFMIExportObjects.h"
#include "tinyxml2/tinyxml2.h"
#include "utilities/stringOps.h"
#include "utilities/zipUtilities.h"
#include <iostream>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#ifndef GRIDDYNFMILIBRARY_BINARY_LOC
#define GRIDDYNFMILIBRARY_BINARY_LOC ""
#endif

#ifndef FMILIBRARY_TYPE
#define FMILIBRARY_TYPE "unknown"
#endif

namespace griddyn
{
namespace fmi
{
fmuBuilder::fmuBuilder () { loadComponents (); }

fmuBuilder::fmuBuilder (std::shared_ptr<gridDynSimulation> gds) : GriddynRunner (std::move(gds)) { loadComponents (); }

void fmuBuilder::loadComponents ()
{
    coord_ = make_owningPtr<fmiCoordinator> ();
    auto gds = getSim ();
    if (gds == nullptr)
    {
        resetSim (std::make_shared<gridDynSimulation> ());
        gds = getSim ();
    }
    gds->add (coord_.get ());
    ri_ = std::make_unique<readerInfo> ();
    loadFmiExportReaderInfoDefinitions (*ri_);
    ri_->captureFiles = true;
}

fmuBuilder::~fmuBuilder () = default;

static const std::set<std::string> valid_platforms{"all",   "windows", "linux",   "macos",    "darwin",  "win32",
                                                   "win64", "linux32", "linux64", "darwin32", "darwin64"};
int fmuBuilder::Initialize (int argc, char *argv[])
{
    // using namespace boost;
    namespace po = boost::program_options;

    if (argc > 0)
    {
        execPath = std::string (argv[0]);
    }

    po::variables_map fmiOptions;

    po::options_description fmiOp ("fmi options");
    // clang-format off
    fmiOp.add_options()
        ("buildfmu", po::value < std::string >(), "fmu file to build")
        ("platform", po::value < std::string >(), "build the fmu for a specific platform win64,win32,linux64,linux32,darwin32,darwin64")
        ("keep_dir","keep the temporary directory after building")
        ("help", "display the help messages");
    // clang-format on
    auto parsed = po::command_line_parser (argc, argv).options (fmiOp).allow_unregistered ().run ();
    po::store (parsed, fmiOptions);
    po::notify (fmiOptions);

    if (fmiOptions.count ("buildfmu") > 0)
    {
        fmuLoc = fmiOptions["buildfmu"].as<std::string> ();
    }

    if (fmiOptions.count ("platform") > 0)
    {
        auto platform_str = fmiOptions["platform"].as<std::string> ();
        if (valid_platforms.find (platform_str) != valid_platforms.end ())
        {
            platform = platform_str;
        }
    }
    // program options control
    if (fmiOptions.count ("help") > 0)
    {
        po::options_description visible ("allowed options");
        visible.add (fmiOp);
        std::cout << visible << '\n';
    }
    return GriddynRunner::Initialize (argc, argv, *ri_, true);
}
using namespace boost::filesystem;
/** helper function to copy a file and overwrite if requested*/
bool testCopyFile (path const& source, path const& dest, bool overwrite = false)
{
    copy_option option = copy_option::fail_if_exists;
    if (overwrite) { option = copy_option::overwrite_if_exists; }

    try
    {
        copy_file (source, dest, option);
        return true;
    }
    catch (filesystem_error const& e)
    {
        return false;
    }
}

void fmuBuilder::MakeFmu (const std::string &fmuLocation)
{
    auto bpath = temp_directory_path ();

    auto fmupath = path (fmuLocation);

    if (fmuLocation.empty ())
    {
        if (!fmuLoc.empty ())
        {
            fmupath = path (fmuLoc);
        }
        else
        {
            fmupath = path ("griddyn.fmu");
        }
    }
    auto fmu_temp_dir = bpath / fmupath.stem ();
    create_directory (fmu_temp_dir);

    copySharedLibrary (fmu_temp_dir.string ());

    path resource_dir = fmu_temp_dir / "resources";
    create_directory (resource_dir);

    path sourcefile = getSim ()->sourceFile;
    auto ext = convertToLowerCase (sourcefile.extension ().string ());
    if (ext[0] == '.')
    {
        ext.erase (0, 1);
    }
    auto newFile = resource_dir;
    if (ext == "xml")
    {
        newFile /= "simulation.xml";
    }
    else if (ext == "json")
    {
        newFile /= "simulation.json";
    }
    else
    {
        if (sourcefile.empty ())
        {
            getSim ()->log (nullptr, print_level::error, "no input file specified");
        }
        else
        {
            getSim ()->log (nullptr, print_level::error, "for fmu's input file must be xml or json");
        }

        return;
    }
    // copy the resource files over to the resource directory
    testCopyFile (getSim ()->sourceFile, newFile, true);

    for (const auto &file : ri_->getCapturedFiles ())
    {
        path f (file);
        if (exists (f))
        {
            testCopyFile (f, resource_dir / f.filename ());
        }
    }
    // now generate the model description file
    generateXML ((fmu_temp_dir / "modelDescription.xml").string ());

    if (fmupath.is_absolute())
    {
        // now zip the fmu
        int status = utilities::zipFolder(fmupath.string(), fmu_temp_dir.string());
        if (status == 0)
        {
            getSim()->log(nullptr, print_level::summary, "fmu created at " + fmupath.string());
        }
        else
        {
            getSim()->log(nullptr, print_level::error, "zip status failure creating " + fmupath.string() + "returned with error code " + std::to_string(status));
        }
    }
    else
    {
        auto path2 = current_path() /fmupath;
        int status = utilities::zipFolder(path2.string(), fmu_temp_dir.string());
        if (status == 0)
        {
            getSim()->log(nullptr, print_level::summary, "fmu created at " + path2.string());
        }
        else
        {
            getSim()->log(nullptr, print_level::error, "zip status failure creating " + fmupath.string() + "returned with error code " + std::to_string(status));
        }
    }


}

void fmuBuilder::copySharedLibrary (const std::string &tempdir)
{
    path binary_dir = tempdir / "binaries";
    create_directory (binary_dir);
    bool copySome = false;
    path executable (execPath);
    path execDir = executable.parent_path ();
    if ((platform == "all") || (platform == "windows") || (platform == "win64"))
    {
        auto source = execDir / "win64" / "fmiGridDynSharedLib.dll";
        if (exists (source))
        {
            create_directory (binary_dir / "win64");
            auto dest = binary_dir / "win64" / "fmiGridDynSharedLib.dll";
            testCopyFile (source, dest);
            copySome = true;
        }
        else
        {
            source = execDir / "win64" / "libfmiGridDynSharedLib.dll";
            if (exists(source))
            {
                create_directory(binary_dir / "win64");
                auto dest = binary_dir / "win64" / "fmiGridDynSharedLib.dll";
                testCopyFile(source, dest);
                copySome = true;
            }
        }
    }

    if ((platform == "all") || (platform == "windows") || (platform == "win32"))
    {
        auto source = execDir / "win32" / "fmiGridDynSharedLib.dll";
        if (exists (source))
        {
            create_directory (binary_dir / "win32");
            auto dest = binary_dir / "win32" / "fmiGridDynSharedLib.dll";
            testCopyFile (source, dest);
            copySome = true;
        }
        else
        {
            source = execDir / "win32" / "libfmiGridDynSharedLib.dll";
            if (exists(source))
            {
                create_directory(binary_dir / "win32");
                auto dest = binary_dir / "win32" / "fmiGridDynSharedLib.dll";
                testCopyFile(source, dest);
                copySome = true;
            }
        }
    }
    if ((platform == "all") || (platform == "linux") || (platform == "linux64"))
    {
        auto source = execDir / "linux64" / "fmiGridDynSharedLib.so";
        if (exists (source))
        {
            create_directory (binary_dir / "linux64");
            auto dest = binary_dir / "linux64" / "fmiGridDynSharedLib.so";
            testCopyFile (source, dest);
            copySome = true;
        }
        else
        {
            source = execDir / "linux64" / "libfmiGridDynSharedLib.so";
            if (exists(source))
            {
                create_directory(binary_dir / "linux64");
                auto dest = binary_dir / "linux64" / "fmiGridDynSharedLib.so";
                testCopyFile(source, dest);
                copySome = true;
            }
        }
    }
    if ((platform == "all") || (platform == "linux") || (platform == "linux32"))
    {
        auto source = execDir / "linux32" / "fmiGridDynSharedLib.so";
        if (exists (source))
        {
            create_directory (binary_dir / "linux32");
            auto dest = binary_dir / "linux32" / "fmiGridDynSharedLib.so";
            testCopyFile (source, dest);
            copySome = true;
        }
        else
        {
            source = execDir / "linux32" / "libfmiGridDynSharedLib.so";
            if (exists(source))
            {
                create_directory(binary_dir / "linux32");
                auto dest = binary_dir / "linux32" / "fmiGridDynSharedLib.so";
                testCopyFile(source, dest);
                copySome = true;
            }
        }

    }
    if ((platform == "all") || (platform == "macos") || (platform == "darwin") || (platform == "darwin64"))
    {
        auto source = execDir / "darwin64" / "fmiGridDynSharedLib.so";
        if (exists (source))
        {
            create_directory (binary_dir / "darwin64");
            auto dest = binary_dir / "darwin64" / "fmiGridDynSharedLib.so";
            testCopyFile (source, dest);
            copySome = true;
        }
        else if (exists (execDir / "darwin64" / "fmiGridDynSharedLib.dylib"))
        {
            source = execDir / "darwin64" / "fmiGridDynSharedLib.dylib";
            create_directory (binary_dir / "darwin64");
            auto dest = binary_dir / "darwin64" / "fmiGridDynSharedLib.dylib";
            testCopyFile (source, dest);
            copySome = true;
        }
        else if (exists(execDir / "darwin64" / "libfmiGridDynSharedLib.dylib"))
        {
            source = execDir / "darwin64" / "libfmiGridDynSharedLib.dylib";
            create_directory(binary_dir / "darwin64");
            auto dest = binary_dir / "darwin64" / "fmiGridDynSharedLib.dylib";
            testCopyFile(source, dest);
            copySome = true;
        }
        else if (exists(execDir / "darwin64" / "libfmiGridDynSharedLib.so"))
        {
            source = execDir / "darwin64" / "libfmiGridDynSharedLib.so";
            create_directory(binary_dir / "darwin64");
            auto dest = binary_dir / "darwin64" / "fmiGridDynSharedLib.so";
            testCopyFile(source, dest);
            copySome = true;
        }
    }
    if ((platform == "all") || (platform == "macos") || (platform == "darwin") || (platform == "darwin32"))
    {
        auto source = execDir / "darwin32" / "fmiGridDynSharedLib.so";
        if (exists (source))
        {
            create_directory (binary_dir / "darwin32");
            auto dest = binary_dir / "darwin32" / "fmiGridDynSharedLib.so";
            testCopyFile (source, dest);
            copySome = true;
        }
        else if (exists (execDir / "darwin32" / "fmiGridDynSharedLib.dylib"))
        {
            source = execDir / "darwin32" / "fmiGridDynSharedLib.dylib";
            create_directory (binary_dir / "darwin32");
            auto dest = binary_dir / "darwin32" / "fmiGridDynSharedLib.dylib";
            testCopyFile (source, dest);
            copySome = true;
        }
        else if (exists(execDir / "darwin32" / "libfmiGridDynSharedLib.dylib"))
        {
            source = execDir / "darwin32" / "libfmiGridDynSharedLib.dylib";
            create_directory(binary_dir / "darwin32");
            auto dest = binary_dir / "darwin32" / "fmiGridDynSharedLib.dylib";
            testCopyFile(source, dest);
            copySome = true;
        }
        else if (exists(execDir / "darwin32" / "libfmiGridDynSharedLib.so"))
        {
            source = execDir / "darwin32" / "libfmiGridDynSharedLib.so";
            create_directory(binary_dir / "darwin32");
            auto dest = binary_dir / "darwin32" / "fmiGridDynSharedLib.so";
            testCopyFile(source, dest);
            copySome = true;
        }
    }

    auto binaryLocPath = path (GRIDDYNFMILIBRARY_BINARY_LOC);
    if (exists (binaryLocPath / "fmiGridDynSharedLib.dll"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "fmiGridDynSharedLib.dll";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dll";
        testCopyFile (source, dest);
        return;
    }
    if (exists(binaryLocPath / "libfmiGridDynSharedLib.dll"))
    {
        create_directory(binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "libfmiGridDynSharedLib.dll";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dll";
        testCopyFile(source, dest);
        return;
    }
    if (exists (binaryLocPath / "fmiGridDynSharedLib.so"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "fmiGridDynSharedLib.so";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.so";
        testCopyFile (source, dest);
        return;
    }
    if (exists(binaryLocPath / "libfmiGridDynSharedLib.so"))
    {
        create_directory(binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "libfmiGridDynSharedLib.so";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.so";
        testCopyFile(source, dest);
        return;
    }
    if (exists (binaryLocPath / "fmiGridDynSharedLib.dylib"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "fmiGridDynSharedLib.dylib";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dylib";
        testCopyFile (source, dest);
        return;
    }
    if (exists(binaryLocPath / "libfmiGridDynSharedLib.dylib"))
    {
        create_directory(binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "libfmiGridDynSharedLib.dylib";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dylib";
        testCopyFile(source, dest);
        return;
    }
    if (copySome)
    {
        return;
    }

// Deal with Visual Studio locations
#ifndef NDEBUG
    if (exists (binaryLocPath / "Debug" / "fmiGridDynSharedLib.dll"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "Debug" / "fmiGridDynSharedLib.dll";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dll";
        testCopyFile (source, dest);
        return;
    }
#else
    if (exists (binaryLocPath / "Release" / "fmiGridDynSharedLib.dll"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        auto source = binaryLocPath / "Release" / "fmiGridDynSharedLib.dll";
        auto dest = binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dll";
        testCopyFile (source, dest);
        return;
    }
#endif
    // now just search the current directory
    if (exists ("fmiGridDynSharedLib.dll"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        testCopyFile ("fmiGridDynSharedLib.dll", binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dll");
        return;
    }
    if (exists("libfmiGridDynSharedLib.dll"))
    {
        create_directory(binary_dir / FMILIBRARY_TYPE);
        testCopyFile("libfmiGridDynSharedLib.dll", binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dll");
        return;
    }
    if (exists ("fmiGridDynSharedLib.so"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        testCopyFile ("fmiGridDynSharedLib.so", binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.so");
        return;
    }
    if (exists("libfmiGridDynSharedLib.so"))
    {
        create_directory(binary_dir / FMILIBRARY_TYPE);
        testCopyFile("libfmiGridDynSharedLib.so", binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.so");
        return;
    }
    if (exists ("fmiGridDynSharedLib.dylib"))
    {
        create_directory (binary_dir / FMILIBRARY_TYPE);
        testCopyFile ("fmiGridDynSharedLib.dylib", binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dylib");
        return;
    }
    if (exists("libfmiGridDynSharedLib.dylib"))
    {
        create_directory(binary_dir / FMILIBRARY_TYPE);
        testCopyFile("libfmiGridDynSharedLib.dylib", binary_dir / FMILIBRARY_TYPE / "fmiGridDynSharedLib.dylib");
        return;
    }
    throw(std::runtime_error("unable to locate shared fmu library file"));
}

void fmuBuilder::generateXML (const std::string &xmlfile)
{
    using namespace tinyxml2;
    XMLDocument doc;
    int index = 1;
    //add the standard xml declaration
    auto dec = doc.NewDeclaration();
    doc.InsertFirstChild(dec);
    //add the main xml root object
    auto pRoot = doc.NewElement ("fmiModelDescription");

    doc.InsertEndChild (pRoot);

    pRoot->SetAttribute ("fmiVersion", "2.0");
    pRoot->SetAttribute ("modelName", getSim ()->getName ().c_str ());
    pRoot->SetAttribute ("guid", "{82072fd0-2f55-4c42-b84c-e47ee14091d0}");

    auto desc = getSim()->getDescription();
    if (!desc.empty())
    {
        pRoot->SetAttribute("description", desc.c_str());
    }

    pRoot->SetAttribute ("version", getSim ()->getString ("version").c_str ());

    // the cosimulation description section
    auto pElement = doc.NewElement ("CoSimulation");
    pElement->SetAttribute ("modelIdentifier", "fmiGridDynSharedLib");
    pElement->SetAttribute ("canHandleVariableCommunicationStepSize", "true");

    pRoot->InsertEndChild (pElement);

    // log categories section

    pElement = doc.NewElement ("LogCategories");

    auto logElement = doc.NewElement ("Category");
    logElement->SetAttribute ("name", "logError");
    pElement->InsertEndChild (logElement);

    logElement = doc.NewElement ("Category");
    logElement->SetAttribute ("name", "logWarning");
    pElement->InsertEndChild (logElement);

    logElement = doc.NewElement ("Category");
    logElement->SetAttribute ("name", "logSummary");
    pElement->InsertEndChild (logElement);

    logElement = doc.NewElement ("Category");
    logElement->SetAttribute ("name", "logNormal");
    pElement->InsertEndChild (logElement);

    logElement = doc.NewElement ("Category");
    logElement->SetAttribute ("name", "logDebug");
    pElement->InsertEndChild (logElement);

    logElement = doc.NewElement("Category");
    logElement->SetAttribute("name", "logTrace");
    pElement->InsertEndChild(logElement);

    pRoot->InsertEndChild (pElement);

    // next load all the scalar variables in the system
    pElement = doc.NewElement ("ModelVariables");

    auto sVariable = doc.NewElement ("ScalarVariable");
    sVariable->SetAttribute ("name", "run_asynchronously");
    sVariable->SetAttribute ("valueReference", 0);

    sVariable->SetAttribute ("description", "set to true to enable GridDyn to run Asynchronously");
    sVariable->SetAttribute ("causality", "parameter");
    sVariable->SetAttribute ("variability", "fixed");
    pElement->InsertEndChild (sVariable);
    ++index;
    auto bType = doc.NewElement ("Boolean");
    bType->SetAttribute ("start", false);
    sVariable->InsertEndChild (bType);

    sVariable = doc.NewElement ("ScalarVariable");
    sVariable->SetAttribute ("name", "record_directory");
    sVariable->SetAttribute ("valueReference", 1);

    auto sType = doc.NewElement ("String");
    sType->SetAttribute ("start", "");
    sVariable->InsertEndChild (sType);

    sVariable->SetAttribute ("description", "set the directory to place GridDyn outputs");
    sVariable->SetAttribute ("causality", "parameter");
    sVariable->SetAttribute ("variability", "fixed");
    pElement->InsertEndChild (sVariable);
    ++index;

    auto fmiInputs = coord_->getInputs ();
    for (auto &input : fmiInputs)
    {
        sVariable = doc.NewElement ("ScalarVariable");
        sVariable->SetAttribute ("name", input.second.name.c_str ());
        sVariable->SetAttribute ("valueReference", input.first);
        auto evntdesc = input.second.evnt->getDescription ();
        if (!evntdesc.empty ())
        {
            sVariable->SetAttribute ("description", evntdesc.c_str ());
        }
        sVariable->SetAttribute ("causality", "input");
        sVariable->SetAttribute ("variability", "continuous");
        auto rType = doc.NewElement ("Real");
        rType->SetAttribute ("start", coord_->getOutput (input.first));
        sVariable->InsertEndChild (rType);
        pElement->InsertEndChild (sVariable);
        ++index;
    }
    auto fmiParams = coord_->getParameters ();
    for (auto &param : fmiParams)
    {
        sVariable = doc.NewElement ("ScalarVariable");
        sVariable->SetAttribute ("name", param.second.name.c_str ());
        sVariable->SetAttribute ("valueReference", param.first);
        auto evntdesc = param.second.evnt->getDescription();
        if (!evntdesc.empty())
        {
            sVariable->SetAttribute("description", evntdesc.c_str());
        }
        sVariable->SetAttribute ("causality", "parameter");
        if (fmiCoordinator::isStringParameter(param))
        {
            sVariable->SetAttribute("variability", "fixed");
            auto sParamType = doc.NewElement("String");
            sParamType->SetAttribute("start", "");
            sVariable->InsertEndChild(sParamType);
        }
        else
        {
            sVariable->SetAttribute("variability", "continuous");

            auto rType = doc.NewElement("Real");
            rType->SetAttribute("start", coord_->getOutput(param.first));
            sVariable->InsertEndChild(rType);
        }

        pElement->InsertEndChild (sVariable);
        ++index;
    }
    std::vector<int> outputIndices;
    auto fmiOutputs = coord_->getOutputs ();
    for (auto &out : fmiOutputs)
    {
        sVariable = doc.NewElement ("ScalarVariable");
        sVariable->SetAttribute ("name", out.second.name.c_str ());
        sVariable->SetAttribute ("valueReference", out.first);
        sVariable->SetAttribute ("causality", "output");
        sVariable->SetAttribute ("variability", "continuous");
        //TODO:: figure out how to generate descriptions
        auto rType = doc.NewElement ("Real");
        sVariable->InsertEndChild (rType);

        pElement->InsertEndChild (sVariable);
        outputIndices.push_back (index);
        ++index;
    }
    pRoot->InsertEndChild (pElement);
    // load the dependencies

    pElement = doc.NewElement ("ModelStructure");
    auto outputs = doc.NewElement ("Outputs");
    for (auto &outind : outputIndices)
    {
        auto out = doc.NewElement ("Unknown");
        out->SetAttribute ("index", outind);
        outputs->InsertEndChild (out);
    }
    pElement->InsertEndChild (outputs);

    auto initUnkn = doc.NewElement ("InitialUnknowns");
    for (auto &outind : outputIndices)
    {
        auto out = doc.NewElement ("Unknown");
        out->SetAttribute ("index", outind);
        initUnkn->InsertEndChild (out);
    }
    pElement->InsertEndChild (initUnkn);

    pRoot->InsertEndChild (pElement);

    auto res=doc.SaveFile (xmlfile.c_str ());
    if (res != XMLError::XML_SUCCESS)
    {
        throw(std::runtime_error("unable to write file"));
    }
}

}  // namespace fmi
}  // namespace griddyn
