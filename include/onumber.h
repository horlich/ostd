/*
 * onumber.h
 *
 *  Created on: 03.01.2022
 *      Author: herbert
 */

#ifndef ONUMBER_H_
#define ONUMBER_H_

#include <random>

namespace ONumber {

/* Prüft, ob Zahl gerade oder ungerade ist: */
bool istGerade(long i);


/* std::negate im Header <functional> ist viel schneller! */
long negate(long l); /* l * -1 binär */


/* Is value a prime number? */
bool isPrimary(const unsigned long value);



class IntGenerator {
//
public:
    IntGenerator(int low, int high);

    int operator()();

private:
    std::mt19937_64 engine;
    std::uniform_int_distribution<> dist;
};



} /* Ende Namespace ONumber */



#endif /* ONUMBER_H_ */
