// --------/  HeapPointer:  /--------------------------

template <class T>
void HeapPointer<T>::deletePointer()
{
    delete StackPointer<T>::getPointer();
    StackPointer<T>::setPointer(nullptr);
}


template <class T>
void HeapPointer<T>::setPointer(T* p)
{
    delete StackPointer<T>::getPointer();
    StackPointer<T>::setPointer(p);
}



// --------/  StackPointer:  /--------------------------

template <class T>
T* StackPointer<T>::getValidPointer() const
{
    if (pointer == nullptr) {
        std::stringstream buf;
        buf << __PRETTY_FUNCTION__ << ": Zeiger für " << getPointerName() << " nicht definiert.";
        throw OException::NullPointerException(buf.str());
    }
    return pointer;
}



// --------/  PointerPool:  /--------------------------

template <class T>
PointerPool<T>::PointerPool(size_t alloc)
{
    if (alloc) {
        allocPointer = this->get_allocator().allocate(alloc);
        allocatedSize = alloc;
    }
}


template <class T>
PointerPool<T>::~PointerPool()
{
    this->reset();
    this->get_allocator().deallocate(allocPointer, allocatedSize);
}



template<typename T>
T* PointerPool<T>::get()
{
    reuseCalls++;
    T* ret;
    if (ppool.empty()) {
        reuseFails++;
        ret = new T;
        this->insert(ret); // Zum Zerstören vormerken...
    }
    else {
        ret = ppool.front();
        ppool.pop();
        ret->resetValues();
    }
    return ret;
}


template <class T>
void PointerPool<T>::put(T* ptr)
{
    if (! ptr) return;
    ppool.push(ptr);
}


template <class T>
void PointerPool<T>::reset()
{
    ppool = std::queue<T*>();
    this->deleteAll();
    this->clear();
}


template<class T>
void PointerPool<T>::debugInfo(std::ostream& os)
{
    int besetzt = this->size() - ppool.size();
    os << "# reuse() " << reuseCalls << " mal aufgerufen, dabei " << reuseFails << " Nullzeiger zurückgegeben.\n# "
       << this->size() << " Zeiger im Pool gespeichert, davon noch " << besetzt << " besetzt." << std::endl;
}




// --------------/  Objektspeicher:  /--------------------------

/* Wirft OFile::CannotOpen */
template <class T>
void Objektspeicher<T>::store(const T& obj) const
{
    std::ofstream os(filename, std::ios::binary);
    if (! os) throw OFile::CannotOpen(OFile::CannotOpen::notOpened(filename));
    os.write(reinterpret_cast<const char*>(&obj), sizeof(T));
    os.close();
}

/* Wirft OFile::CannotOpen */
template <class T>
T Objektspeicher<T>::recall() const
{
    std::ifstream is(filename, std::ios::binary);
    if (! is) throw OFile::CannotOpen(OFile::CannotOpen::notOpened(filename));
    T ret;
    is.read(reinterpret_cast<char*>(&ret), sizeof(T));
    is.close();
    return ret;
}



/*------------------------------/ MappedWriter: /----------------------------*/

template<typename T>
MappedWriter<T>::MappedWriter(size_t sz, int offset_pages)
    : size(sz), offset(offset_pages * sysconf(_SC_PAGESIZE))
{
    if (size <= 0)
        throw OException::IllegalArgumentException("Ungültiger Wert für size");
}


template<typename T>
void MappedWriter<T>::set_address(int fd)
{
    /* Speicherplatz zur Verfügung stellen, sonst Bus-Error: */
    off_t filesize = offset + (size*sizeof(T));
    if (ftruncate(fd, filesize) == -1)
        throw OException::CommandFailed("ftruncate");
    address = static_cast<T*>(mmap(NULL, size*sizeof(T), PROT_WRITE, MAP_SHARED, fd, offset));
    if (address == MAP_FAILED) {
        std::stringstream buf;
        buf << "Mapping (Writer) mißlungen: " << strerror(errno);
        throw OException::Fehler(buf.str());
    }
    close(fd);
}


