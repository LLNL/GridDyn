
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

#include <../ThirdParty/Minizip/minizip.h>
#include <../ThirdParty/Minizip/miniunz.h>

#include "zipUtilities.h"
#include <boost/filesystem.hpp>

static char zipname[] = "minizip";
static char ziparg1[] = "-o";
static char ziparg2[] = "-1";

int zip(const std::string &file, const std::vector<std::string> &filesToZip)
{

#define NUMBER_FIXED_ARGS 4
	

	std::vector<char> fileV(file.c_str(), file.c_str() + file.size() + 1u); //1u for /0 at end of string

	/* Input arguments to the corresponding minizip main() function call */
	/*
	Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]

	-o  Overwrite existing file.zip
	-a  Append to existing file.zip
	-0  Store only
	-1  Compress faster
	-9  Compress better

	-j  exclude path. store only the file name.
	*/
	std::vector<char *> argv{ zipname,ziparg1,ziparg2,fileV.data() };
	std::vector<std::vector<char>> filez(filesToZip.size());
	int argc = NUMBER_FIXED_ARGS + static_cast<int>(filesToZip.size());
	argv.resize(argc + 1, 0);
	
	

	/* need to copy over the arguments since theoretically minizip may modify the input arguments */
	for (size_t kk = 0; kk < filesToZip.size(); kk++) 
	{
		filez[kk].assign(filesToZip[kk].c_str(), filesToZip[kk].c_str()+filesToZip[kk].size() + 1u); //1u to copy the NULL at the end of the string
		argv[NUMBER_FIXED_ARGS + kk] = filez[kk].data();
	}
	/* minunz may change the current working directory */
	auto cpath = boost::filesystem::current_path();
	/* Zip */
	int status = minizip(argc, argv.data());

	/* Reset the current directory */
	boost::filesystem::current_path(cpath);

	return status;
}

static char unzipname[] = "miniunz";
static char unziparg1[] = "-x";
static char unziparg2[] = "-o";
static char unziparg4[] = "-d";

int unzip(const std::string &file, const std::string &directory)
{
	/*
	Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]
	-e  Extract without pathname (junk paths)
	-x  Extract with pathname
	-v  list files
	-l  list files
	-d  directory to extract into
	-o  overwrite files without prompting
	-p  extract crypted file using password
	*/

	
	int argc = 4;
	
	std::vector<char> fileV(file.c_str(), file.c_str() + file.size() + 1u); //1u for /0 at end of string
	std::vector<char> dirV(directory.c_str(), directory.c_str() + directory.size() + 1u);
	std::vector<char *> argv{ unzipname, unziparg1, unziparg2, fileV.data() };

	if (!directory.empty())
	{
		argc = 6;
		argv.resize(6);
		argv[4] = unziparg4;
		argv[5] =dirV.data();

		if (!boost::filesystem::exists(directory))
		{
			boost::filesystem::create_directories(directory);
			if (!boost::filesystem::exists(directory))
			{
				return (-3);
			}
		}
	}

	
	
	/* minunz may change the current working directory */
	auto cpath = boost::filesystem::current_path();
	
	/* Unzip */
	int status = miniunz(argc, argv.data());

	/* Reset the current directory */
	boost::filesystem::current_path(cpath);

	return status;
}