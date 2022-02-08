// primes.c
// Riesenie IJC-DU1, příklad a), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozene: gcc 10.2.0
// 
// - primes.c vrati NUM_OF_PRIMES najvacsich cisel mensich ako MAX_PRIME. 
// - Pomocou direktivy BITSET_ON_STACK je mozne nastavit bitsetu na stack. Pozor na dostatocnu velkost stacku!
// - Program testovany na Intel(R) Core(TM) i5-9300H s priemernym casom pri defaultnych nastaveniach 1.39s
// - S volbou OPTIMALISED_ERATOSTHENES dosahuje priemerny cas 0.45s (na merlin.fit.vutbr.cz primerny cas 0.37s)

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitset.h"
#include "eratosthenes.h"

//Zapne optimalizacie E. sita ako napr. uplne vynechanie parnych cisel,
//specialne makro pre nastavenie bitu
//#define OPTIMALISED_ERATOSTHENES

//!Pred zvolenim zvazte rozsirenie stacku!
#define BITSET_ON_STACK

#define NUM_OF_PRIMES 10
#define MAX_PRIME 200000000 

int print_top_n(bitset_t bitset, int left_to_find){
    // Docasny buffer pre otocenie poradia vysledkov
    int buff_size = left_to_find;
    bitset_index_t *buff = malloc(sizeof(bitset_index_t)*buff_size);
    if(buff == NULL)
        error_exit("Nepodarilo sa alokovat pamat.");

    // Odkonca prehlada bitset a najde left_to_find prvocisel
    for(bitset_index_t i = MAX_PRIME-1; i > 0 && left_to_find; i--){
        if(!bitset_getbit(bitset,i)){
            // Cisla uklada odzadu do bufferu
            buff[--left_to_find] = i;
        }
    }

    // Vypise buffer - preskoci prvych left_to_find poloziek
    for(int i = left_to_find; i < buff_size; i++)
        printf("%lu \n",buff[i]);

    free(buff);
    return 0;
}

int main(){
    unsigned long start_time = clock();

    #ifdef BITSET_ON_STACK
    bitset_create(bitset, MAX_PRIME);
    #else
    bitset_alloc(bitset, MAX_PRIME);
    #endif

    #ifndef OPTIMALISED_ERATOSTHENES
    Eratosthenes(bitset);
    print_top_n(bitset, NUM_OF_PRIMES);
    #else
    bitset[0] /= 2;
    Eratosthenes_optimalised(bitset);
    #endif

    #ifndef BITSET_ON_STACK
    bitset_free(bitset);
    #endif

    fprintf(stderr, "Time=%.3g\n", (double)(clock()-start_time)/CLOCKS_PER_SEC);
    return 0;
}
