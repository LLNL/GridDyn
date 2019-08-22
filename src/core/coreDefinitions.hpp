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

#include "../utilities/indexTypes.hpp"
#include "gmlc/utilities/timeRepresentation.hpp"

#include <string>
#include <type_traits>
#include <vector>

namespace griddyn
{
constexpr double kBigNum (1e49);  //!< a very large number
constexpr int kBigINT (0x7EDCBA98);  //!< a big arbitrary integer

constexpr double kHalfBigNum (kBigNum / 2.0);  //!< half of a very big number

constexpr double kNullVal (-1.456e47);  //!< a very large negative number used for an empty value

#define FUNCTION_EXECUTION_SUCCESS (0)
#define FUNCTION_EXECUTION_FAILURE (-1)

// bookkeeping object changes
#define UPDATE_TIME_CHANGE (1435)
#define OBJECT_NAME_CHANGE (1445)
#define OBJECT_ID_CHANGE (1446)
#define OBJECT_IS_SEARCHABLE (1455)
#define UPDATE_REQUIRED (2010)  //!< indicator that an object is using an update function
#define UPDATE_NOT_REQUIRED (2011)  //!< indicator that an object is no longer using an update function

/** enumeration of print levels for logging
 */
enum class print_level : int
{
    no_print = 0,  //!< never print
    error = 1,  //!< only print errors
    warning = 2,  //!< print/log warning and errors
    summary = 3,  //!< print a summary
    normal = 4,  //!< defualt print level
    debug = 5,  //!< debug level prints
    trace = 6,  //!< trace level printing
};

using id_type_t = std::int64_t;
constexpr auto invalid_id_value = std::numeric_limits<id_type_t>::min ();
using coreTime = TimeRepresentation<count_time<9>>;
using stringVec = std::vector<std::string>;

/** commonly used time expressions*/
constexpr coreTime maxTime = coreTime::maxVal ();
constexpr coreTime negTime = coreTime::minVal ();
constexpr coreTime timeZero = coreTime::zeroVal ();

constexpr coreTime timeOneSecond (1.0);

constexpr coreTime kDayLength (86400.0f);
constexpr coreTime kSmallTime (1e-7);
constexpr coreTime kShortTime (1e-6);

constexpr coreTime operator"" _t (long double val) { return coreTime (val); }

// create an inline check for valid indices
#ifdef UNSIGNED_INDEXING
template <class VX>
inline bool isValidIndex (index_t index, const std::vector<VX> &vector_obj)
{
    return (index < static_cast<count_t>(vector_obj.size());
}
#else
template <class VX>
inline bool isValidIndex (index_t index, const std::vector<VX> &vector_obj)
{
    return ((index >= 0) && (index < static_cast<count_t> (vector_obj.size ())));
}
#endif

template <class IND, class VX>
inline bool isValidIndex (IND index, const std::vector<VX> &vector_obj)
{
    return ((index >= 0) && (index < static_cast<IND> (vector_obj.size ())));
}
}  // namespace griddyn
