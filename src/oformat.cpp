/*
 * oformat.cpp
 *
 *  Created on: 04.05.2017
 *      Author: kanzlei
 */


#include "oformat.h"



using namespace std;
using namespace Format;





void Format::setGlobalDELokale() {
	std::locale::global(std::locale("de_AT.UTF-8"));
}




Putn& Putn::operator()(int i) {
	anz = i;
	return *this;
}


std::ostream& Format::operator<< (std::ostream& os, const Putn& pt) {
	for (int i = pt.getAnz(); i > 0; i--) os.put(pt.getChar());
	return os;
}



std::string Indent::operator()(std::string str) {
	std::stringstream buf;
	char c = getChar();
	for (int i = getAnz(); i > 0; i--) {
		buf.put(c);
	}
	buf << str;
	return buf.str();
}

std::ostream& Format::operator<< (std::ostream& os, const Format::Indent& in) {
	os << in;
	return os;
}





std::ostream& Format::printRahmenZeile(std::ostream& os, int zeilenLaenge, const std::string& links, const std::string& rechts, char zeilenMarker) {
	int zll = StringUtil::u8Strlen(links);
//	cout << links << " hat " << zll << endl;
	int zlr = StringUtil::u8Strlen(rechts);
	int strl = zll + zlr;
	/* Unter Berücksichtigung eines Zwischen(leer)zeichens: */
	if ((strl+1) > zeilenLaenge)	{
		std::stringstream buf;
		buf << "Rahmenzeile '" << links << " " << rechts << "' übersteigt Zeilenlänge " << zeilenLaenge;
		throw TextOverflow(buf.str());
	}
	os << links;
	os.put(' ');
	for (int anzfz = (zeilenLaenge - strl - 2); anzfz > 0; anzfz--) os.put(zeilenMarker);
	os.put(' ');
	os << rechts;
	return os;
}







/*
 *                     Formatierer allgemein:
 * */


Formatierer Formatierer::operator() (int i) {
	argFormat.setVal(i);
	return *this;
}


Formatierer Formatierer::operator() (double d) {
	argFormat.setVal(d);
	return *this;
}

ostream& Format::operator<<(ostream& os, const Formatierer& fmt) {
	return fmt.argFormat.print(os);
}






/*
 *               Binär:
 * */

int Format::toBin(istream& is, ostream& os) {
	int countBytes = 0;
	while (is) {
		char b = is.get();
		if (b == EOF) break;
		if (countBytes++ > 0) os.put(' ');
		for (char i = 7; i >= 0; --i) {
			char x = ((1<<i) & b) ? '1' : '0';
			os.put(x);
		}
	}
	return countBytes;
}


int Format::btoi(const std::string& bin) {
	int ret = 0;
	int exp = 0;
	for (std::string::const_reverse_iterator it = bin.crbegin(); it != bin.crend(); ++it) {
		if (*it == '1') ret += pow(2, exp);
		++exp;
	}
	return ret;
}


class Binaer : public ArgFormat {
	int wert;

	ostream& printValid(ostream& os) const {
		return toBin(os, wert);
	}

public:
	Binaer(int i = 0) : wert(i) {}

	void setVal(int i) { wert = i; setValid(); }
};


Binaer argFormatBinaer;
Formatierer bin(argFormatBinaer);









/*
 *                       Zahlenformat Dkaufm:
 * */


std::ios::fmtflags fmtDkfm(std::ios& stream) {
	std::ios::fmtflags ret = stream.flags();
	stream.setf(std::ios::fixed, std::ios::floatfield);
	stream.precision(2);
	return ret;
}









//------------------/  Nummernordner:  /----------------------------


NummernOrdner::NummernOrdner(size_t eb) : ebenen(eb) {
	if (ebenen == 0) return;
	buf = new int[ebenen] {0};
	stringRep.reserve(20);
}


NummernOrdner::~NummernOrdner() { delete [] buf; }



void NummernOrdner::touch(size_t ebene) {
	if (ebene >= ebenen) throw OException::IndexOutOfBoundsException("[Format::Nummernordner] ungültige Ebene: " + std::to_string(ebene));
	for (size_t i = 0; i < ebenen; i++) {
		int tmp = buf[i];
		if (i < ebene) {
			if (tmp == 0) tmp = 1;
		} else if (i == ebene) {
			tmp++;
		} else {
			tmp = 0;
		}
		buf[i] = tmp;
	}
}




std::string NummernOrdner::toString() const {
	stringRep.clear();
	for (size_t i = 0; ((i < ebenen) && (buf[i] > 0)); ++i){
		if (i>0) stringRep.push_back('.');
		stringRep.append(std::to_string(buf[i]));
	}
	return stringRep;
}



std::ostream& Format::operator<<(std::ostream& os, const NummernOrdner& no) {
	return os << no.toString();
}









/*
 *              Buchstaben-Nummerierung:
 * */




inline void numToAlpha(int num, char* dest, size_t maxlen, char nullWert) {
	//
	// Private Hilfsfunktion für
	// numToLowerAlpha(int num, char* dest, size_t maxlen) und
	// numToUpperAlpha(int num, char* dest, size_t maxlen)
	//
	if (num < 1) {
		*dest = '*';
		dest[1] = 0;
		return;
	}
	// 26 Buchstaben, BASE ist demnach 27:
	static constexpr int BASE = ('z' - 'a' + 1); // = 27
	char buf[maxlen];
	char* buf_it = buf;
	char* buf_last = &(buf[maxlen-1]);
	while (true) {
		int i = num - 1; // weil 'a' == NULLWERT
		int quotient = i / BASE;
		int rest = i % BASE;
		*buf_it = static_cast<char>(nullWert + rest);
		if ((quotient == 0) || (buf_it == buf_last)) break;
		num = quotient;
		++buf_it;
	}
	while (true) {
		*dest++ = *buf_it;
		if (buf_it-- == buf) break;
	}
	*dest = 0;
}



void Format::numToLowerAlpha(int num, char* dest, size_t maxlen) {
	return numToAlpha(num, dest, maxlen, 'a');
}


void Format::numToUpperAlpha(int num, char* dest, size_t maxlen){
	return numToAlpha(num, dest, maxlen, 'A');
}


void testAlphaNum(int max, std::ostream& os) {
	static constexpr int SIZE = 10;
	char buf[SIZE];
	for (int i = 0; i <=max; ++i) {
		os << setw(5) <<  i << " = ";
		numToLowerAlpha(i, buf, SIZE);
		os << buf << " = ";
		numToUpperAlpha(i, buf, SIZE);
		os << buf << "\n";
	}
}




RahmenZeilenModell::RahmenZeilenModell(int zl, char zm) : zeilenlaenge(zl), zeilenmarker(zm) {}

std::ostream& RahmenZeilenModell::printZeile(std::ostream& os, const std::string& links, const std::string& rechts) {
	Format::printRahmenZeile(os, zeilenlaenge, links, rechts, zeilenmarker);
	return os;
}










