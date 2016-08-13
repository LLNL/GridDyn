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


#ifndef FUNCTION_INTERPRETER_H_
#define FUNCTION_INTERPRETER_H_

#include <functional>
#include <vector>
#include <string>

/** @brief enumeration of the different function types*/
enum class function_type
{
	all,  //!< all possible function types
	no_args, //!< functions with no arguments
	arg,   //!< function with 1 argument
	arg2, //!< functions with 2 arguments
	vect_arg,  //!< function with a vector arguments
	vect_arg2  //!< functions with 2 vector arguments
};

/** @brief evaluate a function named by a string
@param[in] ftest the function name to evaluate
@return the evaluated function or nan;
*/
double evalFunction(const std::string &ftest);

/** @brief evaluate a function named by a string with 1 argument
@param[in] ftest the function name to evaluate
@param[in] val the argument value of the function
@return the evaluated function or nan;
*/
double evalFunction(const std::string &ftest, double val);

/** @brief evaluate a function named by a string with 2 arguments
@param[in] ftest the function name to evaluate
@param[in] val1 the first argument value of the function
@param[in] val2 the second argument value of the function
@return the evaluated function or nan;
*/
double evalFunction(const std::string &ftest, double val1, double val2);

/** @brief evaluate a function named by a string with 1 array argument
@param[in] ftest the function name to evaluate
@param[in] arr the array argument for evaluation
@return the evaluated function or nan;
*/
double evalFunction(const std::string &ftest, std::vector<double> arr);

/** @brief evaluate a function named by a string with 2 array arguments
@param[in] ftest the function name to evaluate
@param[in] arr1 the first array argument for evaluation
@param[in] arr2 the second array argument for evaluation
@return the evaluated function or nan;
*/
double evalFunction(const std::string &ftest, std::vector<double> arr1, std::vector<double> arr2);

/** @brief check if a string represents a valid function 
@param[in] ftest the function name to tests
@param[in] ftype the class of functions to check
@return true if the string is a function name false otherwise
*/
bool isFunctionName(const std::string &ftest, function_type ftype = function_type::all);

/** @brief find a no argument function and return the corresponding lambda function
@param[in] the function name
@return a std::Function with the appropriate function
*/
std::function<double()> get0ArgFunction(const std::string &funcName);

/** @brief find a single argument function and return the corresponding lambda function
@param[in] the function name
@return a std::Function with the appropriate function
*/
std::function<double(double)> get1ArgFunction(const std::string &funcName);

/** @brief find a two argument function and return the corresponding lambda function
@param[in] the function name
@return a std::Function with the appropriate function
*/
std::function<double(double, double)> get2ArgFunction(const std::string &funcName);

/** @brief find a function with a single array as an argment and return the corresponding lambda function
@param[in] the function name
@return a std::Function implementing the appropriate function
*/
std::function<double(std::vector<double>)> getArrayFunction(const std::string &funcName);

/** @brief find a function with a two arrays as argments and return the corresponding lambda function
@param[in] the function name
@return a std::Function implementing the appropriate function
*/
std::function<double(std::vector<double>, std::vector<double>)> get2ArrayFunction(const std::string &funcName);


#endif