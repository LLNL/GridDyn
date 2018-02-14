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


// test case for coreObject object

#include "jsonReaderElement.h"
#include "testHelper.h"
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

static const std::string elementReaderTestDirectory (GRIDDYN_TEST_DIRECTORY "/element_reader_tests/");

BOOST_AUTO_TEST_SUITE (jsonElementReader_tests)

BOOST_AUTO_TEST_CASE (jsonElementReader_test1)
{
    jsonReaderElement reader;
    BOOST_REQUIRE (reader.loadFile (elementReaderTestDirectory + "json_test1.json"));
    BOOST_CHECK (reader.isValid ());
    auto firstChild = reader.clone ();
    BOOST_CHECK (firstChild != nullptr);
    BOOST_CHECK (firstChild->getName () == elementReaderTestDirectory + "json_test1.json");

    auto sibling = firstChild->nextSibling ();
    BOOST_CHECK (sibling->isValid () == false);

    auto att1 = firstChild->getFirstAttribute ();
    BOOST_CHECK (att1.getName () == "age");
    BOOST_CHECK (att1.getValue () == 25);
    BOOST_CHECK (att1.getText () == "25");
    auto att2 = firstChild->getNextAttribute ();
    BOOST_CHECK (att2.getName () == "firstName");
    BOOST_CHECK (att2.getValue () == readerNullVal);
    BOOST_CHECK (att2.getText () == "John");
    att2 = firstChild->getNextAttribute ();
    BOOST_CHECK (att2.getName () == "isAlive");
    BOOST_CHECK (att2.getText () == "true");
    att2 = firstChild->getNextAttribute ();
    BOOST_CHECK (att2.getName () == "lastName");
    BOOST_CHECK (att2.getText () == "Smith");

    att2 = firstChild->getNextAttribute ();
    BOOST_CHECK (att2.isValid () == false);

    firstChild->moveToFirstChild ();
    BOOST_CHECK (firstChild->isValid ());
    BOOST_CHECK (firstChild->getName () == "address");
    firstChild->moveToNextSibling ();
    BOOST_CHECK (firstChild->isValid ());
    BOOST_CHECK (firstChild->getName () == "phoneNumbers");
    firstChild->moveToNextSibling ();
    BOOST_CHECK (firstChild->isValid ());
    BOOST_CHECK (firstChild->getName () == "phoneNumbers");
    firstChild->moveToNextSibling ();
    BOOST_CHECK (firstChild->isValid ());
    BOOST_CHECK (firstChild->getName () == "phoneNumbers");
    firstChild->moveToNextSibling ();
    BOOST_CHECK (firstChild->isValid () == false);
    firstChild->moveToParent ();
    BOOST_CHECK (firstChild->getName () == elementReaderTestDirectory + "json_test1.json");
}


BOOST_AUTO_TEST_CASE (jsonElementReader_test2)
{
    jsonReaderElement reader;
    // test a bad file
    reader.loadFile (elementReaderTestDirectory + "xmlElementReader_missing_file.xml");
    std::cout << "NOTE:: this should have a message about a missing file >>testing bad file input\n";
    BOOST_CHECK (reader.isValid () == false);
    reader.loadFile (elementReaderTestDirectory + "json_test2.json");
    BOOST_CHECK (reader.isValid () == true);
    auto firstChild = reader.clone ();

    auto sibling = firstChild->firstChild ();
    BOOST_CHECK (sibling->getName () == "bus");
    auto cChild = sibling->nextSibling ();
    BOOST_CHECK (cChild->isValid () == false);
    sibling->moveToNextSibling ();
    BOOST_CHECK (sibling->isValid () == false);

    auto busElement = firstChild->firstChild ();

    BOOST_CHECK (busElement->getAttributeText ("type") == "SLK");

    // Go through the children
    auto att1 = busElement->getFirstAttribute ();
    BOOST_CHECK (att1.getName () == "angle");
    BOOST_CHECK (att1.getValue () == 0.0);
    att1 = busElement->getNextAttribute ();
    BOOST_CHECK (att1.getName () == "name");
    BOOST_CHECK (att1.getText () == "bus1");
    att1 = busElement->getNextAttribute ();
    att1 = busElement->getNextAttribute ();
    BOOST_CHECK (att1.getName () == "voltage");
    BOOST_CHECK (att1.getText () == "1.04");
    BOOST_CHECK_CLOSE (att1.getValue (), 1.04, 0.000001);

    auto busChild = busElement->firstChild ();
    BOOST_CHECK (busChild->getName () == "generator");
    BOOST_CHECK (busChild->getText ().empty ());
    auto att2 = busChild->getFirstAttribute ();

    BOOST_CHECK (att2.getName () == "name");
    BOOST_CHECK (att2.getText () == "gen1");
    att2 = busChild->getNextAttribute ();
    BOOST_CHECK (att2.getName () == "p");
    BOOST_CHECK_CLOSE (att2.getValue (), 0.7160, 0.00001);

    // move busChild to the parent to make sure they are the same
    busChild->moveToParent ();
    BOOST_CHECK (busChild->getName () == busElement->getName ());

    // Now go back to the first child to do a few checks on attributes
    att1 = firstChild->getAttribute ("name");
    BOOST_CHECK (att1.getName () == "name");
    BOOST_CHECK (att1.getText () == "test1");
}

