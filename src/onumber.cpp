/*
 * onumber.cpp
 *
 *  Created on: 03.01.2022
 *      Author: herbert
 */

#include "onumber.h"

using namespace std;

namespace ONumber {


bool istGerade(long i)
{
   const unsigned char mask = 1; /* nur das 1. Bit ist gesetzt */
   /* Prüft das gesetzte 1. Bit in mask mit i.
    * Wenn gesetzt, dann ist i ungerade:    */
   return !(i & mask); /* "nicht ungerade" */
}


long flip(long l)
{
   return ~l + 1;
}


} /* Ende Namespace ONumber */
