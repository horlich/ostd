/*
 * ostringutil.cpp
 *
 *  Created on: 18.05.2017
 *      Author: kanzlei
 */

#include "ostringutil.h"



using namespace std;
using namespace StringUtil;


std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t> U32_CONVERTER;

string StringUtil::ltrim(const string& str) {
	size_t firstPos = str.find_first_not_of(WHITESPACES);
	if (firstPos == string::npos) return string();
	return str.substr(firstPos);
}

string StringUtil::rtrim(const string& str) {
	size_t lastPos = str.find_last_not_of(WHITESPACES);
	if (lastPos == string::npos) return string();
	return str.substr(0, lastPos+1);
}

string StringUtil::trim(const string& str) {
	size_t firstPos = str.find_first_not_of(WHITESPACES);
	if (firstPos == string::npos) return string();
	size_t lastPos = str.find_last_not_of(WHITESPACES);
	return str.substr(firstPos, lastPos-firstPos+1);
}


int StringUtil::split(const std::string& str, char tz, std::vector<std::string>& vec) {
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



void StringUtil::u8_to_upper(std::string& str) { CharUtil::toUpper(str.begin(), str.end()); }


void StringUtil::u8_to_lower(std::string& str) { CharUtil::toLower(str.begin(), str.end()); }


int StringUtil::parseArgs(const string& str, vector<string> *vec) {
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
			vec->push_back(str.substr(tkbegin,(i-tkbegin)));
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
static const size_t MB_CONVERT_ERRORVAL = (size_t) - 1;

int StringUtil::getWSize(const char* str) {
	// Anzahl an wide-chars in einem multibyte-String
	size_t ret = mbsrtowcs(0, &str, 0, 0);
	if (ret == MB_CONVERT_ERRORVAL) return -1;
	return ret;
}



wstring StringUtil::toWstring(const char* str, size_t maxlen) {
    wchar_t wstr[maxlen];
    if (mbstowcs(wstr, str, maxlen) == MB_CONVERT_ERRORVAL)  { // Fehler beim Konvertieren!
		// TODO throw Exception
		// Wahrscheinlichster Fehler: locale nicht auf 'de_AT.UTF-8' gesetzt.
		// z.B. global im Programm: std::setlocale(LC_ALL, "de_AT.UTF-8");
		return L"Fehler"; // provisorisch
	}
	return wstring(wstr);
}


wstring StringUtil::toWstring(string& s) {
	return toWstring(s.data(), s.size() + 1);
}

int StringUtil::getMBSize(const wchar_t* wstr, size_t maxlen) {
	// Anzahl an Bytes in einem wide string
	maxlen++; // Endzeichen einrechnen!
	char str[maxlen];
	size_t ret = wcstombs(str, wstr, maxlen);
	if (ret == MB_CONVERT_ERRORVAL) return -1;
	return ret;
}


string StringUtil::toMBstring(const wchar_t* wstr, size_t maxlen) {
	maxlen++; // Endzeichen einrechnen!
	char str[maxlen+1];
	if (wcstombs(str, wstr, maxlen) == MB_CONVERT_ERRORVAL)  { // Fehler beim Konvertieren!
		/* TODO throw Exception */
		// Wahrscheinlichster Fehler: locale nicht auf '...UTF-8' gesetzt.
		// z.B. global im Programm: std::setlocale(LC_ALL, "C.UTF-8");
		return "Fehler"; // provisorisch
	}
	return string(str);
}


string StringUtil::toMBstring(wstring& s) {
	return toMBstring(s.data(), (s.size()+1) * MB_CUR_MAX);
}






/*
 *                         UTF-8 Util:
 * */


int StringUtil::u8Strlen(const std::string& str) {
     int ret = 0;
     const char* pt = str.c_str();
     const int maxbytes = 5;
     int clen;
     while (1) {
    	 clen = mblen(pt, maxbytes);
    	 /* fehlerhafter MB-String? */
    	 if (clen < 0) {
    		 throw StringUtil::KeinUTF8("Fehler beim Lesen des UTF8-Strings.\nMöglicherweise wurde die Locale nicht gesetzt.\nz.B. 'setlocale(LC_ALL, \"de_AT.UTF-8\")'");
    	 }
    	 /* Endzeichen? */
    	 if (clen == '\0') break;
    	 ret ++;
    	 pt += clen;
     }
     return ret;
}









/*
 *                         U32String:
 *
 * */



inline void U32String::resetValues() { this->clear(); }


bool U32String::containsGraph() const {
	for (auto it = cbegin(); it != cend(); ++it) {
		if (it->isAscii() &&
			isgraph(static_cast<char>(it->getValue()))) return true;
	}
	return false;
}


std::string U32String::str() const {
	std::stringstream buf;
	for (auto it = cbegin(); it != cend(); ++it) buf << *it;
	return buf.str();
}






istream& StringUtil::operator>>(istream& is, U32String& us) {
	while (is) {
		CharUtil::U32Char uc(is);
		us.push_back(uc);
	}
	return is;
}




ostream& StringUtil::operator<<(ostream& os, const U32String& us) {
	for (U32String::const_iterator it = us.cbegin(); it != us.cend(); ++it) os << *it;
	return os;
}





/*
 *                         U32Token:
 *
 * */


const std::string U32Token::DEFAULT_TRENNER = "-";

U32Token::U32Token() : afterLast('\0'), endsWith(Ending::NULLSTR) {
	reserve(30);
}


U32Token::U32Token(istream& is, const std::string& trenner) : U32Token() {
	init(is, trenner);
}

std::istream& U32Token::init(istream& is, const std::string& trenner) {
	// Whitespaces zu Beginn werden ignoriert.
	// Wird ein Whitespace, SOFT_HYPHEN oder ein Trennzeichen gelesen,
	// dann wird closed = true gesetzt.
	// Whitespaces und SOFT_HYPHEN werden niemals in den Buffer gelesen.
	// Alle anderen Trennzeichen werden noch in den Buffer gelesen.
	// Das erste nicht in den Buffer gesteckte U32Char wird als afterLast gesetzt.
	// Ist afterLast ein Whitespace, so wird als Ending immer SPACE gesetzt!
	// NEWLINE kann nur explizit mit setNewline() gesetzt werden!
	bool closed = false;
	while (is) {
		const auto startpos = is.tellg();
		CharUtil::U32Char uc(is);
		if (is.eof()) break;
		if (uc.isSpace()) {
			if (empty()) continue;
			setAfterLast(uc);
			if (getEnding() == U32Token::Ending::NEWLINE) break;
			// Allfällig schon gesetztes Ending (außer NEWLINE) wird überschrieben:
			setEnding(U32Token::Ending::SPACE);
			closed = true;
			break;
		}
		if (closed) {
			setAfterLast(uc);
			// Zurück zur Streampos vor diesem Zeichen,
			// damit dieses vom nächsten Token nochmals
			// eingelesen werden kann:
			is.seekg(startpos);
			break;
		}
		if (uc == CharUtil::U32Char::SOFT_HYPHEN) {
			closed = true;
			setEnding(U32Token::Ending::SOFT_HYPHEN); // Nicht in den Buffer lesen!
			continue; // nächstes Zeichen noch lesen!
		}
		for (string::const_iterator it = trenner.cbegin(); it != trenner.cend(); ++it) {
			if (uc != *it) continue;
			closed = true;
			setEnding(U32Token::Ending::SEPARATOR);
			break;
		}
		push_back(uc);
	}
	return is;
}


void U32Token::printInfo(ostream& os) {
	os << "U32Token '" << *this << "'\n";
	os << "\tAfterLast: '" << afterLast << "'\n";
	os << "\tEnding: " << endingToString(endsWith) << "\n";
}


void U32Token::setNewline() {
	if (! empty()) setEnding(U32Token::Ending::NEWLINE);
}



void U32Token::resetValues() {
	U32String::resetValues();
	afterLast = CharUtil::U32Char('\0');
	endsWith = Ending::NULLSTR;
}



void U32Token::setAfterLast(CharUtil::U32Char uc) { afterLast = uc; }

void U32Token::setAfterLast(char c) { afterLast = CharUtil::U32Char(c); }

CharUtil::U32Char U32Token::getAfterLast() const { return afterLast; }



/*
 *                         StringVisitor:
 *
 * */




int StringVisitor::parseU32Stream(std::istream& is) {
	int zeichen = 0;
	while (is) {
		CharUtil::U32Char uc(is);
		if (is.eof()) break;
		zeichen++;
		if (visit(uc) == VisitResult::BREAK) break;
	}
	return zeichen;
}


int StringVisitor::parseU32String(std::string str) {
	std::istringstream is(str);
	return parseU32Stream(is);
}


