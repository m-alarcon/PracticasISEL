#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include "fsm.h"
#include "main.h"
#include <pthread.h>

#define GPIO_BUTTON	2
#define GPIO_LED	3
#define GPIO_CUP	4
#define GPIO_COFFEE	5
#define GPIO_MILK	6
#define GPIO_MONEDERO	7
#define GPIO_DEVOLVER   8

#define CUP_TIME	250
#define COFFEE_TIME     3000
#define MILK_TIME	3000

#define PRECIO		50

enum mon_state {
  MON_WAITING,
  MON_NODINERO,
  MON_DINERO,
};

enum cofm_state {
  COFM_WAITING,
  COFM_CUP,
  COFM_COFFEE,
  COFM_MILK,
};

//Declaración de variables sincronización fsm
static int hayDinero = 0;
static pthread_mutex_t hayDinero_mutex;
static int dinero = 0;
static int cafe = 0;
static pthread_mutex_t cafe_mutex;
static int moneda_introd = 0;

static int moneda = 0;
static void moneda_isr (void) { moneda = 1; }

static int button = 0;
static void button_isr (void) { button = 1; }

static int bot_devolver = 0;
static void devolver_isr (void) { bot_devolver = 1; }

static int timer = 0;
static void timer_isr (union sigval arg) { timer = 1; }
static void timer_start (int ms)
{
  timer_t timerid;
  struct itimerspec value;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  value.it_value.tv_sec = ms / 1000;
  value.it_value.tv_nsec = (ms % 1000) * 1000000;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &value, NULL);
}

static int mon_introd (fsm_t* this)
{
  int ret = moneda;
  dinero = moneda_introd;
  moneda = 0;
  moneda_introd = 0;
  return ret;
}

static int esperando_dinero (fsm_t* this)
{
  DEBUG({printf ("La variable bot_devolver es: %i\n", bot_devolver);})
  dinero += moneda_introd;
  moneda_introd = 0;
  DEBUG({printf ("La variable dinero es: %i \n", dinero);})
  if (dinero >= PRECIO && bot_devolver == 0){
    DEBUG({printf ("Se ha metido dinero suficiente \n");})
    return 1;
  } else if (bot_devolver == 1) {
    return 1;
  } else {
    return 0;
  }
}

static int devolver = 0;
static int haciendo_cafe = 0;
static pthread_mutex_t haciendo_cafe_mutex;


static int devolver_cambio (fsm_t* this){
  dinero += moneda_introd;

//  pthread_mutex_lock (&cafe_mutex);
//  pthread_mutex_lock (&haciendo_cafe_mutex);

   if (dinero > PRECIO && cafe == 1){
    devolver = dinero - PRECIO;
    DEBUG({printf ("Hay que devolver: %i cents\n", devolver);})
    dinero = 0;
    devolver = 0;
    return 1;
  } else if (dinero == PRECIO && cafe == 1){
    DEBUG({printf ("No hay que devolver nada\n");})
    dinero = 0;
    return 1;
  } else if (bot_devolver == 1 && haciendo_cafe == 1) {
    devolver = dinero - PRECIO;
    DEBUG({printf ("Hay que devolver: %i cents\n", devolver);})
    dinero = 0;
    bot_devolver = 0;
    return 0;
  } else if (dinero < PRECIO && cafe == 1){
    devolver = dinero;
    dinero = 0;
    DEBUG({printf ("Se ha hecho el cafe y hay que devolver %i cents\n", devolver);})
    return 1;
  } else if (bot_devolver == 1){
    devolver = dinero;
    DEBUG({printf ("Hay que devolver: %i cents\n", devolver);})
    dinero = 0;
    bot_devolver = 0;
    return 1;
  } else {
    return 0;
  }
//  pthread_mutex_unlock (&cafe_mutex);
//  pthread_mutex_unlock (&haciendo_cafe_mutex);

}

