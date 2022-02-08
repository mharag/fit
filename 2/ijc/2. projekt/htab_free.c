// htab_init.c 
// Riesenie IJC-DU2, příklad 2), 17.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

void htab_free(htab_t * t){
    // odstrani vsetky polozky z tabuklky
    htab_clear(t);

    // odstrani tabulku samotnu
    free(t->ptr_array);
    free(t);
}
