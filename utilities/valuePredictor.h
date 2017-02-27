
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
#pragma once
#ifndef VALUE_PREDICTOR_H_
#define VALUE_PREDICTOR_H_

/** @brief class implementing a prediction system
*@details the base class does a linear prediction
*/
template<typename InputType, typename OutputType=InputType, typename SlopeType= OutputType>
class valuePredictor
{
private:
	InputType lastKnownInput;  //!< the last known time
	OutputType lastKnownOutput;  //!< the last known Value
	SlopeType slope;		//!< the rate of change
	
public:

	/** construction the predictor
	* @param[in] input0 the initial input
	@param[in] output0 the initial output
	@param[in] slope [optional] the initial rate of change
	*/
	valuePredictor(InputType input0, OutputType output0, SlopeType slope0 = SlopeType(0)) :lastKnownInput(input0), lastKnownOutput(output0), slope(slope0)
	{

	}
	virtual ~valuePredictor()=default;
	//default copy and copy constructors
	valuePredictor(const valuePredictor &ref) = default; //!< copy constructor does the default thing
	valuePredictor(valuePredictor &&ref) = default;

	valuePredictor &operator=(const valuePredictor &ref) = default;
	valuePredictor &operator=(valuePredictor &&ref) = default;
	/** update the known values
	*@details sets the known values and computes the ramp rate
	*The values input should correspond to the reduction in values so 0.0 for no saturation
	*@param[in] input  the actual input value
	*@param[in] output the actual output value
	*/
	virtual void update(InputType input, OutputType output)
	{
		if (input - lastKnownInput > InputType(0))
		{
			slope = (output - lastKnownOutput) / (input - lastKnownInput);
		}
		lastKnownInput = input;
		lastKnownOutput = output;
	}
	/** @brief set the rate at a user specified value
	*/
	virtual void setSlope(SlopeType newSlope)
	{
		slope = newSlope;
	}
	/** update the saturation type function by enumeration*/
	virtual OutputType predict(InputType input) const
	{
		return lastKnownOutput + (input - lastKnownInput)*slope;
	}
	/** update the saturation type function by enumeration*/
	OutputType operator() (InputType input) const
	{
		return predict(input);
	}
	/** getKnownInput*/
	InputType getKnownInput() const
	{
		return lastKnownInput;
	}
	/** getKnownOutput*/
	OutputType getKnownOutput() const
	{
		return lastKnownOutput;
	}
	/** get the rate of change*/
	SlopeType getSlope() const
	{
		return slope;
	}
};

#endif