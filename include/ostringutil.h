/*
 * ostringutil.h
 *
 *  Created on: 18.05.2017
 *      Author: kanzlei
 */

#ifndef OSTRINGUTIL_H_
#define OSTRINGUTIL_H_

#include <string>
//#include <algorithm>
#include <vector>
//#include <iostream>
//#include <sstream>
//#include <iomanip>
//#include <cwchar>
//#include <locale>
#include <codecvt>
#include <cstring>

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




using StrVec = std::vector<std::string>;


constexpr const char WHITESPACES[] = " \t\n\v\f\r";

std::string rtrim(const std::string& str);

std::string ltrim(const std::string& str);

std::string trim(const std::string& str);

int split(const std::string& str, char tz, StrVec& vec);

int parseArgs(const std::string& str, StrVec& vec);



/*
 *             Umwandlung Multibyte-String <-> Wide-String
 * */


using WC_Converter =  std::wstring_convert<std::codecvt_utf8<wchar_t>>;
/* Usage:
 * WC_Converter conv
 * conv.to_bytes(std::wstring);
 * conv.from_bytes(std::string);
 * conv.converted(); //
 */

using U32_Converter = std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t>;
/*
   Siehe auch std::wbuffer_convert !
*/

 /* Konvertiere den MB-String in einen Wide-String
  * und gib die Anzahl der Wide-Chars zurück.
  * Gibt bei Fehler -1 zurück. Wahrscheinlichster
  * Fehler: locale nicht gesetzt.
  * Siehe auch 'man mblen'    */
int getWSize(const char* str);

/* WC_Converter ist diesen beiden Methoden vorzuziehen! */
std::wstring toWstring(const char* str, size_t maxlen);

std::wstring toWstring(std::string& s);

/* Ermittle die Anzahl an Bytes in einem Wide-String.
 * Gibt bei Fehler -1 zurück. Wahrscheinlichster
 * Fehler: locale nicht gesetzt: */
int getMBSize(const wchar_t* wstr);

/* WC_Converter ist diesen beiden Methoden vorzuziehen! */
std::string toMBstring(const wchar_t* wstr, size_t maxlen);

std::string toMBstring(std::wstring& s);




class StringVisitor {
//
public:
   enum class VisitResult { BREAK, CONTINUE };

   virtual ~StringVisitor() {}


   int parseStream(std::wistream& is);


   int parseString(std::string str);

protected:
   virtual VisitResult visit(wchar_t) = 0;
};



/******** Klasse CharArray: ********/
template <size_t size>
class CharArray {
   /* Für Memory::Objektspeicher. Dort müssen Strings *
    * als Wert-Arrays gespeichert sein.               */
   char buf[size+1] {'\0'};
public:
   void setStr(const char* str);

   std::string getStr() const;

   size_t capacity() const;
};



/* Template-Funktionen einbinden: */
#include "ostringutil.tpp"

} // Ende namespace StringUtil



#endif /* OSTRINGUTIL_H_ */
