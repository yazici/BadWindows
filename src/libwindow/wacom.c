
/******************************************************************************
Copyright 1990 by the Massachusetts Institute of Technology.  All 
rights reserved.

Developed by the Visible Language Workshop at the Media Laboratory, MIT, 
Cambridge, Massachusetts, with support from Hewlett Packard, DARPA, and Nynex.

For use by Suguru Ishizaki.  
This distribution is approved by Nicholas Negroponte, Director of 
the Media Laboratory, MIT.

Permission to use, copy, or modify these programs and their 
documentation for educational and research purposes only and 
without fee is hereby granted, provided that this copyright notice 
appears on all copies and supporting documentation.  For any other 
uses of this software, in original or modified form, including but not 
limited to distribution in whole or in part, specific prior permission 
must be obtained from MIT.  These programs shall not be used, 
rewritten, or adapted as the basis of a commercial software or 
hardware product without first obtaining appropriate licenses from 
MIT.  MIT makes no representations about the suitability of this 
software for any purpose.  It is provided "as is" without express or 
implied warranty."
******************************************************************************/




/***********************************************/
/*                                             */
/*     WACOM.C                                 */
/*                                             */
/*      Helpfull utilities for using the       */
/*      Wacom super digitizer SD-210.          */
/*                                             */
/*           -Dave "IO" Small 10/30/88         */
/*                                             */
/***********************************************/

/***********************************************/
/*                                             */
/*  int fildes;                                */
/*  fildes = open_wacom();                     */
/*      opens the port, and initializes the    */
/*      tablet.                                */
/*                                             */
/*                                             */
/*                                             */
/***********************************************/

#include <stdio.h>
#include <termio.h>
#include <sys/types.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define TABLETPORT "/dev/tty00"
#define DEBUG FALSE
#define WACOMXMAX 31750
#define WACOMYMAX 23100
#define MAX(A,B)  ((A) > (B) ? (A) : (B))
#define MIN(A,B)  ((A) < (B) ? (A) : (B))

#define swap(a, b, temp) (temp = a, a = b, b = temp)

static int wacom_wd = 1280, wacom_ht = 1024;
static float threshold = 0.05;

/*** This data was entered by hand from the response ***/
/***   graph provided by Wacom.  It seems to work.   ***/

static float pressure_table[66] = {
  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.01, 0.02,
  0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.1125, 0.125,
  0.1375, 0.15,  0.167, 0.184, 0.2,
  0.217,  0.234, 0.25,  0.275, 0.3,
  0.317,  0.334, 0.35,  0.375, 0.4,
  0.45,   0.5,   0.55,  0.65,  0.75,
  0.8,    0.85,  0.9,   0.95,  1.0, 1.0};



/*
=========================================================================
set_pressure_max(max)
float max;

 resets max pressure for tablet, and scales up the rest of the
values. 0.0 < max < 1.0.
=========================================================================
*/
set_pressure_max(max)     /* v 1.0b */
     float max;
{
  int i;

  for (i = 0; i < 66; i++) {
    pressure_table[i] /= max;
    if (pressure_table[i] > 1.0)
      pressure_table[i] = 1.0;
  }

}



/*
==========================================================================
open_wacom()

Open tablet, get file discriptor, flush the port, set baud rates, etc.
sets coords wide by tall, (0, 0) is the upper left corner
==========================================================================
*/
int OpenWacom(dev, wide, tall)     /* v 1.0b */
     char *dev;
     int wide, tall;
{
  int Tablet, i;
  char string[20];

  /***** Set up RS232 connection from bobcat *****/
  if ((Tablet = ttyutil_open(dev)) == -1)
    return (-1);
  ttyutil_set_speed(Tablet, B19200);
  ttyutil_set_no_stopbits(Tablet, 1);
  ttyutil_set_charsize(Tablet, CS8);
  ttyutil_parity_off(Tablet);

  /**** Set up tablet defaults ********/
  send_wacom_message(Tablet, "TE1", TRUE, 1, 0);
  send_wacom_message(Tablet, "AS1", TRUE, 0, 150000);
  send_wacom_message(Tablet, "AL1", TRUE, 0, 150000);
  send_wacom_message(Tablet, "BA8", TRUE, 0, 150000);
  send_wacom_message(Tablet, "OC1", TRUE, 0, 150000);
  send_wacom_message(Tablet, "PH1", TRUE, 0, 150000);
  send_wacom_message(Tablet, "IT0", TRUE, 0, 150000);
  send_wacom_message(Tablet, "SR", TRUE, 0, 150000);

  wacom_wd = wide;
  wacom_ht = tall;

  sprintf(string, "SC%d,%d", wacom_wd, wacom_ht);
  send_wacom_message(Tablet, string, TRUE, 0, 150000);

  return(Tablet);
}
/* open_wacom */



