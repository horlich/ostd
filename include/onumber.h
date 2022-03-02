/*
 * onumber.h
 *
 *  Created on: 03.01.2022
 *      Author: herbert
 */

#ifndef ONUMBER_H_
#define ONUMBER_H_

namespace ONumber {

/* Prüft, ob Zahl gerade oder ungerade ist: */
bool istGerade(long i);


/* std::negate im Header <functional> ist viel schneller! */
long negate(long l); /* l * -1 binär */

} /* Ende Namespace ONumber */



#endif /* ONUMBER_H_ */
