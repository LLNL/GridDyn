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






//test case for element readers

#include "testHelper.h"
#include "tinyxmlReaderElement.h"
#include "tinyxml2ReaderElement.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <iostream>

static const std::string elementReaderTestDirectory(GRIDDYN_TEST_DIRECTORY "/element_reader_tests/");

BOOST_AUTO_TEST_SUITE(elementReader_tests)

BOOST_AUTO_TEST_CASE(tinyxmlElementReader_test1)
{
	tinyxmlReaderElement reader;
	BOOST_REQUIRE(reader.loadFile(elementReaderTestDirectory + "xmlElementReader_test.xml"));
	auto firstChild = reader.clone();
	BOOST_CHECK(firstChild != nullptr);
	BOOST_CHECK(firstChild->getName() == "griddyn");
	//There is only one child
	auto sibling = firstChild->nextSibling();
	BOOST_CHECK(sibling == nullptr);
	sibling = firstChild->firstChild();
	BOOST_CHECK(sibling->getName() == "bus");
	auto cChild = sibling->nextSibling();
	BOOST_CHECK(cChild == nullptr);
	sibling->moveToNextSibling();
	BOOST_CHECK(sibling->isValid() == false);

	auto busElement= firstChild->firstChild();
	auto busChild = busElement->firstChild("type");
	BOOST_REQUIRE(busChild != nullptr);
	BOOST_CHECK(busChild->getName() == "type");

	auto val = busChild->getText();
	BOOST_CHECK(val == "SLK");

	//Go through the children
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "angle");
	BOOST_CHECK(busChild->getValue() == 0.0);
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "voltage");
	BOOST_CHECK(busChild->getText() == "1.04");
	BOOST_CHECK_CLOSE(busChild->getValue(),1.04,0.000001);

	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "generator");
	BOOST_CHECK(busChild->getText().empty());
	auto genChild = busChild->firstChild();
	BOOST_CHECK(genChild->getName() == "P");
	BOOST_CHECK_CLOSE(genChild->getValue(), 0.7160,0.00001);

	//Get the generator attributes
	auto att = busChild->getFirstAttribute();
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "gen1");
	//move busChild to the parent to make sure they are the same
	busChild->moveToParent();
	BOOST_CHECK(busChild->getName() == busElement->getName());

	//Now go back to the first child to do a few checks on attributes
	att = firstChild->getAttribute("name");
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "test1");

}


BOOST_AUTO_TEST_CASE(tinyxmlElementReader_test2)
{
	tinyxmlReaderElement reader;
	//test a bad file
	reader.loadFile(elementReaderTestDirectory + "xmlElementReader_testbbad.xml");
	std::cout << "NOTE:: this should have a message testing bad xml input and not fault\n";
	BOOST_CHECK(reader.isValid() == false);
	reader.loadFile(elementReaderTestDirectory + "xmlElementReader_test2.xml");
	BOOST_CHECK(reader.isValid() == true);

	auto main= reader.clone();
	//there should be 9 elements
	auto sub = main->firstChild();
	for (int kk = 0; kk < 8; ++kk)
	{
		sub->moveToNextSibling();
	}
	auto name = sub->getName();
	BOOST_CHECK(name == "elementWithAttributes");
	auto att = sub->getFirstAttribute();
	BOOST_CHECK(att.getText() == "A");
	att = sub->getNextAttribute();
	BOOST_CHECK(att.getValue() == 46);
	att = sub->getNextAttribute();
	BOOST_CHECK_CLOSE(att.getValue(), 21.345,0.000001);
	att = sub->getNextAttribute();
	BOOST_CHECK(att.getValue()==readerNullVal);
	BOOST_CHECK(att.getText() == "happy");
	att = sub->getNextAttribute();
	BOOST_CHECK(att.isValid() == false);

}