template<typename T>
MappedWriter<T>::~MappedWriter()
{
    if (munmap(address, size*sizeof(T)) != 0)
        std::cerr << "Writer: munmap() mißlungen!";
}


template<typename T>
T* MappedWriter<T>::writeArray(const T* source, size_t anzahl)
{
    if (anzahl > size)
        throw OException::IndexOutOfBoundsException("Speicherplatz zu klein für Array");
    int bytes = anzahl * sizeof(T);
    void* v = memcpy(address, source, bytes);
    return static_cast<T*>(v);
}



/*------------------------------/ MappedFileWriter: /----------------------------*/


template <typename T>
MappedFileWriter<T>::MappedFileWriter(const std::string& path, size_t size, int offset_pages)
    : MappedWriter<T>(size, offset_pages)
{
    if (path.empty())
        throw OException::IllegalArgumentException("Leeren Pfadnamen übergeben");
    int fd = open(path.c_str(), O_RDWR  | O_CREAT, 0644);
    if (fd == -1)
        throw OFile::CannotOpen(OFile::CannotOpen::notOpened(path));
    MappedWriter<T>::set_address(fd);
}



/*------------------------------/ PosixShmWriter: /----------------------------*/


template<typename T>
PosixShmWriter<T>::PosixShmWriter(const std::string& name_, size_t size, int offset_pages)
    : MappedWriter<T>(size, offset_pages), name(name_)
{
    if ( name.empty() || (name[0] != '/') )
        throw OException::IllegalArgumentException("Name muß mit einem Slash (/) beginnen");
    int fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd == -1)
        throw OException::CommandFailed("shm_open");
    MappedWriter<T>::set_address(fd);
}



/*------------------------------/ MappedReader: /----------------------------*/


template<typename T>
void MappedReader<T>::set_address(int fd)
{
    struct stat sb;
    if (fstat(fd, &sb) == -1)
        throw OException::CommandFailed("fstat");
    int filesize = sb.st_size;
    if ((filesize % sizeof(T)) > 0)
        throw OException::Fehler("Datei enthält keinen passenden Typ");
    size = filesize / sizeof(T);
    address = static_cast<T*>(mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, offset));
    if (address == MAP_FAILED) {
        std::stringstream buf;
        buf << "Mapping (Reader) mißlungen: " << strerror(errno);
        throw OException::Fehler(buf.str());
    }
    close(fd);
}


template<typename T>
MappedReader<T>::~MappedReader()
{
    if (munmap(address, size*sizeof(T)) != 0)
        std::cerr << "Reader: munmap() mißlungen!";
}



/*------------------------------/ MappedFileReader: /----------------------------*/


template<typename T>
MappedFileReader<T>::MappedFileReader(const std::string& file, int offset_pages)
    : MappedReader<T>(offset_pages)
{
    int fd = open(file.c_str(), O_RDONLY);
    if (fd == -1) {
        std::stringstream buf;
        buf << "Datei '" << file << "' konnte nicht geöffnet werden: "
            << strerror(errno);
        throw OFile::CannotOpen(buf.str());
    }
    MappedReader<T>::set_address(fd);
}



/*------------------------------/ PosixShmReader: /----------------------------*/


template<typename T>
PosixShmReader<T>::PosixShmReader(const std::string& name, int offset_pages)
    : MappedReader<T>(offset_pages)
{
    if ( name.empty() || (name[0] != '/') )
        throw OException::IllegalArgumentException("Name muß mit einem Slash (/) beginnen");
    int fd = shm_open(name.c_str(), O_RDONLY, 0);
    if (fd == -1) {
        std::stringstream buf;
        buf << "Mapping '/dev/shm" << name << "' konnte nicht geöffnet werden: "
            << strerror(errno);
        throw OFile::CannotOpen(buf.str());
    }
    MappedReader<T>::set_address(fd);
}
