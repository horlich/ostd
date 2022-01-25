/*
 * ostringutil.cpp
 *
 *  Created on: 18.05.2017
 *      Author: kanzlei
 */

#include "ostringutil.h"



using namespace std;


namespace StringUtil {


static const std::string MESS_U8_LESEFEHLER("Fehler beim Lesen des UTF8-Strings.\nMöglicherweise wurde die Locale nicht gesetzt.\nz.B. 'setlocale(LC_ALL, \"de_AT.UTF-8\")'");



string ltrim(const string& str) {
	size_t firstPos = str.find_first_not_of(WHITESPACES);
	if (firstPos == string::npos) return string();
	return str.substr(firstPos);
}

string rtrim(const string& str) {
	size_t lastPos = str.find_last_not_of(WHITESPACES);
	if (lastPos == string::npos) return string();
	return str.substr(0, lastPos+1);
}

string trim(const string& str) {
	size_t firstPos = str.find_first_not_of(WHITESPACES);
	if (firstPos == string::npos) return string();
	size_t lastPos = str.find_last_not_of(WHITESPACES);
	return str.substr(firstPos, lastPos-firstPos+1);
}


int split(const std::string& str, char tz, std::vector<std::string>& vec) {
	size_t start = 0;
	size_t end = 0;
	int count = 0;
	while (end != std::string::npos) {
		end = str.find_first_of(tz,start);
		vec.push_back(str.substr(start, (end - start)));
		count++;
		start = end+1;
	}
	return count;
}




int parseArgs(const string& str, StrVec& vec) {
	/* Trennt einen String nach Whitespaces.
	 * Innerhalb von Anführungszeichen " oder ' werden auch
	 * Whitespaces mitgeschrieben.
	 * Schreibt die erzeugten Tokens in vec.
	 * Gibt die Anzahl der erzeugten Tokens zurück */
	int i = 0;
	int tkbegin = 0;
	int tokens = 0;
	bool recording = false;
	bool end_found = false;
	char quote = 0;

	for (string::const_iterator it = str.cbegin();; it++) {
		char found = *it;
		if ( it == str.cend() ) {
 			if ( recording ) end_found = true;
 		} else if ( found == quote ) {
 			end_found = true;
 		} else if ( isspace(found) && recording ) {
			if (quote == 0) {
				end_found = true;
			}
		} else if ( recording == false && isspace(found) == false ) {
			// Tokenbegin gefunden...
			tkbegin = i;
			tokens++;
			recording = true;
			if ( (found == '\'') or (found == '"') ) {
				quote = found; // Beginn quoted string
				tkbegin++; // Quote nicht aufnehmen!
			}
		}
		if (end_found) {
			recording = false;
			end_found = false;
			quote = 0;
			vec.push_back(str.substr(tkbegin,(i-tkbegin)));
		}
		if (it == str.cend()) break; // sonst Endlos-Schleife!
		i++;
	}
	return tokens;
}




/*
 *             Umwandlung Multibyte-String <-> Wide-String
 * */


// siehe man mbstowcs:
static const size_t MB_CONVERT_ERRORVAL = (size_t)-1;

int getWSize(const char* str) {
	// Anzahl an wide-chars in einem multibyte-String
	size_t ret = mbsrtowcs(nullptr, &str, 0, 0);
	if (ret == MB_CONVERT_ERRORVAL) return -1;
	return ret;
}



wstring toWstring(const char* str, size_t maxlen) {
    wchar_t wstr[maxlen];
    if (mbstowcs(wstr, str, maxlen) == MB_CONVERT_ERRORVAL) throw CharUtil::KeinUtf8(MESS_U8_LESEFEHLER);
	return wstring(wstr);
}


wstring toWstring(string& s) {
	return toWstring(s.data(), s.size() + 1);
}

int getMBSize(const wchar_t* wstr) {
	// Anzahl an Bytes in einem wide string
//	maxlen++; // Endzeichen einrechnen!
//	char str[maxlen];
	size_t ret = wcstombs(nullptr, wstr, 0);
	if (ret == MB_CONVERT_ERRORVAL) return -1;
	return ret;
}


string toMBstring(const wchar_t* wstr, size_t maxlen) {
	maxlen++; // Endzeichen einrechnen!
	char str[maxlen+1];
	if (wcstombs(str, wstr, maxlen) == MB_CONVERT_ERRORVAL) throw CharUtil::KeinUtf8(MESS_U8_LESEFEHLER);
	return string(str);
}


string toMBstring(wstring& s) {
	return toMBstring(s.data(), (s.size()+1) * MB_CUR_MAX);
}



/*
 *                         StringVisitor:
 *
 * */




int StringVisitor::parseStream(std::wistream& is) {
	int zeichen = 0;
	while (is) {
      wchar_t wc = is.get();
		if (is.eof()) break;
		zeichen++;
		if (visit(wc) == VisitResult::BREAK) break;
	}
	return zeichen;
}


int StringVisitor::parseString(std::string str) {
   WC_Converter conv;
	std::wstringstream is;
	is << conv.from_bytes(str);
	return parseStream(is);
}





} // Ende Namespace StringUtil
