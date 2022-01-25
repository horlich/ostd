#ifndef OUNICODE_H_INCLUDED
#define OUNICODE_H_INCLUDED


//#include <ostream>
//#include <istream>
//#include <iostream>
//#include <sstream>
//#include <cmath>
//#include <iomanip>
//#include <vector>

#include "oexception.h"
#include "oformat.h"


namespace OUnicode {



// Einen char32_t einlesen:
// Nicht-UTF8-Zeichen werden ignoriert.
char32_t read_U32_from_U8(std::istream& is);




class KeinUtf8 : public OException::ParseException  {
public:
	KeinUtf8(const std::string& mess) : ParseException(mess) {}

	KeinUtf8(const char* mess = "Kein UTF-8-Zeichen") : ParseException(mess) {}
};




void toUpper(std::string::iterator begin, std::string::iterator end);

void toLower(std::string::iterator begin, std::string::iterator end);






/*---------------------------------------------------------
 *                       U32Char:
 * --------------------------------------------------------
 * */

class U32Char {
	char32_t value;

public:
	static constexpr char32_t FIRST_PRINTABLE = 0x20;

	static constexpr char32_t       NB_SPACE = 0xA0;
	static constexpr char32_t    SOFT_HYPHEN = 0xAD;
	static constexpr char32_t        EN_DASH = 0x2013;
	static constexpr char32_t        EM_DASH = 0x2014;
	static constexpr char32_t         BULLET = 0x2022;
	static constexpr char32_t     MIDDLE_DOT = 0xB7;
	static constexpr char32_t           EURO = 0x20AC;
	static constexpr char32_t LINE_SEPARATOR = 0x2028;

	// Umlaute und 'ß'
	static constexpr char32_t LATIN_CAPITAL_AE = 0xC4; // 'Ä'
	static constexpr char32_t LATIN_CAPITAL_OE = 0xD6; // 'Ö'
	static constexpr char32_t LATIN_CAPITAL_UE = 0xDC; // 'Ü'
	static constexpr char32_t   LATIN_SMALL_SZ = 0xDF; // 'ß'
	static constexpr char32_t   LATIN_SMALL_AE = 0xE4; // 'ä'
	static constexpr char32_t   LATIN_SMALL_OE = 0xF6; // 'ö'
	static constexpr char32_t   LATIN_SMALL_UE = 0xFC; // 'ü'


	U32Char(char32_t val = 0) :
			value { val } {
	}

	U32Char(std::istream& is) :
			value { read_U32_from_U8(is) } {
	}

	inline char32_t getValue() const { return value; }

	inline bool isAscii() const { return value < 0x80; }

	// Gibt auch für ASCII-Zeichen true aus:
	inline bool isLatin1() const { return value < 0x100; }


	bool isSpace();

	U32Char toLower() const;

	U32Char toUpper() const;

	bool toUtf8(char* buf, size_t bufsize) const;

//	void printInfo(std::ostream& os = std::cout) const;


	/* adapt: true = Ä->AE ä->ae; false = Ä->Ae ä->ae
	 * undef: Zeichen, wenn Umwandlung nicht gelingt. */
	std::ostream& toAscii(std::ostream&, bool adapt, char undef) const;


	inline bool operator==(const U32Char& uc) const {
		return this->getValue() == uc.getValue();
	}

	inline bool operator==(char32_t c) const {
		return this->getValue() == c;
	}

	inline bool operator!=(const U32Char& uc) const {
		return !(*this == uc);
	}

	inline bool operator!=(char32_t c) const {
		return !(*this == c);
	}

};


//--------------/  Operatoren zu U32Char:  /------------------


std::ostream& operator<<(std::ostream&, const U32Char&);




//--------------/  Testmethoden zu U32Char:  /------------------


void testU32Char(char32_t begin = U32Char::FIRST_PRINTABLE, char32_t last = 0x1000, std::ostream& = std::cout);



void testToUpperLower(std::ostream& = std::cout);





class U32CharCounter {
	/*
	 * Solange Multibyte-Zeichen unvollständig eingelesen
	 * sind, geben put() und getCount() eine Kommazahl
	 * aus. Die Ziffern vor dem Komma bezeichnen die
	 * vollständig eingelesenen Zeichen. Ist das letzte
	 * Zeichen nicht vollständig eingelesen, so wird
	 * zu dieser Zahl 0,5 addiert.
	 * Setzt einen fehlerfreien U32-String voraus!
	 * */
private:
	int bytesLeft 	= 0;
	int u32complete = 0;
	int bytesTotal	= 0;
public:
	float getCount() const;

	inline int bytesMissing() const { return bytesLeft; }

	inline int getBytesTotal() const { return bytesTotal; }

	float put(char c);
};



class U32String : public std::vector<U32Char> {
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

   static std::string endingToString(Ending end)
   {
      switch (end) {
      case Ending::NULLSTR:
         return "NULLSTR";
      case Ending::SOFT_HYPHEN:
         return "SOFT_HYPHEN";
      case Ending::SEPARATOR:
         return "SEPARATOR";
      case Ending::SPACE:
         return "SPACE";
      case Ending::NEWLINE:
         return "NEWLINE";
      default:
         ;
      }
      return "FEHLER";
   }

private:
   // afterLast ist NICHT Bestandteil des U8Token, sondern
   // muß gegebenenfalls in ein neues U8Token eingelesen werden!
   U32Char afterLast;
   Ending endsWith;
   static const std::string DEFAULT_TRENNER;

public:
   U32Token();

   U32Token(std::istream&, const std::string& trenner = DEFAULT_TRENNER);

   virtual ~U32Token() = default;

   virtual void resetValues();

   // Gegebenenfalls ist vorher resetValues() aufzurufen!
   std::istream& init(std::istream&, const std::string& = DEFAULT_TRENNER);

   void setAfterLast(U32Char uc);

   void setAfterLast(char c);

   U32Char getAfterLast() const;

   inline void setEnding(Ending e)
   {
      endsWith = e;
   }

   inline Ending getEnding() const
   {
      return endsWith;
   }

   void setNewline();

   void printInfo(std::ostream& os);
};



class StringVisitor {
//
public:
   enum class VisitResult { BREAK, CONTINUE };

   virtual ~StringVisitor() {}


   int parseU32Stream(std::istream& is);


   int parseU32String(std::string str);

protected:
   virtual VisitResult visit(const U32Char&) = 0;
};


void printInfo(std::ostream& os, const U32Char& uc);






} // Ende Namespace OUnicode


#endif // OUNICODE_H_INCLUDED
