/*
 * ofile.h
 *
 *  Created on: 04.05.2017
 *      Author: kanzlei
 */

#ifndef OFILE_H_
#define OFILE_H_

#include <string>
#include <vector>
#include <string.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ios>
#include <ctype.h>
#include <stdexcept>

#include "oexception.h"
#include "ostringutil.h"
#include "oconsole.h"





namespace OFile {






/*
 *                     Path:
 *
 * */

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
	Path(const std::string& pathname = OConsole::getWorkingDirektory());

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








/*
 *                     Filestat:
 *
 * */


class Filestat {
	struct stat statBuf;
	int statresult;
public:
	/* throws CannotRead: */
	Filestat(const std::string& path, bool followLinks = false);
	/*
	 *  Details siehe man stat(2) und man inode(7):
	 *  Die mask-values S_ISDIR, S_ISREG, S_ISLINK, etc.
	 *  sind in man inode(7) beschrieben.
	 */
	const struct stat* getStatus() const {
		return &statBuf;
	}

	inline bool fileExists() const {
		return (statresult == 0);
	}

	inline bool isDirectory() const {
		return S_ISDIR(statBuf.st_mode);
	}

	inline bool isRegularFile() const {
		return S_ISREG(statBuf.st_mode);
	}

	inline bool isSymbolicLink() const {
		return S_ISLNK(statBuf.st_mode);
	}

	// gibt -1 zurück, wenn nicht existiert:
	inline int getSize() const { return (fileExists()) ? statBuf.st_size : -1; }

	// gibt -1 zurück, wenn nicht existiert:
	long int lastModifiedSecond() const;

	// gibt -1 zurück, wenn nicht existiert:
	long int lastModifiedNanoSecond() const;
};

/* Umgeht die Notwendigkeit, erst ein Filestat
 * konstruieren zu müssen: */
bool fileExists(const char* filename);






/*
 *                     File:
 *
 * */
class Ordner; // Vorwärtsdeklaration für OFile::getParent()...
class Datei;




class File { // ABSTRAKTE KLASSE!
	Path path;

public:
	File(const Path& p);

	/* throws InvalidPathname: */
	File(const std::string& dirname, const std::string& basename);

	/* throws InvalidPathname: */
	File (const Path& parent, const std::string& basename);

	virtual ~File() = default;

	virtual Path getPath() const { return path;	}

	/* throws CannotRead: */
	virtual Filestat stat() const;

	virtual Ordner getParent() const;

	virtual Ordner mkParent(mode_t mode = 0755) const;

};











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







// Siehe java.nio.file.FileVisitResult
enum fileVisitResult { CONTINUE, SKIP_SIBLINGS, SKIP_SUBTREE, TERMINATE };

/*
 *                     Ordner:
 *
 * */

class Ordner : public File {
	//
public:
	Ordner(const Path& p) : File(p) {}

	/* throws InvalidPathname: */
	Ordner(std::string pathname) : File(pathname) {}

	virtual ~Ordner(){}
};





class DirEntry : public File {
	/* DirEntry ist ein beliebiges File das GARANTIERT existiert. */
private:
	/* Siehe dazu man readdir(3) */
	dirent entry;

public:
	/* throws InvalidPathname: */
	DirEntry(const Path& parent, dirent* e);

	virtual ~DirEntry() = default;

	bool isSubFile() const;

	bool isDirectory() const { return entry.d_type == DT_DIR; }

	bool isRegularFile() const { return entry.d_type == DT_REG; }

	bool isSymbolicLink() const { return entry.d_type == DT_LNK; }

	long int getInodeNo() const { return entry.d_ino; }
};




class DirVisitor {
	/*  Abstrakte Klasse.
	 *  Abgeleitete Klassen müssen die Methoden
	 *  - entryFound(DirEntry)
	 *  - createNew(const std::string& path)
	 *  überschreiben.  */
private:
	Path dirPath;
    DIR *dstream = nullptr;
    struct dirent *entry = nullptr;
    /* subdirs sammelt die hier erzeugten DirVisitor-Objekte
     * für den Destruktur: */
    std::vector<DirVisitor*>* subdirs = nullptr;

protected:
	/* Soll der Besuch fortgesetzt oder abgebrochen werden? */
	virtual bool continueVisit(fileVisitResult res);

	/* Wird vor execute aufgerufen: */
	virtual void beforeStart() {};

	virtual fileVisitResult entryFound(DirEntry&) = 0;

	/* Kümmert sich NICHT um die Zerstörung! */
	virtual DirVisitor* createNew(const Path& path) = 0;

	/* Kümmert sich um die Zerstörung: */
	virtual DirVisitor* newChild(const DirEntry&);

public:
	DirVisitor(const Path& p);

	/* throws InvalidPathname: */
	DirVisitor(const std::string& pathname);

	virtual ~DirVisitor();

	/* depth ist die Rekursionstiefe.
	 * depth == 1 heißt: nicht rekursiv
	 * depth == 0 heißt: unbeschränkt rekursiv
	 * depth >= 2 heißt: in die Subverzeichnisse absteigen */
	/* throws
	 * - FileNotFound
	 * - NotaDirectory
	 * - CannotRead
	 * - OFileException: */
	virtual int execute(int depth = 0);

	virtual Path getSubPath(const DirEntry& de);
};




/* readdir und walkFileTree sind veraltet! Besser mit DirVisitor!!! */
inline bool readdir(const char* path, fileVisitResult (*visitFile)(const char* path, struct dirent* entry));
void walkFileTree(const char* path, fileVisitResult (*visitFile)(const char* path, struct dirent* entry));


bool mkpath( std::string path, mode_t mode = 0775);







//class FileReader;

/*
 *                     Datei:
 *
 * */

class Datei : public File {
private:
public:
	Datei(const Path& p) : File(p){}

	Datei(std::string pathname) : File(pathname) {}

	virtual ~Datei(){}

	/* throws CannotOpen: */
	std::ifstream* openStream(std::ifstream* is) const;

	bool canRead() const;

//	FileReader getReader() const;

};

/* throws CannotRead: */
int getFileSize(std::ifstream* is);










/*
 *                     FileHandle:
 *
 * */



//class FileHandle {
//	Datei datei;
//public:
//	FileHandle(Datei d) : datei(d) {}
//
//	virtual ~FileHandle() {}
//
//	virtual std::ios* getStream() = 0;
//
//	std::locale imbueAtUtf();
//
//	inline Datei getDatei() const { return datei; }
//};
//
//
//
//
//
//class FileReader : public FileHandle {
//	//
//	std::ifstream* stream;
//
//	// Kopieren verbieten durch privat-Setzen:
//	FileReader& operator()(FileReader& fr){ return fr; }
//	FileReader& operator=(FileReader& fr) { return fr; }
//
//	inline void aufraeumen();
//
//public:
//	/* throws CannotOpen: */
//	FileReader(Datei);
//
//	~FileReader();
//
//	inline std::ifstream* getStream() { return stream; }
//
//	inline int getFileSize() { return OFile::getFileSize(stream); }
//};












} // Ende namespace OFile

#endif /* OFILE_H_ */
