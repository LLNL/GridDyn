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

#ifndef GRIDDYNEXCEPTIONS_H_

#define GRIDDYNEXCEPTIONS_H_

#include "coreObject.h"
#include <exception>

/** exception class for use in gridDyn*/
class gridDynException : public std::exception
{
public:
	gridDynException() {};
	virtual const char *what() const noexcept override
	{
		return "GridDyn exception";
	}
};

class coreObjectException : public gridDynException
{
protected:
	coreObject *throwingObject;  //<!* the object that threw the exception
public:
	explicit coreObjectException(coreObject *obj);
	virtual const char *what() const noexcept override
	{
		return "core object exception";
	}
	/** return the full name of the object that threw the exception*/
	std::string who() const noexcept;
	/** change the object for use with a cascading object*/
	void updateObject(coreObject *newobj)
	{
		throwingObject = newobj;
	}
};

/** exception for use when an object is added to another object but the object
is not of a type that can be added
*/
class unrecognizedObjectException : public coreObjectException
{
public:
	explicit unrecognizedObjectException(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "unrecognized object";
	}
};

class objectAddFailure : public coreObjectException
{
public:
	explicit objectAddFailure(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "failure to add object";
	}
};

class objectRemoveFailure : public coreObjectException
{
public:
	explicit objectRemoveFailure(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "failure to remove object";
	}
};

class unrecognizedParameter : public std::invalid_argument
{
public:
	unrecognizedParameter():std::invalid_argument("unrecognized Parameter") {};
};

class invalidParameterValue : public std::invalid_argument
{
public:
	invalidParameterValue() :std::invalid_argument("invalid parameter value") {};
};

class cloneFailure : public coreObjectException
{
public:
	explicit cloneFailure(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "clone failure";
	}
};


class fileOperationError : public gridDynException
{
private:
	std::string message;
public:
	explicit fileOperationError(const std::string &error_message = "file operation error"):message(error_message) {};
	virtual const char *what() const noexcept override
	{
		return message.c_str();
	}
};
class invalidFileName : public fileOperationError
{
public:
	explicit invalidFileName(const std::string &error_message = "file name is invalid"):fileOperationError(error_message) {};
};
#endif

