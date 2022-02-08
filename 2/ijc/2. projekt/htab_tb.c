#include <stdio.h>
#include "htab.h"

int main(){
    htab_t *t = htab_init(5); 
    htab_pair_t *record = htab_lookup_add(t, "test"); 
    record = htab_lookup_add(t, "tasest"); 
    record = htab_lookup_add(t, "tewdst"); 
    record = htab_lookup_add(t, "tesbt"); 
    record = htab_lookup_add(t, "tes1t"); 
    record->value = 100;


    htab_free(t);

    htab_pair_t *record2 = htab_lookup_add(t, "test"); 

    printf("%d\n", record2->value);

    return 0;
}
