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


void printInfo(std::ostream& os, const CharUtil::U32Char& uc) {
     static constexpr int BUFSIZE = 5;
     static char buf[BUFSIZE];
     os.fill('0');
     os << "\nZeichen U+" << std::hex << std::setw(4)  << uc.getValue() << " = '" << uc << "'\n";
     Format::toBin(os, uc.getValue());
     uc.toUtf8(buf, BUFSIZE);
     for (int i = 1; i <= buf[0]; ++i) {
          unsigned char c = buf[i];
          os << "\n   " << i << ". Byte: 0x" << std::setw(4)  << std::hex << static_cast<unsigned int>(c) << " = ";
//          Format::toBin(os, c); // Wegen Fehlermeldung auskommentiert!!!
     }
     CharUtil::U32Char lower = uc.toLower();
     if (lower != uc) os << "\nto_lower: '" << lower << "'\n";
     os << "\n";
}

