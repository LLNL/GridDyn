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

#include "fncsSupport.h"
#include "fncsTest.h"
#include "fncsLibrary.h"
#include "core/objectFactory.h"
#include "fncsSource.h"
#include "fncsLoad.h"
#include "gridBus.h"
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include <iostream>
#include <complex>
#include <memory>

bool timeConversionCheck()
{
	coreTime val = 4.5234235;
	auto fncsTime = gd2fncsTime(val);
	coreTime ret = fncs2gdTime(fncsTime);
	
	return (std::abs(val - ret) < 0.0000001);

}
//test complex number conversions to strings
#ifdef USE_DUMMY_FNCS
bool testComplexConversion()
{
	fncs::clear();
	fncsSendComplex("testkey", 4.3, 3.252662);

	auto val = fncsGetComplex("testkey");
	if (std::abs(val.real() - 4.3) > 0.0000001)
	{
		return false;
	}
	
	if (std::abs(val.imag() - 3.252662) > 0.00000001)
	{
		return false;
	}

	fncsSendComplex("testkey", std::complex<double>(4.3, 3.252662));

	val = fncsGetComplex("testkey");
	if (std::abs(val.real() - 4.3) > 0.0000001)
	{
		return false;
	}

	if (std::abs(val.imag() - 3.252662) > 0.00000001)
	{
		return false;
	}

	fncsSendComplex("testkey", 0, 3.252662);

	val = fncsGetComplex("testkey");
	if (std::abs(val.real() - 0) > 0.0000001)
	{
		return false;
	}

	if (std::abs(val.imag() - 3.252662) > 0.00000001)
	{
		return false;
	}

	fncsSendVal("testkey", 4.3);

	val = fncsGetComplex("testkey");
	if (std::abs(val.real() - 4.3) > 0.0000001)
	{
		return false;
	}

	if (std::abs(val.imag() - 0.0) > 0.00000001)
	{
		return false;
	}
	return true;
}

//test the FNCS source object
bool testFNCSsource()
{
	fncs::clear();
	coreObject *obj = coreObjectFactory::instance()->createObject("source", "fncs", "fncsSource");

	if (!dynamic_cast<fncsSource *>(obj))
	{
		return false;
	}
	auto src = static_cast<fncsSource *>(obj);

	obj->set("key", "sourceTestVal");
	fncs::publish("sourceTestVal", "4.345");
	src->set("period", 1.0);
	
	src->dynInitializeA(timeZero, 0);
	IOdata out;
	src->dynInitializeB(noInputs, noInputs, out);
	
	src->timestep(1.0, noInputs,cLocalbSolverMode);

	if (std::abs(src->getOutput(0) - 4.345) > 0.0000001)
	{
		std::cout << "get value failed\n";
		return false;
	}
	fncs::publish("sourceTestVal", "6.0");
	src->timestep(2.0, noInputs, cLocalbSolverMode);
	if (std::abs(src->getOutput(0) - 6.0) > 0.0000001)
	{
		return false;
	}
	src->setFlag("predictive", true);
	fncs::publish("sourceTestVal", "7.0");
	src->timestep(3.0, noInputs, cLocalbSolverMode);
	src->timestep(3.5, noInputs, cLocalbSolverMode);
	double val = src->getOutput(0);
	if (std::abs(src->getOutput(0) - 7.5) > 0.0000001) //checking if interpolating is working properly
	{
		std::cout << "predictive operation failed\n";
		return false;
	}

	src->setFlag("step", true);
	src->timestep(4.2, noInputs, cLocalbSolverMode);
	val = src->getOutput(0);
	if (std::abs(src->getOutput(0) - 7.0) > 0.0000001) //checking if interpolating is working properly
	{
		std::cout << "step wise advancement failed\n";
		return false;
	}
	src->setFlag("interpolate", true);
	src->timestep(5.0, noInputs, cLocalbSolverMode);
	fncs::publish("sourceTestVal", "6.0");
	src->timestep(6.0, noInputs, cLocalbSolverMode);
	src->timestep(6.3, noInputs, cLocalbSolverMode);

	val = src->getOutput(0);
	if (std::abs(src->getOutput(0) - 6.7) > 0.0000001) //checking if interpolating is working properly
	{
		std::cout << "interpolation failed\n";
		return false;
	}
	return true;
}
//test the FNCS load object
bool testFNCSload()
{
	fncs::clear();
	std::unique_ptr<gridDynSimulation> gds = std::make_unique<gridDynSimulation>();
	std::string tfile1 = std::string(FNCS_TEST_DIRECTORY "/fncs_load_test1.xml");

	loadFile(gds.get(), tfile1);
	auto ld1 = dynamic_cast<fncsLoad *>(gds->find("bus#1/load#0"));
	if (ld1 == nullptr)
	{
		std::cout << "fncs load1 not created\n";
		return false;
	}
	fncsSendComplex("fncsload", 0.5, 0.1);
	gds->dynInitialize();

	double rp = ld1->getRealPower();
	double qp = ld1->getReactivePower();
	if ((std::abs(rp - 0.5) > 0.00001)|| (std::abs(qp - 0.1) > 0.00001))
	{
		std::cout << "mismatch in power levels\n";
		return false;
	}
	gds->run(1.0);
	fncsSendComplex("fncsload", 0.3, 0.2);
	gds->run(4.0);
	auto cv = fncsGetComplex("fncsvoltage");
	double vmag = std::abs(cv);

	fncsSendComplex("fncsload", 0.5, 0.4);
	gds->run(8.0);
	auto cv2 = fncsGetComplex("fncsvoltage");

	if (vmag < std::abs(cv2))
	{
		std::cout << "mismatch in power levels\n";
		return false;
	}
	return true;
}

