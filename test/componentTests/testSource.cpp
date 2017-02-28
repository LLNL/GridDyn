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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "readElement.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "sourceModels/sourceTypes.h"
#include "timeSeries.h"
#include <cstdio>

#define SOURCE_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/source_tests/"

BOOST_FIXTURE_TEST_SUITE (source_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (source_test1)
{
  gridSource *src1 = new gridSource ("gs",45.0);

  BOOST_CHECK_CLOSE (src1->getOutput(), 45.0,0.001);
  src1->set ("value", 22.0);
  BOOST_CHECK_CLOSE (src1->getOutput(), 22.0, 0.001);
  delete src1;

  src1 = new rampSource ("rs",1.0);

  BOOST_CHECK_CLOSE (src1->getOutput (), 1.0, 0.001);
  src1->set ("rate", 0.5);
  src1->dynInitializeA (0.0, 0);  
  src1->timestep (3.0, noInputs, cLocalSolverMode);
  double rval = src1->getOutput();
  BOOST_CHECK_CLOSE (rval, 2.5, 0.001);
  src1->set ("rate", -0.75);
  src1->timestep (5.0, noInputs, cLocalSolverMode);
  rval = src1->getOutput();
  BOOST_CHECK_CLOSE (rval, 1.0, 0.001);
  delete src1;

  src1 = new pulseSource ("ps",1.0);

  BOOST_CHECK_CLOSE (src1->getOutput (), 1.0, 0.001);
  src1->set ("type","square");
  src1->set ("period", 2);
  src1->set ("a", 1.0);
  src1->dynInitializeA (0.0, 0);
  src1->timestep (2.5, noInputs, cLocalSolverMode);
  rval = src1->getOutput();
  BOOST_CHECK_CLOSE (rval, 2.0, 0.001);
  src1->set ("period", 1);
  src1->timestep (2.75, noInputs, cLocalSolverMode);
  rval = src1->getOutput();
  BOOST_CHECK_CLOSE (rval, 1.0, 0.001);
  delete src1;

}

BOOST_AUTO_TEST_SUITE_END()