/*
==========================================================================
newsample_wacom(fildes, x, y, pressure, button, Valid)
int fildes;
float *x, *y, *pressure;
int *button, *Valid;

Samples the tablet and returns x value, y value, button and pressure.
  Valid = 0,  bad sample for x, y, or pressure
  Valid = 1,  good sample
==========================================================================
*/

newsample_wacom(fildes, x, y, pressure, button, Valid)     /* v 1.0b */
     int fildes;
     float *x, *y, *pressure;
     int *button, *Valid;
{
  int numread,done;
  static unsigned char data[100];
  short X, Y, A, sign, RDY, PC, PRSH;
  static float last_x = 0.0, last_y = 0.0, temp;
  int p,count;

/*  ttyutil_clear(fildes); */
  count = 0;

  /* get start */
  data[0] = 0;
  while ((data[0] != '#') && (data[0] != '!') && (data[0] != '*'))  /* && (data[0] != 10)) */
    if (read(fildes, &data[0], 1));  /*  printf("%c %d\n",data[0],data[0]);  */

  while (data[0] != ',')
    if (read(fildes, &data[0], 1));  /*  printf("%c %d\n",data[0],data[0]);  */

  /* get x */
  count = 0;
  data[0] = 0;
  numread = 0;
  done = 0;
  while (!done)
    if (read(fildes, &data[numread], 1))
      {
	if (data[numread] == ',') done = 1;
	else numread++;
      }
    else count++;

  data[numread] = 0;

  X = atoi(data);
  *x = (float)X;
/*  printf("X %s   x %d  wasted %d\n",data,X,count); */

  /* get y */
  count = 0;
  data[0] = 0;
  numread = 0;
  done = 0;
  while (!done)
    if (read(fildes, &data[numread], 1))
      {
	if (data[numread] == ',') done = 1;
	else numread++;
      }
    else count++;

  data[numread] = 0;

  Y = atoi(data);
  *y = (float)Y;
/*  printf("Y %s   y %d  wasted %d\n",data,Y,count); */

  /* get a */
  count = 0;
  data[0] = 0;
  numread = 0;
  done = 0;
  while (!done)
    if (read(fildes, &data[numread], 1))
      {
	if (data[numread] == 13) done = 1;
	else numread++;
      }
    else count++;

  data[numread] = 0;

  A = atoi(data);
  p = A + 30;
  p = MIN(MAX(p, 0),65);

  *pressure = pressure_table[p];
  if (*pressure > 0.0) *button = 1;
  else *button = 0;
  *Valid = 1;

  *x = MIN(MAX(*x,0.0),wacom_wd);
  *y = MIN(MAX(*y,0.0),wacom_ht);

    /*** Check to see if the location jumped - I think that this is
      caused by a bad cable **********/

  if (((last_x - *x) > 50.0) ||
      ((last_y - *y) > 50.0) ||
      ((last_x - *x) < -50.0) ||
      ((last_y - *y) < -50.0))
    {
      *Valid = FALSE; 
      swap(*x,last_x,temp);
      swap(*y, last_y,temp);
      *pressure = 0.0;
      *button = 0;
      /*      printf("The pen jumped! Check the cable!\n"); */
    }
  else {
    last_x = *x;
    last_y = *y;
  }

#if 0
  while ((!done) && (numread < 100))
    if (read(fildes, &data[numread], 1))
      {
/*	printf("%c ",data[numread]); */
	if (data[numread] == 13) done = 1;
	numread ++;
      }
    else count++;

/*  printf("\n");
  printf("%d bad reads\n",count); */

#endif

}
/* newsample_wacom */





