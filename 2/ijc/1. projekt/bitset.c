// bitset.c
// Riesenie IJC-DU1, příklad a), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozeno: gcc 10.2.0
// 
// - Subor bitset.c umoznuje pracu s bitset. 
// - Pre pouzitie INLINE funkcii definujte USE_INLINE pri preklade.

#include "bitset.h"

#ifdef USE_INLINE
extern void bitset_free(bitset_t size);
extern unsigned long bitset_size(bitset_t bitset);
extern void bitset_setbit(bitset_t bitset, bitset_index_t index, char value);
extern unsigned long bitset_getbit(bitset_t bitset, bitset_index_t index);
#endif
