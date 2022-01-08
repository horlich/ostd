#include <string>
#include "otime.h"


using namespace std;


namespace Zeit {





/*
 *                   Statische Methoden:
 * */

bool setAtLocale() { return (setlocale(LC_TIME, "de_AT.UTF-8") != 0); }


bool istSchaltjahr(int jahr) {
	return  (( (jahr % 4 == 0) && (jahr % 100 != 0) ) || (jahr % 400 == 0));
}

Day today() {
	OTime ot;
	return ot.getDay();
}

OTime now() { return OTime(); }



/*
 *                   Klasse: Year
 * */



Year::Year(size_t zahl) :jz(zahl) {}



Year& Year::operator()(size_t j){
	jz = j;
	return *this;
}







/*
 *                          Klasse: Month
 * */

Month::Month(size_t zahl, const Year& j) : mz(zahl), jahr(j) {}



Month::Month(const OTime* ptr) {
	if (! ptr) {
		OTime jetzt;
		ptr = &jetzt;
	}
	mz = ptr->mm();
	jahr(ptr->jj());
}


Month& Month::operator()(size_t mm, const Year& j) {
	mz = mm;
	jahr = j;
	return *this;
}


/* Validiere: */
Month::operator bool() const {
	return ((mz) && (mz < 13));
}


int Month::days() const {
	switch (mz) {
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;
	case 2:
		return (jahr.istSchaltjahr()) ? 29 : 28;
	default:
		return 31;
	}
}


void Month::validate() const {
	if (! *this)
		throw OException::IllegalArgumentException("Ungültige Monatszahl" + std::to_string(mz));
}










/*
 *                          Klasse: Day
 * */


inline void Day::init(const Zeit::OTime* ptr) {
	if (! ptr) {
		Zeit::OTime jetzt;
		ptr = &jetzt;
	}
	tz    = ptr->tt();
	monat = ptr->getMonth();
}


/* private, nicht sichtbare Funktion! *
 * size(buf) muß >= 3 !!!             */
void myparse(int* buf, istream& parse, const Zeit::OTime* ot = nullptr) {
	int bufpos = -1; // nichts geschrieben...

	while (parse) {
		static int i;
		parse >> i;
		if (parse.fail()) break;
		buf[++bufpos] = abs(i); // allf. Vorzeichen ignorieren...
		parse.ignore(); // Trennzeichen...
	}

	Zeit::OTime jetzt;
	if (bufpos < 2) { // Datum noch unvollständig...
		if (! ot) {
			ot = &jetzt;
		}
		buf[2] = ot->jj();
		if (bufpos < 1) buf[1] = ot->mm();
		if (bufpos < 0) buf[0] = ot->tt();
	}
	/* Datum validieren: */
	bool fehler = false;
	if ((buf[1] > 12) || (buf[1] < 1) || (buf[0] < 1))
		{ fehler = true; }
	else {
		switch (buf[1]) {
		case 2:
		{
			int maxday = Zeit::istSchaltjahr(buf[2]) ? 29 : 28;
			if (buf[0] > maxday) { fehler = true; }
			break;
		}
		case 4:
		case 6:
		case 9:
		case 11:
			if (buf[0] > 30) { fehler = true; }
			break;
		default:
			if (buf[0] > 31) { fehler = true; }
		}
	}
	if (fehler) {
		ostringstream os;
		os << "Ungültiges Datum: " << buf[0] << "." << buf[1] << "." << buf[2];
		throw OException::IllegalArgumentException(os.str());
	}
}


inline void Day::parse(istream& parse, const Zeit::OTime* ot = nullptr) {
	int buf[3] {0};
	myparse(buf, parse, ot);
	tz   = buf[0];
	monat(buf[1], buf[2]);
}


Day parseDay(std::istream& parse, const Zeit::OTime* ot) {
	int buf[3] {0};
	myparse(buf, parse, ot);
	Day ret(buf[0], buf[1], buf[2]);
	return ret;
}



//Day::Day() : Day(Zeit::now()) {}
Day::Day() { /* Alle Members sind bereits definiert */ }


Day::Day(const std::string& param, const Zeit::OTime* ot) {
	if (param.empty()) {
		init(ot);
	} else {
		std::istringstream is(param);
		parse(is, ot);
	}
}


Day::Day(const Zeit::OTime& t) : Day(t.tt(), t.getMonth()) {}


Day::Day(size_t t, size_t m, size_t j) : monat(m, j), tz(t) {}


Day::Day(size_t tt, const Month& m) : monat(m), tz(tt) {}


OTime Day::getTime() const {
	return OTime(tz, monat.mm(), monat.year().jj());
}


string Day::toString() const {
	char buf[11];
	sprintf(buf, "%02d.%02d.%04d", tz, monat.mm(), monat.year().jj());
	return buf;
}



std::string Day::toSQL() const {
	char buf[11];
	sprintf(buf, "%4d-%02d-%02d", monat.year().jj(), monat.mm(), static_cast<int>(tz));
	return buf;
}

Day Day::deltaDays(int i) {
	return getTime().deltaDays(i);
}



Day::operator bool() const {
	return monat && (tz > 0) && (tz <= monat.days());
}



ostream& operator<<(ostream& os, const Day& td) {
	return os << td.toString();
}


std::istream& operator>>(std::istream& is, Day& td) {
	td.parse(is, nullptr);
	return is;
}




SQLDay::SQLDay(std::string sql) {
	setValues(sql);
}


void SQLDay::setValues(const std::string& sql) {
	int y = stoi(sql.substr(0,4));
	int m = stoi(sql.substr(5,2));
	int d = stoi(sql.substr(8,2));
	Day::setValues(d,m,y);
}





/*
 *                          Klasse: OTime
 * */

inline void OTime::init(long int epoch) {
	myTime = static_cast<time_t>(epoch);
	localtime_r(&myTime, &localTime);
}

OTime::OTime() {
	init(time(0));

}

OTime::OTime(const char* timestr, const char* format) {
	// Siehe dazu man strptime...
	strptime(timestr, format, &localTime);
	myTime = mktime(&localTime);
}

OTime::OTime(int tag, int monat, int jahr) {
	localTime.tm_sec = localTime.tm_min = localTime.tm_hour = 0;
	localTime.tm_mday = tag;
	localTime.tm_mon = monat - 1;
	localTime.tm_year = jahr - 1900;
	localTime.tm_isdst = 0; // Sommerzeit ausschalten
	myTime = mktime(&localTime);
}

OTime::OTime(long int epoch) {
	init(epoch);
}


string OTime::formatiere(const char* format, size_t capacity) const {
	// Wenn capacity zu klein gewählt ist, dann ist
	// der Output undefiniert!
	char buf[++capacity]; // Endzeichen hinzurechnen!
	OTime::formatiere(buf, capacity, format);
	return buf;
}

size_t OTime::formatiere(char* chbuf, size_t bufsize, const char* format) const {
    return strftime(chbuf, bufsize, format, &localTime); }


struct tm OTime::localtime() const { return localTime; }


Day OTime::deltaDays(int d) const {
	OTime t(epoch() + (d * 86400));
	return Day(t);
}

ostream& operator<<(ostream& os, OTime& t) {
	return os << t.now19();
}

} // namespace Zeit
