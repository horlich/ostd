/*
 * otestutil.h
 *
 *  Created on: 29.08.2017
 *      Author: kanzlei
 */

#ifndef OTESTUTIL_H_
#define OTESTUTIL_H_


#include <chrono>
//#include <ratio>
//#include <ctime>
#include <iostream>
#include <string>


namespace TestUtil {

typedef void (*fp_testrun)();

void printTime(fp_testrun methode, const std::string& testName, std::ostream& os = std::cout);










} // Ende Namespace TestUtil




#endif /* OTESTUTIL_H_ */