/*
==========================================================================
sample_wacom(fildes, x, y, pressure, button, Valid)
int fildes;
float *x, *y, *pressure;
int *button, *Valid;

Samples the tablet and returns x value, y value, button and pressure.
  Valid = 0,  bad sample for x, y, or pressure
  Valid = 1,  good sample
==========================================================================
*/

sample_wacom(fildes, x, y, pressure, button, Valid)     /* v 1.0b */
     int fildes;
     float *x, *y, *pressure;
     int *button, *Valid;
{
  int numread;
  static unsigned char data[7];
  short X, Y, A, sign, RDY, PC, PRSH;
  int count = 0,i;
  static float last_x = 0.0, last_y = 0.0, temp;
  static int first =1;
  int p;

  ttyutil_clear(fildes);

  data[0] = 0;
  while (data[0] < 128) {
    numread = read(fildes, &data[0], 1);
/*    if (numread) printf("read a nonsync byte %d\n",data[0]);  */
  }
  
  while (numread < 7 && count < 1000)
    numread += read(fildes, &data[numread], 7 - numread), count++;

  
/*
  printf("done sample, badcount = %d\n",count);
  printf("info = ");
  for (i = 0; i < 7; i++) printf("%x ",data[i]);
  printf("\n");
*/

  if (numread == 7 && data[0] > 127) {
    RDY = (data[0] & 0x40) >> 6;
    PC = (data[0] & 0x20) >> 5;
    PRSH =  (data[0] & 0x10) >> 4;
    X = data[2] | (data[1] << 7) | ((data[0] & 0x03) << 14);
    Y = data[5] | (data[4] << 7) | ((data[3] & 0x03) << 14);

    *x = (float)X;
    *y = (float)Y;

    /* NOTE: as near as I can tell, the PC bit is ALWAYS on, so you can't
       tell the difference between the puck and the pen */

    if (PC) {     /*** pressure pen or stylus is being used ***/
      sign = (data[6] & 0x40);
      A = (data[6] & 0x7f);
      if (sign) A -= 127;

      p = A + 30;
      if (p < 0) p = 0;
      else if (p > 65) p = 65;

      *pressure = pressure_table[p];
      if (*pressure > 0.0) *button = 1;
      else *button = 0;
    }
    else {     /*** puck ***/
      *button = (int) (data[6] & 0x07);
      if (*button > 0) *pressure = 1.0;
      else *pressure = 0.0;
    }

    /*** Check to see if pen is out of range (returns last good point) ***/
    if (RDY != 1) {
      *Valid = FALSE;
      *x = last_x;
      *y = last_y;
      *pressure = 0.0;
      *button = 0;
    }
    /*** Check to see if the location jumped - I think that this is
      caused by a bad cable **********/
    else if (((last_x - *x) > 50.0) ||
	     ((last_y - *y) > 50.0) ||
	     ((last_x - *x) < -50.0) ||
	     ((last_y - *y) < -50.0)) {
      *Valid = FALSE;
      swap (*x, last_x, temp);
      swap (*y, last_y, temp);
      *pressure = 0.0;
      *button = 0;
      /*printf("The pen jumped! Check the cable!\n");*/
    }
    /*** A good read ***/
    else {
      *Valid = TRUE;
      last_x = *x;
      last_y = *y;
    }
  }
  /*** Timed out read on RS-232, return last good point ***/
  else {
    ttyutil_clear(fildes);
    *Valid = FALSE;
    *x = last_x;
    *y = last_y;
    *pressure = 0.0;
    *button = 0;
    if (FALSE) printf("Error: numread = %d, count = %d\n", numread, count);
  }



}
/* sample_wacom */




send_wacom_message(fildes, input, terminate, sec, microsec)     /* v 1.0b */
     int fildes;
     char *input;
     int terminate, sec, microsec;
{
  char string[20];

  if (terminate) {
    sprintf(string, "%s%c", input, 13);
    ttyutil_write(fildes, string, strlen(string), 0);
  }
  else
    ttyutil_write(fildes, input, strlen(input), 0);

  please_wait(sec, microsec);
}



please_wait(sec, microsec)     /* v 1.0b */
     int sec, microsec;
{
  struct timeval to, realval;

  realval.tv_sec = sec;
  realval.tv_usec = microsec;

  to = realval;
  select(0, 0, 0, 0, &to); /* using select for 200 millisecond delay */
}

