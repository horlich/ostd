/*
 * otime.h
 *
 *  Created on: 08.05.2017
 *      Author: kanzlei
 */

#ifndef OTIME_H_
#define OTIME_H_

//#include<sstream>

#include "oexception.h"
#include "oformat.h"
#include "debug.h"




namespace Zeit { // namespace Zeit


class OTime;


// Gibt zurück, ob die Locale-Umstellung gelungen ist:
bool setAtLocale();


bool istSchaltjahr(int jahr);




class Year {
   //
   size_t jz;

public:
   Year(size_t zahl = 0);

   inline int jj() const
   {
      return jz;
   }

   inline bool istSchaltjahr() const
   {
      return Zeit::istSchaltjahr(jz);
   }

   Year& operator()(size_t zahl);

   Year& operator=(const Year& j);

   /* Gültige Jahreszahl? */
   operator bool() const;

   constexpr Year& operator++();   /* Präfix-Inkrementor */

   Year operator++(int);   /* Postfix-Inkrementor */

   constexpr Year& operator--();   /* Präfix-Inkrementor */

   Year operator--(int);   /* Postfix-Inkrementor */
};







class Month {
   //
   size_t mz;
   Year jahr;

public:
   // validiert nicht!
   Month(size_t zahl = 0, const Year& j = Year(0));

   /* Wenn nullptr übergeben, wird aktueller Monat erzeugt: */
   Month(const OTime&);

   // validiert nicht!
   Month& operator()(size_t mz, const Year&);

   // Validiere: Gültige Monatszahl?
   operator bool() const;

   Month& operator++();

   Month operator++(int);

   Month& operator--();

   Month operator--(int);

   inline int mm() const
   {
      return mz;
   }

   inline int jj() const
   {
      return jahr.jj();
   }

   std::string name() const;

   inline const Year& year() const
   {
      return jahr;
   }

   // Anzahl Tage dieses Monats:
   int days() const;
};









class Day {
private:
   Month monat;
   size_t tz{0};

   inline void init(const Zeit::OTime*);

   inline void parse(std::istream&, const Zeit::OTime*); // validiert nicht!

protected:
   virtual void setValues(int d, int m, int j)
   {
      tz = d;
      monat = Month(m, j);
   }

public:
   Day(); /* erzeugt 00.00.0000 */

   /* wirft IllegalArgumentException, wenn Datum nicht erkannt wird: */
   Day(const std::string& parse, const Zeit::OTime* now = nullptr);

   Day(const Zeit::OTime&);

   // validiert nicht!
   Day(size_t t, size_t m, size_t j);

   // validiert nicht!
   Day(size_t tt, const Month& = Month());

   virtual ~Day() = default;

   OTime getTime() const;

   inline int tt() const
   {
      return tz;
   }

   inline int mm() const
   {
      return monat.mm();
   }

   inline int jj() const
   {
      return monat.jj();
   }

   inline const Month& getMonth() { return monat; }

   std::string toString() const;

   std::string toSQL() const;

   Day deltaDays(int i);

   /* Enthält gültigen Datumswert? */
   operator bool() const;

   Day& operator++();

   Day operator++(int);

   Day& operator--();

   Day operator--(int);

   friend std::istream& operator>>(std::istream& is, Day& td);
};

/* validiert nicht! */
Day parseDay(std::istream&, const Zeit::OTime* = nullptr);

std::ostream& operator<<(std::ostream& os, const Day& td);

Day today();





class SQLDay : public Day {
public:
//	SQLDay() : Day(0,0,0) {} // Erzeugt einen ungültigen Wert!
   SQLDay() = default;

   /* Argument muß strikt im SQL-Format
    * sein, sonst verhält sich das Programm
    * undefiniert! */
   SQLDay(std::string sql);

   virtual ~SQLDay() = default;

   using Day::setValues;
   virtual void setValues(const std::string& sql);
};



using fmt = const char*;


class OTime {
   /*****************************************************************
    * Dazu gibt es eine Boost-Bibliothek:
    * https://dieboostcppbibliotheken.de/boost.datetime-kalenderdaten
    *****************************************************************/
   time_t myTime;
   struct tm localTime;
   inline void init(long int);

public:
   /* Definierte Formatstrings für formatiere(): */
   static constexpr const char* FMT_DATE_10  = "%d.%m.%Y";
   static constexpr const char* FMT_NOW_19   = "%d.%m.%Y %H:%M:%S";
   static constexpr const char* FMT_NOW_8    = "%H:%M:%S";
   static constexpr const char* FMT_TMSTP_84 = "%Y%m%d.%H%M"; // Timestamp mit Format 8.4

   OTime();

   OTime(const char* timestr, const char* format);

   OTime(int tag, int monat, int jahr);

   OTime(long int epoch);

   long int epoch() const
   {
      return static_cast<long int>(myTime);
   }

   // Siehe dazu man localtime...
   const struct tm& localtime() const;

   inline int tt() const
   {
      return localTime.tm_mday;
   }

   inline int mm() const
   {
      return localTime.tm_mon + 1;
   }

   inline int jj() const
   {
      return localTime.tm_year + 1900;
   }

   inline Year getYear() const
   {
      return Year(jj());
   }

   inline Month getMonth() const
   {
      return Month(mm(), getYear());
   }

   inline Day getDay() const
   {
      return Day(tt(), getMonth());
   }

   Day deltaDays(int d) const;

   /* Damit richtig formatiert wird, muß mit
      os.imbue() die Locale gesetzt werden, z.B.
      std::locale mylocale("");
      os.imbue(mylocale);                        */
   std::ostream& formatiere(const char* format, std::ostream& os = std::cout) const;

   inline std::ostream& printDate(std::ostream& os = std::cout) const {
      return formatiere(FMT_DATE_10, os);
   }
};


std::ostream& operator<<(std::ostream& os, OTime& t);

OTime now();

/* Testet alle Funktionen durch: */
void testAll();


} // namespace Zeit


#endif /* OTIME_H_ */
