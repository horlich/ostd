

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
