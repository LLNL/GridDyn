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

#include "yamlReaderElement.h"

#include "stringConversion.h"
#include <cassert>
#include <fstream>
#include <iostream>

static const std::string nullStr = std::string("");

yamlReaderElement::yamlElement::yamlElement(YAML::Node vElement, std::string newName) : name(newName), element(vElement)
{
	elementIndex = 0;

	if (element.IsSequence())
	{
		arraytype = true;
		arrayIndex = 0;
		while ((arrayIndex < element.size()) && (element[arrayIndex].IsNull()))
		{
			++arrayIndex;
		}
	}
}

void yamlReaderElement::yamlElement::clear()
{
	element = YAML::Null;
	elementIndex = 0;
	arrayIndex = 0;
	arraytype = false;
	name = nullStr;
}

yamlReaderElement::yamlReaderElement()
{

}

yamlReaderElement::yamlReaderElement(const std::string &filename)
{
	loadFile(filename);
}



yamlReaderElement::~yamlReaderElement()
{
}

void yamlReaderElement::clear()
{
	current.clear();
	parents.clear();

}

bool yamlReaderElement::isValid() const
{
	return (!(current.isNull()));
}

bool yamlReaderElement::isDocument() const
{
	if (parents.empty())
	{
		if (doc)
		{
			return true;
		}
	}
	return false;
}


std::shared_ptr<readerElement> yamlReaderElement::clone() const
{
	auto ret = std::make_shared<yamlReaderElement>();
	ret->parents = parents;
	ret->current = current;
	ret->doc = doc;
	return ret;
}

bool yamlReaderElement::loadFile(const std::string &filename)
{
	std::ifstream file(filename);
	if (file.is_open())
	{
		doc = YAML::LoadFile(filename);
		if (doc.IsDefined())
		{
			return true;
		}
		return false;
	}
	else
	{
		std::cerr << "unable to open file " << filename << '\n';
		doc = YAML::Node{};
		clear();
		return false;
	}

}


bool yamlReaderElement::parse(const std::string &inputString)
{

	doc = YAML::Load(inputString);

	if (doc.IsDefined())
	{
		return true;
	}
	return false;

}

std::string yamlReaderElement::getName() const
{
	return current.name;
}

double yamlReaderElement::getValue() const
{
	if ((current.isNull())||(!current.getElement().IsScalar()))
	{
		return readerNullVal;
	}
	return numeric_conversionComplete<double>(current.getElement().as<std::string>(),readerNullVal);
}

std::string yamlReaderElement::getText() const
{
	if ((current.isNull()) || (!current.getElement().IsScalar()))
	{
		return nullStr;
	}
	return current.getElement().as<std::string>();


}

std::string yamlReaderElement::getMultiText(const std::string sep) const
{
	if (current.isNull())
	{
		return nullStr;
	}
	if (current.getElement().IsScalar())
	{
		return current.getElement().as<std::string>();
	}
	if (current.getElement().IsSequence())
	{
		std::string ret;

	}
	return nullStr;
}
//no 
bool yamlReaderElement::hasAttribute(const std::string &attributeName) const
{
	if (current.isNull())
	{
		return false;
	}

	if (current.getElement()[attributeName])
	{
		return (isAttribute(current.getElement()[attributeName]));
	}
	else
	{
		return false;
	}
}

bool yamlReaderElement::isAttribute(const YAML::Node &testValue) const
{
	return testValue.IsScalar();
}


bool yamlReaderElement::isElement(const YAML::Node &testValue) const
{
	return ((testValue.IsMap()) || (testValue.IsSequence()));
}

bool yamlReaderElement::hasElement(const std::string &elementName) const
{
	if (current.isNull())
	{
		return false;
	}

	if (current.getElement()[elementName])
	{
		return (isElement(current.getElement()[elementName]));
	}

	return false;
}

readerAttribute yamlReaderElement::getFirstAttribute()
{
	if (current.isNull())
	{
		return readerAttribute();
	}

	attIterator = current.getElement().begin();
	auto elementEnd = current.getElement().end();


	while (attIterator != elementEnd)
	{
		if (isAttribute(*attIterator))
		{
			return readerAttribute(attIterator->Tag(), attIterator->as<std::string>());
		}
		++attIterator;
	}


	return readerAttribute();
}

readerAttribute yamlReaderElement::getNextAttribute()
{
	if (current.isNull())
	{
		return readerAttribute();
	}
	auto elementEnd = current.getElement().end();

	if (attIterator == elementEnd)
	{
		return readerAttribute();
	}
	++attIterator;
	while (attIterator != elementEnd)
	{
		if (isAttribute(*attIterator))
		{
			return readerAttribute(attIterator->Tag(), attIterator->as<std::string>());
		}
		++attIterator;
	}
	return readerAttribute();
}

