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



/*-----------------------/ GetSize: /------------------------*/



namespace fs = std::filesystem;

GetSize::GetSize(const std::string& dir)
{
   fs::path p(dir);
   if (! fs::exists(dir))
      throw FileNotFound(FileNotFound::notFound(dir));
   if (! fs::is_directory(p))
      throw NotaDirectory(NotaDirectory::notaDir(dir));
   for (const fs::directory_entry& entry : fs::recursive_directory_iterator(p)) {
      if ((! entry.is_regular_file()) || (entry.is_symlink())) continue;
      sum += entry.file_size();
   }
}


std::ostream& operator<<(std::ostream& os, const GetSize& sz)
{
   /* TODO: System::humanReadableBytes() implementieren! */
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



/*------------------------/ Unix Domain Socket /---------------------------------*/


    UDSocket::UDSocket(const std::string& path) : sock_path(path)
    {
        if (std::filesystem::exists(path)) {
            std::stringstream buf;
            buf << "Datei '" << path << "' existiert schon";
            throw OFile::CannotCreate(buf.str());
        }
        init_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (init_fd == -1)
            throw OException::CommandFailed("socket");
        size_t maxlen = sizeof(address.sun_path) - 1;
        if (path.size() > maxlen) {
            std::stringstream buf;
            buf << "Adresse länger als maxlen=" << maxlen;
            throw OException::IllegalArgumentException(buf.str());
        }
        memset(&address, 0, addr_size); /* address mit Nullen überschreiben */
        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, path.data(), maxlen);
    }



    UDSocketServer::UDSocketServer(const std::string& path) : UDSocket(path)
    {
        if (bind(init_fd, (struct sockaddr*)&address, addr_size) == -1)
            throw OException::CommandFailed("bind");
        static constexpr int backlog = 5;
        if (listen(init_fd, backlog) == -1)
            throw OException::CommandFailed("listen");
    }


    FDesc UDSocketServer::accept()
    {
        FDesc fd = ::accept(init_fd, nullptr, nullptr);
        if (fd == -1)
            throw OException::CommandFailed("accept");
        return fd;
    }


    UDSocketServer::~UDSocketServer() {
        if (! fs::remove(sock_path)) {
            std::cerr << "Socketdatei '" << sock_path << "' konnte nicht gelöscht werden";
        }
    }


    UDSocketClient::UDSocketClient(const std::string& path) : UDSocket(path) {}



    FDesc UDSocketClient::connect() {
        if (::connect(init_fd, (struct sockaddr*)&address, addr_size) == -1)
            throw OException::CommandFailed("connect");
        return init_fd;
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


CannotCreate::CannotCreate(const std::string& what) : FileAccessException(what) {}


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
















} // Ende namespace OFile
