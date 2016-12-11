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

#ifndef GRIDDYNEXCEPTIONS_H_

#define GRIDDYNEXCEPTIONS_H_

#include "gridCore.h"
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
	coreObjectException(coreObject *obj);
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


class invalidObjectException : public coreObjectException
{
public:
	invalidObjectException(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "invalid object";
	}
};

class objectAddFailure : public coreObjectException
{
public:
	objectAddFailure(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "failure to add object";
	}
};

class objectRemoveFailure : public coreObjectException
{
public:
	objectRemoveFailure(coreObject *obj) :coreObjectException(obj) {};
	virtual const char *what() const noexcept override
	{
		return "failure to remove object";
	}
};

class unrecognizedParameter : public gridDynException
{
public:
	unrecognizedParameter() {};
	virtual const char *what() const noexcept override
	{
		return "unrecognized Parameter";
	}
};

class invalidParameterValue : public gridDynException
{
public:
	invalidParameterValue(){};
	virtual const char *what() const noexcept override
	{
		return "invalid parameter value";
	}
};

class cloneFailure : public coreObjectException
{
public:
	cloneFailure(coreObject *obj) :coreObjectException(obj) {};
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
	fileOperationError(std::string error_message = "fileOperation error"):message(error_message) {};
	virtual const char *what() const noexcept override
	{
		return message.c_str();
	}
};

class invalidFileName : public fileOperationError
{
public:
	invalidFileName(std::string error_message = "file name is invalid"):fileOperationError(error_message) {};
};
#endif

