/*
 * otestutil.cpp
 *
 *  Created on: 29.08.2017
 *      Author: kanzlei
 */

#include "otestutil.h"



namespace TestUtil {


using std::chrono::steady_clock;

void printTime(fp_testrun meth, const std::string& testName, std::ostream& os) {
		os << "[TestUtil::printTime] *** Starte Test \'" << testName << "\'...\n";
		auto anfang = steady_clock::now();
		(*meth)(); // Testprogramm ausführen...
		std::chrono::nanoseconds ns = steady_clock::now() - anfang;
		double d = std::chrono::duration_cast<std::chrono::microseconds>(ns).count();
		os << "[TestUtil::printTime] *** Test \'" << testName << "\' nach " << d / 1000 << " ms beendet." << std::endl;
}













} // Ende Namespace TestUtil