readerAttribute yamlReaderElement::getAttribute(const std::string & attributeName) const
{
	if (hasAttribute(attributeName))
	{
		return readerAttribute(attributeName, current.getElement()[attributeName].as<std::string>());
	}
	return readerAttribute();
}

std::string yamlReaderElement::getAttributeText(const std::string &attributeName) const
{
	if (hasAttribute(attributeName))
	{
		return current.getElement()[attributeName].as<std::string>();
	}
	return nullStr;
}

double yamlReaderElement::getAttributeValue(const std::string &attributeName) const
{
	if (hasAttribute(attributeName))
	{
		return numeric_conversionComplete(current.getElement()[attributeName].as<std::string>(), readerNullVal);
	}
	return readerNullVal;
}

std::shared_ptr<readerElement> yamlReaderElement::firstChild() const
{
	if (current.isNull())
	{
		return nullptr;
	}

	auto newElement = clone();
	newElement->moveToFirstChild();
	return newElement;

}

std::shared_ptr<readerElement> yamlReaderElement::firstChild(const std::string &childName) const
{
	if (current.isNull())
	{
		return nullptr;
	}
	auto newElement = clone();
	newElement->moveToFirstChild(childName);
	return newElement;
}




void yamlReaderElement::moveToFirstChild()
{
	if (current.isNull())
	{
		return;
	}
	current.elementIndex = 0;
	auto elementIterator = current.getElement().begin();
	auto endIterator = current.getElement().end();

	while (elementIterator != endIterator)
	{

		if (isElement(*elementIterator))
		{
			parents.push_back(current);
			current = yamlElement(*elementIterator, elementIterator->Tag());
			return;
		}
		++elementIterator;
		++current.elementIndex;
	}
	parents.push_back(current);
	current.clear();

}

void yamlReaderElement::moveToFirstChild(const std::string &childName)
{
	if (current.isNull())
	{
		return;
	}

	if (current.getElement()[childName])
	{
		if (isElement(current.getElement()[childName]))
		{
			parents.push_back(current);
			current = yamlElement(current.getElement()[childName], childName);
			return;
		}
	}

	parents.push_back(current);
	current.clear();

}

void yamlReaderElement::moveToNextSibling()
{
	if (current.isNull())
	{
		return;
	}
	++current.arrayIndex;
	while (current.arrayIndex < current.count())
	{
		if (!current.getElement().IsNull())
		{
			return;
		}
		++current.arrayIndex;
	}
	if (parents.empty())
	{
		current = yamlElement();
		return;
	}
	//there are no more elements in a potential array
	auto elementIterator = parents.back().getElement().begin();
	auto endIterator = parents.back().getElement().end();
	++parents.back().elementIndex;
	//iterators don't survive copy so have to move the iterator to the next element index
	for (int ii = 0; ii < parents.back().elementIndex; ++ii)
	{
		++elementIterator;
		if (elementIterator == endIterator)
		{
			current.clear();
		}
	}
	//Now find the next valid element
	while (elementIterator != endIterator)
	{

		if (isElement(*elementIterator))
		{
			current = yamlElement(*elementIterator, elementIterator->Tag());
			return;
		}
		++elementIterator;
		++parents.back().elementIndex;
	}
	current.clear();

}

void yamlReaderElement::moveToNextSibling(const std::string &siblingName)
{
	if (current.isNull())
	{
		return;
	}
	if (siblingName == current.name)
	{
		++current.arrayIndex;
		while (current.arrayIndex < current.count())
		{
			if (!current.getElement().IsNull())
			{
				return;
			}
			++current.arrayIndex;
		}
		current.clear();
	}
	else
	{
		if (parents.back().getElement()[siblingName])
		{
			if (isElement(parents.back().getElement()[siblingName]))
			{
				current = yamlElement(parents.back().getElement()[siblingName], siblingName);
				return;
			}
		}
	}


}

void yamlReaderElement::moveToParent()
{
	if (parents.empty())
	{
		return;
	}
	current = parents.back();
	parents.pop_back();

}

std::shared_ptr<readerElement> yamlReaderElement::nextSibling() const
{
	if (current.isNull())
	{
		return nullptr;
	}
	auto newElement = clone();
	newElement->moveToNextSibling();
	return newElement;
}

std::shared_ptr<readerElement> yamlReaderElement::nextSibling(const std::string &siblingName) const
{
	if (current.isNull())
	{
		return nullptr;
	}
	auto newElement = clone();
	newElement->moveToNextSibling(siblingName);
	return newElement;
}


void yamlReaderElement::bookmark()
{
	bookmarks.push_back(std::static_pointer_cast<yamlReaderElement> (clone()));
}

void yamlReaderElement::restore()
{
	if (bookmarks.empty())
	{
		return;
	}
	parents = bookmarks.back()->parents;
	current = bookmarks.back()->current;
	bookmarks.pop_back();
}
