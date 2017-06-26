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

#ifndef GRID_GRABBER_SET_H_
#define GRID_GRABBER_SET_H_
#pragma once

#include "core/objectOperatorInterface.hpp"
#include "gridDynDefinitions.hpp"
#include <memory>

template<class Y>
class matrixData;

namespace utilities
{
template< typename X, typename Y, typename Z>
class valuePredictor;
}

namespace griddyn
{

class gridCore;
class stateGrabber;
class gridGrabber;
class stateData;
class solverMode;

/** class pairing up basicGrabbers and state grabbers in a single interface
*/
class grabberSet : public objectOperatorInterface
{
private:
	std::shared_ptr<gridGrabber> grab; //!< the non state grabber
	std::shared_ptr<stateGrabber> stGrab; //!< the state grabber
	index_t seqID=0;	//!< the last sequence ID for the state grabber
	bool usePredictor=false; //!< indicator that the Grabber should use a predictor 
	double lastValue = 0;  //!< the last value captured related to seqID;
	std::unique_ptr<utilities::valuePredictor<coreTime, double, double>> predictor;
	
public:

	grabberSet(const std::string &fld, coreObject *obj,bool step_only=false);
	grabberSet(index_t noffset, coreObject *obj);

	grabberSet(std::shared_ptr<gridGrabber> ggrab, std::shared_ptr<stateGrabber> stgrab);
	virtual ~grabberSet();


	/** clone function
	*@param[in] ggb a pointer to another gridGrabber function if we are cloning on existing object
	*@return a shared_ptr to another GridGrabber*/
	virtual std::shared_ptr<grabberSet> clone(std::shared_ptr<grabberSet> ggb = nullptr) const;
	/** update the field of grabber
	*@param[in]  fld the new field to capture
	*@throw unrecognized parameter exception if fld is not available
	*/
	virtual void updateField(const std::string &fld);
	/** replace the grabbers with a new pair
	*/
	virtual void updateGrabbers(std::shared_ptr<gridGrabber> ggrab, std::shared_ptr<stateGrabber> stgrab);
	
	/** actually go and get the data
	*@return the value produced by the grabber*/
	virtual double grabData();
	/** @brief grab a vector of data
	*@param[out] data the vector to store the data in
	*/
	virtual void grabData(std::vector<double> &data);
	/** @brief get the descriptions of the data
	*@param[out] desc_list  the list of descriptions
	**/
	virtual double grabData(const stateData &sD, const solverMode &sMode);
	virtual void outputPartialDerivatives(const stateData &sD, matrixData<double> &md, const solverMode &sMode);
	//virtual void getDoutDt(const stateData &sD, const solverMode &sMode) const;
	virtual void getDesc(std::vector<std::string > &desc_list) const;
	virtual const std::string &getDesc() const;
	virtual std::string getDesc();
	void setDescription(const std::string &newDesc);
	virtual void updateObject(coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
	virtual coreObject * getObject() const override;
	virtual void getObjects(std::vector<coreObject *> &objects) const override;

	void setGain(double newGain);
	bool stateCapable() const;
	bool hasJacobian() const;
};

}//namespace griddyn
#endif