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

bool fileExists(const char* filename) {
	Filestat fs(filename);
	return fs.fileExists();
}





/*
 *
 *                         P A T H :
 *
 * */

bool _haslrwsp(const std::string& str) {
	if (str.empty()) return false;
	if (isspace(str.front()) || isspace(str.back())) return true;
	return false;
}


void Path::parseFirstArg(const std::string& pathname) {
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


void Path::parseBasename(const std::string& basename) {
	if (basename.empty()) throw InvalidPathname("Leerstring als Basename");
	/* basename darf keinen Slash enthalten: */
	if (basename.find_first_of('/') != string::npos)
		throw InvalidPathname("Ungültiger Basename '" + basename + "'");
	if (_haslrwsp(basename))
		throw InvalidPathname("Pfadname mit unerlaubten Leerzeichen");
	push_back(basename);
}


Path::Path(const std::string& pathname) {
	parseFirstArg(pathname);
}


Path::Path(const std::string& dirname, const std::string& basename) {
	parseFirstArg(dirname);
	parseBasename(basename);
}


Path::Path(const Path& parent, const std::string& basename) {
	insert(cbegin(), parent.cbegin(), parent.cend());
	isAbs = parent.isAbs;
	parseBasename(basename);
}


Path::Path(Path::const_iterator first, Path::const_iterator last, bool isAbsolute) :
	std::vector<std::string>(first, last), isAbs(isAbsolute) {}

std::ostream& Path::print(std::ostream& os) const {
	if (isAbsolute()) os.put('/');
	int i = 0;
	for (auto it = begin(); it != end(); it++) {
		if (i++ > 0) os.put('/');
		os << *it;
	}
	return os;
}

std::string Path::toString() const {
	if (! pathname_cached.empty()) return pathname_cached;
	std::stringstream buf;
	print(buf);
	pathname_cached = buf.str();
	return toString();
}

Path Path::getParent() const {
	if (isRoot()) return Path("/");
	Path ret(cbegin(), (cend()-1), isAbsolute());
	return ret;
}


std::string Path::getBasename() const {
	if (isRoot()) return std::string();
	return back();
}


std::ostream& operator<<(std::ostream& os, const Path& p) {
	p.print(os);
	return os;
}


/*
 *                           Klasse: Filestat
 * */

Filestat::Filestat(const std::string& path, bool followLinks) {
	// stat/lstat läuft bereits hier bei der Initialisierung!
	statresult = followLinks ? stat(path.c_str(), &statBuf) : lstat(path.c_str(), &statBuf);
	if ((statresult == -1) && (errno != ENOENT)) {
		/* ENOENT (=existiert nicht) ist kein Fehler!
		 * alle anderen schon: */
		throw CannotRead(CannotRead::notRead(path));
	}
}


long int Filestat::lastModifiedSecond() const {
	if (!fileExists()) return -1;
	return statBuf.st_mtim.tv_sec;
}


long int Filestat::lastModifiedNanoSecond() const {
	if (!fileExists()) return -1;
	return statBuf.st_mtim.tv_nsec;
}




/*
 *                           Klasse: File
 * */


File::File(const Path& p) : path(p) {};

File::File(const std::string& dirname, const std::string& basename) :
	path(dirname, basename) {}

File::File (const Path& parent, const std::string& basename) :
	path(parent, basename) {}



Filestat File::stat() const {
	return Filestat(path.toString().c_str());
}




Ordner File::getParent() const {
	Ordner ret(getPath().getParent());
	return ret;
}




Ordner File::mkParent(mode_t mode) const {
	Ordner parent = getParent();
	mkpath(parent.getPath().toString().c_str(), mode);
	return parent;
}





/*
 *                           Exceptions:
 * */


OFileException::OFileException(const std::string& what) : Fehler(what) {}

FileAccessException::FileAccessException(const std::string& what) : OFileException(what) {}

FileNotFound::FileNotFound(const std::string& what) : FileAccessException(what) {}


std::string FileNotFound::notFound(const std::string& path) {
		return "Datei '" + path + "' nicht gefunden.";
	}


CannotOpen::CannotOpen(const std::string& what) : FileAccessException(what) {}


std::string CannotOpen::notOpened(const std::string& path) {
		return "Kann Datei '" + path + "' nicht öffnen.";
	}


CannotRead::CannotRead(const std::string& what) : FileAccessException(what) {}


std::string CannotRead::notRead(const std::string& path) {
	return "Kann Datei '" + path + "' nicht lesen.";
}



NotaDirectory::NotaDirectory(const std::string& what) : FileAccessException(what) {}

std::string NotaDirectory::notaDir(const std::string& path) {
	return "Datei '" + path + "' ist kein Verzeichnis.";
}


InvalidPathname::InvalidPathname(const std::string& what) : OFileException(what) {}






/*
 *                           Klasse: Ordner
 * */


DirEntry::DirEntry(const Path& parent, dirent* e) :
		File(parent, e->d_name), entry(*e) {
}


bool DirEntry::isSubFile() const {
	std::string basename = getPath().getBasename();
	int sz = basename.size();
	if (sz > 2) return true;
	if (basename.at(0) == '.') {
		if (sz == 1) return false; // "."
		if (basename.at(1) == '.') return false; // ".."
	}
	/* filename besteht aus 2 Zeichen, wobei eines davon kein Punkt ist */
	return true;
}


//std::string DirEntry::getPath() {
//	if (! pathname.empty()) return pathname;
//	if (dirname.empty()) {
//		pathname.assign(basename);
//	} else {
//		pathname.assign(dirname + "/" + basename);
//	}
//	return getPath();
//}


DirVisitor::DirVisitor(const Path& p) : dirPath(p) {}

DirVisitor::DirVisitor(const std::string& p) : dirPath(p) {}


DirVisitor::~DirVisitor() {
	if (subdirs == nullptr) return;
	for (auto it = subdirs->begin(); it != subdirs->end(); it++) {
		delete *it;
	}
	delete subdirs;
}

bool DirVisitor::continueVisit(fileVisitResult res) {
	/* Soll der Besuch fortgesetzt oder abgebrochen werden? */
	return (res != TERMINATE);
}

/* Kümmert sich um die Zerstörung: */
DirVisitor* DirVisitor::newChild(const DirEntry& de) {
	if (subdirs == nullptr) subdirs = new std::vector<DirVisitor*>;
	DirVisitor* subdir = createNew(getSubPath(de));
	subdirs->push_back(subdir);
	return subdir;
}


Path DirVisitor::getSubPath(const DirEntry& de) {
	return Path(dirPath, de.getPath().getBasename());
}


int DirVisitor::execute(int depth) {
	if (depth < 0) depth = 0;
	std::string dirname = dirPath.toString();
	dstream = opendir(dirname.c_str());
	if (dstream == NULL) {
		switch (errno) {
		case ENOENT:
			throw FileNotFound(FileNotFound::notFound(dirname));
		case ENOTDIR:
			throw NotaDirectory(NotaDirectory::notaDir(dirname));
		case EACCES:
			throw CannotRead(CannotRead::notRead(dirname));
		default:
			throw OFileException("Fehler in 'int DirVisitor::execute(int depth)'.");
			/* noch mehr errno unter 'man opendir(3)' */
		}
	}
	beforeStart();
	static int count = 0;
	for (;;) {
		errno = 0;
		entry = ::readdir(dstream);
		if (entry == nullptr) {
			if (errno == 0) break; // end of stream
			throw OFile::CannotRead("readdir-Lesefehler");
		}
		DirEntry de(dirPath, entry);
		if (! de.isSubFile()) continue; /* '.' und '..' aussortieren...  */
		count++;
		fileVisitResult res = entryFound(de);
		if (de.isDirectory() && (depth != 1)) { /* Rekursiv werden? */
			int newDepth = (depth == 0) ? 0 : (depth - 1); /* 0 bleibt 0! */
			DirVisitor* dv = newChild(de);
			dv->execute(newDepth);
		}
		if (! continueVisit(res)) break;
	}
	closedir(dstream);
	return count;
}



inline bool readdir(const char* path, fileVisitResult (*visitFile)(const char* path, struct dirent* entry)) {
    // siehe man readdir(3)
    DIR *dstream;
    struct dirent *entry;
    int plen = strlen(path);
    dstream = opendir(path);
    fileVisitResult fvResult;
    bool terminated = false;
    for (;;) {
        if (! (entry = ::readdir(dstream))) break;
        char *name = entry->d_name;
        if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0)) continue;
        // Path des neuen entry ermitteln:
        int nlen = strlen(name);
        char subpath[plen + nlen + 2]; // Endzeichen und '/' dazurechnen!
        strncpy(subpath, path, plen+1);
        subpath[plen] = '/';
        strncpy(&subpath[plen+1], name, nlen+1);
        // fvResult ermitteln:
        fvResult = visitFile(subpath, entry);
        terminated = (fvResult == TERMINATE);
        if (fvResult == SKIP_SUBTREE) continue;
        if (terminated) break;
        // Rekursiv: readdir mit dem ermittelten subpath:
        if (entry->d_type == DT_DIR)
            terminated = readdir(subpath, visitFile);
        if (terminated || (fvResult == SKIP_SIBLINGS)) break;
    }
    closedir(dstream);
    return terminated;
}



