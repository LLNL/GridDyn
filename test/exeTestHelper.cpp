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

#include "exeTestHelper.h"
#include <cstdlib>
#include <fstream>
#include <streambuf>
#include <boost/filesystem.hpp>

int exeTestRunner::counter = 1;

exeTestRunner::exeTestRunner ()
{
    ++counter;
    buildOutFile ();
}

exeTestRunner::exeTestRunner (const std::string &baseLocation, const std::string &target)
{
    ++counter;
    buildOutFile ();
    active = findFileLocation (baseLocation, target);
    if (!(system (nullptr)))
    {
        active = false;
    }
}

exeTestRunner::exeTestRunner (const std::string &baseLocation,
                              const std::string &baseLocation2,
                              const std::string &target)
{
    ++counter;
    buildOutFile ();
    active = findFileLocation (baseLocation, target);
    if (!active)
    {
        active = findFileLocation (baseLocation2, target);
    }
    if (!(system (nullptr)))
    {
        active = false;
    }
}

void exeTestRunner::buildOutFile ()
{

	auto pth = boost::filesystem::temp_directory_path();
	pth /= ("exeText_" + std::to_string(counter) + ".out");
	outFile = pth.string();
   
}

bool exeTestRunner::findFileLocation (const std::string &baseLocation, const std::string &target)
{
    boost::filesystem::path sourcePath (baseLocation);

    auto tryPath1 = sourcePath / target;
    if (boost::filesystem::exists (tryPath1))
    {
        exeString = tryPath1.string ();
        return true;
    }

    auto tryPath2 = sourcePath / (target + ".exe");
    if (boost::filesystem::exists (tryPath2))
    {
        exeString = tryPath2.string ();
        return true;
    }
#ifndef NDEBUG
    auto tryPathD1 = sourcePath / "Debug" / target;
    if (boost::filesystem::exists (tryPathD1))
    {
        exeString = tryPathD1.string ();
        return true;
    }

    auto tryPathD2 = sourcePath / "Debug" / (target + ".exe");
    if (boost::filesystem::exists (tryPathD2))
    {
        exeString = tryPathD2.string ();
        return true;
    }
#endif
    auto tryPathR1 = sourcePath / "Release" / target;
    if (boost::filesystem::exists (tryPathR1))
    {
        exeString = tryPathR1.string ();
        return true;
    }

    auto tryPathR2 = sourcePath / "Release" / (target + ".exe");
    if (boost::filesystem::exists (tryPathR2))
    {
        exeString = tryPathR2.string ();
        return true;
    }

    boost::filesystem::path tryPatht1 = target;
    if (boost::filesystem::exists (tryPatht1))
    {
        exeString = tryPatht1.string ();
        return true;
    }

    boost::filesystem::path tryPatht2 = (target + ".exe");
    if (boost::filesystem::exists (tryPatht2))
    {
        exeString = tryPatht2.string ();
        return true;
    }
    return false;
}

int exeTestRunner::run (const std::string &args) const
{
    if (!active)
    {
        return -101;
    }
    std::string rstr = exeString + " " + args;
    return system (rstr.c_str ());
}

std::string exeTestRunner::runCaptureOutput (const std::string &args) const
{
    if (!active)
    {
        return "invalid executable";
    }
    std::string rstr = exeString + " " + args + " > " + outFile;
    //printf ("string %s\n", rstr.c_str ());
    system (rstr.c_str ());

    //printf (" after system call string %s\n", rstr.c_str ());
    std::ifstream t (outFile);
    std::string str ((std::istreambuf_iterator<char> (t)), std::istreambuf_iterator<char> ());

    remove (outFile.c_str ());
    return str;
}
