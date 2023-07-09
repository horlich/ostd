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
#include <regex>

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
               toUpper/toLower:
*/
/* Siehe Kapitel 28.2 der ISO 2020
   Das Template convenience ist für alle Methoden,
   die in Kapitel 28.2 angeführt sind, um diese
   iterativ auf einen String anzuwenden.
   Methoden brauchen #include <locale>:     */
template <typename charType>
std::basic_string<charType> convenience(const std::basic_string<charType>& ws, charType(*func_pointer)(charType, const std::locale&));

/* Spezialisierungen: */

std::string toUpper(const std::string& ws);

std::wstring toUpper(const std::wstring& ws);

std::string toLower(const std::string& ws);

std::wstring toLower(const std::wstring& ws);


/* Siehe Breymann S. 708
   Verändert den übergebenen String selbst.
   Als Argument kann jeder basic_string übergeben werden:
   ACHTUNG: diese Variante ist fast zehnmal LANGSAMER als
   die oben dargestellte mit conveniance!!              */
template <typename charType> /* toUpper */
void gross(std::basic_string<charType>& str, const std::locale& loc = std::locale("")) {
   std::use_facet<std::ctype<charType>>(loc).toupper(str.data(), str.data() + str.length());
}

template <typename charType> /* toLower */
void klein(std::basic_string<charType>& str, const std::locale& loc = std::locale("")) {
   std::use_facet<std::ctype<charType>>(loc).tolower(str.data(), str.data() + str.length());
}



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


/* Mit den Methoden von std::ctype: */




/*-----------------------/ String-Tokenizer /-------------------------------*/

/* Siehe dazu auch 'man strtok' */


template <typename charType>
using Xiterator = std::regex_iterator<typename std::basic_string<charType>::const_iterator>;

template <typename charType>
using BStrVec = std::vector<std::basic_string<charType>>;

template <typename charType>
BStrVec<charType> tokenize_regex(const std::basic_string<charType>& str, const charType* muster);

std::vector<std::wstring> tokenize_ws(const std::wstring& str);

/* Fügt in einen String '\n' ein, sodaß linewidth nicht
   überschritten wird:                                      */
std::wstring wrap (const std::wstring& str, int linewidth);

/* Gibt einen Vector aus den umgebrochenen Zeilen zurück: */
std::vector<std::wstring> wrap_vec (const std::wstring& str, int linewidth);


/*--------------------------/ StringVisitor: /----------------------------*/

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
