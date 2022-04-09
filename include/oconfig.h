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
#include <fstream>
#include <sstream>
#include <filesystem>
//#include <stdexcept>

#include "oexception.h"
#include "ofile.h"
#include "ostringutil.h"


namespace Config {


class ConfigSyntaxException : public OException::ParseException {
private:
    int lineNr;
public:
    ConfigSyntaxException(int line, const std::string& mess);
};


struct ConfigMissingKey : public OException::ParseException {
    ConfigMissingKey(const std::string& message);

    static std::string missingKey(const std::string& keyname);
};


using ConfigMap = std::map<std::string, std::string>;

/* Syntax der Config-Dateien: '[key]=[value]\n'
   Leerzeilen oder Zeilen, die mit '#' beginnen,
   werden ignoriert: */
ConfigMap readConfig(const std::filesystem::path& file);


}; // namespace Config

#endif /* OCONFIG_H_ */
