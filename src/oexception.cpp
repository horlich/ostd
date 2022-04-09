/*
 * oexception.cpp
 *
 *  Created on: 17.05.2017
 *      Author: kanzlei
 */

#include "oexception.h"

using namespace std;
using namespace OException;


std::string funcNameMess__(const char* funcname, const std::string& message) {
	std::stringstream buf;
	buf << funcname << ": " << message;
	return buf.str();
}

std::string commandFailedMess__(const std::string& command) {
    std::stringstream buf;
    buf << "Befehl '" << command << "' mißlungen";
    return buf.str();
}


Fehler::Fehler(const std::string& mess) : message(mess) {}

Fehler::Fehler(const char* funcname, const std::string& message) :
		Fehler(funcNameMess__(funcname, message)) {}



const char* Fehler::what() const noexcept {
	return message.c_str();
}


NullPointerException::NullPointerException(const char* funcname, const std::string& message) :
		Fehler(funcname, message) {}



IndexOutOfBoundsException::IndexOutOfBoundsException(const char* funcname, const std::string& message) :
		Fehler(funcname, message) {}


CommandFailed::CommandFailed(const std::string& command)
        : Fehler(commandFailedMess__(command)) {}


ostream& operator<<(ostream& os, Fehler& e) {
	os << "FEHLER: " << e.what();
	return os;
}

