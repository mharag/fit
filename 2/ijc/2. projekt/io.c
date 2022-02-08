// htab_private.h 
// Riesenie IJC-DU2, příklad 2), 18.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdio.h>
#include <ctype.h>
#include "io.h"

int read_word(char *s, int max, FILE *f){
    char c;
    int index = 0;

    // preskoci zaciatocne biele znaky
    while(isspace(c = fgetc(f)))
        ;

    do{
        if(c == EOF){
            s[index] = '\0';
            return EOF;
        }
        // zapise max. max-1 chars - ostatne preskoci
        if(index < max-1){
            s[index++] = c;
        }
    } while(!isspace(c = fgetc(f)));
    s[index] = '\0';

    return index;
}
