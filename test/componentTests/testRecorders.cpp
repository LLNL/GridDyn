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

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "gridDyn.h"
#include "gridDynFileInput.h"
#include "testHelper.h"
#include "collector.h"
#include "gridEvent.h"
#include "timeSeriesMulti.h"
#include <cstdio>
#include <cmath>

//test case for coreObject object

#define RECORDER_TEST_DIRECTORY GRIDDYN_TEST_DIRECTORY "/recorder_tests/"
static const std::string collector_test_directory = std::string(GRIDDYN_TEST_DIRECTORY "/recorder_tests/");

BOOST_FIXTURE_TEST_SUITE (recorder_tests, gridDynSimulationTestFixture)

BOOST_AUTO_TEST_CASE (tsMulti_tests)
{
  timeSeriesMulti<> ts2;
  timeSeriesMulti<> ts3 (1,10);
  std::vector<double> tv (10);
  std::vector<double> val (10);
  ts2.setCols (1);
  double t = 0.0;
  for (int kk = 0; kk < 10; ++kk)
    {
      ts2.addData (t, 4.5);
      tv[kk] = t;
      val[kk] = 4.5;
      t = t + 1.0;
    }
  BOOST_CHECK_EQUAL (ts2.count, 10u);
  ts3.addData (tv, val);
  BOOST_CHECK_EQUAL (ts3.count, 10u);

  BOOST_CHECK_SMALL (compare (&ts2,&ts3), 0.0001);

  ts3.setCols (4);
  ts3.addData (val, 1);
  ts3.addData (val, 2);
  ts3.addData (val, 3);
  ts3.data[3][4] = 6.5;

  BOOST_CHECK_CLOSE (compare (&ts2, &ts3, 0, 3), 2.0, 0.0001);

}

BOOST_AUTO_TEST_CASE (file_save_tests)
{
  timeSeriesMulti<> ts2(1);
  double t = 0.0;
  for (int kk = 0; kk < 10; ++kk)
    {
      ts2.addData (t, 4.5);
      t = t + 1.0;
    }
  BOOST_CHECK_EQUAL (ts2.count, 10u);
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "ts_test.dat");
  ts2.writeBinaryFile (fname);

  timeSeriesMulti<> ts3;

  ts3.loadBinaryFile (fname);
  BOOST_CHECK_EQUAL (ts3.cols, 1u);
  BOOST_CHECK_EQUAL (ts3.count, 10u);
  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
  int ret = remove (fname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (file_save_tests2)
{
  timeSeriesMulti<> ts2;
  ts2.setCols (4); //test the set cols method
  BOOST_CHECK(ts2.cols == 4);

  std::vector<double> vt {4.5,5.5,6.5,7.5};

  double t = 0.0;
  for (int kk = 0; kk < 30; ++kk)
    {
      ts2.addData (t, vt);
      t = t + 1.0;
    }
  BOOST_CHECK_EQUAL (ts2.count, 30u);
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "ts_test2.dat");
  ts2.writeBinaryFile (fname);

  timeSeriesMulti<> ts3(fname);

  BOOST_CHECK_EQUAL (ts3.cols, 4u);
  BOOST_CHECK_EQUAL (ts3.count, 30u);
  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);

  ts3.data[3][2] = 7;
  double diff = compare (&ts2, &ts3);
  BOOST_CHECK_CLOSE (diff, 0.5, 0.001);
  BOOST_CHECK_EQUAL (ts3.count, ts3.data[3].size ());
  int ret = remove (fname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);
}

BOOST_AUTO_TEST_CASE (recorder_test1)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test.xml");
  gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  gds->consolePrintLevel = print_level::debug;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 1);
  gds->set ("recorddirectory", collector_test_directory);
 
  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "loadrec.dat");
  timeSeriesMulti<> ts3(recname);
  BOOST_CHECK(ts3.fields[0] == "load3:power");
  BOOST_CHECK_EQUAL (ts3.count, 31u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL(ret, 0);
  

  

}

BOOST_AUTO_TEST_CASE (recorder_test2)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test2.xml");
  gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 1);
  gds->set ("recorddirectory", collector_test_directory);
  
  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "genrec.dat");
  timeSeriesMulti<> ts3(recname);

  BOOST_CHECK_EQUAL (ts3.count, 31u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  int ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

}

