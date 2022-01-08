/*
 * oconfig.h
 *
 *  Created on: 17.05.2017
 *      Author: kanzlei
 */

#ifndef OCONFIG_H_
#define OCONFIG_H_

#include <bitset>
#include <map>
#include <stdexcept>

#include "oexception.h"
#include "ofile.h"
#include "ostringutil.h"


namespace Property {



class ConfigSyntaxException : public OException::ParseException {
private:
	const OFile::Datei* configdatei;
	unsigned int lineNr;
public:
	ConfigSyntaxException(const OFile::Datei* d, unsigned int line, const std::string& mess) :
		ParseException(mess), configdatei(d), lineNr(line){}
};




class SimpleConfigDatei : public OFile::Datei {
	/*
	 *    Enthält Key-Wert-Paare, die durch das erste in der Zeile
	 *    vorkommende '=' getrennt sind.
	 *    Auskommentieren mit '#'.
	 *
	 * */
private:
	std::map<std::string, std::string> wertehash;
public:
	SimpleConfigDatei(const OFile::Path& p) : Datei(p) {};

	void readMe();

	std::string getVal(const std::string& key);

	std::string getVal(const char* key) { return getVal(std::string(key)); }
};


}; // namespace Property

#endif /* OCONFIG_H_ */
