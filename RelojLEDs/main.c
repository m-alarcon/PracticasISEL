#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "tasks.h"
#include "screen.h"
#include "main.h"

static void* modificarHora (void* arg) {
  struct timeval next_activation;
  struct timeval period = { 0, 1000000 };
  
  gettimeofday (&next_activation, NULL);
  while (1) {
    delay_until (&next_activation);
    timeval_add (&next_activation, &next_activation, &period);
    switch (screen_getchar()){
      case 'h':
        printf( "\n Has pulsado H");   
        pthread_mutex_lock (&horario_mutex);
        t.hora++;
        if (t.hora==24)
          t.hora=00;
        pthread_mutex_unlock (&horario_mutex);
        break;

      case 'm':
        printf( "\n Has pulsado M");
        pthread_mutex_lock (&horario_mutex);
        t.minutos++;
        if (t.minutos==60){
          t.minutos = 00;
          t.hora++;
        }
        pthread_mutex_unlock (&horario_mutex);

    }
    codificarEnMatriz();
  }
  return NULL;
}

static void* contadorTiempo (void* arg) {
  struct timeval next_activation;
  struct timeval period = { 0, 60000000 };
  
  gettimeofday (&next_activation, NULL);
  while (1) {
    delay_until (&next_activation);
    timeval_add (&next_activation, &next_activation, &period);

    pthread_mutex_lock (&horario_mutex);
    horaAux = t.hora;
    minutosAux = t.minutos;
    pthread_mutex_unlock (&horario_mutex);

    if (minutosAux<59)
      minutosAux++;
    else {
      minutosAux = 00;
      horaAux++;
    }
    if (horaAux==24)
      horaAux = 00;

    pthread_mutex_lock (&horario_mutex);
    t.hora = horaAux;
    t.minutos = minutosAux;
    pthread_mutex_unlock (&horario_mutex);

    codificarEnMatriz();
  }  
  return NULL;
}

static void* pintarHora (void* arg){
  struct timeval next_activation;
  struct timeval period = { 0, 2000 };
  
  gettimeofday (&next_activation, NULL);
  while (1) {
    delay_until (&next_activation);
    timeval_add (&next_activation, &next_activation, &period);
    if (flag_sensor=='1') {
      flag_sensor='0';

      pthread_mutex_lock (&matriz_mutex);
      for(int i=0;i<8;i++){
        for(int j=0;j<23;j++){
          matrizDeLedsPintar[i][j]=matrizDeLeds[i][j];
        }
      }
      pthread_mutex_unlock(&matriz_mutex);

      for(int i=0;i<23;i++){
        struct timespec delay = { 0, 2525250L };
        pinta_columna(i);
        nanosleep (&delay, NULL);
      }
    }
  }
  return NULL;
}

static void codificarEnMatriz(){

  limpiaMatrizNumero();
  limpiaMatrizLEDS();

  pthread_mutex_lock (&matriz_mutex);
  matrizDeLeds[2][11]=1;
  matrizDeLeds[5][11]=1;
  pthread_mutex_unlock (&matriz_mutex);

  getDigitosHoraMinuto();
  minuto2aux= minuto2;
  codificaNumero(hora1);
  incluirDigitos(1);
  codificaNumero(hora2);
  incluirDigitos(2);
  codificaNumero(minuto1);
  incluirDigitos(3);
  minuto2 = minuto2aux;
  codificaNumero(minuto2);
  incluirDigitos(4);
}