static int button_pressed (fsm_t* this){

//    pthread_mutex_lock (&hayDinero_mutex);
//    pthread_mutex_lock (&haciendo_cafe_mutex);

  if (hayDinero == 1 && button == 1 && bot_devolver == 0){
    int ret = button;
    haciendo_cafe = 1;
    hayDinero = 0;
    button = 0;
    return ret;
  } else {
    return 0;
  }
//    pthread_mutex_unlock (&hayDinero_mutex);
//    pthread_mutex_unlock (&haciendo_cafe_mutex);

}

static int timer_finished (fsm_t* this)
{
  int ret = timer;
  timer = 0;
  return ret;
}

static void cup (fsm_t* this)
{
  digitalWrite (GPIO_LED, LOW);
  digitalWrite (GPIO_CUP, HIGH);
  timer_start (CUP_TIME);
}

static void coffee (fsm_t* this)
{
  digitalWrite (GPIO_CUP, LOW);
  digitalWrite (GPIO_COFFEE, HIGH);
  timer_start (COFFEE_TIME);
}

static void milk (fsm_t* this)
{
  digitalWrite (GPIO_COFFEE, LOW);
  digitalWrite (GPIO_MILK, HIGH);
  timer_start (MILK_TIME);
}

static void finish (fsm_t* this)
{
  digitalWrite (GPIO_MILK, LOW);
  digitalWrite (GPIO_LED, HIGH);
//  pthread_mutex_lock (&cafe_mutex);
  cafe = 1;
//  pthread_mutex_unlock (&cafe_mutex);
}

static void hay_dinero (fsm_t* this)
{
//  pthread_mutex_lock (&hayDinero_mutex);
  hayDinero = 1;
//  pthread_mutex_unlock (&hayDinero_mutex);
}

static void cafe_servido (fsm_t* this)
{
//    pthread_mutex_lock (&hayDinero_mutex);
    hayDinero = 0;
//    pthread_mutex_unlock (&hayDinero_mutex);
//    pthread_mutex_lock (&cafe_mutex);
    cafe = 0;
//    pthread_mutex_unlock (&cafe_mutex);
//    pthread_mutex_lock (&haciendo_cafe_mutex);
    haciendo_cafe = 0;
//    pthread_mutex_unlock (&haciendo_cafe_mutex);
}

// Descripción fsm monedero
static fsm_trans_t monedero[] = {
  { MON_WAITING,  mon_introd,       MON_NODINERO,              },
  { MON_NODINERO, esperando_dinero, MON_DINERO,   hay_dinero   },
  { MON_DINERO,   devolver_cambio,  MON_WAITING,  cafe_servido },
  { -1, NULL, -1, NULL },
};

// Explicit FSM description
static fsm_trans_t cofm[] = {
  { COFM_WAITING, button_pressed, COFM_CUP,     cup    },
  { COFM_CUP,     timer_finished, COFM_COFFEE,  coffee },
  { COFM_COFFEE,  timer_finished, COFM_MILK,    milk   },
  { COFM_MILK,    timer_finished, COFM_WAITING, finish },
  {-1, NULL, -1, NULL },
};


// Utility functions, should be elsewhere

// res = a - b
void
timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    --res->tv_sec;
    res->tv_usec += 1000000;
  }
}

// res = a + b
void
timeval_add (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_usec / 1000000 + b->tv_usec / 1000000;
  res->tv_usec = a->tv_usec % 1000000 + b->tv_usec % 1000000;
}

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}

