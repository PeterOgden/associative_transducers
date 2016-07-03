/*
 * TestRunner.cpp
 *
 *  Created on: May 15, 2012
 *      Author: pko11
 */

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TextOutputter.h>

#include <iostream>

int main()
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
	CppUnit::TestResult controller;
	CppUnit::TestResultCollector result;

	controller.addListener(&result);

	runner.addTest(registry.makeTest());

	std::ofstream xmlout("results.xml");
	CppUnit::XmlOutputter out(&result, xmlout);
	CppUnit::TextOutputter console(&result, std::cout);

	runner.run(controller, "");

	out.write();
	console.write();
	return result.wasSuccessful()? 0: 1;

}