static void pinta_columna(int columna){
  
  if(matrizDeLedsPintar[0][columna]==1)
    screen_printxy(columna,0,"*");
  else
    screen_printxy(columna,0," ");
  if(matrizDeLedsPintar[1][columna]==1)
    screen_printxy(columna,1,"*");
  else
    screen_printxy(columna,1," ");
  if(matrizDeLedsPintar[2][columna]==1)
    screen_printxy(columna,2,"*");
  else
    screen_printxy(columna,2," ");
  if(matrizDeLedsPintar[3][columna]==1)
    screen_printxy(columna,3,"*");
  else
    screen_printxy(columna,3," ");
  if(matrizDeLedsPintar[4][columna]==1)
    screen_printxy(columna,4,"*");
  else
    screen_printxy(columna,4," ");
  if(matrizDeLedsPintar[5][columna]==1)
    screen_printxy(columna,5,"*");
  else
    screen_printxy(columna,5," ");
  if(matrizDeLedsPintar[6][columna]==1)
    screen_printxy(columna,6,"*");
  else
    screen_printxy(columna,6," ");
  if(matrizDeLedsPintar[7][columna]==1)
    screen_printxy(columna,7,"*");
  else
    screen_printxy(columna,7," ");

/*  digitalWrite(0,matrizDeLedsPintar[0][columna]);
/  digitalWrite(1,matrizDeLedsPintar[1][columna]);
/  digitalWrite(2,matrizDeLedsPintar[2][columna]);
/  digitalWrite(3,matrizDeLedsPintar[3][columna]);
/  digitalWrite(4,matrizDeLedsPintar[4][columna]);
/  digitalWrite(5,matrizDeLedsPintar[5][columna]);
/  digitalWrite(6,matrizDeLedsPintar[6][columna]);
/  digitalWrite(7,matrizDeLedsPintar[7][columna]); */
}

static void getDigitosHoraMinuto(){
  pthread_mutex_lock (&horario_mutex);
  horaAux = t.hora;
  minutosAux = t.minutos;
  pthread_mutex_unlock (&horario_mutex);

  hora1= horaAux/10;
  hora2= horaAux-(hora1*10);

  minuto1= minutosAux/10;
  minuto2= minutosAux-(minuto1*10);
}

static void codificaNumero(int numero){
  limpiaMatrizNumero();
  switch (numero){

    case 0:
      for(int f=0;f<7;f++){
        matrizNumero[f][1]=1;
        matrizNumero[f][4]=1;
      }
      matrizNumero[0][2]=1;
      matrizNumero[0][3]=1;
      matrizNumero[6][2]=1;
      matrizNumero[6][3]=1;
    break;

    case 1:
      for (int f=0;f<7;f++)
        matrizNumero[f][4]=1;

      matrizNumero[1][3]=1;
      matrizNumero[2][2]=1;
    break;

    case 2:
      for (int f=1;f<5;f++){
        matrizNumero[0][f]=1;
        matrizNumero[3][f]=1;
        matrizNumero[6][f]=1;
      }
        matrizNumero[1][4]=1;
        matrizNumero[2][4]=1;
        matrizNumero[4][1]=1;
        matrizNumero[5][1]=1;
    break;

    case 3:
      for (int f=1;f<5;f++){
          matrizNumero[0][f]=1;
          matrizNumero[3][f]=1;
          matrizNumero[6][f]=1;
        }
          matrizNumero[1][4]=1;
          matrizNumero[2][4]=1;
          matrizNumero[4][4]=1;
          matrizNumero[5][4]=1;
    break;

    case 4:
      for (int f=0;f<7;f++)
          matrizNumero[f][4]=1;
      for (int g=0;g<4;g++)
          matrizNumero[g][1]=1;
      matrizNumero[3][2]=1;
      matrizNumero[3][3]=1;
    break;

    case 5:
        for (int f=1;f<5;f++){
          matrizNumero[0][f]=1;
          matrizNumero[3][f]=1;
          matrizNumero[6][f]=1;
        }
          matrizNumero[1][1]=1;
          matrizNumero[2][1]=1;
          matrizNumero[4][4]=1;
          matrizNumero[5][4]=1;
    break;

    case 6:
       for (int f=1;f<5;f++){
          matrizNumero[0][f]=1;
          matrizNumero[3][f]=1;
          matrizNumero[6][f]=1;
        }
          matrizNumero[1][1]=1;
          matrizNumero[2][1]=1;
          matrizNumero[4][4]=1;
          matrizNumero[5][4]=1;
          matrizNumero[4][1]=1;
          matrizNumero[5][1]=1;
    break;

    case 7:
      for(int g=0;g<7;g++)
        matrizNumero[g][4]=1;
      for(int h=1;h<5;h++)
        matrizNumero[0][h]=1;   
    break;

    case 8:
      for(int f=0;f<7;f++){
        matrizNumero[f][1]=1;
        matrizNumero[f][4]=1;
      }
      matrizNumero[0][2]=1;
      matrizNumero[0][3]=1;
      matrizNumero[6][2]=1;
      matrizNumero[6][3]=1;
      matrizNumero[3][2]=1;
      matrizNumero[3][3]=1;
    break;   

    case 9:
       for (int f=1;f<5;f++){
          matrizNumero[0][f]=1;
          matrizNumero[3][f]=1;
        }
          matrizNumero[1][4]=1;
          matrizNumero[2][4]=1;
          matrizNumero[4][4]=1;
          matrizNumero[5][4]=1;
          matrizNumero[6][4]=1;
          matrizNumero[1][1]=1;
          matrizNumero[2][1]=1;
    break;

  }
}

