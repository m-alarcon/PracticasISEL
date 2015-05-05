#ifndef MAIN_H
#define MAIN_H

static int matrizDeLeds[8][23];
char flag_sensor;


static void* pintarX(void*);
static void* simulaSensor(void*);
static void codificarEnMatriz();
static void pinta_columna();
static void limpiaMatrizLEDS();

#endif
