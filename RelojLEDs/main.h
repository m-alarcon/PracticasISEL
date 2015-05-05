#ifndef MAIN_H
#define MAIN_H


#define GPIO_BUTTON_M  15
#define GPIO_BUTTON_H  16

struct horario{
  int hora;
  int minutos;
};

struct horario t;

static int matrizDeLeds[8][23];
int matrizDeLedsPintar[8][23];
int matrizNumero[8][5];

int horaAux;
int minutosAux;
int hora1;
int hora2;
int minuto1;
int minuto2;
int minuto2aux;

char flag_sensor;

static void* modificarHora(void*);
static void* contadorTiempo(void*);
static void* pintarHora(void*);
static void* simulaSensor(void*);
static void codificarEnMatriz();
static void pinta_columna();
static void getDigitosHoraMinuto();
static void codificaNumero();
static void limpiaMatrizNumero();
static void limpiaMatrizLEDS();
static void incluirDigitos();

static pthread_mutex_t horario_mutex;
static pthread_mutex_t matriz_mutex;

#endif
