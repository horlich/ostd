/*
 * ocharutil.h
 *
 *  Created on: 20.06.2017
 *      Author: kanzlei
 */

#ifndef OCHARUTIL_H_
#define OCHARUTIL_H_

//#include <ostream>
//#include <istream>
//#include <iostream>
//#include <sstream>
//#include <cmath>
//#include <iomanip>

//#include "oformat.h"
#include "oexception.h"



namespace CharUtil {


constexpr wchar_t FIRST_PRINTABLE   = 0x20;
constexpr wchar_t LAST_ASCII        = 0x7F; // 127

constexpr wchar_t       NB_SPACE    = 0xA0;
constexpr wchar_t    SOFT_HYPHEN    = 0xAD;
constexpr wchar_t        EN_DASH    = 0x2013;
constexpr wchar_t        EM_DASH    = 0x2014;
constexpr wchar_t         BULLET    = 0x2022;
constexpr wchar_t     MIDDLE_DOT    = 0xB7;
constexpr wchar_t           EURO    = 0x20AC;
constexpr wchar_t LINE_SEPARATOR    = 0x2028;

// Umlaute und 'ß'
constexpr wchar_t LATIN_CAPITAL_AE  = 0xC4; // 'Ä'
constexpr wchar_t LATIN_CAPITAL_OE  = 0xD6; // 'Ö'
constexpr wchar_t LATIN_CAPITAL_UE  = 0xDC; // 'Ü'
constexpr wchar_t   LATIN_SMALL_SZ  = 0xDF; // 'ß'
constexpr wchar_t   LATIN_SMALL_AE  = 0xE4; // 'ä'
constexpr wchar_t   LATIN_SMALL_OE  = 0xF6; // 'ö'
constexpr wchar_t   LATIN_SMALL_UE  = 0xFC; // 'ü'



// gibt den inneren Zahlenwert der Ziffer zurück,
// z.B. 6 für '6'
// Bei Fehler wird -1 zurückgegeben.
int numVal(char c);


void testNumVal(std::ostream& = std::cout);


/*
   Aus Kapitel 28.2 der ISO 2020:
   (Braucht #include <locale>)
   template<class charT> bool isspace (charT c, const locale& loc);
   template<class charT> bool isprint (charT c, const locale& loc);
   template<class charT> bool iscntrl (charT c, const locale& loc);
   template<class charT> bool isupper (charT c, const locale& loc);
   template<class charT> bool islower (charT c, const locale& loc);
   template<class charT> bool isalpha (charT c, const locale& loc);
   template<class charT> bool isdigit (charT c, const locale& loc);
   template<class charT> bool ispunct (charT c, const locale& loc);
   template<class charT> bool isxdigit(charT c, const locale& loc);
   template<class charT> bool isalnum (charT c, const locale& loc);
   template<class charT> bool isgraph (charT c, const locale& loc);
   template<class charT> bool isblank (charT c, const locale& loc);
   template<class charT> charT toupper(charT c, const locale& loc);
   template<class charT> charT tolower(charT c, const locale& loc);
*/


class KeinUtf8 : public OException::ParseException  {
public:
	KeinUtf8(const std::string& mess) : ParseException(mess) {}

	KeinUtf8(const char* mess = "Kein UTF-8-Zeichen") : ParseException(mess) {}
};










}; // Ende Namespace CharUtil






#endif /* OCHARUTIL_H_ */
