/*
 * ostringutil.cpp
 *
 *  Created on: 18.05.2017
 *      Author: kanzlei
 */

#include "ostringutil.h"



using namespace std;


namespace StringUtil {


static const std::string MESS_U8_LESEFEHLER("Fehler beim Lesen des UTF8-Strings.\nMöglicherweise wurde die Locale nicht gesetzt.\nz.B. 'setlocale(LC_ALL, \"de_AT.UTF-8\")'");



string ltrim(const string& str)
{
   size_t firstPos = str.find_first_not_of(WHITESPACES);
   if (firstPos == string::npos) return string();
   return str.substr(firstPos);
}

string rtrim(const string& str)
{
   size_t lastPos = str.find_last_not_of(WHITESPACES);
   if (lastPos == string::npos) return string();
   return str.substr(0, lastPos+1);
}

string trim(const string& str)
{
   size_t firstPos = str.find_first_not_of(WHITESPACES);
   if (firstPos == string::npos) return string();
   size_t lastPos = str.find_last_not_of(WHITESPACES);
   return str.substr(firstPos, lastPos-firstPos+1);
}


int split(const std::string& str, char tz, std::vector<std::string>& vec)
{
   size_t start = 0;
   size_t end = 0;
   int count = 0;
   while (end != std::string::npos) {
      end = str.find_first_of(tz,start);
      vec.push_back(str.substr(start, (end - start)));
      count++;
      start = end+1;
   }
   return count;
}

/*
                toUpper/toLower:
*/

std::wstring toUpper(const std::wstring& ws)
{
   return convenience<wchar_t>(ws, std::toupper<wchar_t>);
}


std::wstring toLower(const std::wstring& ws)
{
   return convenience<wchar_t>(ws, std::tolower<wchar_t>);
}

std::string toUpper(const std::string& ws)
{
   return convenience<char>(ws, std::toupper<char>);
}


std::string toLower(const std::string& ws)
{
   return convenience<char>(ws, std::tolower<char>);
}




int parseArgs(const string& str, StrVec& vec)
{
   /* Trennt einen String nach Whitespaces.
    * Innerhalb von Anführungszeichen " oder ' werden auch
    * Whitespaces mitgeschrieben.
    * Schreibt die erzeugten Tokens in vec.
    * Gibt die Anzahl der erzeugten Tokens zurück */
   int i = 0;
   int tkbegin = 0;
   int tokens = 0;
   bool recording = false;
   bool end_found = false;
   char quote = 0;

   for (string::const_iterator it = str.cbegin();; it++) {
      char found = *it;
      if ( it == str.cend() ) {
         if ( recording ) end_found = true;
      }
      else if ( found == quote ) {
         end_found = true;
      }
      else if ( isspace(found) && recording ) {
         if (quote == 0) {
            end_found = true;
         }
      }
      else if ( recording == false && isspace(found) == false ) {
         // Tokenbegin gefunden...
         tkbegin = i;
         tokens++;
         recording = true;
         if ( (found == '\'') or (found == '"') ) {
            quote = found; // Beginn quoted string
            tkbegin++; // Quote nicht aufnehmen!
         }
      }
      if (end_found) {
         recording = false;
         end_found = false;
         quote = 0;
         vec.push_back(str.substr(tkbegin,(i-tkbegin)));
      }
      if (it == str.cend()) break; // sonst Endlos-Schleife!
      i++;
   }
   return tokens;
}




/*
 *             Umwandlung Multibyte-String <-> Wide-String
 * */


// siehe man mbstowcs:
static const size_t MB_CONVERT_ERRORVAL = (size_t)-1;

int getWSize(const char* str)
{
   // Anzahl an wide-chars in einem multibyte-String
   size_t ret = mbsrtowcs(nullptr, &str, 0, 0);
   if (ret == MB_CONVERT_ERRORVAL) return -1;
   return ret;
}



wstring toWstring(const char* str, size_t maxlen)
{
   wchar_t wstr[maxlen];
   if (mbstowcs(wstr, str, maxlen) == MB_CONVERT_ERRORVAL) throw CharUtil::KeinUtf8(MESS_U8_LESEFEHLER);
   return wstring(wstr);
}


wstring toWstring(string& s)
{
   return toWstring(s.data(), s.size() + 1);
}

int getMBSize(const wchar_t* wstr)
{
   // Anzahl an Bytes in einem wide string
//	maxlen++; // Endzeichen einrechnen!
//	char str[maxlen];
   size_t ret = wcstombs(nullptr, wstr, 0);
   if (ret == MB_CONVERT_ERRORVAL) return -1;
   return ret;
}


string toMBstring(const wchar_t* wstr, size_t maxlen)
{
   maxlen++; // Endzeichen einrechnen!
   char str[maxlen+1];
   if (wcstombs(str, wstr, maxlen) == MB_CONVERT_ERRORVAL) throw CharUtil::KeinUtf8(MESS_U8_LESEFEHLER);
   return string(str);
}


string toMBstring(wstring& s)
{
   return toMBstring(s.data(), (s.size()+1) * MB_CUR_MAX);
}



/*
 *                         Tokenizer:
 *
 * */

std::vector<std::wstring> tokenize_ws(const wstring& str)
{
   return tokenize_regex(str, L"\\S+");
}

std::wstring wrap (const std::wstring& str, int linewidth)
{
   std::locale loc("");
   std::wstring ret;
   std::wstring buf;
   int consumed = 0; /* Anzahl der in der aktuellen Zeile bereits enthaltenen Zeichen. */
   int newwidth = 0;
   for (wchar_t ch : str) {
      newwidth = consumed + 2 + buf.length(); /* Leerzeichen und neu geholtes Zeichen berücksichtigen! */
      if (ch == '\n') { /* Erzwungener Zeilenumbruch */
         if (newwidth > linewidth) { /* String *vor* dem erzwungenen Zeilenumbruch ist bereits zu lang */
            ret.push_back('\n');
         }
         else if (consumed > 0) { /* nicht am Zeilenanfang! */
            ret.push_back(' ');
         }
         ret.append(buf);
         ret.push_back('\n');
         buf.clear();
         consumed = 0;
      }
      else if (std::isspace(ch, loc)) {
         if (buf.empty()) {
            /* nichts, weil unnötiges Leerzeichen... */
         }
         else if (newwidth <= linewidth) { /* buf paßt noch in die Zeile */
            if (consumed > 0) ret.push_back(' '); /* nicht am Zeilenanfang! */
            ret.append(buf);
            consumed += (buf.length() + 1);
            buf.clear();
         }
         else { /* buf paßt nicht mehr in die Zeile -> neue Zeile beginnen */
            ret.push_back('\n');
            ret.append(buf);
            consumed = buf.length();
            buf.clear();
         }
      }
      else {
         buf.push_back(ch);
      }
   }
   /* Alle Zeichen eingelesen -> Buffer noch leeren: */
   if (! buf.empty()) {
      if (newwidth <= linewidth) {
         ret.push_back(' ');
      }
      else { /* buf paßt nicht mehr in die Zeile */
         ret.push_back('\n');
      }
      ret.append(buf);
   }
   return ret;
}



std::vector<std::wstring> wrap_vec (const std::wstring& str, int linewidth)
{
   std::locale loc("");
   std::vector<std::wstring> ret;
   std::wstring zeile;
   std::wstring buf;
   int newwidth = 0;
   for (wchar_t ch : str) {
      newwidth = zeile.length() + 2 + buf.length(); /* Leerzeichen und neu geholtes Zeichen berücksichtigen! */
      if (ch == '\n') { /* Erzwungener Zeilenumbruch */
         if (newwidth > linewidth) { /* String *vor* dem erzwungenen Zeilenumbruch ist bereits zu lang */
            ret.push_back(zeile);
            zeile.clear();
         }
         else if (zeile.size() > 0) { /* nicht am Zeilenanfang! */
            zeile.push_back(' ');
         }
         zeile.append(buf);
         buf.clear();
         ret.push_back(zeile);
         zeile.clear();
      }
      else if (std::isspace(ch, loc)) {
         if (buf.empty()) {
            /* nichts, weil unnötiges Leerzeichen... */
         }
         else if (newwidth <= linewidth) { /* buf paßt noch in die Zeile */
            if (zeile.length() > 0) zeile.push_back(' '); /* nicht am Zeilenanfang! */
            zeile.append(buf);
            buf.clear();
         }
         else { /* buf paßt nicht mehr in die Zeile -> neue Zeile beginnen */
            ret.push_back(zeile);
            zeile = buf;
            buf.clear();
         }
      }
      else {
         buf.push_back(ch);
      }
   }
   /* Alle Zeichen eingelesen -> Zeile und Buffer noch verarbeiten: */
   if (! buf.empty()) {
      if (newwidth <= linewidth) {
         zeile.push_back(' ');
         zeile.append(buf);
      }
      else { /* buf paßt nicht mehr in die Zeile */
         ret.push_back(zeile);
         zeile = buf;
      }
   }
   if (! zeile.empty()) ret.push_back(zeile);
   return ret;
}



/*
 *                         StringVisitor:
 *
 * */




int StringVisitor::parseStream(std::wistream& is)
{
   int zeichen = 0;
   while (is) {
      wchar_t wc = is.get();
      if (is.eof()) break;
      zeichen++;
      if (visit(wc) == VisitResult::BREAK) break;
   }
   return zeichen;
}


int StringVisitor::parseString(std::string str)
{
   WC_Converter conv;
   std::wstringstream is;
   is << conv.from_bytes(str);
   return parseStream(is);
}





} // Ende Namespace StringUtil
