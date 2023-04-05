/*
 * ocharutil.cpp
 *
 *  Created on: 20.06.2017
 *      Author: kanzlei
 */

#include "ounicode.h"

using namespace std;



namespace OUnicode {





/*---------------------------------------------------------
 *                 private Hilfsklasse U8Seq:
 * --------------------------------------------------------
 * */


struct U8SeqModel {
     static constexpr size_t    NUMERIC_BASE = 0x40; // 0x40 == 0xbf-0x80 + 1
     static constexpr size_t START_FOLGEBYTE = 0x80;

     const size_t count_bytes;      // 1, 2, 3, 4
     const char32_t starts_with;    // 0, 0x80, 0x800, 0x10000
     const size_t first_startbyte;  // 0, 0xc0, 0xe0, 0xf0
     const size_t startbyte_steps;  // 0, 0x40, 0x1000, 0x40000

     U8SeqModel(size_t cb, char32_t stw, size_t fst, size_t ss) :
          count_bytes{cb},
          starts_with{stw},
          first_startbyte{fst},
          startbyte_steps{ss} {}
};




bool operator==(const U8SeqModel& ls, const U8SeqModel& rs) {
     return (ls.count_bytes == rs.count_bytes);
}


bool operator !=(const U8SeqModel& ls, const U8SeqModel& rs) {
     return (! (ls == rs));
}




//-----------------/  Statische Sequenzobjekte:  /-------------------

// Enthalten abstrakte Informationen über Bytesequenzen

static const U8SeqModel       ASCII( 1, 0,       0,    0 );
static const U8SeqModel   TWO_BYTES( 2, 0x80,    0xc0, U8SeqModel::NUMERIC_BASE );
static const U8SeqModel THREE_BYTES( 3, 0x800,   0xe0, 0x1000 );
static const U8SeqModel  FOUR_BYTES( 4, 0x10000, 0xf0, 0x40000 );
static const U8SeqModel     NO_UTF8( 0, 0,       0,    0);




//-----------------/  Methoden für U8Seq:  /-----------------

inline const U8SeqModel& getU8Seq(char32_t value) {
     const U8SeqModel* ret = &ASCII;
     for (const U8SeqModel* us : { &TWO_BYTES, &THREE_BYTES, &FOUR_BYTES }) {
          if (value < us->starts_with) return *ret;
          ret = us;
     }
     return *ret; // FOUR_BYTES!
}


inline const U8SeqModel& examineStartByte(unsigned char c) {
     int leadingTrue = 0;
     for(int i = 7; i >= 0; --i) {
          if (! ((c >> i) & 1)) break; // bei einer Null abbrechen
          ++leadingTrue;
     }
     switch (leadingTrue) {
          case 0:  return ASCII;
          case 2:  return TWO_BYTES;
          case 3:  return THREE_BYTES;
          case 4:  return FOUR_BYTES;
          default: return NO_UTF8;
     }
}






char32_t read_U32_from_U8(std::istream& is) {
     while (is) {
          unsigned char c = is.get();
          if (is.eof()) return 0; // bei gültigem UTF8-Code
                                  // geht hier nichts verloren.
          const U8SeqModel seq = examineStartByte(c);
          if (seq == ASCII) return static_cast<char32_t>(c);
          if (seq == NO_UTF8) continue; // gelesenes Zeichen ignorieren...
          // seq ist eine gültige UTF-8-Sequenz...
          char32_t ret = (c - seq.first_startbyte) * seq.startbyte_steps;
          // Stream wird weiter ausgelesen:
          for (size_t bytepos = 2; bytepos <= seq.count_bytes; ++bytepos) {
               // 2. - 4. Byte bearbeiten...
               c = is.get();
               if (is.eof()) return 0; // Nur bei fehlerhaftem UTF-8 denkbar...
               if (bytepos == seq.count_bytes) {
                    // letztes Byte - ist immer ein Folgebyte!
                    ret += c - U8SeqModel::START_FOLGEBYTE;
                    return ret;
               }
               // U32-Wert des 1. und 2. Folgebytes berechnen, soweit
               // dieses kein letztes Byte ist. Die Schleife für das
               // letzte Byte ist weiter oben definiert!
               // Das 4. Byte kommt also nie hierher...
               int potenz = seq.count_bytes - bytepos;
               ret += (c -U8SeqModel::START_FOLGEBYTE) * std::pow(U8SeqModel::NUMERIC_BASE, potenz);
          }
     }
     return 0; // Kommt bei gültigem UTF-8 gar nicht hierher.
}




void toUpperLower_priv(std::string::iterator begin, std::string::iterator end, bool to_upper) {
	while (begin < end) {
		U8SeqModel seq = examineStartByte(*begin);
		if (seq == NO_UTF8) continue;
		if (seq == ASCII) {
			*begin = (to_upper) ? std::toupper(*begin) : std::tolower(*begin);
			++begin;
		} else if (seq.count_bytes == 2) {
			// Bearbeitet werden nur 2-Byte-Zeichen,
			// deren Startbyte == 0xc3.
			// Für die Berechnungen muß unsigned char garantiert sein!
			// *begin garantiert das nicht; daher umwandeln:
			unsigned char startByte = *begin;
			unsigned char folgeByte = *(++begin);
			if (startByte == 0xc3) {
				if (to_upper) {
					// 0x97 == Multiplikationszeichen
					// 0xb7 == Divisionszeichen
					// 0x9f == 'ß'
					if ((folgeByte != 0xb7) && (folgeByte > 0x9f)) {
						folgeByte -= 0x20; // toUpper...
					}
				} else {
					if ((folgeByte != 0x97) && (folgeByte < 0x9f)) {
						folgeByte += 0x20; // toLower...
					}
				}
				// jetzt von unsigned char wieder rückverwandeln:
				*begin = folgeByte;
			}
			++begin;
		} else {
			// Zeichen größer als 2 Byte ignorieren:
			begin += seq.count_bytes;
		}
	}
}



void toUpper(std::string::iterator begin, std::string::iterator end) {
	toUpperLower_priv(begin, end, true);
}

void toLower(std::string::iterator begin, std::string::iterator end) {
	toUpperLower_priv(begin, end, false);
}








/*---------------------------------------------------------
 *                       U32Char:
 * --------------------------------------------------------
 * */



//---------------/  U32Char direkt:  /------------------





bool U32Char::isSpace() {
	if (! isAscii()) return false;
	return (std::isspace(static_cast<char>(value)));
}


U32Char U32Char::toLower() const {
	if (value < 0x80) {
		char c = static_cast<char>(value);
		return U32Char(std::tolower(c));
	}
	// Ende ASCII-Großbuchstaben:
	// (0xd7 ist das Multiplikationszeichen)
	if ((value < 0xc0) || (value == 0xd7))
		return U32Char(value);
	// Jetzt beginnen wieder Großbuchstaben...
	if (value < 0xdf)
		return U32Char(value + 0x20);
	// Ende Großbuchstaben:
	if (value < 0x100)
		return U32Char(value);
	// Ab hier folgt der Kleinbuchstabe direkt dem Großbuchstaben
	// Großbuchstaben haben die geraden Zahlen
	if (value < 0x130) {
		return (value % 2) ? U32Char(value) : U32Char(value + 1);
	}
	// Ab hier wird es zu komplex...
	return U32Char(value);
}


U32Char U32Char::toUpper() const {
	if (value < 0x80) {
		char c = static_cast<char>(value);
		return U32Char(std::toupper(c));
	}
	// Ende ASCII-Großbuchstaben:
	// (oxf7 ist das Divisionszeichen)
	if ((value < 0xe0) || (value == 0xf7))
		return U32Char(value);
	// Jetzt beginnen wieder Kleinbuchstaben...
	if (value < 0x100)
		return U32Char(value - 0x20);
	// Ende Kleinbuchstaben:
	// Ab hier folgt der Kleinbuchstabe direkt dem Großbuchstaben
	// Kleinbuchstaben haben die ungeraden Zahlen
	if (value < 0x130) {
		return (value % 2) ? U32Char(value - 1): U32Char(value);
	}
	// Ab hier wird es zu komplex...
	return U32Char(value);
}





bool U32Char::toUtf8(char* buf, size_t bufsize) const {
     // in buf[0] wird die Anzahl der Bytes
     // geschrieben, das erste Byte steht demnach
     // in buf[1].
     // Deshalb muß bufsize > needsU8Bytes() sein!
     // Gibt bei Fehler false zurück.
     // Siehe dazu Wikipedia: UTF-8
     const U8SeqModel seq = getU8Seq(value);
     if (bufsize <= seq.count_bytes) {
          buf[0] = 0; // Fehler!
          return false;
     }
     buf[0] = seq.count_bytes;
     if (seq == ASCII) {
          buf[1] = value;
          return true;
     }
     // buf[1] ist ein Startbyte...
     buf[1] = seq.first_startbyte + (value / seq.startbyte_steps);

     // Folgebytes von der kleinsten Stelle an aufarbeiten:
     char32_t pot = value;
     for (size_t i : {0, 1, 2}) {
          buf[seq.count_bytes - i] = (pot % U8SeqModel::NUMERIC_BASE) +U8SeqModel::START_FOLGEBYTE;
          if (seq.count_bytes == (i + 2)) return true;
          pot /= U8SeqModel::NUMERIC_BASE;
     }
     return false; // Kann nicht sein, weil needs niemals
                   // größer als 4 ist.
}




// Gehört zu std::ostream& U32Char::toAscii(std::ostream& os, bool adapt, char undef):
inline void adaptCapital(std::ostream& os, char c, bool adapt) {
	char sc = (adapt ? 'E' : 'e');
	os.put(c);
	os.put(sc);
}


std::ostream& U32Char::toAscii(std::ostream& os, bool adapt, char undef) const {
	if (isAscii()) {
		os.put(static_cast<char>(value));
	} else if (! isLatin1()) {
		os.put(undef);
	} else { // Latin1 aber kein Ascii...
		switch (getValue()) {
		case LATIN_CAPITAL_AE:
			adaptCapital(os, 'A', adapt);
			break;
		case LATIN_CAPITAL_OE:
			adaptCapital(os, 'O', adapt);
			break;
		case LATIN_CAPITAL_UE:
			adaptCapital(os, 'U', adapt);
			break;
		case LATIN_SMALL_AE:
			os << "ae";
			break;
		case LATIN_SMALL_OE:
			os << "oe";
			break;
		case LATIN_SMALL_UE:
			os << "ue";
			break;
		case LATIN_SMALL_SZ:
			os << "ss";
			break;
		default:
			os.put(undef);
		}
	}
	return os;
}




std::ostream& operator<<(std::ostream& os, const U32Char& uc) {
     static constexpr int BUFSIZE = 5;
     static char buf[BUFSIZE];
     if (! uc.toUtf8(buf, BUFSIZE)) {
          os.put('?');
          return os;
     }
     for (int i = 1; i <= buf[0]; ++i) {
          os.put(buf[i]);
     }
     return os;
}







float U32CharCounter::getCount() const {
	return (bytesLeft == 0) ? (float)u32complete : ((float)u32complete + 0.5);
}



float U32CharCounter::put(char c) {
	if (bytesLeft > 0) { // Bin in einem Multibyte-Zeichen
		if (--bytesLeft == 0) u32complete++;
	} else { // Bin am Beginn eines neuen Zeichens
		int leadingTrue = 0; // führende Einsen
		for(int i = 7; i >= 0; --i) {
		   if (! ((c >> i) & 1)) break; // bei einer Null abbrechen
		   ++leadingTrue;
		}
		bytesTotal = leadingTrue + 1;
		if (leadingTrue == 0) { // ASCII-Zeichen
		  u32complete++;
		} else {
		  bytesLeft = leadingTrue;
		}
	}
	return getCount();
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






istream& operator>>(istream& is, U32String& us) {
	while (is) {
		U32Char uc(is);
		us.push_back(uc);
	}
	return is;
}




ostream& operator<<(ostream& os, const U32String& us) {
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
		U32Char uc(is);
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
		if (uc == U32Char::SOFT_HYPHEN) {
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
	afterLast = U32Char('\0');
	endsWith = Ending::NULLSTR;
}



void U32Token::setAfterLast(U32Char uc) { afterLast = uc; }

void U32Token::setAfterLast(char c) { afterLast = U32Char(c); }

U32Char U32Token::getAfterLast() const { return afterLast; }



/*
 *                         StringVisitor:
 *
 * */




int StringVisitor::parseU32Stream(std::istream& is) {
	int zeichen = 0;
	while (is) {
		U32Char uc(is);
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



//void printInfo(std::ostream& os, const U32Char& uc) {
//     static constexpr int BUFSIZE = 5;
//     static char buf[BUFSIZE];
//     os.fill('0');
//     os << "\nZeichen U+" << std::hex << std::setw(4)  << uc.getValue() << " = '" << uc << "'\n";
//     Format::toBin(os, uc.getValue());
//     uc.toUtf8(buf, BUFSIZE);
//     for (int i = 1; i <= buf[0]; ++i) {
//          unsigned char c = buf[i];
//          os << "\n   " << i << ". Byte: 0x" << std::setw(4)  << std::hex << static_cast<unsigned int>(c) << " = ";
////          Format::toBin(os, c); // Wegen Fehlermeldung auskommentiert!!!
//     }
//     U32Char lower = uc.toLower();
//     if (lower != uc) os << "\nto_lower: '" << lower << "'\n";
//     os << "\n";
//}



} // Ende Namespace OUnicode