BOOST_AUTO_TEST_CASE (recorder_test3)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test3.xml");
  gds = static_cast<gridDynSimulation *>(readSimXMLFile(fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 3);
  gds->set ("recorddirectory", collector_test_directory);
  
  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "genrec.dat");
  timeSeriesMulti<> ts3(recname);

  BOOST_CHECK_EQUAL (ts3.count, 121u);
  BOOST_CHECK_EQUAL (ts3.cols, 4u);
  int ret = remove(recname.c_str());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  ts3.loadBinaryFile (recname);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  ret = remove(recname.c_str());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "loadrec.dat");
  ts3.loadBinaryFile (recname);
 

  BOOST_CHECK_EQUAL (ts3.count, 31u);
  BOOST_CHECK_EQUAL (ts3.cols, 1u);
	ret = remove(recname.c_str());
  BOOST_CHECK_EQUAL (ret, 0);

}

//testing the recorder as subObject and recorder found via link
BOOST_AUTO_TEST_CASE (recorder_test4)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test4.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeriesMulti<> ts3(recname);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2(recname);

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}


//testing the vector recorders of the simulation object
BOOST_AUTO_TEST_CASE (recorder_test5)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test5.xml");
  gds = dynamic_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busVrec.dat");
  timeSeriesMulti<> ts3;
  ts3.loadBinaryFile (recname);
 

  BOOST_CHECK_EQUAL(ts3.fields[2], "bus3:voltage");
  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 4u);
  int ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);
  recname = std::string (RECORDER_TEST_DIRECTORY "linkVrec.dat");

  ts3.loadBinaryFile (recname);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 5u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

}

// testing multiple items in a field
BOOST_AUTO_TEST_CASE (recorder_test6)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test6.xml");
  readerConfig::setPrintMode (0);
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeriesMulti<> ts3;
  ts3.loadBinaryFile (recname);
  

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  int ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);
 

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}

// testing multiple :: naming for fields
BOOST_AUTO_TEST_CASE (recorder_test7)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test7.xml");
  readerConfig::setPrintMode (0);
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val=gds->getInt ("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeriesMulti<> ts3;
  ts3.loadBinaryFile (recname);

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 2u);
  int ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);
  

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 2u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}

// testing multiple :: naming for fields and units
BOOST_AUTO_TEST_CASE (recorder_test8)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test8.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);
  int val = gds->getInt("recordercount");
  BOOST_CHECK_EQUAL (val, 2);
  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();

  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec.dat");
  timeSeriesMulti<> ts3;
  ts3.loadBinaryFile (recname);
 

  BOOST_CHECK_EQUAL (ts3.count, 61u);
  BOOST_CHECK_EQUAL (ts3.cols, 3u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  //check to make sure the conversion is correct
  BOOST_CHECK_SMALL (ts3.data[0][3] * 180 / kPI - ts3.data[2][3],0.0001);


  recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);
  

  BOOST_CHECK_EQUAL (ts2.count, 61u);
  BOOST_CHECK_EQUAL (ts2.cols, 3u);
  ret = remove (recname.c_str ());

  BOOST_CHECK_EQUAL (ret, 0);

  BOOST_CHECK_SMALL (compare (&ts2, &ts3), 0.00001);
}

//testing gain and bias
BOOST_AUTO_TEST_CASE (recorder_test9)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test9.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);
 

  BOOST_CHECK_EQUAL (ts2.count, 21u);
  BOOST_CHECK_EQUAL (ts2.cols, 4u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (ts2.data[1][2] - 1.0, ts2.data[3][2], 0.0001);


}

//testing multiple grabber calculations
BOOST_AUTO_TEST_CASE (recorder_test10)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test10.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);
  

  BOOST_CHECK_EQUAL (ts2.count, 11u);
  BOOST_CHECK_EQUAL (ts2.cols, 3u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (ts2.data[0][2] - ts2.data[1][2], ts2.data[2][2], 0.0001);
  BOOST_CHECK_CLOSE (ts2.data[0][8] - ts2.data[1][8], ts2.data[2][8], 0.0001);

}

