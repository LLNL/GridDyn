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

#ifndef OPERATING_BOUNDARY_H_
#define OPERATING_BOUNDARY_H_


#include <string>
#include <vector>

namespace utilities
{

/** class to define a lookup table based operating boundary, for min and max limits, values are interpolated in intermediate points with a linear interpolator*/
class OperatingBoundary
{
private:
	/** simple structor for containing points*/
	using opPoint=struct
	{
		double ind;
		double depMin;
		double depMax;
	};
	double indMin = -1e47; //!< the independent variable lower limit
	double indMax = 1e47;	//!< the independent variable upper limit

	//these variables are all for keeping a cache
	mutable double cRangeLow = 1e47;
	mutable double cRangeHigh = -1e47;

	mutable double depMin = -1e47;
	mutable double depMax = 1e47;
	mutable double slopeMin = 0.0;
	mutable double slopeMax = 0.0;
	std::vector<opPoint> operatingPoints; //!< storage for the operating points
	mutable int lastIndex = 0;
	
public:
	/** default constructor*/
	OperatingBoundary() = default;
	/** constructor defining a bounding box*/
	OperatingBoundary(double indMn, double indMx, double depMn, double depMx);
	/** add an intermediate point
	@param[in] indPt the independent variable value
	@param[in] the lower bound
	@param[in] the upper bound
	*/
	void addPoint(double indPt, double depLow, double depHigh);

	/** add a set of points to the upper and lower bounds
	@param[in] indPts the independent variable values
	@param[in] lowPts  vector of values for the lower bound
	@param[in] highPts vector of values for the upper bound
	*/
	void addPoints(const std::vector<double> &inPts, const std::vector<double> &lowPts, const std::vector<double> &highPts);
	/** set range of valid independent values*/
	void setValidRange(double iMin, double iMax);

	/** get the upper bound for a given value*/
	double getMax(double val) const;
	/** get the lower bound for a given value*/
	double getMin(double val) const;
	/** get both the lower and upper bounds*/
	std::pair<double, double> getLimits(double val) const;
	/** get the rate of change in upper bound for a given value (aka the slope of the interpolator)*/
	double dMaxROC(double val) const;
	/** get the rate of change in lower bound for a given value (aka the slope of the interpolator)*/
	double dMinROC(double val) const;
	/** get the rate of change in lower and upper bounds for a given value (aka the slope of the interpolator)*/
	std::pair<double, double> getLimitsROC(double val) const;
	/** clear the table*/
	void clear();
private:
	/** function to update the cache values*/
	void updateRange(double val) const;
};

} // namespace utilities

#endif  //OPERATING_BOUNDARY_H_
