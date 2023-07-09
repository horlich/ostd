/*
 * debug.cpp
 *
 *  Created on: 08.09.2019
 *      Author: kanzlei
 */

#include "./debug.h"

using namespace std;


DebugMess::DebugMess() : os("/tmp/debugmessages.txt") {}

DebugMess::~DebugMess() {
	os.close();
}

DebugMess MESSAGE;


void binda(const char* file, int line) {
	cout << ">>" << file << " Z." << line << endl;
	cout.flush();
}


std::string getFileLine(const char* file, int line) {
	std::stringstream buf;
	buf << ">>" << file << " Z." << line << std::endl;
	return buf.str();
}


