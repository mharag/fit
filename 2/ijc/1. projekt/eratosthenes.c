// bitset.h
// Riesenie IJC-DU1, příklad a), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozeno: gcc 10.2.0


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitset.h"

void Eratosthenes(bitset_t bitset){
    bitset_index_t top_boundary = sqrt(bitset_size(bitset));
    for(bitset_index_t i = 2; i < top_boundary; i++){
        if(!bitset_getbit(bitset,i)){
            for(bitset_index_t j = i*i; j < bitset_size(bitset); j += i){
                bitset_setbit(bitset, j, 1);
            }
        }
    }
}

// makro odstranuje dve zbytocne podmienky - kontrolu indexu (je kontrolovany priamov algoritne)
//                                         - zistenie pozadovanej hodnoty (nastavujeme len na true)
#define bitset_setbit_to_true(name,index) name[(index)/FIELD_SIZE+1] |= 1UL<<(index)%FIELD_SIZE
void Eratosthenes_optimalised(bitset_t bitset){
    // zaciatok od cisla 3 - parne cisla sa uplne vynechavaju
    bitset_index_t actual_prime = 3;

    // nasledujuce prvocislo (ktoreho nasobky maju byt vyskrtnute) sa nehlada pomocou cyklu ale radsej
    // je za toto cislo oznacene (X/actual_prime) - kde x je najmensi nasobok cisla actual_prime 
    // ktory este nebolo oznaceny
    bitset_index_t next_prime = 0;

    bitset_index_t top_boundary = sqrt(bitset_size(bitset)*2);
    while(actual_prime < top_boundary){
        // premenna j reprezentuje nasobky cisla actual_prime 
        // treba prepocitavat cisla na indexy v pamati - parne cisla su vynechane
        // cisla su ulozene na indexoch (cislo-1)/2
        // nasobky mensie ako actual_prime^2 netreba prehladavat - boli vyskrtnute uz skor
        bitset_index_t j = (actual_prime*(actual_prime+2))/2;

        // po nasobkoch j sa posuvame vzdy po actual_prime*2 (staci testovat kazdy druhy nasobok pretoze ostatne su parne)
        // v skutocnosti vsak pripocitavame iba actual_prime (odlachci nas to od jedneho delenie pri prevode na skutocne indexy v pamati)
        
        // v tomto cykle hladame najmensie neoznaceny nasobok s ktoreho vypocitame next_prime
        for(; j < bitset_size(bitset); j += actual_prime){
            if(!bitset_getbit(bitset, j)){
                next_prime = (j*2+1)/actual_prime;
                bitset_setbit_to_true(bitset, j);
                break;
            }
        }

        // vyskrtne vsetky nasobky actual_prime
        // j je skutocny index v pamati a pripocitavame (actual_prime*2)/2 ako bolo popisane skor
        for(; j < bitset_size(bitset); j += actual_prime){
            bitset_setbit_to_true(bitset, j);
        }
 
        actual_prime = next_prime;
        next_prime = 0;
    }

    // ulozenie bez parnych cisel potrebuje specialne vypisovanie
    // tento cyklus je len provizorny a bolo by dobre ho prepisat :D
    int n = 10;
    for(bitset_index_t i = bitset[0]-1; i > 0; i--){
        if(!bitset_getbit(bitset,i)){
            printf("%lu \n",i*2+1);
            if(!--n)
                break;
        }
    }
}
