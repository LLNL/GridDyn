/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
  * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/



#include "fmiSubModel.h"
#include "fmi_importGD.h"
#include <fmilib.h>
#include <JM/jm_portability.h>
#include <boost/filesystem.hpp>

static  jm_callbacks callbacks;

static fmi2_callback_functions_t callBackFunctions;
static int cinit = 0;

void importlogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
	printf("module = %s, log level = %d: %s\n", module, log_level, message);
}


fmiSubModel::fmiSubModel(fmi_import_context_t *ctx) :gridSubModel("fmisub_#"),context(ctx)
{
	if (!context)
	{
		if (cinit == 0)
		{
			callbacks.malloc = malloc;
			callbacks.calloc = calloc;
			callbacks.realloc = realloc;
			callbacks.free = free;
			callbacks.logger = importlogger;
			callbacks.log_level = jm_log_level_warning;
			callbacks.context = 0;
			cinit = 1;
		}
		context = fmi_import_allocate_context(&callbacks);
	}
}

fmiSubModel::~fmiSubModel()
{
	fmi_import_free_context(context);
}

std::string fmiSubModel::extractFMU()
{
	auto version = getFmuVersion(context, fmu_name, fmu_dir);
	switch (version)
	{
	case fmi_version_1_enu:
	case fmi_version_2_0_enu:
		valid = true;
		break;
	case fmi_version_unknown_enu:
	case fmi_version_unsupported_enu:
		break;

	}
	return fmu_dir;
}


stringVec fmiSubModel::getOutputNames() const
{
	return stringVec(0);
}

stringVec fmiSubModel::getInputNames() const
{
	return stringVec(0);
}


fmiSubModel *makefmiSubModel(const std::string &fmu_path)
{
	if (cinit == 0)
	{
		callbacks.malloc = malloc;
		callbacks.calloc = calloc;
		callbacks.realloc = realloc;
		callbacks.free = free;
		callbacks.logger = importlogger;
		callbacks.log_level = jm_log_level_warning;
		callbacks.context = 0;
		cinit = 1;
	}
	auto context = fmi_import_allocate_context(&callbacks);
	std::string ndir;
	fmiSubModel *fs=nullptr;
	auto vers = getFmuVersion(context, fmu_path, ndir);
	switch (vers)
	{
	case fmi_version_1_enu:
		fs = new fmiSubModel1(context);
		fs->set("fmu_dir", ndir);
		break;
	case fmi_version_2_0_enu:
		fs = new fmiSubModel2(context);
		fs->set("fmu_dir", ndir);
		break;
	case fmi_version_unknown_enu:
	case fmi_version_unsupported_enu:
		break;

	}
	return fs;
}

fmi_version_enu_t getFmuVersion(fmi_import_context_t *context, const std::string &fmu_path, std::string &fmu_dir)
{
	boost::filesystem::path fmuPath(fmu_path);

	fmi_version_enu_t version = fmi_version_unknown_enu;
	boost::filesystem::path extractPath;
	if (!fmu_dir.empty())
	{
		extractPath = boost::filesystem::path(fmu_dir);
	}
	else
	{
		std::string ext = fmuPath.extension().string();
		if (is_directory(fmuPath))
		{
			extractPath = fmuPath;
			fmu_dir = extractPath.string();
		}
		else if (ext == ".fmu")
		{
			if (boost::filesystem::exists(fmuPath))
			{
				extractPath = fmuPath.parent_path() / fmuPath.stem();
				fmu_dir = extractPath.string();
			}
		}
		else
		{
			return version;
		}

	}
	if (!is_directory(extractPath))
	{
		if (!create_directory(extractPath))
		{
			version = fmi_version_unsupported_enu;
			return version;
		}
	}

	version = fmi_import_get_fmi_version(context, fmuPath.string().c_str(), extractPath.string().c_str());
	return version;
}


outputEstimator::outputEstimator(std::vector<int> sDep, std::vector<int> iDep)
{
	stateDep = sDep;
	inputDep = iDep;
	stateDiff.resize(stateDep.size(), 0);
	inputDiff.resize(inputDep.size(), 0);
	prevStates.resize(stateDep.size());
	prevInputs.resize(inputDep.size());
}

double outputEstimator::estimate(double t, const IOdata &args, const double state[])
{
	double val = prevValue;
	size_t kk;
	for (kk = 0; kk < stateDep.size(); ++kk)
	{
		val += (state[stateDep[kk]] - prevStates[kk])*stateDiff[kk];
	}
	for (kk = 0; kk < inputDep.size(); ++kk)
	{
		val += (args[inputDep[kk]] - prevInputs[kk])*inputDiff[kk];
	}
	val += timeDiff*(t - time);
	return val;
}

bool outputEstimator::update(double t, double val, const IOdata &args, const double state[])
{
	time = t;
	
	double pred = estimate(t, args, state);
	prevValue = val;
	size_t kk;
	for (kk = 0; kk < stateDep.size(); ++kk)
	{
		prevStates[kk] = state[stateDep[kk]];
	}
	for (kk = 0; kk < inputDep.size(); ++kk)
	{
		prevInputs[kk] = args[inputDep[kk]];
	}
	double diff = std::abs(pred - val);
	if ((diff>1e-4) && (diff / (std::max)(std::abs( pred), std::abs(val))>0.02))
	{
		return true;
	}
	return false;
}