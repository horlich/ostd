#include <string>
#include "otime.h"


using namespace std;


namespace Zeit {





/*
 *                   Statische Methoden:
 * */

bool setAtLocale()
{
   return (setlocale(LC_TIME, "de_AT.UTF-8") != 0);
}


bool istSchaltjahr(int jahr)
{
   return  (( (jahr % 4 == 0) && (jahr % 100 != 0) ) || (jahr % 400 == 0));
}

Day today()
{
   OTime ot;
   return ot.getDay();
}

OTime now()
{
   return OTime();
}



/*
 *                   Klasse: Year
 * */



Year::Year(size_t zahl) :jz(zahl) {}



Year& Year::operator()(size_t j)
{
   jz = j;
   return *this;
}


Year& Year::operator=(const Year& j)
{
   jz = j.jz;
   return *this;
}


Year::operator bool() const
{
   constexpr int gregkal_start = 1582; /* Einführung gregorianischer Kalender */
   return ( (jz > gregkal_start) && (jz < 2400) );
}


constexpr Year& Year::operator++()   /* Präfix-Inkrementor */
{
   ++jz;
   return *this;
}



Year Year::operator++(int)   /* Postfix-Inkrementor */
{
   Year tmp(*this);
   ++(*this);
   return tmp;
}


constexpr Year& Year::operator--()   /* Präfix-Inkrementor */
{
   --jz;
   return *this;
}


Year Year::operator--(int)   /* Postfix-Inkrementor */
{
   Year tmp(*this);
   --(*this);
   return tmp;
}





/*
 *                          Klasse: Month
 * */

Month::Month(size_t zahl, const Year& j) : mz(zahl), jahr(j) {}


Month::Month(const OTime& t) : mz(t.mm()), jahr(t.jj()) {}


Month& Month::operator()(size_t mm, const Year& j)
{
   mz = mm;
   jahr = j;
   return *this;
}


/* Validiere: */
Month::operator bool() const
{
   return ((mz > 0) && (mz < 13) && jahr);
}

Month& Month::operator++()
{
   if (++mz > 12) {
      ++jahr;
      mz = 1;
   }
   return *this;
}


Month Month::operator++(int)
{
   Month tmp = *this;
   ++(*this);
   return tmp;
}

Month& Month::operator--()
{
   if (--mz < 1) {
      --jahr;
      mz = 12;
   }
   return *this;
}


Month Month::operator--(int)
{
   Month tmp = *this;
   --(*this);
   return tmp;
}



std::string Month::name() const
{
   constexpr const char* namen[] {"Jänner", "Februar", "März", "April", "Mai", "Juni",
                                  "Juli", "August", "September", "Oktober", "November", "Dezember"
                                 };
   if (! *this) return "Ungültig";
   return namen[mz-1];
}


int Month::days() const
{
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












/*
 *                          Klasse: Day
 * */


inline void Day::init(const Zeit::OTime* ptr)
{
   if (! ptr) {
      Zeit::OTime jetzt;
      ptr = &jetzt;
   }
   tz    = ptr->tt();
   monat = ptr->getMonth();
}


/* private, nicht sichtbare Funktion! *
 * size(buf) muß >= 3 !!!             */
void myparse(int* buf, istream& parse, const Zeit::OTime* ot = nullptr)
{
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
   else if (buf[2] < 100) buf[2] += 2000;   /* bei zweistelliger Jahreseingabe */
}


inline void Day::parse(istream& parse, const Zeit::OTime* ot = nullptr)
{
   int buf[3] {0};
   myparse(buf, parse, ot);
   tz   = buf[0];
   monat(buf[1], buf[2]);
}


Day parseDay(std::istream& parse, const Zeit::OTime* ot)
{
   int buf[3] {0};
   myparse(buf, parse, ot);
   Day ret(buf[0], buf[1], buf[2]);
   return ret;
}



Day::Day() { /* Alle Members sind bereits definiert */ }


Day::Day(const std::string& param, const Zeit::OTime* ot)
{
   if (param.empty()) {
      init(ot);
   }
   else {
      std::istringstream is(param);
      parse(is, ot);
   }
}


Day::Day(const Zeit::OTime& t) : Day(t.tt(), t.getMonth()) {}


Day::Day(size_t t, size_t m, size_t j) : monat(m, j), tz(t) {}


Day::Day(size_t tt, const Month& m) : monat(m), tz(tt) {}


OTime Day::getTime() const
{
   return OTime(tz, monat.mm(), monat.year().jj());
}


Day& Day::operator++()
{
   if (static_cast<int>(++tz) > monat.days()) {
      tz = 1;
      ++monat;
   }
   return *this;
}

Day Day::operator++(int)
{
   Day tmp(*this);
   ++(*this);
   return tmp;
}

Day& Day::operator--()
{
   if (static_cast<int>(--tz) < 1) {
      --monat;
      tz = monat.days();
   }
   return *this;
}

Day Day::operator--(int)
{
   Day tmp(*this);
   --(*this);
   return tmp;
}


string Day::toString() const
{
   char buf[11];
   sprintf(buf, "%02d.%02d.%04d", static_cast<int>(tz), monat.mm(), monat.year().jj());
   return buf;
}



std::string Day::toSQL() const
{
   char buf[11];
   sprintf(buf, "%4d-%02d-%02d", monat.year().jj(), monat.mm(), static_cast<int>(tz));
   return buf;
}

Day Day::deltaDays(int i)
{
   return getTime().deltaDays(i);
}



Day::operator bool() const
{
   return monat && (tt() <= monat.days());
}



ostream& operator<<(ostream& os, const Day& td)
{
   const char forig = os.fill('0');
   for (int i : {td.tt(), td.mm()}) os << setw(2) << i << '.';
   os << setw(4) << td.jj();
   os.fill(forig);
   return os;
}


std::istream& operator>>(std::istream& is, Day& td)
{
   td.parse(is, nullptr);
   return is;
}




SQLDay::SQLDay(std::string sql)
{
   setValues(sql);
}


void SQLDay::setValues(const std::string& sql)
{
   int y = stoi(sql.substr(0,4));
   int m = stoi(sql.substr(5,2));
   int d = stoi(sql.substr(8,2));
   setValues(d,m,y);
}





/*
 *                          Klasse: OTime
 * */

inline void OTime::init(long int epoch)
{
   myTime = static_cast<time_t>(epoch);
   localtime_r(&myTime, &localTime);
}

OTime::OTime()
{
   init(time(0));

}

OTime::OTime(const char* timestr, const char* format)
{
   // Siehe dazu man strptime...
   strptime(timestr, format, &localTime);
   myTime = mktime(&localTime);
}

OTime::OTime(int tag, int monat, int jahr)
{
   localTime.tm_sec = localTime.tm_min = localTime.tm_hour = 0;
   localTime.tm_mday = tag;
   localTime.tm_mon = monat - 1;
   localTime.tm_year = jahr - 1900;
   localTime.tm_isdst = 0; // Sommerzeit ausschalten
   myTime = mktime(&localTime);
}

OTime::OTime(long int epoch)
{
   init(epoch);
}


string OTime::formatiere(const char* format, size_t capacity) const
{
   // Wenn capacity zu klein gewählt ist, dann ist
   // der Output undefiniert!
   char buf[++capacity]; // Endzeichen hinzurechnen!
   OTime::formatiere(buf, capacity, format);
   return buf;
}

size_t OTime::formatiere(char* chbuf, size_t bufsize, const char* format) const
{
   return strftime(chbuf, bufsize, format, &localTime);
}


struct tm OTime::localtime() const {
   return localTime;
}


Day OTime::deltaDays(int d) const
{
   OTime t(epoch() + (d * 86400));
   return Day(t);
}

ostream& operator<<(ostream& os, OTime& t)
{
   return os << t.now19();
}



void testAll() {
   std::cout << "Teste otime:\n";
   std::cout << "Heute ist der " << Zeit::today() << '\n';
   Zeit::Day tag1;
   std::cout << "Undefiniertes Datum: " << tag1 << '\n';
   std::string str{"2.4.12"};
   Zeit::Day tag2(str);
   std::cout << "Formatiere '" << str << "' zu " << tag2 << '\n';
   std::cout << "Monatsname ist " << tag2.getMonth().name() << ".\n";
   std::cout << "Teste Operatoren ++ und --:\n";
   Zeit::Day tag3("29.12.2008");
   constexpr int maxi{5};
   for (int i = 0; i < maxi; ++i) {
      cout << tag3++ << '\n';
   }
   for (int i = 0; i < maxi; ++i) {
      cout << tag3-- << '\n';
   }
   cout << "Ende Test otime.\n";
}


} // namespace Zeit