BOOST_AUTO_TEST_CASE (jsonElementReader_test3)
{
    jsonReaderElement reader (elementReaderTestDirectory + "xmlElementReader_test2.xml");
    std::cout << "NOTE:: this should have a message indicating format error >>testing bad file input\n";
    BOOST_CHECK (reader.isValid () == false);
    reader.loadFile (elementReaderTestDirectory + "json_test3.json");
    BOOST_CHECK (reader.isValid () == true);
    // test traversal using move commands
    auto main = reader.clone ();
    // bookmark the top level
    main->bookmark ();
    main->moveToFirstChild ("bus");
    main->moveToFirstChild ();
    // traverse to the generator
    BOOST_CHECK (main->getName () == "generator");
    main->restore ();
    // restore to the root
    BOOST_CHECK (main->isDocument () == true);
    // traverse to the second bus and check name
    main->moveToFirstChild ("bus");
    main->moveToNextSibling ("bus");
    BOOST_CHECK (main->isValid ());
    BOOST_CHECK (main->getAttributeText ("name") == "bus2");
    main->moveToParent ();
    main->moveToFirstChild ();
    main->moveToFirstChild ();
    // check we are in the generator now
    BOOST_CHECK (main->getAttributeText ("name") == "gen1");
    // make a bookmark
    main->bookmark ();
    // traverse to the second bus
    main->moveToParent ();
    main->moveToNextSibling ("bus");
    BOOST_CHECK_CLOSE (main->getAttributeValue ("voltage"), 1.01, 0.0000001);
    // traverse to the parent
    main->moveToParent ();
    // restore and check if we are in the generator again
    main->restore ();
    BOOST_CHECK_CLOSE (main->getAttributeValue ("p"), 0.7160, 0.0000001);
}

BOOST_AUTO_TEST_CASE (jsonElementReader_test4)
{
    /*auto reader = std::make_shared<jsonReaderElement>(xmlTestDirectory + "xmlElementReader_test3.xml");
    BOOST_CHECK(reader->getName() == "main_element");

    auto main = reader->clone();
    reader = nullptr;
    BOOST_CHECK(main->getName() == "main_element");
    main->bookmark();
    main->moveToFirstChild();
    auto tstr = main->getMultiText(", ");
    BOOST_CHECK(tstr == "part1, part2, part3");
    main->moveToFirstChild();
    //att1 is 23t"  should return as not a value
    double val = main->getAttributeValue("att1");
    BOOST_CHECK(val == kNullVal);
    main->moveToFirstChild();

    val = main->getValue();
    BOOST_CHECK(val == kNullVal);
    BOOST_CHECK(main->getText() == "45.3echo");
    main->restore();
    BOOST_CHECK(main->getName() == "main_element");
    */
}

BOOST_AUTO_TEST_SUITE_END ()