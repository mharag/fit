// steg_decode.c
// Riesenie IJC-DU1, příklad b), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozene: gcc 10.2.0
// 
// steg_decode.c hlada "tajnu spravu" podla zadania DU-1 IJC

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ppm.h"
#include "error.h"
#include "bitset.h"
#include "eratosthenes.h"

#define MESSAGE_START 23
#define MAX_IMAGE_SIZE (8000*8000*3)

void print_secret_message(struct ppm *p){
    unsigned long image_size = p->xsize * p->ysize * 3;
    bitset_alloc(bitset, image_size);
    Eratosthenes(bitset);

    unsigned char character = 0;
    unsigned long bit_of_char = 0;
    for(bitset_index_t i = MESSAGE_START; i < image_size; i++){
        if(!bitset_getbit(bitset,i)){
            if(i >= p->xsize*p->ysize)
                break;
            else{
                //          {ziska LSB bit}<<{uklada od LSB po MSB}
                character += (p->data[i]&1)<<(bit_of_char++);
                if(bit_of_char == CHAR_BIT){
                    printf("%c", character);
                    if(!character)
                        break;
                    character = 0;
                    bit_of_char = 0;
                }
            }
        }
    }
    printf("\n");

    bitset_free(bitset);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        error_exit("Nespravne zadane parametre!");
    }

    struct ppm *image;
    image = ppm_read(argv[1]);
    if(image == NULL)
        return -1;

    print_secret_message(image);

    ppm_free(image);
    return 0;
}
