/*
 * ocontainer.h
 *
 *  Created on: 26.06.2017
 *      Author: kanzlei
 */

#ifndef OCONTAINER_H_
#define OCONTAINER_H_


#include <queue>
#include <stack>
#include <vector>
#include <iostream>


namespace OContainer {


template <class T>
class PointerQueue : public std::queue<T*> {
public:
	virtual ~PointerQueue(){ while (deleteNext()); }

	bool deleteNext();

	inline void resize(int newSize);
};



template <class T>
class PointerStack : public std::stack<T*> {
public:
	virtual ~PointerStack(){ while (deleteNext()); }

	bool deleteNext();
};



template <class T>
class PointerVec : public std::vector<T*> {
public:
	// Führt delete aus und zeigt dann auf nullptr.
	// Gibt die Adresse des nullptr zurück:
	static void setNull(typename PointerVec<T>::iterator it);

	// Zerstört alle Elemente und macht dann std::vector<T*>::clear()
	virtual void reset();

	virtual ~PointerVec();
};









/*
 *                         Source-Code:
 * */

template <class T>
bool PointerQueue<T>::deleteNext() {
	if (this->empty()) return false;
	delete this->front();
	this->pop();
	return true;
}



template <class T>
inline void PointerQueue<T>::resize(int newSize) {
	while ((int)this->size() > newSize) deleteNext();
}



template <class T>
bool PointerStack<T>::deleteNext() {
	if (this->empty()) return false;
	delete this->top();
	this->pop();
	return true;
}




template <class T>
void PointerVec<T>::reset() {
	for (typename PointerVec<T>::iterator it = this->begin(); it != this->end(); ++ it) delete (*it);
	this->clear();
}


template <class T>
PointerVec<T>::~PointerVec() {
	for (typename PointerVec<T>::iterator it = this->begin(); it != this->end(); ++ it) delete (*it);
}




template <class T>
void PointerVec<T>::setNull(typename PointerVec<T>::iterator it) {
	T* ptr = *it;
	delete ptr;
	*it = nullptr;
//	return it;
}




}; // Ende Namespace OContainer




#endif /* OCONTAINER_H_ */
