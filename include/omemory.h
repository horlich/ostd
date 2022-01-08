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
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

#include "oexception.h"
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

	bool hasValidPointer() const { return pointer != nullptr; }

	std::string getPointerName() const { return name; }

	virtual void setPointer(T* p) { pointer = p; }

	virtual void deletePointer() { setPointer(nullptr); }

	T* getPointer() const { return pointer; }

	/* Wirft NullPointerException: */
	T* getValidPointer() const;

	/* Wirft NullPointerException: */
	T& getReference() const { return  *(getValidPointer()); }
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

	virtual ~HeapPointer() { deletePointer(); }

	void deletePointer() override;

	void setPointer(T* p) override;
};





/*----------/  dynCast  /-------------------------*/

/* From muß polymorph sein, also einen virtuellen Destruktor besitzen! */
template<class From, class To>
static To* dynCast(From* v) {
	if (v == nullptr) return nullptr;
	To* ret = dynamic_cast<To*>(v);
	if (ret == nullptr) throw OException::BadCast(__PRETTY_FUNCTION__, "dynamic_cast mißlungen.");
	return ret;
}




template <class T>
void HeapPointer<T>::deletePointer() {
	delete StackPointer<T>::getPointer();
	StackPointer<T>::setPointer(nullptr);
}


template <class T>
void HeapPointer<T>::setPointer(T* p) {
	delete StackPointer<T>::getPointer();
	StackPointer<T>::setPointer(p);
}


template <class T>
T* StackPointer<T>::getValidPointer() const {
	if (pointer == nullptr) {
		std::stringstream buf;
		buf << __PRETTY_FUNCTION__ << ": Zeiger für " << getPointerName() << " nicht definiert.";
		throw OException::NullPointerException(buf.str());
	}
	return pointer;
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

	static void delPt(T* ptr) { delete ptr; }

	void deleteAll() {
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







// --------/  Methoden:  /--------------------------


template <class T>
PointerPool<T>::PointerPool(size_t alloc) {
	if (alloc) {
		allocPointer = this->get_allocator().allocate(alloc);
		allocatedSize = alloc;
	}
}


template <class T>
PointerPool<T>::~PointerPool() {
	this->reset();
	this->get_allocator().deallocate(allocPointer, allocatedSize);
}



template<typename T>
T* PointerPool<T>::get() {
	reuseCalls++;
	T* ret;
	if (ppool.empty()) {
		reuseFails++;
		ret = new T;
		this->insert(ret); // Zum Zerstören vormerken...
	} else {
		ret = ppool.front();
		ppool.pop();
		ret->resetValues();
	}
	return ret;
}


template <class T>
void PointerPool<T>::put(T* ptr) {
	if (! ptr) return;
	ppool.push(ptr);
}


template <class T>
void PointerPool<T>::reset() {
	ppool = std::queue<T*>();
	this->deleteAll();
	this->clear();
}


template<class T>
void PointerPool<T>::debugInfo(std::ostream& os) {
	int besetzt = this->size() - ppool.size();
	os << "# reuse() " << reuseCalls << " mal aufgerufen, dabei " << reuseFails << " Nullzeiger zurückgegeben.\n# "
		<< this->size() << " Zeiger im Pool gespeichert, davon noch " << besetzt << " besetzt." << std::endl;
}














}; // Ende Namespace Memory




#endif /* OMEMORY_H_ */