BOOST_AUTO_TEST_CASE (recorder_test11)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test11.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);
  

  BOOST_CHECK_EQUAL (ts2.count, 11u);
  BOOST_CHECK_EQUAL (ts2.cols, 4u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (ts2.data[0][2] - (ts2.data[1][2] - ts2.data[2][2]), ts2.data[3][2], 0.0001);
  BOOST_CHECK_CLOSE (ts2.data[0][8] - (ts2.data[1][8] - ts2.data[2][8]), ts2.data[3][8], 0.0001);

}

//testing function calls
BOOST_AUTO_TEST_CASE (recorder_test12)
{
  std::string fname = std::string (RECORDER_TEST_DIRECTORY "recorder_test12.xml");
  gds = static_cast<gridDynSimulation *> (readSimXMLFile (fname));
  BOOST_CHECK_EQUAL (readerConfig::warnCount, 0);
  gds->consolePrintLevel = print_level::no_print;
  gds->solverSet("dynamic", "printlevel", 0);

  gds->set ("recorddirectory", collector_test_directory);

  gds->run ();


  std::string recname = std::string (RECORDER_TEST_DIRECTORY "busrec2.dat");
  timeSeriesMulti<> ts2;
  ts2.loadBinaryFile (recname);


  BOOST_CHECK_EQUAL (ts2.count, 11u);
  BOOST_CHECK_EQUAL (ts2.cols, 3u);
  int ret = remove (recname.c_str ());
  BOOST_CHECK_EQUAL (ret, 0);
  BOOST_CHECK_CLOSE (sin (ts2.data[0][2] - ts2.data[1][2]), ts2.data[2][2], 0.0001);
  BOOST_CHECK_CLOSE (sin (ts2.data[0][8] - ts2.data[1][8]), ts2.data[2][8], 0.0001);

}

//test and invalid input
BOOST_AUTO_TEST_CASE(recorder_test_bad_input)
{
	std::string fname = collector_test_directory+"recorder_test_invalid_field1.xml";
	printf("NOTE:  this should produce some warning messages\n");
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
	
	BOOST_CHECK_GT(readerConfig::warnCount, 0);
	delete gds;
	readerConfig::warnCount = 0;

	fname = collector_test_directory + "recorder_test_invalid_field2.xml";
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));

	BOOST_CHECK_GT(readerConfig::warnCount, 0);
	delete gds;
	readerConfig::warnCount = 0;
	gds = nullptr;
	
}

//testing if the recorders have any material impact on the results
BOOST_AUTO_TEST_CASE(recorder_test_period)
{
	std::string fname = collector_test_directory+"recorder_test_sineA.xml";
	gds = static_cast<gridDynSimulation *> (readSimXMLFile(fname));
	BOOST_CHECK_EQUAL(readerConfig::warnCount, 0);
	gds->consolePrintLevel = print_level::no_print;
	gds->solverSet("dynamic", "printlevel", 0);

	gds->set("recorddirectory", collector_test_directory);

	gds->run();

	std::string recname = std::string(RECORDER_TEST_DIRECTORY "recorder_dataA.dat");
	timeSeriesMulti<> tsA(recname);

	std::string fname2 = collector_test_directory + "recorder_test_sineB.xml";
	gds2 = static_cast<gridDynSimulation *> (readSimXMLFile(fname2));
	BOOST_CHECK_EQUAL(readerConfig::warnCount, 0);
	gds2->consolePrintLevel = print_level::no_print;
	gds2->solverSet("dynamic", "printlevel", 0);

	gds2->set("recorddirectory", collector_test_directory);

	gds2->run();

	std::string recname2 = std::string(RECORDER_TEST_DIRECTORY "recorder_dataB.dat");
	timeSeriesMulti<> tsB(recname2);

	size_t diffc = 0;
	BOOST_REQUIRE((tsA.count - 1) * 4 == (tsB.count - 1));
	for (decltype(tsA.count) ii=1;ii<tsA.count;++ii)
	{
		for (decltype(tsA.cols) jj = 0; jj<tsA.cols; ++jj)
		if (std::abs(tsA.data[jj][ii]-tsB.data[jj][4*ii])>1e-4) //TODO:: this is still small but bigger than it really should be
		{
			++diffc;
		}
	}
	BOOST_CHECK(diffc == 0);
	remove(recname.c_str());
	remove(recname2.c_str());
}

BOOST_AUTO_TEST_SUITE_END ()
