/*
 * ocharutil.cpp
 *
 *  Created on: 20.06.2017
 *      Author: kanzlei
 */

#include "ocharutil.h"

using namespace std;



namespace CharUtil {





int numVal(char c) {
	if (! isdigit(c)) return -1;
	return c - '0';
}



void testNumVal(ostream& os) {
	os << "Teste CharUtil::numVal()\n";
	for (char c = '+'; c < '?'; ++c) {
		os << "  '" << c << "' ~ " << CharUtil::numVal(c) << "\n";
	}
	os << "Ende Test." << endl;
}






} // Ende Namespace CharUtil


