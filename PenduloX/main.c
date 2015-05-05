#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "tasks.h"
#include "screen.h"
#include "main.h"

static void* pintarX (void* arg){
  struct timeval next_activation;
  struct timeval period = { 0, 2000 };

  gettimeofday (&next_activation, NULL);
  while (1) {
    delay_until (&next_activation);
    timeval_add (&next_activation, &next_activation, &period);
    if (flag_sensor=='1') {
      flag_sensor='0';

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
  matrizDeLeds[3][11]=1;
  matrizDeLeds[2][10]=1;
  matrizDeLeds[2][12]=1;
  matrizDeLeds[1][9]=1;
  matrizDeLeds[1][13]=1;
  matrizDeLeds[0][8]=1;
  matrizDeLeds[0][14]=1;
  matrizDeLeds[4][10]=1;
  matrizDeLeds[4][12]=1;
  matrizDeLeds[5][9]=1;
  matrizDeLeds[5][13]=1;
  matrizDeLeds[6][8]=1;
  matrizDeLeds[6][14]=1;
}

static void pinta_columna(int columna){

  if(matrizDeLeds[0][columna]==1)
    screen_printxy(columna,0,"*");
  else
    screen_printxy(columna,0," ");
  if(matrizDeLeds[1][columna]==1)
    screen_printxy(columna,1,"*");
  else
    screen_printxy(columna,1," ");
  if(matrizDeLeds[2][columna]==1)
    screen_printxy(columna,2,"*");
  else
    screen_printxy(columna,2," ");
  if(matrizDeLeds[3][columna]==1)
    screen_printxy(columna,3,"*");
  else
    screen_printxy(columna,3," ");
  if(matrizDeLeds[4][columna]==1)
    screen_printxy(columna,4,"*");
  else
    screen_printxy(columna,4," ");
  if(matrizDeLeds[5][columna]==1)
    screen_printxy(columna,5,"*");
  else
    screen_printxy(columna,5," ");
  if(matrizDeLeds[6][columna]==1)
    screen_printxy(columna,6,"*");
  else
    screen_printxy(columna,6," ");
  if(matrizDeLeds[7][columna]==1)
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

static void limpiaMatrizLEDS(){
  for(int i=0;i<8;i++){
    for(int j=0;j<23;j++){
      matrizDeLeds[i][j]=0;
    }
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

  limpiaMatrizLEDS();
  codificarEnMatriz();

  flag_sensor = '0';

  screen_init(2);

  pthread_t equis, sensor;
  void* ret;

  create_task (&equis, pintarX, NULL, 1000 , 2, 1024);
  create_task (&sensor, simulaSensor, NULL, 60000, 1, 1024);

  pthread_join(equis, &ret);
  pthread_join(sensor, &ret);
  return 0;
}
