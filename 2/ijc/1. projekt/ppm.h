// bitset.c
// Riesenie IJC-DU1, příklad a), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozeno: gcc 10.2.0
// ppm.h poskytuje rozhranie pre pracu s .ppm obrazkami

#ifndef _PPM_H
#define _PPM_H

struct ppm {
    unsigned xsize;
    unsigned ysize;
    char *data;    // RGB bajty, celkem 3*xsize*ysize
};

//nacita obrazok s nazvom "filename" s pamate 
struct ppm * ppm_read(const char * filename);

void ppm_free(struct ppm *p);

#endif
