/*
 * oexception.cpp
 *
 *  Created on: 17.05.2017
 *      Author: kanzlei
 */

#include "oexception.h"

using namespace std;
using namespace OException;


std::string OException::envNameMess(const char* funcname, const std::string& message) {
	std::stringstream buf;
	buf << funcname << ": " << message;
	return buf.str();
}



Fehler::Fehler(const std::string& mess) : message(mess) {}

Fehler::Fehler(const char* funcname, const std::string& message) :
		Fehler(envNameMess(funcname, message)) {}



const char* Fehler::what() const noexcept {
	return message.c_str();
}


NullPointerException::NullPointerException(const char* funcname, const std::string& message) :
		Fehler(funcname, message) {}

//std::string _badMess(const char* funcname, const std::string& message) {
//	std::stringstream buf;
//	buf << funcname << ": " << message;
//	return buf.str();
//}

IndexOutOfBoundsException::IndexOutOfBoundsException(const char* funcname, const std::string& message) :
		Fehler(funcname, message) {}



ostream& operator<<(ostream& os, Fehler& e) {
	os << "FEHLER: " << e.what();
	return os;
}

