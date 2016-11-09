/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "gridDynActions.h"
#include "basicDefs.h"
#include "stringOps.h"

gridDynAction::gridDynAction()
{

}

gridDynAction::gridDynAction(const std::string &operation)
{
	process(operation);
}


void gridDynAction::reset()
{
	command = gd_action_t::ignore;
	string1 = "";
	string2 = "";
	val_double = kNullVal;
	val_double2 = kNullVal;
	val_int = -1;
}

int gridDynAction::process(const std::string &operation)
{
	/* (s) string,  (d) double,  (i) int, (X)* optional, (s|d|i), string or double or int*/
	auto ssep = splitline(operation, " ",delimiter_compression::on); 
	size_t sz = ssep.size();
	for (size_t kk = 0; kk < sz; ++kk)
	{
		if (ssep[kk][0] == '#')  //clear all the comments
		{
			sz=kk;
		}
	}
	reset();
	if (sz==0)  //check if there was no command
	{
		return OBJECT_ADD_SUCCESS;
	}
	std::string cmd = convertToLowerCase(ssep[0]);
	
	int out = OBJECT_ADD_SUCCESS;
	if (cmd == "ignore") //ignore XXXXXX
	{
		command = gd_action_t::ignore;
	}
	else if (cmd == "set")  //set parameter(s) value(d)
	{
		command = gd_action_t::set;
		if (sz >= 3)
		{
			string1 = ssep[1];
			val_double = doubleRead(ssep[2], kNullVal);
			if (val_double == kNullVal)
			{
				string2 = ssep[2];
			}
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "setall")//setall  objecttype(s) parameter(s) value(d)
	{
		command = gd_action_t::setall;
		if (sz >= 4)
		{
			string1 = ssep[1];
			string2 = ssep[2];
			val_double = doubleRead(ssep[3], kNullVal);
			if (val_double == kNullVal)
			{
				out = OBJECT_ADD_FAILURE;
			}
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "setsolver")  //setsolver mode solver
	{
		command = gd_action_t::setsolver;
		if (sz >= 3)
		{
			string1 = ssep[1];
			val_int = intRead(ssep[2], -435);//-435 is some random number with no meaning outside this call
			if (val_int == -435)
			{
				string1 = ssep[2];
			}
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "settime") //settime newtime(d)
	{
		command = gd_action_t::settime;
		if (sz >= 2)
		{
			val_double = doubleRead(ssep[1], kNullVal);
			if (val_double == kNullVal)
			{
				out = OBJECT_ADD_FAILURE;
			}
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}

	}
	else if (cmd == "print") //print parameter(s) setstring(s)
	{
		command = gd_action_t::print;
		if (sz >= 3)
		{
			string1 = ssep[1];
			string2 = ssep[2];
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "powerflow") //powerflow
	{
		command = gd_action_t::powerflow;
	}
	else if (cmd == "step") //step solutionType*
	{
		command = gd_action_t::step;
		if (sz > 1)
		{
			string1 = ssep[1];
		}
	}
	else if (cmd == "eventmode")  //eventmode tstop*  tstep*
	{
		command = gd_action_t::eventmode;
		if (sz > 1)
		{
			val_double = doubleRead(ssep[1], kNullVal);
			if (sz > 2)
			{
				val_double2 = doubleRead(ssep[2], kNullVal);
			}
		}
	}
	else if (cmd == "initialize") //initialize 
	{
		command = gd_action_t::initialize;
	}
	else if (cmd == "dynamic") //dynamic "dae"|"part"|"decoupled" stoptime(d)* steptime(d)*
	{
		if (sz == 1)
		{
			command = gd_action_t::dynamicDAE;
		}
		else
		{
			makeLowerCase(ssep[1]);
			if (ssep[1] == "dae")
			{
				command = gd_action_t::dynamicDAE;
				if (sz > 2)
				{
					val_double = doubleRead(ssep[2], kNullVal);
				}
			}
			else if ((ssep[1]=="part")||(ssep[1]=="partitioned"))
			{
				command = gd_action_t::dynamicPart;
				if (sz > 2)
				{
					val_double = doubleRead(ssep[2], kNullVal);
				}
				if (sz > 3)
				{
					val_double2 = doubleRead(ssep[3], kNullVal);
				}
			}
			else if (ssep[1] == "decoupled")
			{
				command = gd_action_t::dynamicDecoupled;
				if (sz > 2)
				{
					val_double = doubleRead(ssep[2], kNullVal);
				}
				if (sz > 3)
				{
					val_double2 = doubleRead(ssep[3], kNullVal);
				}
			}
			else
			{
				command = gd_action_t::dynamicDAE;
				val_double=doubleRead(ssep[2], kNullVal);
				if (val_double == kNullVal)
				{
					out = OBJECT_ADD_FAILURE;
				}
				else if (sz > 2)
				{
					val_double2 = doubleRead(ssep[3], kNullVal);
				}
			}
			
		}
	}
	else if (cmd == "dynamicdae") //dynamicdae stoptime(d)*
	{
		command = gd_action_t::dynamicDAE;

		if (sz > 1)
		{
			val_double = doubleRead(ssep[2], kNullVal);
		}

	}
	else if (cmd == "dynamicpart") //dynamicpart stoptime(d)* steptime(d)*
	{
		command = gd_action_t::dynamicPart;

		if (sz > 1)
		{
			val_double = doubleRead(ssep[2], kNullVal);
			if (sz >2)
			{
				val_double2 = doubleRead(ssep[3], kNullVal);
			}
		}

	}
	else if (cmd == "dynamicdecoupled") //dynamicdecoupled stop(d)* step(d)*
	{
		command = gd_action_t::dynamicPart;

		if (sz > 1)
		{
			val_double = doubleRead(ssep[2], kNullVal);
			if (sz >2)
			{
				val_double2 = doubleRead(ssep[3], kNullVal);
			}
		}

	}
	else if (cmd == "reset") //reset level(i)
	{
		command = gd_action_t::reset;
		if (sz > 1)
		{
			val_int = intRead(ssep[1], -435);
			if (val_int == -435)
			{
				out = OBJECT_ADD_FAILURE;
			}
		}
		else
		{
			val_int = 0;
		}
	}
	else if (cmd == "iterate") //iterate interval(d)* stoptime(d)*
	{
		command = gd_action_t::iterate;
		if (sz > 1)
		{
			
			val_double = doubleRead(ssep[2], kNullVal);
			if (sz > 2)
			{
				val_double2 = doubleRead(ssep[3], kNullVal);
			}
		}

	}
	else if (cmd == "run") //run time(d)*
	{
		command = gd_action_t::run;
		if (sz > 1)
		{
			val_double = doubleRead(ssep[1], kNullVal);
			if (val_double == kNullVal)
			{
				out = OBJECT_ADD_FAILURE;
			}
		}
		else
		{
			val_double = kNullVal;
		}
	}
	else if (cmd == "save") //save subject(s) file(s)
	{
		command = gd_action_t::save;
		if (sz > 2)
		{
			string1 = ssep[1];
			string2 = ssep[2];
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "load") //load subect(s) file(s)
	{
		command = gd_action_t::load;
		if (sz > 2)
		{
			string1 = ssep[1];
			string2 = ssep[2];
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "add") // add addstring(s)
	{
		command = gd_action_t::add;
		if (sz > 1)
		{
			string1 = ssep[1];
			for (size_t kk = 2; kk < sz; ++kk)
			{
				string1 += " " + ssep[kk];
			}
		}
		else
		{
			out = OBJECT_ADD_FAILURE;
		}
	}
	else if (cmd == "rollback") //rollback point(s|d)
	{
		command = gd_action_t::rollback;
		if (sz > 1)
		{
			val_double = doubleRead(ssep[1], kNullVal);
			if (val_double == kNullVal)
			{
				string1 = ssep[1];
			}
		}
		else
		{
			string1 = "last";
		}

	}
	else if (cmd == "checkpoint") //checkpoint name(s)
	{
		command = gd_action_t::checkpoint;
		if (sz > 1)
		{
			val_double = doubleRead(ssep[1], kNullVal);
			if (val_double == kNullVal)
			{
				string1 = ssep[1];
			}
		}
		else
		{
			string1 = "";
		}
	}

	else if (cmd == "contingency") // contingency mode|filename output_file|method 
	{
		command = gd_action_t::contingency;
		string1 = ssep[1];
		if (sz > 2)
		{
			string2 = ssep[2];
		}
	}
	else if (cmd == "continuation")
	{

	}
	return out;
}