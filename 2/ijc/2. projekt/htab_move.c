// htab_move.c
// Použijte: g++ -std=c++11 -O2
// Příklad použití STL kontejneru unordered_map<>
// Program počítá četnost slov ve vstupním textu,
// slovo je cokoli oddělené "bílým znakem"

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

htab_t *htab_move(size_t n, htab_t *from){
    htab_t *t = htab_init(n);
    
    htab_pair_t *new;
    for(size_t i = 0; i < from->arr_size; i++){
        while(from->ptr_array[i] != NULL){
            // vymaze prvu polozku listu a ako zaciatok listu nastavi dalsiu polozku v poradi
            struct htab_item *temp = from->ptr_array[i];
            from->ptr_array[i] = from->ptr_array[i]->next;

            // prida polozku do novej tabulky
            new = htab_lookup_add(t, temp->value.key);
            new->value = temp->value.value;

            free((void*)temp->value.key);
            free(temp);
        }
    }

    return t;
}