static void limpiaMatrizNumero(){
  
  for(int i=0;i<8;i++)
  {
    for(int j=0;j<23;j++)
    {
      matrizNumero[i][j]=0; 
    }
  } 
}

static void limpiaMatrizLEDS(){
  pthread_mutex_lock (&matriz_mutex);
  for(int i=0;i<8;i++){
    for(int j=0;j<23;j++){
      matrizDeLeds[i][j]=0; 
    }
  }
  pthread_mutex_unlock (&matriz_mutex); 
}

static void incluirDigitos(int indice){
  switch (indice) {
    case 1:
      pthread_mutex_lock (&matriz_mutex);
      for(int i=0;i<8;i++){
        for(int j=0;j<5;j++){
          matrizDeLeds[i][j]=matrizNumero[i][j];
        }
      }
      pthread_mutex_unlock (&matriz_mutex); 
    break;

    case 2:
      pthread_mutex_lock (&matriz_mutex);
      for(int i=0;i<8;i++){
          for(int j=0;j<5;j++){
            matrizDeLeds[i][j+5]=matrizNumero[i][j];
          }
        }
      pthread_mutex_unlock (&matriz_mutex); 
    break;

    case 3:
      pthread_mutex_lock (&matriz_mutex);
      for(int i=0;i<8;i++){
          for(int j=0;j<5;j++){
            matrizDeLeds[i][j+12]=matrizNumero[i][j];
          }
        }
      pthread_mutex_unlock (&matriz_mutex); 
    break;

    case 4:
      pthread_mutex_lock (&matriz_mutex);
      for(int i=0;i<8;i++){
          for(int j=0;j<5;j++){
            matrizDeLeds[i][j+17]=matrizNumero[i][j];
          }
        }
      pthread_mutex_unlock (&matriz_mutex); 
    break;
  }
}

static void* simulaSensor(void* arg){
  struct timeval next_activation;
  struct timeval period = { 0, 111000 };

  gettimeofday (&next_activation, NULL);
  while (1) {
    delay_until (&next_activation);
    timeval_add (&next_activation, &next_activation, &period);

    flag_sensor='1';
  }
  return NULL;
}

int main (void){

  limpiaMatrizNumero();
  limpiaMatrizLEDS();

  minutosAux = 00;
  horaAux = 00;

  time_t hora;
  struct tm *tiempo;
  hora = time(NULL);
  tiempo = localtime(&hora);
  t.hora = tiempo->tm_hour;
  t.minutos = tiempo->tm_min;

  flag_sensor = '0'; //ESCRITURA ATÓMICA EN CHAR, NOS EVITAMOS UN MUTEX

  screen_init(2);

  pthread_t modificador, temporizador, reloj, sensor;
  void* ret;

  init_mutex (&horario_mutex, 4);
  init_mutex (&matriz_mutex, 4);

  create_task (&modificador, modificarHora , NULL, 1000 , 1, 1024);
  create_task (&temporizador, contadorTiempo , NULL, 60000, 2, 1024);
  create_task (&reloj, pintarHora , NULL, 2 , 4, 1024);
  create_task (&sensor, simulaSensor , NULL, 111, 3, 1024);


  pthread_join(modificador, &ret);
  pthread_join(temporizador, &ret);
  pthread_join(reloj, &ret);
  pthread_join(sensor, &ret);
  return 0;
}