BOOST_AUTO_TEST_CASE(tinyxmlElementReader_test3)
{
	tinyxmlReaderElement reader(elementReaderTestDirectory + "xmlElementReader_test2.xml");
	//test traversal using move commands
	auto main = reader.clone();
	
	main->moveToFirstChild("subelementA");
	BOOST_CHECK(main->getName() == "subelementA");
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid()==false);
	//now move back to parent
	main->moveToParent();
	BOOST_CHECK(main->getName() == "main_element");
	//now loop through subElementB
	main->moveToFirstChild("subelementB");
	BOOST_CHECK(main->getName() == "subelementB");
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->getName() == "subelementB");
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->isValid() == false);

	//test traversal with different element names
	main->moveToParent();
	main->moveToFirstChild("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("elementWithAttributes");
	BOOST_CHECK(main->isValid());

	//Test traversal with invalid names
	main->moveToFirstChild("badElementName");
	BOOST_CHECK(main->isValid()==false);
	main->moveToParent();
	BOOST_CHECK(main->isValid());
	main->moveToFirstChild("badElementName");
	main->moveToFirstChild("badElementName");
	main->moveToParent();
	BOOST_CHECK(main->isValid());
	main->moveToParent();
	BOOST_CHECK(main->isValid());
	auto name = main->getName();
	BOOST_CHECK(name == "main_element");
	main->moveToParent();
	
	BOOST_CHECK(main->isDocument());
}

BOOST_AUTO_TEST_CASE(tinyxmlElementReader_testParse)
{
	std::string XMLtestString = R"xml(<?xml version="1.0" encoding="utf - 8"?>
<!--xml file to test the xml - reader functions--> 
		<GridDyn name = "test1" version = "0.0.1">
		<bus name = "bus1">
		<type>SLK</type>
		<angle>0</angle>
		<voltage>1.04</voltage>
		<generator name = "gen1">
		<P>0.7160</P>
		</generator>
		</bus>
		</GridDyn>)xml";

	tinyxmlReaderElement reader;
	reader.parse(XMLtestString);
	BOOST_REQUIRE(reader.isValid());

	auto firstChild = reader.clone();
	BOOST_CHECK(firstChild != nullptr);
	BOOST_CHECK(firstChild->getName() == "GridDyn");
	//There is only one child
	auto sibling = firstChild->nextSibling();
	BOOST_CHECK(sibling == nullptr);
	sibling = firstChild->firstChild();
	BOOST_CHECK(sibling->getName() == "bus");
	auto cChild = sibling->nextSibling();
	BOOST_CHECK(cChild == nullptr);
	sibling->moveToNextSibling();
	BOOST_CHECK(sibling->isValid() == false);

	auto busElement = firstChild->firstChild();
	auto busChild = busElement->firstChild("type");
	BOOST_REQUIRE(busChild != nullptr);
	BOOST_CHECK(busChild->getName() == "type");

	auto val = busChild->getText();
	BOOST_CHECK(val == "SLK");

	//Go through the children
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "angle");
	BOOST_CHECK(busChild->getValue() == 0.0);
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "voltage");
	BOOST_CHECK(busChild->getText() == "1.04");
	BOOST_CHECK_CLOSE(busChild->getValue(), 1.04, 0.000001);

	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "generator");
	BOOST_CHECK(busChild->getText().empty());
	auto genChild = busChild->firstChild();
	BOOST_CHECK(genChild->getName() == "P");
	BOOST_CHECK_CLOSE(genChild->getValue(), 0.7160, 0.00001);

	//Get the generator attributes
	auto att = busChild->getFirstAttribute();
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "gen1");
	//move busChild to the parent to make sure they are the same
	busChild->moveToParent();
	BOOST_CHECK(busChild->getName() == busElement->getName());

	//Now go back to the first child to do a few checks on attributes
	att = firstChild->getAttribute("name");
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "test1");

}

BOOST_AUTO_TEST_CASE(tinyxmlElementReader_test4)
{
	auto reader = std::make_shared<tinyxmlReaderElement>(elementReaderTestDirectory + "xmlElementReader_test3.xml");
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
	BOOST_CHECK(val == readerNullVal);
	main->moveToFirstChild();

	val = main->getValue();
	BOOST_CHECK(val == readerNullVal);
	BOOST_CHECK(main->getText() == "45.3echo");
	main->restore();
	BOOST_CHECK(main->getName() == "main_element");
	//test iterated and reverse moving bookmarks
	main->bookmark();
	main->moveToFirstChild();
	main->moveToFirstChild();
	main->bookmark();
	main->moveToParent();
	main->moveToParent();
	main->restore();
	BOOST_CHECK(main->getName() == "subelementA");
	main->restore();
	BOOST_CHECK(main->getName() == "main_element");
}

