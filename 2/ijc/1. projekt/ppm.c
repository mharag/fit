// ppm.c
// Riesenie IJC-DU1, příklad b), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozene: gcc 10.2.0
// 
// Subor poskytuje potrebne nastroje pre nacitanie .ppm obrazku do pamati 


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ppm.h"
#include "error.h"
#include "bitset.h"
#include "eratosthenes.h"

#define MAX_RGB_VALUE 255
struct ppm * ppm_read(const char * filename){

    //Nacitanie suboru
    FILE *file = fopen(filename, "rb");
    if(file == NULL){
        warning_msg("Nepodarilo sa otvorit subor %s.\n", filename);
        goto clean_nothing;
    }

    //Kontrola formatu
    char file_format[3];
    if (!fgets(file_format, sizeof(file_format), file)) {
        warning_msg("Nepodarilo sa nacitat format suboru.\n");
        goto clean_file;
    }
    if (!(file_format[0] == 'P' && file_format[1] == '6')){
        warning_msg("Nepodporovany format suboru. Podporovany format: P6\n");
        goto clean_file;
    }

    struct ppm *img; 
    img = malloc(sizeof(struct ppm));
    if(img == NULL){
        warning_msg("Nepodarilo sa alokovat pamat pre ulozenie obrazku.\n");
        goto clean_img;
    }

    //Zistenie velkosi & alokovanie priestoru
    if (fscanf(file, "%u %u\n", &img->xsize, &img->ysize) != 2) {
        warning_msg("Nepodarilo sa nacitat velkost obrazku.\n");
        goto clean_img;
    }
    img->data = malloc(img->xsize * img->ysize *3);
    if(img->data == NULL){
        warning_msg("Nepodarilo sa alokovat pamat pre ulozenie obrazku.\n");
        goto clean_img_data;
    }

    //Kontrola farebneho rozsahu
    int color_depth;
    if (fscanf(file, "%d\n", &color_depth) != 1) {
        warning_msg("Nepodarilo sa nacitat farebniy rozsah.\n");
        goto clean_img_data;
    }
    if (color_depth != MAX_RGB_VALUE) {
        warning_msg("Nepodporovany farebny rozsah. Podporovany rozsah: 0..255\n");
        goto clean_img_data;
    }

    //read pixel data from file
    if (fread(img->data, img->xsize*3, img->ysize, file) != img->ysize) {
        warning_msg("Nepodarilo sa nacitat data obrazku.\n");
        goto clean_img_data;
    }

    //Spravne ukoncenie
    fclose(file);
    return img;

    //Chybove ukoncenie
    clean_img_data:
        free(img->data);
    clean_img:
        free(img);
    clean_file:
        fclose(file);
    clean_nothing:
        return NULL;
}

void ppm_free(struct ppm *p){
    free(p->data);
    free(p);
}
