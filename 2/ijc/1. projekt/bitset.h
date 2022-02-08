// bitset.h
// Riesenie IJC-DU1, příklad a), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozeno: gcc 10.2.0
// 
// - Subor bitset.h umoznuje pracu s bitset. 
// - Pre pouzitie INLINE funkcii definujte USE_INLINE pri preklade.

#ifndef _BITSET_H
#define _BITSET_H

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include "error.h"

#define FIELD_SIZE (sizeof(unsigned long)*8) 
#define TEST_INDEX(index,size)\
    ((index) >= (size)?\
    error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu", (unsigned long)(index), (unsigned long)(size)-1),1:0) 

typedef unsigned long bitset_t[];
typedef unsigned long bitset_index_t;

//Vytvori bitove pole na zasobniku. Na nultom prvku bude celkova velkost pola
//Skutocna obsadena velkost sa moze lisit max. o FIELD_SIZE-1 bitov
#define bitset_create(name,size)\
    _Static_assert((size) > 0, "Velkost pola musi byt nenulova kladna hodnota.");\
    unsigned long name[((size)+FIELD_SIZE-1)/FIELD_SIZE+1] = {(size), 0,}

//Vytvori bitove dynamicky alokovane pole. Na nultom prvku bude celkova velkost pola
//Skutocna obsadena velkost sa moze lisit max. o FIELD_SIZE-1 bitov
#define bitset_alloc(name, size)\
    assert((size) > 0 && (size) <= ULONG_MAX);\
    unsigned long *name = calloc(((size)+FIELD_SIZE-1)/FIELD_SIZE+1, sizeof(unsigned long));\
    if(name == NULL)\
        error_exit("bitset_alloc: Chyba alokace pamětii");\
    name[0] = (size)


#ifdef USE_INLINE
inline void bitset_free(bitset_t size){
    free(size);
}

inline unsigned long bitset_size(bitset_t bitset){
    return bitset[0];
}

inline void bitset_setbit(bitset_t bitset, bitset_index_t index, char value){
    TEST_INDEX(index, bitset[0]);
    bitset[index/FIELD_SIZE+1] ^= (-(unsigned long)(!!value) ^ bitset[index/FIELD_SIZE+1]) & 1UL<<index%FIELD_SIZE;
}

inline unsigned long bitset_getbit(bitset_t bitset, bitset_index_t index){
    TEST_INDEX(index, bitset[0]);
    return (bitset[index/FIELD_SIZE+1] & ((unsigned long)1 << index%FIELD_SIZE));
}

#else

//Odstrani bitove pole s nazvom name 
#define bitset_free(name) free(name)
   
#define bitset_size(name) name[0]

#define bitset_setbit(name,index,value)\
    TEST_INDEX(index, bitset[0]);\
    name[(index)/FIELD_SIZE+1] ^= (-(unsigned long)(!!value) ^ name[(index)/FIELD_SIZE+1]) & 1UL<<(index)%FIELD_SIZE

#define bitset_getbit(name, index)\
    (TEST_INDEX(index,name[0]),(name[((index)/FIELD_SIZE)+1] & (unsigned long)1 << (index)%FIELD_SIZE))

#endif

#endif