BOOST_AUTO_TEST_CASE(tinyxml2ElementReader_test1)
{
	tinyxml2ReaderElement reader;
	BOOST_REQUIRE(reader.loadFile(elementReaderTestDirectory + "xmlElementReader_test.xml"));
	auto firstChild = reader.clone();
	BOOST_CHECK(firstChild != nullptr);
	BOOST_CHECK(firstChild->getName() == "griddyn");
	//There is only one child
	auto sibling = firstChild->nextSibling();
	BOOST_CHECK(sibling == nullptr);
	sibling = firstChild->firstChild();
	BOOST_CHECK(sibling->getName() == "bus");
	auto cChild = sibling->nextSibling();
	BOOST_CHECK(cChild == nullptr);
	sibling->moveToNextSibling();
	BOOST_CHECK(sibling->isValid() == false);

	auto busElement = firstChild->firstChild();
	auto busChild = busElement->firstChild("type");
	BOOST_REQUIRE(busChild != nullptr);
	BOOST_CHECK(busChild->getName() == "type");

	auto val = busChild->getText();
	BOOST_CHECK(val == "SLK");

	//Go through the children
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "angle");
	BOOST_CHECK(busChild->getValue() == 0.0);
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "voltage");
	BOOST_CHECK(busChild->getText() == "1.04");
	BOOST_CHECK_CLOSE(busChild->getValue(), 1.04, 0.000001);

	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "generator");
	BOOST_CHECK(busChild->getText().empty());
	auto genChild = busChild->firstChild();
	BOOST_CHECK(genChild->getName() == "P");
	BOOST_CHECK_CLOSE(genChild->getValue(), 0.7160, 0.00001);

	//Get the generator attributes
	auto att = busChild->getFirstAttribute();
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "gen1");
	//move busChild to the parent to make sure they are the same
	busChild->moveToParent();
	BOOST_CHECK(busChild->getName() == busElement->getName());

	//Now go back to the first child to do a few checks on attributes
	att = firstChild->getAttribute("name");
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "test1");

}


BOOST_AUTO_TEST_CASE(tinyxml2ElementReader_test2)
{
	tinyxml2ReaderElement reader;
	//test a bad file
	reader.loadFile(elementReaderTestDirectory + "xmlElementReader_testbbad.xml");
	BOOST_CHECK(reader.isValid() == false);
	reader.loadFile(elementReaderTestDirectory + "xmlElementReader_test2.xml");
	BOOST_CHECK(reader.isValid() == true);

	auto main = reader.clone();
	//there should be 9 elements
	auto sub = main->firstChild();
	for (int kk = 0; kk < 8; ++kk)
	{
		sub->moveToNextSibling();
	}
	auto name = sub->getName();
	BOOST_CHECK(name == "elementWithAttributes");
	auto att = sub->getFirstAttribute();
	BOOST_CHECK(att.getText() == "A");
	att = sub->getNextAttribute();
	BOOST_CHECK(att.getValue() == 46);
	att = sub->getNextAttribute();
	BOOST_CHECK_CLOSE(att.getValue(), 21.345, 0.000001);
	att = sub->getNextAttribute();
	BOOST_CHECK(att.getValue() == readerNullVal);
	BOOST_CHECK(att.getText() == "happy");
	att = sub->getNextAttribute();
	BOOST_CHECK(att.isValid() == false);

}

BOOST_AUTO_TEST_CASE(tinyxml2ElementReader_test3)
{
	tinyxml2ReaderElement reader(elementReaderTestDirectory + "xmlElementReader_test2.xml");
	//test traversal using move commands
	auto main = reader.clone();

	main->moveToFirstChild("subelementA");
	BOOST_CHECK(main->getName() == "subelementA");
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementA");
	BOOST_CHECK(main->isValid() == false);
	//now move back to parent
	main->moveToParent();
	BOOST_CHECK(main->getName() == "main_element");
	//now loop through subElementB
	main->moveToFirstChild("subelementB");
	BOOST_CHECK(main->getName() == "subelementB");
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->getName() == "subelementB");
	main->moveToNextSibling("subelementB");
	BOOST_CHECK(main->isValid() == false);

	//test traversal with different element names
	main->moveToParent();
	main->moveToFirstChild("subelementA");
	BOOST_CHECK(main->isValid());
	main->moveToNextSibling("elementWithAttributes");
	BOOST_CHECK(main->isValid());

	//Test traversal with invalid names
	main->moveToFirstChild("badElementName");
	BOOST_CHECK(main->isValid() == false);
	main->moveToParent();
	BOOST_CHECK(main->isValid());
	main->moveToFirstChild("badElementName");
	main->moveToFirstChild("badElementName");
	main->moveToParent();
	BOOST_CHECK(main->isValid());
	main->moveToParent();
	BOOST_CHECK(main->isValid());
	auto name = main->getName();
	BOOST_CHECK(name == "main_element");
	main->moveToParent();
	name = main->getName();
	BOOST_CHECK(main->isDocument());
}