bool testFNCSCollector()
{
	fncs::clear();
	std::unique_ptr<gridDynSimulation> gds = std::make_unique<gridDynSimulation>();
	std::string tfile1 = std::string(FNCS_TEST_DIRECTORY "/fncs_collector_test1.xml");

	loadFile(gds.get(), tfile1);
	
	int cc = gds->getInt("collectorcount");
	if (cc != 1)
	{
		std::cout << "incorrect number of collectors\n";
		return false;
	}
	gds->run(2.0);
	auto val = fncsGetVal("p1");
	if (std::abs(val-0.35)>0.000001)
	{
		std::cout << "incorrect bus load\n";
		return false;
	}

	auto keys = fncs::get_keys();
	if (keys.size() != 9)
	{
		std::cout << "incorrect number of keys\n";
		return false;
	}
	fncsSendComplex("fncsload1", 0.45, 0.2);
	fncsSendComplex("fncsload2", 0.54, 0.31);

	gds->run(4.0);
	val = fncsGetVal("p1");
	if (std::abs(val - 0.45)>0.000001)
	{
		std::cout << "incorrect bus real load\n";
		return false;
	}
	val = fncsGetVal("q2");
	if (std::abs(val - 0.31)>0.000001)
	{
		std::cout << "incorrect bus reactive load\n";
		return false;
	}
	auto v1 = fncsGetComplex("fncsvoltage1");
	auto v2 = fncsGetComplex("fncsvoltage2");
	if (std::abs(v1 -  v2)>0.000001)
	{
		std::cout << "incorrect voltage\n";
		return false;
	}
	return true;
}

#endif

bool runFNCStests()
{
	bool out = true;
	bool res = timeConversionCheck();
	out = out && res;
	if (!out)
	{
		std::cout << "timeConversionCheck Failed\n";
	}
#ifdef USE_DUMMY_FNCS
	res = testComplexConversion();
	out = out && res;
	if (!out)
	{
		std::cout << "complex Conversion Failed\n";
	}
	
	res = testFNCSsource();
	out = out && res;
	if (!out)
	{
		std::cout << "FNCS source test Failed\n";
	}

	res = testFNCSload();
	out = out && res;
	if (!out)
	{
		std::cout << "FNCS load test Failed\n";
	}

	res = testFNCSCollector();
	out = out && res;
	if (!out)
	{
		std::cout << "FNCS Collector test Failed\n";
	}
#endif
	return out;
}

