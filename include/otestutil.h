/*
 * otestutil.h
 *
 *  Created on: 29.08.2017
 *      Author: kanzlei
 */

#ifndef OTESTUTIL_H_
#define OTESTUTIL_H_


#include <chrono>
#include <ratio>
#include <ctime>
#include <iostream>


namespace TestUtil {

typedef void (*fp_testrun)();

using std::chrono::steady_clock;

void printTime(fp_testrun meth, const std::string& testName, std::ostream& os = std::cout) {
		os << "[printTime] *** Starte Test \'" << testName << "\'...\n";
		auto anfang = steady_clock::now();
		(*meth)(); // Testprogramm ausführen...
		std::chrono::nanoseconds ns = steady_clock::now() - anfang;
		double d = std::chrono::duration_cast<std::chrono::microseconds>(ns).count();
		os << "[printTime] *** Test \'" << testName << "\' nach " << d / 1000 << " ms beendet." << std::endl;
}










} // Ende Namespace TestUtil




#endif /* OTESTUTIL_H_ */
