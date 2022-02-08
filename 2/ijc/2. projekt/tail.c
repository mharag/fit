// tail.c 
// Riesenie IJC-DU2, příklad 2), 18.4.2021 
// Autor: Miroslav Harag, FIT VUT 
// Prelozene: gcc 10.2.0 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_OF_LINES 10
#define MAX_LINE_LENGTH 511
enum modes{
    LAST_N_LINES,      //vypise poslednych n riadkov
    LAST_N_LINES_CROP, //vypise a oreze poslednych n riadkov
    FROM_LINE_N        //vypise vsetky riadky od riadky n po koniec
};

typedef char *line;
struct circle_buffer {
    int size, oldest;
    line *storage;
};

void read_into_circle_buffer(FILE *source, struct circle_buffer *buf, int mode){
    int c;
    int index = 0;
    while((c = fgetc(source)) != EOF){
        if(c == '\n'){
            buf->storage[buf->oldest][index  ] = '\n';
            buf->storage[buf->oldest][index+1] = '\0';
            index = 0;
            buf->oldest = (buf->oldest+1) % buf->size;
        }
        else{
            if(index >= MAX_LINE_LENGTH-1){
                if(mode == LAST_N_LINES){
                    fprintf(stderr, "Prekrocenie maximalnej dlzky riadku!\n");
                    mode = LAST_N_LINES_CROP;
                }
            }
            else{
                buf->storage[buf->oldest][index++] = c;
            }
        }
    }
}

void print_circle_buffer(struct circle_buffer *buf){
    for(int i = 0; i < buf->size; i++){
        int real_index = (i+buf->oldest)%buf->size;
        printf("%s", buf->storage[real_index]);
    }
}

int last_n_lines(int n, FILE *source){
    //vytvorenie bufferu
    struct circle_buffer buf = {.size   = n, 
                                .oldest = 0};
    buf.storage = malloc(buf.size * sizeof(line));
    if(buf.storage == NULL){ 
        fprintf(stderr, "ERROR: Nepodarilo sa alokovat miesto pre cyklicky buffer.\n");
        return 1;
    }
    for(int i = 0; i < buf.size; i++){
        buf.storage[i] = malloc( (MAX_LINE_LENGTH+1) * sizeof(char) );
        if(buf.storage[i] == NULL){
            fprintf(stderr, "ERROR: Nepodarilo sa alokovat miesto pre cyklicky buffer.\n");
            free(buf.storage); 
            return 1;
        } 
        buf.storage[i][0] = '\0';
    }

    read_into_circle_buffer(source, &buf, LAST_N_LINES); 
    print_circle_buffer(&buf);

    //odstranenie bufferu
    for(int i = 0; i < buf.size; i++){
        free(buf.storage[i]);
    }
    free(buf.storage);

    return 0;
}
int from_line_n(int n, FILE *source){
    char *line = NULL;
    line = malloc( (MAX_LINE_LENGTH+1) * sizeof(char*) );
    if(line == NULL){
        fprintf(stderr, "ERROR: Nepodarilo sa alokovat buffer pre riadok.\n");
        return 1;
    }

    //ignoruje prvych n riadkov
    while(--n && fgets(line, MAX_LINE_LENGTH, source))
        ;
    //vsetky ostatne riadky hned vracia na STDOUT
    while(fgets(line, MAX_LINE_LENGTH, source)){
        printf("%s", line);
    }

    free(line);
    return 0;
}

int main(int argc, char *argv[]){
    int mode = LAST_N_LINES;
    int n = NUM_OF_LINES;
    FILE *source = stdin;

    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-n")){
            if(argc <= i+1){
                fprintf(stderr, "ERROR: Zle zadane argumenty!\n");
                return 1;
            }
            if(argv[i+1][0] == '+'){
                mode = FROM_LINE_N;
            }

            n = strtol(argv[++i], NULL, 10);
        }
        else{
            source = fopen(argv[i], "rb");
            if(source == NULL){
                fprintf(stderr, "ERROR: Nepodarilo sa otvorit pozadovany subor!\n");
                return 1;
            }
        }
    }
    if(n <= 0){
        fprintf(stderr, "ERROR: Pocet riadkov musi byt nezaporne cele cislo!\n");
        return 1;
    }

    if(mode == LAST_N_LINES){
        if(last_n_lines(n, source))
            return 1;
    }
    else if(mode == FROM_LINE_N){
        if(from_line_n(n, source))
           return 1; 
    }

    fclose(source);
    return 0;
}
