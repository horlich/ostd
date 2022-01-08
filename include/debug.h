/*
 * debug.h
 *
 *  Created on: 08.09.2019
 *      Author: kanzlei
 */

#ifndef DEBUG_H_
#define DEBUG_H_

//#include "ofile.h"

#include <iostream>
#include <iomanip>
#include <fstream>

#include "ocharutil.h"
#include "oformat.h"


void binda(const char* file, int line);

std::string getFileLine(const char* file, int line);


#define BINDA binda( __FILE__ , __LINE__ );

#define FILELINE getFileLine( __FILE__ , __LINE__ )

#define PRINTFUNC std::cout << ">>>  " << __PRETTY_FUNCTION__ << " #" << __LINE__ << "\n";

#define PRINTVAL(x) (std::cout << ">>> " << __PRETTY_FUNCTION__ << " #" << __LINE__ << "\n    " << (#x) << " = " << (x) << endl );





class DebugMess {
private:
	std::ofstream os;
public:
	DebugMess();

	virtual ~DebugMess();

	void println(const std::string& str) { os << str << std::endl; os.flush(); }

	std::ofstream& stream() { return os; }
};

extern DebugMess MESSAGE;

#define PRINTDEBUGMESS MESSAGE.println(FILELINE);


void printInfo(std::ostream& os, const CharUtil::U32Char& uc);






#endif /* DEBUG_H_ */
