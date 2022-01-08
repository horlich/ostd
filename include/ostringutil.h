/*
 * ostringutil.h
 *
 *  Created on: 18.05.2017
 *      Author: kanzlei
 */

#ifndef OSTRINGUTIL_H_
#define OSTRINGUTIL_H_

#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cwchar>
#include <locale>
#include <codecvt>

#include "oexception.h"
#include "ocharutil.h"
#include "debug.h"




namespace StringUtil {





class OStringUtilException : public OException::Fehler {
public:
	OStringUtilException(const std::string& message) :
		Fehler(message) {}

	virtual ~OStringUtilException() = default;
};




class KeinUTF8 : public OStringUtilException {
public:
	KeinUTF8(const std::string& message) :
		OStringUtilException(message) {}

	virtual ~KeinUTF8() = default;
};






typedef std::vector<std::string> StrVec;


const char WHITESPACES[] = " \t\n\v\f\r";

std::string rtrim(const std::string& str);

std::string ltrim(const std::string& str);

std::string trim(const std::string& str);

int split(const std::string& str, char tz, std::vector<std::string>& vec);




void u8_to_upper(std::string& str);


void u8_to_lower(std::string& str);



int parseArgs(const std::string& str, std::vector<std::string> *vec);









/*
 *             Umwandlung Multibyte-String <-> std::u32string
 * */

extern std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t> U32_CONVERTER;
	// Usage:
	// U32_CONVERTER.to_bytes(u32string)
	// U32_CONVERTER.from_bytes(std::string);



/*
 *             Umwandlung Multibyte-String <-> Wide-String
 * */

int getWSize(const char* str);

std::wstring toWstring(const char* str, size_t maxlen);

std::wstring toWstring(std::string& s);


int getMBSize(const wchar_t* wstr, size_t maxlen);

std::string toMBstring(const wchar_t* wstr, size_t maxlen);

std::string toMBstring(std::wstring& s);





// Länge des Strings unter Berücksichtigung der UTF-Chars:
int u8Strlen(const std::string& str);





class ArgParser : protected StrVec {
private:
public:
	int parse(std::string& str) {
		clear();
		StringUtil::parseArgs(str, this);
		return size();
	}

	std::string getArg(int i) const {
		return at(i);
	}

	inline int argc() const { return size(); }

	inline bool empty() const { return StrVec::empty(); }
};







class U32String : public std::vector<CharUtil::U32Char> {
public:
	virtual ~U32String() = default;

	virtual void resetValues();

	bool containsGraph() const;

	std::string str() const;
};

std::ostream& operator<<(std::ostream&, const U32String&);

std::istream& operator>>(std::istream&, U32String&);






class U32Token : public U32String {
public:
	enum class Ending { NULLSTR, SOFT_HYPHEN, SEPARATOR, SPACE, NEWLINE };

	static std::string endingToString(Ending end) {
		switch (end) {
		case Ending::NULLSTR: return "NULLSTR";
		case Ending::SOFT_HYPHEN: return "SOFT_HYPHEN";
		case Ending::SEPARATOR: return "SEPARATOR";
		case Ending::SPACE: return "SPACE";
		case Ending::NEWLINE: return "NEWLINE";
		default:;
		}
		return "FEHLER";
	}

private:
	// afterLast ist NICHT Bestandteil des U8Token, sondern
	// muß gegebenenfalls in ein neues U8Token eingelesen werden!
	CharUtil::U32Char afterLast;
	Ending endsWith;
	static const std::string DEFAULT_TRENNER;

public:
	U32Token();

	U32Token(std::istream&, const std::string& trenner = DEFAULT_TRENNER);

	virtual ~U32Token() = default;

	virtual void resetValues();

	// Gegebenenfalls ist vorher resetValues() aufzurufen!
	std::istream& init(std::istream&, const std::string& = DEFAULT_TRENNER);

	void setAfterLast(CharUtil::U32Char uc);

	void setAfterLast(char c);

	CharUtil::U32Char getAfterLast() const;

	inline void setEnding(Ending e) { endsWith = e; }

	inline Ending getEnding() const { return endsWith; }

	void setNewline();

	void printInfo(std::ostream& os);
};




class StringVisitor {

public:
	enum class VisitResult { BREAK, CONTINUE };

	virtual ~StringVisitor() {}


	int parseU32Stream(std::istream& is);


	int parseU32String(std::string str);

protected:
	virtual VisitResult visit(const CharUtil::U32Char&) = 0;
};






} // Ende namespace ostringutil



#endif /* OSTRINGUTIL_H_ */
