/*
 * oexception.h
 *
 *  Created on: 08.05.2017
 *      Author: kanzlei
 */

#ifndef OEXCEPTION_H_
#define OEXCEPTION_H_

#include <string>
#include <sstream>
#include <string.h>
#include <iostream>
#include <exception>


namespace OException {



class Fehler : public std::exception {
    /* Oberste (Default-) Klasse von OException */
private:
    std::string message;

protected:
    void setMessage(const std::string& mess)
    {
        message = mess;
    }

    Fehler() : message("Undefinierter Fehler") {}

public:
    Fehler(const std::string& mess);

    Fehler (const char* funcname, const std::string& message);

    virtual ~Fehler() = default;

    virtual const char* what() const noexcept override ;
};







class ParseException : public Fehler {
public:
    ParseException(const std::string& message) :
        Fehler(message) {}

    virtual ~ParseException() = default;
};


class NullPointerException : public Fehler {
public:
    NullPointerException(const std::string& message) :
        Fehler(message) {}

    NullPointerException (const char* funcname, const std::string& message);

    virtual ~NullPointerException() = default;
};


class IllegalArgumentException : public Fehler {
public:
    IllegalArgumentException(const std::string& message) :
        Fehler(message) {}

    virtual ~IllegalArgumentException() = default;
};


class IllegalOperationException : public Fehler {
public:
    IllegalOperationException(const std::string& message) :
        Fehler(message) {}

    virtual ~IllegalOperationException() = default;
};


class OperationFailedException : public Fehler {
public:
    OperationFailedException(const std::string& message) :
        Fehler(message) {}

    virtual ~OperationFailedException() = default;
};


class IndexOutOfBoundsException : public Fehler {
public:
    IndexOutOfBoundsException(const std::string& message) :
        Fehler(message) {}

    IndexOutOfBoundsException(const char* funcname, const std::string& message);

    virtual ~IndexOutOfBoundsException() = default;
};


class BadCast : public Fehler {
public:
    BadCast(const std::string& message) :
        Fehler(message) {}

    BadCast(const char* funcname, const std::string& message);

    virtual ~BadCast() = default;
};


struct CommandFailed : public Fehler {
    /* Wenn die übergebene errno == 0, dann wird sie ignoriert */
    CommandFailed(const std::string& command, int error_no = 0);

    virtual ~CommandFailed() = default;
};


} // Ende namespace OException

/* Muß außerhalb des Namespace stehen, weil
 * es sonst nicht gefunden wird: */
std::ostream& operator<<(std::ostream& os, OException::Fehler& e);


#endif /* OEXCEPTION_H_ */