static void create_task (pthread_t* tid, void *(*f)(void *), void* arg,
	 int period_ms, int prio, int stacksize){
  pthread_attr_t attr;
  struct sched_param sparam;
  struct timespec next_activation;
  struct timespec period = { 0, 0 };

  sparam.sched_priority = sched_get_priority_min (SCHED_FIFO) + prio;
  clock_gettime (CLOCK_REALTIME, &next_activation);
  next_activation.tv_sec += 1;
  period.tv_sec  = period_ms / 1000;
  period.tv_nsec = (period_ms % 1000) * 1000;

  pthread_attr_init (&attr);
  pthread_attr_setstacksize (&attr, stacksize);
  pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
  pthread_attr_setschedparam (&attr, &sparam);
  pthread_create (tid, &attr, f, arg);
  pthread_make_periodic_np (pthread_self(), &next_activation, &period);
}

static void init_mutex (pthread_mutex_t* m, int prioceiling)
{
  pthread_mutexattr_t attr;
  pthread_mutexattr_init (&attr);
  // pthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_PROTECT);
  pthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_INHERIT);
  pthread_mutex_init (m, &attr);
  // pthread_mutex_setprioceiling
  //   (&m, sched_get_priority_min (SCHED_FIFO) + prioceiling);
}

fsm_t* mon_fsm;
fsm_t* cofm_fsm;

static void* mon_fire (void* arg)
{
  while (scanf("%d %d %d %d %d", &button, &moneda, &moneda_introd, &timer, &bot_devolver) == 5) {
    printf ("El estado del monedero es: %i \n", mon_fsm->current_state);
    pthread_wait_np ((unsigned long*) arg);
    pthread_mutex_lock (&hayDinero_mutex);
    pthread_mutex_lock (&cafe_mutex);
    pthread_mutex_lock (&haciendo_cafe_mutex);
    fsm_fire(mon_fsm);
    pthread_mutex_unlock (&hayDinero_mutex);
    pthread_mutex_unlock (&cafe_mutex);
    pthread_mutex_unlock (&haciendo_cafe_mutex);
  }
  return 0;
}

static void* cofm_fire (void* arg)
{
  while (scanf("%d %d %d %d %d", &button, &moneda, &moneda_introd, &timer, &bot_devolver) == 5) {
    printf ("El estado de la maq de cafe es: %i \n", cofm_fsm->current_state);
    pthread_wait_np ((unsigned long*) arg);
    pthread_mutex_lock (&hayDinero_mutex);
    pthread_mutex_lock (&cafe_mutex);
    pthread_mutex_lock (&haciendo_cafe_mutex);
    fsm_fire(cofm_fsm);
    pthread_mutex_unlock (&hayDinero_mutex);
    pthread_mutex_unlock (&cafe_mutex);
    pthread_mutex_unlock (&haciendo_cafe_mutex);
  }
  return 0;
}

int main (){

  mon_fsm  = fsm_new (monedero);
  cofm_fsm = fsm_new (cofm);

  pthread_t coffee, money;
  void* ret;

  init_mutex (&hayDinero_mutex, 2);
  init_mutex (&cafe_mutex, 1);
  init_mutex (&haciendo_cafe_mutex, 1);

  create_task (&money,  mon_fire,  NULL, 300, 2, 1024);
  create_task (&coffee, cofm_fire, NULL, 300, 1, 1024);

  pthread_join(money, &ret);
  pthread_join(coffee, &ret);

  wiringPiSetup();
  pinMode (GPIO_BUTTON, INPUT);
  wiringPiISR (GPIO_BUTTON, INT_EDGE_FALLING, button_isr);
  pinMode (GPIO_CUP, OUTPUT);
  pinMode (GPIO_COFFEE, OUTPUT);
  pinMode (GPIO_MILK, OUTPUT);
  pinMode (GPIO_LED, OUTPUT);
  digitalWrite (GPIO_LED, HIGH);
  pinMode (GPIO_MONEDERO, INPUT);
  pinMode (GPIO_DEVOLVER, INPUT);
  wiringPiISR (GPIO_MONEDERO, INT_EDGE_FALLING, moneda_isr);
  wiringPiISR (GPIO_DEVOLVER, INT_EDGE_FALLING, devolver_isr);

  return 0;
}
