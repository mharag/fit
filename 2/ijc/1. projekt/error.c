// error.c
// Riesenie IJC-DU1, příklad b), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozene: gcc 10.2.0
// 
// error.c poskytuje nastroje pre hlasenie chyb na stderr


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "error.h"

void warning_msg(const char *fmt, ...){
    va_list arguments;
    va_start (arguments, fmt);
    fprintf(stderr, "CHYBA: ");
    vfprintf (stderr, fmt, arguments);
    va_end (arguments);
}

void error_exit(const char *fmt, ...){
    va_list arguments;
    va_start(arguments, fmt);
    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, arguments);
    va_end (arguments);
    exit(1);
}
