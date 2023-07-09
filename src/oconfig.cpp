/*
 * oconfig.cpp
 *
 *  Created on: 17.05.2017
 *      Author: kanzlei
 */

#include "oconfig.h"


using namespace std;



namespace Config {


ConfigSyntaxException::ConfigSyntaxException(int line, const std::string& mess)
    : ParseException(mess), lineNr(line) {}


ConfigMissingKey::ConfigMissingKey(const std::string& message)
    : ParseException(message) {}


std::string ConfigMissingKey::missingKey(const std::string& keyname)
{
    std::stringstream buf;
    buf << "Fehlerhafte Config-Datei: Key '" << keyname << "' nicht gefunden";
    return buf.str();
}


ConfigMap readConfig(const std::filesystem::path& file)
{
    if (! std::filesystem::exists(file))
        throw OFile::FileNotFound(OFile::FileNotFound::notFound(file.string()));
    ConfigMap map;
    static constexpr int bufsiz {300};
    char buf[bufsiz];
    std::ifstream is(file);
    int lnr = 0;
    while (is.getline(buf, bufsiz)) {
        ++lnr;
        std::string line(buf);
        if (line.empty() || (line[0] == '#')) continue;
        int pos = line.find('=');
        if (pos == std::string::npos)
            throw ConfigSyntaxException(lnr, "Formatfehler");
        std::string key = line.substr(0, pos);
        std::string value = line.substr(++pos);
        map[key] = value;
    }
//    for (auto pair : map)
//        std::cout << "Key=" << pair.first << ", Value=" << pair.second << '\n';
    return map;
}


}; // namespace Config

