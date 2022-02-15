/*
 * ofile.cpp
 *
 *  Created on: 04.05.2017
 *      Author: kanzlei
 */

//#include <iostream>
#include "ofile.h"


using namespace std;


namespace OFile {

/*
 *
 *                       Statische Methoden:
 *
 * */




/*
 *
 *                         P A T H :
 *
 * */

bool _haslrwsp(const std::string& str)
{
   if (str.empty()) return false;
   if (isspace(str.front()) || isspace(str.back())) return true;
   return false;
}


void Path::parseFirstArg(const std::string& pathname)
{
   if (pathname.empty()) throw InvalidPathname("Leerstring als Pfadname");
   if ((pathname.size() == 1) && (pathname.at(0) == '/')) return;
   std::vector<std::string> vec;
   StringUtil::split(pathname, '/', vec);
   if (any_of(vec.begin(), vec.end(), _haslrwsp))
      throw InvalidPathname("Unerlaubtes Leerzeichen in Pfadnamen.");
   if (vec.at(0).empty()) isAbs = true; /* Absoluter Pfadname */
   for (auto it = vec.begin(); it != vec.end(); it++) {
      /* Doppelte Slashes und End-Slashes werden ignoriert. */
      if (! it->empty()) push_back(*it);
   }
}


void Path::parseBasename(const std::string& basename)
{
   if (basename.empty()) throw InvalidPathname("Leerstring als Basename");
   /* basename darf keinen Slash enthalten: */
   if (basename.find_first_of('/') != string::npos)
      throw InvalidPathname("Ungültiger Basename '" + basename + "'");
   if (_haslrwsp(basename))
      throw InvalidPathname("Pfadname mit unerlaubten Leerzeichen");
   push_back(basename);
}


Path::Path(const std::string& pathname)
{
   parseFirstArg(pathname);
}


Path::Path(const std::string& dirname, const std::string& basename)
{
   parseFirstArg(dirname);
   parseBasename(basename);
}


Path::Path(const Path& parent, const std::string& basename)
{
   insert(cbegin(), parent.cbegin(), parent.cend());
   isAbs = parent.isAbs;
   parseBasename(basename);
}


Path::Path(Path::const_iterator first, Path::const_iterator last, bool isAbsolute) :
   std::vector<std::string>(first, last), isAbs(isAbsolute) {}

std::ostream& Path::print(std::ostream& os) const
{
   if (isAbsolute()) os.put('/');
   int i = 0;
   for (auto it = begin(); it != end(); it++) {
      if (i++ > 0) os.put('/');
      os << *it;
   }
   return os;
}

std::string Path::toString() const
{
   if (! pathname_cached.empty()) return pathname_cached;
   std::stringstream buf;
   print(buf);
   pathname_cached = buf.str();
   return toString();
}

Path Path::getParent() const
{
   if (isRoot()) return Path("/");
   Path ret(cbegin(), (cend()-1), isAbsolute());
   return ret;
}


std::string Path::getBasename() const
{
   if (isRoot()) return std::string();
   return back();
}


std::ostream& operator<<(std::ostream& os, const Path& p)
{
   p.print(os);
   return os;
}



/*-----------------------/ GetSize: /------------------------*/



using std::filesystem::directory_entry;
using std::filesystem::recursive_directory_iterator;

GetSize::GetSize(const std::string& dir)
{
   for (const directory_entry& entry : recursive_directory_iterator(dir)) {
      if ((! entry.is_regular_file()) || (entry.is_symlink())) continue;
      sum += entry.file_size();
   }
}


std::ostream& operator<<(std::ostream& os, const GetSize& sz)
{
   long long sum = sz.size();
   if (sum < 1e3) {
      os << sum << " Bytes";
      return os;
   }
   auto oldf = os.setf(std::ios::fixed, std::ios::floatfield);
   auto oldp = os.precision(1);
   double quotient;
   if (sum < 1e6) {
      quotient = sum / 1.0e3;
      os << quotient << " K";
   }
   else if (sum < 1e9) {
      quotient = sum / 1.0e6;
      os << quotient << " M";
   }
   else {
      quotient = sum / 1.0e9;
      os << quotient << " G";
   }
   os.setf(oldf);
   os.precision(oldp);
   return os;
}





/*
 *                           Exceptions:
 * */


OFileException::OFileException(const std::string& what) : Fehler(what) {}

FileAccessException::FileAccessException(const std::string& what) : OFileException(what) {}

FileNotFound::FileNotFound(const std::string& what) : FileAccessException(what) {}


std::string FileNotFound::notFound(const std::string& path)
{
   return "Datei '" + path + "' nicht gefunden.";
}


CannotOpen::CannotOpen(const std::string& what) : FileAccessException(what) {}


std::string CannotOpen::notOpened(const std::string& path)
{
   return "Kann Datei '" + path + "' nicht öffnen.";
}


CannotRead::CannotRead(const std::string& what) : FileAccessException(what) {}


std::string CannotRead::notRead(const std::string& path)
{
   return "Kann Datei '" + path + "' nicht lesen.";
}



NotaDirectory::NotaDirectory(const std::string& what) : FileAccessException(what) {}

std::string NotaDirectory::notaDir(const std::string& path)
{
   return "Datei '" + path + "' ist kein Verzeichnis.";
}


InvalidPathname::InvalidPathname(const std::string& what) : OFileException(what) {}






/*
                     FTW_Demo:
*/

int FTW_Demo::visit(const char *fpath, const struct stat *sb,
                    int typeflag, struct FTW *ftwbuf)
{
   /* ftwbuf->base zeigt in pathname den Beginn des basename an,
      ftwbuf->level die Rekursionstiefe.
      Zu stat siehe 'man stat(2)'.                             */
   constexpr int indentSteps{3}; /* Einrückungstiefe */
   const int indent = indentSteps * ftwbuf->level;
   char indentstr[indent + 1];
   for (int i = 0; i < indent; ++i) indentstr[i] = ' ';
   indentstr[indent] = '\0'; /* Indent-String fertig erzeugt... */
   switch (typeflag) {
   case FTW_F: { /* Datei gefunden */
      /* Basename der Datei ausgeben: */
      cout << indentstr << (fpath + ftwbuf->base) << " [";
      /* stat benutzen (Zeit der letzen Änderung): */
      Zeit::OTime(sb->st_mtim.tv_sec).formatiere(Zeit::OTime::FMT_NOW_19);
      cout << "]\n";
      break;
   }
   case FTW_D:
   case FTW_DNR: /* Ordner gefunden */
      cout << indentstr << fpath << '\n';
      break;
   case FTW_SL:
      cout << "Link\n";
      break;
   default:
      cout << "Unbekannt: " << fpath << '\n';
   }
   return FTW_CONTINUE;
}

int FTW_Demo::walk(const char* path) const
{
   return ::nftw(path, visit, 10, FTW_ACTIONRETVAL);
}





bool mkpath( const std::string& path, mode_t mode )
{
   bool bSuccess = false;
   int nRC = ::mkdir( path.c_str(), mode );
   if( nRC == -1 ) {
      switch( errno ) {
      case ENOENT:
         //parent didn't exist, try to create it
         if( mkpath( path.substr(0, path.find_last_of('/')) ) )
            //Now, try to create again.
            bSuccess = (0 == ::mkdir( path.c_str(), 0775 ));
         else
            bSuccess = false;
         break;
      case EEXIST:
         //Done!
         bSuccess = true;
         break;
      default:
         bSuccess = false;
         break;
      }
   }
   else
      bSuccess = true;
   return bSuccess;
}







int getFileSize(ifstream* is)
{
   if (! is->is_open())
      throw CannotRead("OFile::getFileSize: Geschlossener Stream");
   streampos pos = is->tellg();
   is->seekg (0, is->end);
   int fileSize = is->tellg();
   is->seekg (pos);
   return fileSize;
}














} // Ende namespace OFile
