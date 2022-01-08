/*
 * oformat.h
 *
 *  Created on: 04.05.2017
 *      Author: kanzlei
 */

#ifndef OFORMAT_H_
#define OFORMAT_H_

#include<sstream>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include <locale>
#include <wchar.h>
#include <cmath>
#include <math.h>
#include <stack>
#include <locale>

#include "ocharutil.h"
#include "ostringutil.h"
#include "oexception.h"
#include "otime.h"


namespace Format {




class OFormatException : public OException::Fehler {
public:
	OFormatException(const std::string& message) :
		Fehler(message) {}

	virtual ~OFormatException() = default;
};


/* Globale Lokale auf de_AT.UTF-8 setzen: */
void setGlobalDELokale();


class TextOverflow : public OFormatException {
public:
	TextOverflow(const std::string& message) :
		OFormatException(message) {}

	virtual ~TextOverflow() = default;
};





/*  Dasselbe Zeichen mehrfach ausgeben:
 *  Putn ist ein Funktionsobjekt (Funktor)
 *  wg. Überladung des operator()
 *
 *  Anwendungsbeispiel:
 *  Putn punkte('.');
 *  cout << punkte(4);
 *  Gibt 4 Punkte aus.                   */
class Putn {
//
private:
	char zeichen;
	int anz;

public:
	Putn(char c, int i = 0) : zeichen(c), anz(i) {};

	virtual ~Putn() = default;

	Putn& operator()(int i = 0);

	int getAnz() const { return anz; }

	char getChar() const { return zeichen; }
};

std::ostream& operator<< (std::ostream& os, const Putn& pt);






/* Gibt Text eingerückt aus: */
class Indent : public Putn {
public:
	Indent(int i = 0, char c = ' ') : Putn(c, i) {}

	std::string operator()(std::string str);
};


std::ostream& operator<< (std::ostream& os, const Indent& in);







std::ostream& printRahmenZeile(std::ostream& os, int zeilenLaenge, const std::string& links, const std::string& rechts, char zeilenMarker = '.');












/*
 *
 *               Spezielle Formatierer:
 *
 * */


class ArgFormat {
	// Alles, was als 'ostream << name(arg)' verwendet werden kann
	bool validVal;

protected:
	virtual std::ostream& printValid(std::ostream&) const = 0;

	inline void setValid(bool val = true) { validVal = val; }

public:
	ArgFormat() : validVal(false) {}

	virtual ~ArgFormat() {}

	std::ostream& print(std::ostream& os) const {
		if (! validVal) return os << "???";
		return printValid(os);
	}

	// Unterklassen, die den Wert definieren, setzen setValid(true):
	virtual void setVal(int) { validVal = false; }

	virtual void setVal(double d) { validVal = false; }
};


class Formatierer {
	ArgFormat& argFormat;
	public:
		Formatierer(ArgFormat& af) : argFormat(af) {}

		Formatierer operator() (int);

		Formatierer operator() (double);

		friend std::ostream& operator<<(std::ostream&, const Formatierer&);
};


std::ostream& operator<<(std::ostream&, const Formatierer&);










/*
 *             Kaufmännisches Zahlenformat:
 * */

// Gibt die alten Flags des Stream zurück.
// Voraussetzung, damit korrekt formatiert: std::ios::imbue(locale("de_AT.UTF-8"))
std::ios::fmtflags fmtDkfm(std::ios&);









/*
 *             Binäres Format:
 * */

/* Weil ein Template, ist die Definition in dieser Datei weiter unten: */
template <class N> std::ostream& toBin(std::ostream&, N);


// Gibt die binäre Struktur eines istream aus:
int toBin(std::istream& is, std::ostream& os);


// Binäre Sequenz in eine int umwandeln:
// Geht vorerst nur für positive Zahlen
int btoi(const std::string& bin);


// Usage: bin(int) // Ausgabe im binären Format
// z.B. cout << bin('f');
extern Formatierer bin;















class NummernOrdner final {
	//
	int* buf { nullptr };
	size_t ebenen {0};
	mutable std::string stringRep; // Ausreichend großer Buffer für Umwandlungen

	NummernOrdner(const NummernOrdner&) = delete; // Kopieren verboten

	NummernOrdner& operator=(const NummernOrdner&) = delete; // Zuweisen verboten

public:
	NummernOrdner(size_t ebenen = 1);


	NummernOrdner(NummernOrdner&&) = default;


	~NummernOrdner();


	void touch(size_t ebene);


	std::string toString() const;
};



std::ostream& operator<<(std::ostream& os, const NummernOrdner& no);








/*
 *              Buchstaben-Nummerierung:
 *
 * */

void numToLowerAlpha(int num, char* dest, size_t maxlen);


void numToUpperAlpha(int num, char* dest, size_t maxlen);


void testAlphaNum(int max, std::ostream& = std::cout);






class RahmenZeilenModell {
private:
	int zeilenlaenge;
	char zeilenmarker;
public:
	RahmenZeilenModell(int zl, char zm = '.');

	std::ostream& printZeile(std::ostream& os, const std::string& links, const std::string& rechts);
};






/*
 *                               Template-Definitionen:
 *
 * */



template <class N>
std::ostream& Format::toBin(std::ostream& os, N zahl) {
	for(int i=sizeof(N)*8-1; i>=0; --i) {
		char c = ((zahl>>i) & 1) ? '1' : '0';
		os.put(c);
		if (i%8==0) os.put(' ');
	}
	return os;
}






}; // Ende namespace Format

#endif /* OFORMAT_H_ */
