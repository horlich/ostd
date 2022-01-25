
/* Template-Sources für ocontainer.h */

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


