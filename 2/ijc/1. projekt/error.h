// error.h
// Riesenie IJC-DU1, příklad b), 18.3.2021
// Autor: Miroslav Harag, FIT VUT
// Prelozene: gcc 10.2.0
// 
// error.h definuje rozhranie pre hlasenie chyb na stderr

#ifndef _ERROR_H
#define _ERROR_H

//Na stderr vypise "CHYBA: " + chybovu hlasku formatovanu rovnako ako printf
void warning_msg(const char *fmt, ...);

//Na stderr vypise "CHYBA: " + chybovu hlasku formatovanu rovnako ako printf a ukonci program
void error_exit(const char *fmt, ...);

#endif
