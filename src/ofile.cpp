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
//    if (std::filesystem::exists(path)) {
//        std::stringstream buf;
//        buf << "Datei '" << path << "' existiert schon";
//        throw OFile::CannotCreate(buf.str());
//    }
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
    if (std::filesystem::exists(path)) {
        std::stringstream buf;
        buf << "Datei '" << path << "' existiert schon";
        throw OFile::CannotCreate(buf.str());
    }
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
        throw OException::CommandFailed("accept", errno);
    return fd;
}


UDSocketServer::~UDSocketServer()
{
    if (! fs::remove(sock_path)) {
        std::cerr << "Socketdatei '" << sock_path << "' konnte nicht gelöscht werden";
    }
}


UDSocketClient::UDSocketClient(const std::string& path) : UDSocket(path) {}



FDesc UDSocketClient::connect()
{
    if (::connect(init_fd, (struct sockaddr*)&address, addr_size) == -1)
        throw OException::CommandFailed("connect", errno);
    return init_fd;
}


//std::string read_socket(OFile::FDesc fd) {
//    static constexpr size_t bufsize = 0xfff;
//    static char buf[bufsize+1];
//    std::stringstream sstr;
//    while (read(fd, buf, bufsize) > 0) sstr << buf;
//    return sstr.str();
//}

std::string read_socket(FDesc fd) {
    static constexpr size_t bufsize = 0xffe;
    static char buf[bufsize+1]; // 0xfff
    long len = 0;
    /* Header (= Länge der Nachricht) einlesen: */
    if (::read(fd, &len, sizeof(long)) == -1)
        throw OException::CommandFailed("read [long]", errno);
    std::stringstream sstr;
    for (;;) {
        long l = len;
        if (l > bufsize) l = bufsize;
        if (::read(fd, buf, l) == -1)
            throw OException::CommandFailed("read [str]", errno);
        buf[l] = '\0';
        sstr << buf;
        len -= bufsize;
        if (len <= 0) break;
    }
    return sstr.str();
}


void write_socket(FDesc fd, const std::string& text) {
    long len = static_cast<long>(text.length());
    /* Header (= Länge der Nachricht) schreiben: */
    if (::write(fd, &len, sizeof(long)) == -1 )
        throw OException::CommandFailed("write [long]", errno);
    if (::write(fd, text.data(), text.length()) != static_cast<ssize_t>(text.length()))
        throw OException::CommandFailed("write [string]", errno);
}


/*----------------------------/ LogFile: /-------------------------*/


LogFile::LogFile(const std::string& path)
{
    progname = std::filesystem::path(getenv("_")).filename().string();
    stream.open(path, ios_base::out | ios_base::app);
    if (! stream)
        throw OFile::CannotOpen("Konnte Logdatei nicht öffnen");
    /* das Facet-Objekt, auf das der Zeiger 'new std::numpunct<char>' zeigt,
       wird durch den Destructor von std::lokale ordnungsgemäß zerstört.
       Die Ausnahmebestimmung im zweiten Argument bewirkt, daß bei den
       PIDs keine Tausenderpunkte angezeigt werden:                       */
    stream.imbue(std::locale(std::locale(""), new std::numpunct<char>));
}


void LogFile::printMessage(const std::string& message)
{
    Zeit::OTime tm;
    tm.formatiere("%b %d %H:%M:%S ", stream);
    stream << progname << " [" << getpid() << "] " << message << std::endl;
}


LogFile::~LogFile()
{
    printMessage("Schließe Log-Stream...");
    stream.close();
}



/*-------------------------/ File-Locking: /------------------------*/


bool lock_file(OFile::FDesc fd, Lock::Type lock_type, bool nonblock, FileRegion reg)
{
    /*
        lock_type = F_RDLCK | F_WRLCK | F_UNLCK
        File muß entsprechend der lock_type zum Lesen oder Schreiben geöffnet sein!
        Siehe dazu fcntl(2)
     */
    struct flock flockstr;
    flockstr.l_type     = static_cast<short>(lock_type);
    flockstr.l_whence   = reg.whence;
    flockstr.l_start    = reg.start;
    flockstr.l_len      = reg.length;

    int cmd = (nonblock) ? F_SETLK : F_SETLKW;
    if (fcntl(fd, cmd, &flockstr) == 0) return true; /* SUCCESS! */
    switch (errno) {
    case EACCES:
    case EAGAIN:
        return false; /* Aktiver Filelock bereits vorhanden */
    default: {
        std::stringstream buf;
        buf << "File-Lock mißlungen: " << strerror(errno);
        throw OFile::FileAccessException(buf.str());
    }
    }
}


/*-----------------------------------/ PidFile: /---------------------------*/


PidFile::PidFile(const std::string& path)
    : file_descriptor(open(path.c_str(), O_RDWR | O_CREAT, 0644))
{
    if (file_descriptor == -1)
        throw OException::CommandFailed("open", errno);
}


PidFile::~PidFile()
{
    close();
}


void PidFile::close()
{
    if (::ftruncate(file_descriptor, 0) == -1)
        std::cerr << "ftruncate() mißlungen!\n";
    ::close(file_descriptor);
}


bool PidFile::lock()
{
    if (! lock_file(file_descriptor, Lock::Type::write))
        return false;
    /* Aktuellen Inhalt der Datei löschen: */
    if (::ftruncate(file_descriptor,0) == -1)
        throw OException::CommandFailed("ftruncate", errno);
    std::string pid = std::to_string(getpid());
    int written = ::write(file_descriptor, pid.data(), pid.size());
    if (written != static_cast<int>(pid.size()))
        throw OException::CommandFailed("write");
    return true;
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
