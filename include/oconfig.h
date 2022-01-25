/*
 * oconfig.h
 *
 *  Created on: 17.05.2017
 *      Author: kanzlei
 */

#ifndef OCONFIG_H_
#define OCONFIG_H_

//#include <bitset>
#include <map>
#include <unistd.h>
//#include <stdexcept>

#include "oexception.h"
#include "ofile.h"
#include "ostringutil.h"


namespace Property {



class ConfigSyntaxException : public OException::ParseException {
private:
	int lineNr;
public:
	ConfigSyntaxException(int line, const std::string& mess) :
		ParseException(mess), lineNr(line){}
};




class SimpleConfigDatei {
	/*
	 *    Enthält Key-Wert-Paare, die durch das erste in der Zeile
	 *    vorkommende '=' getrennt sind.
	 *    Auskommentieren mit '#'.
	 *
	 * */
private:
	std::map<std::string, std::string> wertehash;
	const char* path;
public:
	SimpleConfigDatei(const char* _path) : path{_path} {};

	void readMe();

	std::string getVal(const std::string& key);

	std::string getVal(const char* key) { return getVal(std::string(key)); }
};


}; // namespace Property

#endif /* OCONFIG_H_ */
