/*
 * omemory.h
 *
 *  Created on: 19.06.2017
 *      Author: kanzlei
 */

#ifndef OMEMORY_H_
#define OMEMORY_H_

#include <set>
#include <queue>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#include "oexception.h"
#include "ofile.h"
#include "debug.h"


namespace Memory {




// --------/  StackPointer:  /--------------------------

/* Für Variablen, die im Stack gespeichert sind. *
 * Werden automatisch zerstört.                  */
template <class T>
class StackPointer { /* OHNE DELETE */
private:
    std::string name;
    T* pointer = nullptr; /* Kümmert sich NICHT um die Zerstörung des Pointers!  */

public:
    StackPointer(const std::string& pname, T* pt = nullptr) : name(pname), pointer(pt) {}

    virtual ~StackPointer() = default;

    bool hasValidPointer() const
    {
        return pointer != nullptr;
    }

    std::string getPointerName() const
    {
        return name;
    }

    virtual void setPointer(T* p)
    {
        pointer = p;
    }

    virtual void deletePointer()
    {
        setPointer(nullptr);
    }

    T* getPointer() const
    {
        return pointer;
    }

    /* Wirft NullPointerException: */
    T* getValidPointer() const;

    /* Wirft NullPointerException: */
    T& getReference() const
    {
        return  *(getValidPointer());
    }
};



// --------/  HeapPointer:  /--------------------------

/* Für Variablen, die mit new erzeugt wurden.          *
 * Sind im Heap gespeichert und müssen zerstört werden */
template <class T>
class HeapPointer : public StackPointer<T> { /* MIT DELETE */
    /*
     *       HeapPointer besorgt auch die Zerstörung des Pointers.
     * */
public:
    HeapPointer(const std::string& pname, T* pt = nullptr) : StackPointer<T>(pname, pt) {}

    virtual ~HeapPointer()
    {
        deletePointer();
    }

    void deletePointer() override;

    void setPointer(T* p) override;
};





/*----------/  dynCast  /-------------------------*/

/* From muß polymorph sein, also einen virtuellen Destruktor besitzen! */
template<class From, class To>
static To* dynCast(From* v)
{
    if (v == nullptr) return nullptr;
    To* ret = dynamic_cast<To*>(v);
    if (ret == nullptr) throw OException::BadCast(__PRETTY_FUNCTION__, "dynamic_cast mißlungen.");
    return ret;
}







// --------/  PointerPool:  /--------------------------


template <class T>
class PointerPool final : private std::set<T*> {
    //
    // Die Klasse T muß die Methode resetValues()
    // implementiert haben, bei der alle Variablen auf
    // den init-Status zurückgesetzt werden.
    //
    typename std::allocator<T*>::pointer allocPointer = nullptr;
    size_t allocatedSize = 0;

    static void delPt(T* ptr)
    {
        delete ptr;
    }

    void deleteAll()
    {
        for_each (this->cbegin(), this->cend(), *(delPt));
    }

    std::queue<T*> ppool;

    int reuseCalls = 0;
    int reuseFails = 0;

public:
    PointerPool(size_t alloc = 0);

    ~PointerPool();

    void put(T* ptr);

    void reset();

    // Gebrauchte Objekte werden mit zurückgesetzten Variablen geliefert,
    // sodaß sie mit bloßem init() initialisiert werden können.
    T* get();

    void debugInfo(std::ostream& = std::cout);
};



// --------------/  Objektspeicher:  /--------------------------

template <class T>
class Objektspeicher {
    /* Speichert Objekte in Binärdateien...               */
    /* T darf keine Zeiger enthalten, weil dann nur diese */
    /* und nicht auch der Inhalt gespeichert werden! Bei  */
    /* recall() zeigen die Zeiger dann ins Leere. :-(     */
private:
    const char* filename;

public:
    Objektspeicher(const char* fn) : filename{fn} {}

    /* Wirft OFile::CannotOpen */
    void store(const T& obj) const;

    /* Wirft OFile::CannotOpen */
    T recall() const;
};



/*-----------------------/ Mapping /----------------------------*/


template<typename T>
class MappedWriter {
    //
    T* address          = nullptr;
    size_t size         = 0; /* = bytes/sizeof(T) */
    off_t offset        = 0;

protected:
    void set_address(int fd);

    /* Zu den Argumenten siehe mmap(2)
       offset_pages ist das Offset in Page-Einheiten (á 4096 bytes) */
    MappedWriter(size_t size, int offset_pages);

public:
    virtual ~MappedWriter();

    T* getAddress() const { return address; }

    T* writeArray(const T* source, size_t anzahl);

    void write(const T& obj) { *address = obj; }

}; // class MappedWriter




template<typename T>
class MappedFileWriter : public MappedWriter<T> {
    //
public:
    /* Zu den Argumenten siehe mmap(2)
       offset_pages ist das Offset in Page-Einheiten (á 4096 bytes) */
    MappedFileWriter(const std::string& path, size_t size, int offset_pages = 0);

    virtual ~MappedFileWriter() = default;

}; // class MappedFileWriter




template<typename T>
class PosixShmWriter : public MappedWriter<T> {
    /* ACHTUNG: beim Linken muß die Bibliothik librt (-lrt)
       eingebunden werden! */
    std::string name;

public:
    /* Die erzeugte Datei kann mit 'ls /dev/shm' angezeigt werden.
       ACHTUNG: Datei existiert nur im Hauptspeicher wird deshalb
       bei einem Reboot gelöscht! */
    PosixShmWriter(const std::string& name, size_t size, int offset_pages = 0);

    virtual ~PosixShmWriter() = default;

    void unlink() {
        if (shm_unlink(name.c_str()) == -1)
            throw OException::CommandFailed("shm_unlink");
    }
}; // class PosixShmWriter



template<typename T>
class MappedReader {
    //
    T* address      = nullptr;
    size_t size     = 0; /* bytes/sizeof(T) */
    off_t offset    = 0;

protected:
    MappedReader(int offset_pages = 0)
        : offset(offset_pages * sysconf(_SC_PAGESIZE)) {}

    void set_address(int fd);

public:
    virtual ~MappedReader();

    T* begin() { return address; }

    T* end() { return address + size; }
}; // class MappeReader



template<typename T>
class MappedFileReader : public MappedReader<T> {
    //
public:
    MappedFileReader(const std::string& file, int offset_pages = 0);

    virtual ~MappedFileReader() = default;
}; // class MappedFileReader



template<typename T>
class PosixShmReader : public MappedReader<T> {
public:
    PosixShmReader(const std::string& name, int offset_pages = 0);

    virtual ~PosixShmReader() = default;
}; // class PosixShmReader



/*
 *        Projekt: pwritev und preadv für Datenbank
 *
 * */




// --------/  Methoden:  /--------------------------
/* werden inkludiert durch: */
#include "omemory.tpp"


}; // Ende Namespace Memory




#endif /* OMEMORY_H_ */
