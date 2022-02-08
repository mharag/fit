// eratosthenes.c
// Riesenie IJC-DU1, příklad a), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozeno: gcc 10.2.0

#ifndef _ERATOSTHENES_H
#define _ERATOSTHENES_H

#include "bitset.h"

// Vyplni bitset podla algoritmu Eratosthenove sito
// Vo vyslednom bitsete budu vsetky prvocisla oznacene nulovou hodnotou
void Eratosthenes(bitset_t bitset);

// Vyplni bitset podla upraveneho algoritmu Eratosthenove sito
void Eratosthenes_optimalised(bitset_t bitset);

#endif
