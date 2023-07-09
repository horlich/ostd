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
#include "ocontainer.tpp"


}; // Ende Namespace OContainer




#endif /* OCONTAINER_H_ */