void walkFileTree(const char* path, fileVisitResult (*visitFile)(const char* path, struct dirent* entry)) {
    readdir(path, visitFile);
}


bool mkpath( std::string path, mode_t mode ) {
    bool bSuccess = false;
    int nRC = ::mkdir( path.c_str(), mode );
    if( nRC == -1 ) {
        switch( errno ) {
            case ENOENT:
                //parent didn't exist, try to create it
                if( mkpath( path.substr(0, path.find_last_of('/')) ) )
                    //Now, try to create again.
                    bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
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











/*
 *                           Klasse: Datei
 * */


ifstream* Datei::openStream(ifstream* is) const {
	is->open(getPath().toString().c_str());
	if (! is->is_open())
		throw CannotOpen(CannotOpen::notOpened(getPath().toString()));
	return is;
}


bool Datei::canRead() const {
	return (access(getPath().toString().c_str(), R_OK) == 0);
}


//int Datei::getSize() const { return stat().getSize(); }


int getFileSize(ifstream* is) {
	if (! is->is_open())
		throw CannotRead("OFile::getFileSize: Geschlossener Stream");
	streampos pos = is->tellg();
    is->seekg (0, is->end);
    int fileSize = is->tellg();
    is->seekg (pos);
    return fileSize;
}


//FileReader Datei::getReader() const {
//	return FileReader(*this);
//}



/*
 *                     FileHandle:
 *
 * */



//locale FileHandle::imbueAtUtf() { return getStream()->imbue(locale("de_AT.UTF-8")); }
//
//
//inline void FileReader::aufraeumen() {
//	try {
//		if (stream->is_open()) stream->close();
//	} catch (...) {}
//	delete stream;
//}
//
//
//FileReader::FileReader(Datei d) : FileHandle(d) {
//	stream = new ifstream;
//	stream->open(getDatei().getPath().toString().c_str());
//	if (! stream->is_open()) {
//		aufraeumen();
//		throw CannotOpen(CannotOpen::notOpened(getDatei().getPath().toString()));
//	}
//}
//
//FileReader::~FileReader() { aufraeumen(); }













} // Ende namespace OFile
