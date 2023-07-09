/*
 * ofile.h
 *
 *  Created on: 04.05.2017
 *      Author: kanzlei
 */

#ifndef OFILE_H_
#define OFILE_H_

#include <algorithm>
#include <filesystem>
#include <ftw.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include "oexception.h"
#include "ostringutil.h"
#include "oconsole.h"





namespace OFile {



using FDesc = int; /* Filedescriptor */


/***************************| stat |*****************************/
/*
 *  Methoden für stat
 *  Details siehe man stat(2) und man inode(7):
 *  Die mask-values S_ISDIR, S_ISREG, S_ISLINK, etc.
 *  sind in man inode(7) beschrieben.
 */



/*------------------/ FileRegion: /---------------------*/

struct FileRegion {
    short whence;
    off_t start, length;
    /*
       siehe struct flock in fcntl(2)
       whence = SEEK_SET | SEEK_CUR | SEEK_END
     */
    FileRegion(short whence_ = SEEK_SET, off_t start_ = 0, off_t length_ = 0)
        : whence(whence_), start(start_), length(length_) {}
}; // class FileRegion





/*
 *            FTW_Demo (File Tree Walk):
 */


class FTW_Demo { /* Siehe dazu 'man nftw' */

    /* Diese Methode wird auf jeden gefundenen Pfad angewendet: */
    static int visit(const char *fpath, const struct stat *sb,
                     int typeflag, struct FTW *ftwbuf);

public:
    /* Durchlaufe path und Subverzeichnisse: */
    int walk(const char* path) const;
};


/******************| opendir und Verzeichnisse |********************/

/* Siehe dazu 'man opendir(3)' 'man readdir(3)' */

/* Ordner-Pfad (rekursiv) erzeugen */
/* Siehe dazu 'man mkdir(2)':      */
bool mkpath(const std::string& path, mode_t mode = 0775);



/*-----------------------/ GetSize: /---------------------*/

class GetSize {
    /* Speicherplatz des Ordnerinhaltes ermitteln */
    long long sum = 0;

public:
    GetSize(const std::string& dir);

    inline long long size() const
    {
        return sum;
    }
};

std::ostream& operator<<(std::ostream& os, const GetSize& sz);




/*************************| Dateien |******************************/

/* Dateien-Tests für Existenz und Rechte mit faccessat() in 'man access(2)' */



/*----------------------------/ LogFile: /-------------------------*/


class LogFile {
    //
    std::ofstream stream;
    std::string progname;

public:
    LogFile(const std::string& path);

    void printMessage(const std::string& message);

    virtual ~LogFile();

}; // class LogFile


//LogFile& operator<<(LogFile& lf, const std::string& message);



/*-------------------------/ File-Locking: /------------------------*/

namespace Lock {

enum class Type { read = F_RDLCK, write = F_WRLCK, unlock = F_UNLCK };

} // namespace Lock

bool lock_file(OFile::FDesc fd, Lock::Type lock_type, bool nonblock = true, FileRegion reg = FileRegion());



/*-----------------------------------/ PidFile: /---------------------------*/

class PidFile {
    //
    FDesc file_descriptor;

public:
    PidFile(const std::string& path);

    ~PidFile();

    /* Wenn der Lock nicht gesetzt werden kann,
       wird false zurückgegeben: */
    bool lock();

    void close();
}; // class PidFile









/*-----------------------/ Unix Domain Socket: /----------------------------------*/

class UDSocket {
    //
protected:
    FDesc init_fd = 0;
    struct sockaddr_un address;
    static constexpr size_t addr_size = sizeof(struct sockaddr_un);
    std::string sock_path;

    UDSocket(const std::string& path);

    virtual ~UDSocket() = default;
}; // class UDSocket


class UDSocketServer : public UDSocket {
    //
public:
    UDSocketServer(const std::string& path);

    FDesc accept();

    virtual ~UDSocketServer();

}; // class UDSocketServer


class UDSocketClient : public UDSocket {
    //
public:
    UDSocketClient(const std::string& path);

    FDesc connect();

}; // class UDSocketClient


std::string read_socket(OFile::FDesc fd);


void write_socket(OFile::FDesc fd, const std::string& text);




/*
 *                     Exceptions:
 *
 * */


class OFileException : public OException::Fehler {
public:
    OFileException(const std::string& what);

    virtual ~OFileException() = default;
};


class FileAccessException : public OFileException {
public:
    FileAccessException(const std::string& what);

    virtual ~FileAccessException() = default;
};



class FileNotFound : public FileAccessException {
public:
    FileNotFound(const std::string& what);

    virtual ~FileNotFound() = default;

    /* Generiere Fehlermeldung für what(): */
    static std::string notFound(const std::string& path);
};



class CannotOpen : public FileAccessException {
public:
    CannotOpen(const std::string& what);

    virtual ~CannotOpen() = default;

    /* Genaeriere Fehlermeldung für what(): */
    static std::string notOpened(const std::string& path);
};


class CannotRead : public FileAccessException {
public:
    CannotRead(const std::string& what);

    virtual ~CannotRead() = default;

    static std::string notRead(const std::string& path);
};


class CannotCreate : public FileAccessException {
public:
    CannotCreate(const std::string& what);

    virtual ~CannotCreate() = default;
};


class NotaDirectory : public FileAccessException {
public:
    NotaDirectory(const std::string& what);

    virtual ~NotaDirectory() = default;

    static std::string notaDir(const std::string& path);
};



class InvalidPathname : public OFileException {
public:
    InvalidPathname(const std::string& what);

    virtual ~InvalidPathname() = default;
};






} // Ende namespace OFile

#endif /* OFILE_H_ */
