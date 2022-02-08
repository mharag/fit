// wordcount.c 
// Riesenie IJC-DU2, příklad 2), 18.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "htab.h"
#include "io.h"

// povolenim sa zapne automaticka balancia load factor-u hash tabulky
// podobna implementacii unordered_map v c++
// (MOVETEST je urcene pre test funkcie htab_move nie pre optimalizaciu)
//#define MOVETEST
//#define HTAB_INIT_SIZE 13

// nahradenie hashovacej funkcie z kniznice za vlastnu
//#define HASHTABLE

// optimalna velkost hash tabulky zavisi na velkosti vstupu
// pre moje testovacie vstupy sa mi najviac osvedcili cisla v okoli 10000
// vyberal som prvocisla kvoli znizeniu poctu kolizii
#define HTAB_INIT_SIZE 10007 

#define MAX_WORD_LEN 127

//Funckia pre vypis obsahu htab_pair_t
void print_htab_pair(htab_pair_t *data){
    printf("%s\t%d\n", data->key, data->value);
}

#ifdef HASHTEST
size_t htab_hash_function(const char *str) {
    (void)str;
    
    size_t result = 0;

    int i = 0;
    while(str[i] != '\0'){
         result += str[i];
         i++;
    }
    return result;
}
#endif

int main(){
    htab_t *t = NULL;
    t = htab_init(HTAB_INIT_SIZE);
    if(t == NULL){
        return 1;
    }

    char buffer[MAX_WORD_LEN + 1];

    while(read_word(buffer, MAX_WORD_LEN+1, stdin) != EOF){
        htab_pair_t *htab_entry = htab_lookup_add(t, buffer);
        htab_entry->value++;

        #ifdef MOVETEST
        // ak je load_factor vacsi ako 1 dvojnasobne sa zvacsi velkost tabulky
        if(htab_size(t) > htab_bucket_count(t)){
            htab_t *new_table = htab_move(htab_bucket_count(t) * 2, t);
            htab_clear(t);
            t = new_table;
        }
        #endif
    }

    htab_for_each(t, &print_htab_pair);

    htab_free(t);
    return 0;
}
