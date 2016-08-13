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

#ifndef BASICDEFS_H_
#define BASICDEFS_H_

const double kDayLength (86500.0f);
const double kSmallTime (1e-9);
const double kShortTime (1e-6);

const double kPI (3.1415926535897931);  //defining a Pi constant
const double kWS (60.0 * 2.0 * kPI);  //radians per second at 60 Hz
const double kWS50 (50.0 * 2.0 * kPI); //radians per second at 50 Hz

const double kMin_Res (1e-7);

#define PARAMETER_FOUND (0)
#define PARAMETER_NOT_FOUND (-1)
#define INVALID_PARAMETER_VALUE (-2)
#define PARAMETER_VALUE_ABOVE_LIMIT (-3)
#define PARAMETER_VALUE_BELOW_LIMIT (-4)
#define INVALID_PARAMETER_UNIT (-5)

#define LOADED (0)
#define NOT_LOADED (-1)

#define OBJECT_ALREADY_MEMBER (1)
#define OBJECT_ADD_SUCCESS (0)
#define OBJECT_NOT_MEMBER (1)
#define OBJECT_REMOVE_SUCCESS (0)
#define OBJECT_ADD_FAILURE (-1)
#define OBJECT_NOT_RECOGNIZED (-2)
#define OBJECT_NOT_FOUND (-3)
#define OBJECT_REMOVE_FAILURE (-1)

#define FUNCTION_EXECUTION_SUCCESS (0)
#define FUNCTION_EXECUTION_FAILURE (-1)


//A few ERROR codes
#define NO_SLACK_BUS_FOUND (-2)
//alert codes

#define TRANSLINE_ANGLE_TRIP (21)
#define TRANSLINE_LIMIT_TRIP (22)
//generator trip conditions
#define GENERATOR_UNDERFREQUENCY_TRIP (31)
#define GENERATOR_FAULT (32)
#define GENERATOR_OVERSPEED_TRIP (33)
//bus fault conditions
#define BUS_UNDER_POWER (40)
#define BUS_UNDER_VOLTAGE (41)
#define BUS_UNDER_FREQUENCY (42)
//load trip conditions
#define LOAD_TRIP (50)
#define UNDERVOLT_LOAD_TRIP (51)
#define UNDERFREQUENCY_LOAD_TRIP (52)
#define UNBALANCED_LOAD_ALERT (54)
//switch change messages
#define SWITCH_OPEN (60)
#define SWITCH_CLOSE (61)
#define SWITCH1_CLOSE (62)
#define SWITCH1_OPEN (63)
#define SWITCH2_CLOSE (64)
#define SWITCH2_OPEN (65)
//fuse change messages
#define FUSE_BLOWN (70)
#define FUSE_BLOWN_CURRENT (71)
#define FUSE_BLOWN_ANGLE (72)
#define FUSE1_BLOWN_ANGLE (73)
#define FUSE1_BLOWN_CURRENT (74)
#define FUSE2_BLOWN_ANGLE (75)
#define FUSE2_BLOWN_CURRENT (76)
//breaker change messages
#define BREAKER_TRIP (80)
#define BREAKER_TRIP_CURRENT (81)
#define BREAKER_TRIP_ANGLE (82)
#define BREAKER1_TRIP_CURRENT (83)
#define BREAKER1_TRIP_ANGLE (84)
#define BREAKER2_TRIP_CURRENT (85)
#define BREAKER2_TRIP_ANGLE (86)
#define BREAKER_RECLOSE (87)
#define BREAKER1_RECLOSE (88)
#define BREAKER2_RECLOSE (89)

//bookkeeping object changes
#define UPDATE_TIME_CHANGE (1435)
#define OBJECT_NAME_CHANGE (1445)
#define OBJECT_ID_CHANGE (1446)
#define OBJECT_IS_SEARCHABLE (1455)

//logging codes
#define GD_TRACE_PRINT 6
#define GD_DEBUG_PRINT 5
#define GD_NORMAL_PRINT 4
#define GD_SUMMARY_PRINT 3
#define GD_WARNING_PRINT 2
#define GD_ERROR_PRINT 1
#define GD_NO_PRINT 0

#endif