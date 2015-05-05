#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include "screen.h"
#include "tasks.h"

static
void*
refresh_screen (void* arg)
{
  struct timeval next_activation;
  struct timeval now, timeout;
  struct timeval period = { 0, 100000 };

  gettimeofday (&next_activation, NULL);
  while (1) {
    gettimeofday (&now, NULL);
    timeval_sub (&timeout, &next_activation, &now);
    select (0, NULL, NULL, NULL, &timeout) ;
    timeval_add (&next_activation, &next_activation, &period);

    screen_refresh();
  }
}


static struct termios oldtc, newtc;
static char* screen;
static int columns;
static int lines;

static pthread_t t_screen;
static pthread_mutex_t m_scr;

static char*
scr (int x, int y)
{
  if (x >= columns)
    x = columns - 1;
  if (y >= lines)
    y = lines - 1;
  return screen + y * (columns + 1) + x;
}

static
int getenv_int (const char *var, int defval)
{
  char* val = getenv (var);
  return val? atoi(val) : defval;
}

void
screen_init (int prio)
{
  int y;
  columns = getenv_int ("COLUMNS", 25);
  lines = getenv_int ("LINES", 10);

  screen = (char *) malloc ((columns + 1) * lines);
  //Pone todos los objetos en blanco.
  memset (screen, ' ', (columns + 1) * lines);

  for (y = 0; y < lines; ++y)
    *scr(columns, y) = '\0';

  init_mutex (&m_scr, prio);
  puts ("\e[2J");

  tcgetattr(0, &oldtc);
  newtc = oldtc;
  newtc.c_lflag &= ~ICANON;
  newtc.c_lflag |= ECHO;
  tcsetattr(0, TCSANOW, &newtc);

  create_task (&t_screen, refresh_screen, NULL, 1000, 1, 1024);
}

void
screen_refresh (void)
{
  int y, x;

  pthread_mutex_lock (&m_scr);
  for (y = 0; y < lines; ++y) {
    for (x=0; x< columns; ++x) {
    printf ("\e[%d;%df%s", y+1, x+1, scr(x,y));
    }
  }
  pthread_mutex_unlock (&m_scr);
  printf ("\e[%d;2f>>> ", lines);
  fflush (stdout);
}

void
screen_printxy (int x, int y, const char* txt)
{
  pthread_mutex_lock (&m_scr);
  snprintf (scr(x,y), columns - x, "%s", txt);
  pthread_mutex_unlock (&m_scr);
}

int
screen_getchar (void)
{
  fd_set rds;
  struct timeval t = {0, 0};
  int ch = 0;

  FD_ZERO (&rds);
  FD_SET (0, &rds);
  pthread_mutex_lock (&m_scr);
  if (select (1, &rds, NULL, NULL, &t) > 0) {
    ch = getchar();
  }
  pthread_mutex_unlock (&m_scr);
  return ch;
}