BOOST_AUTO_TEST_CASE(tinyxml2ElementReader_testParse)
{
	std::string XMLtestString = R"xml(<?xml version="1.0" encoding="utf - 8"?>
<!--xml file to test the xml - reader functions--> 
		<GridDyn name = "test1" version = "0.0.1">
		<bus name = "bus1">
		<type>SLK</type>
		<angle>0</angle>
		<voltage>1.04</voltage>
		<generator name = "gen1">
		<P>0.7160</P>
		</generator>
		</bus>
		</GridDyn>)xml";

	tinyxml2ReaderElement reader;
	reader.parse(XMLtestString);

	BOOST_REQUIRE(reader.isValid());
	auto firstChild = reader.clone();
	BOOST_CHECK(firstChild != nullptr);
	BOOST_CHECK(firstChild->getName() == "GridDyn");
	//There is only one child
	auto sibling = firstChild->nextSibling();
	BOOST_CHECK(sibling == nullptr);
	sibling = firstChild->firstChild();
	BOOST_CHECK(sibling->getName() == "bus");
	auto cChild = sibling->nextSibling();
	BOOST_CHECK(cChild == nullptr);
	sibling->moveToNextSibling();
	BOOST_CHECK(sibling->isValid() == false);

	auto busElement = firstChild->firstChild();
	auto busChild = busElement->firstChild("type");
	BOOST_REQUIRE(busChild != nullptr);
	BOOST_CHECK(busChild->getName() == "type");

	auto val = busChild->getText();
	BOOST_CHECK(val == "SLK");

	//Go through the children
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "angle");
	BOOST_CHECK(busChild->getValue() == 0.0);
	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "voltage");
	BOOST_CHECK(busChild->getText() == "1.04");
	BOOST_CHECK_CLOSE(busChild->getValue(), 1.04, 0.000001);

	busChild->moveToNextSibling();
	BOOST_CHECK(busChild->getName() == "generator");
	BOOST_CHECK(busChild->getText().empty());
	auto genChild = busChild->firstChild();
	BOOST_CHECK(genChild->getName() == "P");
	BOOST_CHECK_CLOSE(genChild->getValue(), 0.7160, 0.00001);

	//Get the generator attributes
	auto att = busChild->getFirstAttribute();
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "gen1");
	//move busChild to the parent to make sure they are the same
	busChild->moveToParent();
	BOOST_CHECK(busChild->getName() == busElement->getName());

	//Now go back to the first child to do a few checks on attributes
	att = firstChild->getAttribute("name");
	BOOST_CHECK(att.getName() == "name");
	BOOST_CHECK(att.getText() == "test1");

}

BOOST_AUTO_TEST_CASE(tinyxml2ElementReader_test4)
{
	auto reader = std::make_shared<tinyxml2ReaderElement>(elementReaderTestDirectory + "xmlElementReader_test3.xml");
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
	BOOST_CHECK(val == readerNullVal);
	main->moveToFirstChild();

	val = main->getValue();
	BOOST_CHECK(val == readerNullVal);
	BOOST_CHECK(main->getText() == "45.3echo");
	main->restore();
	BOOST_CHECK(main->getName() == "main_element");
	
	//test a downward moving bookmark
	main->bookmark();
	main->moveToFirstChild();
	main->moveToFirstChild();
	main->bookmark();
	main->moveToParent();
	main->moveToParent();
	main->restore();
	BOOST_CHECK(main->getName() == "subelementA");
	main->restore();
	BOOST_CHECK(main->getName() == "main_element");
}

BOOST_AUTO_TEST_SUITE_END()