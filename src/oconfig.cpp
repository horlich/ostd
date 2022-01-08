/*
 * oconfig.cpp
 *
 *  Created on: 17.05.2017
 *      Author: kanzlei
 */

#include "oconfig.h"


using namespace std;



namespace Property {


const string KEY_CONFIGFILE_READ("key_configfile_read");

void SimpleConfigDatei::readMe() {
	ifstream is(getPath().toString().c_str());
	bool isComment = false;
	if (! is.is_open())
		throw OFile::CannotOpen(getPath().toString());
	char c;
	int lineNr = 1;
	string buf;
	buf.reserve(300);
	wertehash.clear();
	wertehash[KEY_CONFIGFILE_READ] = string("true");
	while (is.good()) {
		c = is.get();
		if (c == '\n') {
			// buf auswerten...
			if (! buf.empty()) {
				size_t trennPos = buf.find('=');
				if (trennPos == string::npos)
					throw ConfigSyntaxException(this, lineNr, "Trennzeichen fehlt");
				string key(StringUtil::rtrim(buf.substr(0, trennPos)));
				string val(StringUtil::trim(buf.substr(trennPos+1)));
				wertehash[key] = val;
			}
			isComment = false;
			lineNr++;
			buf.clear();
		}
		if (buf.empty() && isspace(c)) continue;
		if (isComment) continue;
		if (c == '#') {
			isComment = true;
		} else buf.append(sizeof(c), c);
	}
	is.close();
}



string SimpleConfigDatei::getVal(const string& key) {
	try {
		if (! wertehash.count(KEY_CONFIGFILE_READ)) {
			readMe();
			return getVal(key);
		}
		return string(wertehash.at(key));
	} catch (out_of_range& e) { // Key existiert nicht
		return string();
	}
}

}; // namespace Property

