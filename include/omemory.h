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
//#include <iostream>
//#include <fstream>
//#include <algorithm>
//#include <string>
//#include <sstream>
//#include <string_view>

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


/*
 *        Projekt: pwritev und preadv für Datenbank
 * 
 * */




// --------/  Methoden:  /--------------------------
/* werden inkludiert durch: */
#include "omemory.tpp"


}; // Ende Namespace Memory




#endif /* OMEMORY_H_ */
