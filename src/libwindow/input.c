

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





#include <stdio.h>
#include <starbase.c.h>
#include <windows.c.h>

#define MIN_PRESSURE 0.5

/*** Global store of wacom fildes ***/
int wacom_id;

/*** Initializing Routines ***/



/****
*****  InitMouse
*****  =========
*****  This function calls Openmouse
*****  and returns an InDev structure
*****  that desribes its attributes.
****/

struct InDev *InitMouse(string,wide, tall)     /* v 1.0b */
     char *string;
     int wide, tall;
{
  struct InDev *locator = NULL;
  int mouse;

  mouse = OpenLocator(string, wide, tall);

  set_p1_p2(mouse, FRACTIONAL, 0.2, 0.2, 0.2, .8, 0.8, 0.8);
  if (mouse >= 0)
    {
      locator = (struct InDev *)malloc(sizeof(struct InDev));
      locator->fildes = mouse;
      locator->width  = wide;
      locator->height = tall;
      locator->buttons = 2;
    }

  return(locator);
}



/****
*****  InitWacom
*****  =========
*****  This function calls Openwacom
*****  and returns an InDev structure
*****  that desribes its attributes.
****/

struct InDev *Wacom(wide, tall)     /* v 1.0b */
     int wide, tall;
{
   struct InDev *locator = NULL;
   int wacom;

   wacom = OpenWacom("/dev/tty00/", wide, tall);

   if (wacom >= 0) {
      locator = (struct InDev *)malloc(sizeof(struct InDev));
      locator->fildes = wacom;
      locator->width  = wide;
      locator->height = tall;
      locator->buttons = 0;
      }

   wacom_id = wacom;
   return(locator);
}



/****
*****  InitKnob1
*****  =========
*****  This function calls Openknob1
*****  and returns an InDev structure
*****  that desribes its attributes.
****/

struct InDev *InitKnob1(wide, tall)     /* v 1.0b */
     int wide, tall;
{
  struct InDev *locator;

  locator = (struct InDev *)malloc(sizeof(struct InDev));
  locator->fildes = OpenLocator("/dev/knob1/", wide, tall);
  locator->width  = wide;
  locator->height = tall;
  locator->buttons = 0;

   return(locator);
}

struct InDev *InitKnob2(wide, tall)     /* v 1.0b */
     int wide, tall;
{
  struct InDev *locator;

  locator = (struct InDev *)malloc(sizeof(struct InDev));
  locator->fildes = OpenLocator("/dev/knob2/", wide, tall);
  locator->width  = wide;
  locator->height = tall;
  locator->buttons = 0;

   return(locator);
}

struct InDev *InitKnob3(wide, tall)     /* v 1.0b */
     int wide, tall;
{
  struct InDev *locator;

  locator = (struct InDev *)malloc(sizeof(struct InDev));
  locator->fildes = OpenLocator("/dev/knob3/", wide, tall);
  locator->width  = wide;
  locator->height = tall;
  locator->buttons = 0;

  return(locator);
}



/****
*****  InitTablet
*****  ==========
*****  This function calls OpenLocator
*****  and returns an InDev structure
*****  that desribes its attributes.
****/

struct InDev *InitTablet(wide, tall)     /* v 1.0b */
     int wide, tall;
{
  struct InDev *locator;

  locator = (struct InDev *)malloc(sizeof(struct InDev));
  locator->fildes = OpenLocator("/dev/tablet/", wide, tall);
  locator->width  = wide;
  locator->height = tall;

  locator->buttons = 1;
  return(locator);
}



/****
*****  OpenLocator
*****  ===========
*****  This opens the starbase
*****  input device and returns
*****  a file descriptor.
****/

OpenLocator(name, wide, tall)     /* v 1.0b */
     char *name;
     int wide, tall;
{
  int locator, dummy = 0;

  locator = gopen(name, INDEV, "hp-hil", 0);
  gescape(locator, TRIGGER_ON_RELEASE, dummy, dummy);
  mapping_mode(locator, TRUE);
  vdc_extent(locator, (float)-20.0, (float)tall, (float)0, (float)wide-1, -20.0, (float)1);
  return(locator);
}




/****
*****  Locator
*****  Handling
*****  Routines
****/

ReadButton(locator)     /* v 1.0b */
     int locator;
{
  int valid, value;
  sample_choice(locator, 1, &valid, &value);
  return(value);
}



ReadAll(locator, a, b, p)     /* v 1.0b */
     int locator, *a, *b;
     float *p;
{
  int valid, value, button;
  float fx, fy, fz;

  if (locator == wacom_id)
    {
      sample_wacom(locator, &fx, &fy, p, &button, &valid);
      *a = (fx+.5);
      *b = (fy+.5);
      return (button);
    }
  else
    {
      sample_locator(locator, 1, &valid, &fx, &fy, &fz);
      sample_choice(locator, 1, &valid, &value);
      *a = (fx+.5);
      *b = (fy+.5);
      if (value) *p = 1.0;
      else *p = 0.0;
   return(value);
    }
}



ReadLoc(locator, a, b)     /* v 1.0b */
     int locator, *a, *b;
{
  int valid;
  float fx, fy, fz;

  sample_locator(locator, 1, &valid, &fx, &fy, &fz);
  *a = (fx+.5);
  *b = (fy+.5);
}



ReadLoc3(locator, a, b, c)     /* v 1.0b */
     int locator, *a, *b, *c;
{
  int valid;
  float fx, fy, fz;
  sample_locator(locator, 1, &valid, &fx, &fy, &fz);
  *a = (fx+.5);
  *b = (fy+.5);
  *c = (fz+.5);
}



MoveLoc(locator, x, y)     /* v 1.0b */
     int locator, x, y;
{
  int valid;
  float fx, fy, fz;

  if (locator == wacom_id) return(0);
  fx = x;
  fy = y;
  fz = 0;
  set_locator(locator, 1, fx, fy, fz);
}



MoveLoc3(locator, x, y, z)     /* v 1.0b */
     int locator, x, y, z;
{
  int valid;
  float fx, fy, fz;
  fx = x;
  fy = y;
  fz = z;
  set_locator(locator, 1, fx, fy, fz);
}



GetPoint(device, display, x, y, button)     /* v 1.0b */
     int device, display, *x, *y, *button;
{
  int value, valid;
  float fx, fy, fz, ox, oy;

  sample_choice(device, 1, &valid, &value);
  while(value<=0)
    {
      sample_locator(device, 1, &valid, &fx, &fy, &fz);
      sample_choice(device, 1, &valid, &value);
      if ((fx!=ox)||(fy!=oy))
	echo_update(display, 0, fx, fy, fz);
      ox = fx;
      oy = fy;
    }
  *x = fx;
  *y = fy;
  *button = abs(value);
}


SetMouseSpeed(mouse,speed)
int mouse, speed;
{
  switch(speed) {
  case (0):
    set_p1_p2(mouse, FRACTIONAL, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    break;
  case (1):
    set_p1_p2(mouse, FRACTIONAL, 0.2, 0.2, 0.2, .8, 0.8, 0.8);
    break;
  case (2):
    set_p1_p2(mouse, FRACTIONAL, 0.3, 0.3, 0.3, .7, 0.7, 0.7);
    break;
  case (3):
    set_p1_p2(mouse, FRACTIONAL, 0.4, 0.4, 0.4, .6, 0.6, 0.6);
    break;
  }
}
