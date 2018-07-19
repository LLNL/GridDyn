/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#pragma once

#include "griddyn/compiler-config.h"
#include <vector>

#ifdef HAVE_STRING_VIEW
#include <string_view>
namespace utilities
{
	using string_view = std::string_view;
}
#else
#ifdef HAVE_EXP_STRING_VIEW
#include <experimental/string_view>
namespace utilities
{
	using string_view = std::experimental::string_view;
}
#else
#include <boost/version.hpp>
#if BOOST_VERSION / 100 % 1000 >= 61
#include <boost/utility/string_view.hpp>
namespace utilities
{
	using string_view=boost::string_view;
}
#else
#include <boost/utility/string_ref.hpp>
namespace utilities
{
    using string_view = boost::string_ref;
}
#endif  //boost old version
#endif
#endif
namespace utilities
{
	using string_viewVector = std::vector<string_view>;
}
