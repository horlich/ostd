/*
 * ofile.h
 *
 *  Created on: 04.05.2017
 *      Author: kanzlei
 */

#ifndef OFILE_H_
#define OFILE_H_

//#include <string>
//#include <vector>
//#include <string.h>
#include <algorithm>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <fcntl.h>
//#include <dirent.h>
//#include <errno.h>
//#include <unistd.h>
//#include <fstream>
//#include <iostream>
//#include <sstream>
//#include <ios>
//#include <ctype.h>
//#include <stdexcept>
#include <ftw.h>

#include "oexception.h"
#include "ostringutil.h"
#include "oconsole.h"





namespace OFile {






/*
 *                     Path:
 *
 * */
 
 /* ACHTUNG: Seit C++17 gibt es die Filesystem Library
  * Header <filesystem>
  * https://en.cppreference.com/w/cpp/filesystem
  * So auch: std::filesystem::path !!               */

class Path : protected std::vector<std::string> {
	//
private:
	bool isAbs = false; // leading '/'?
	/* Wichtig: Bei Objektänderungen muß pathname_cached
	 * GELÖSCHT werden! */
	mutable std::string pathname_cached;
	/* throws InvalidPathname: */
	inline void parseFirstArg(const std::string& pathname);
	/* throws InvalidPathname: */
	inline void parseBasename(const std::string& basename);

public:
	/* throws InvalidPathname: */
	Path(const std::string& pathname = OConsole::getWorkingDirectory());

	/* throws InvalidPathname: */
	Path(const std::string& dirname, const std::string& basename);

	/* throws InvalidPathname: */
	Path(const Path& parent, const std::string& basename);

	Path(Path::const_iterator first, Path::const_iterator last, bool isAbsolute);

	bool isRoot() const { return (empty());	}

	bool isAbsolute() const { return isAbs; }

	std::ostream& print(std::ostream& os) const;

	std::string toString() const;

	Path getParent() const;

	std::string getBasename() const;
};


std::ostream& operator<<(std::ostream&, const Path&);








/***************************| stat |*****************************/
/*
 *  Methoden für stat
 *  Details siehe man stat(2) und man inode(7):
 *  Die mask-values S_ISDIR, S_ISREG, S_ISLINK, etc.
 *  sind in man inode(7) beschrieben.
 */








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



/*************************| Dateien |******************************/

/* Dateien-Tests für Existenz und Rechte mit faccessat() in 'man access(2)' */

/* throws CannotRead: */
int getFileSize(std::ifstream* is);




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

	/* Generiere Fehlermeldung für what(): */
	static std::string notOpened(const std::string& path);
};



class CannotRead : public FileAccessException {
public:
	CannotRead(const std::string& what);

	virtual ~CannotRead() = default;

	static std::string notRead(const std::string& path);
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
