

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





/***
****  PALETTE.C
****  by
****  David Small
****  12/3/88
***/

#include <stdio.h>
#include <starbase.c.h>
#include <windows.c.h>
#include <structures.h>

/* #include <astructs.h> */
#include <davestructs.h>

#define  ICONSDIR "icons

extern int CurrentRed, CurrentGreen, CurrentBlue;
extern float CurrentHue, CurrentSat, CurrentVal;



MakeHandyPalette(Root, BobWin, echo, x, y)     /* v 1.0b */
struct Window *Root, *BobWin;
struct curstruct *echo;
int x, y;
{
  struct Window *NotePane, *CloseEditPane;
  struct Window *SizerWin, *CloserWin, *DragBar;
  struct Window *HandyWin;
  struct Window *addcolor, *file, *menu, *item;
  struct Window *loadcolor, *savecolor;
  struct OutDev *display;
  struct InDev  *locator;
  struct HandyPaletteStruct *params, *HandyPaletter();
  int xx, w;

  display = Root->display;
  locator = Root->input;

  /* Create EditPane with child windows */
  NotePane = MakeWindow(display, locator, Root, x, y, 218, 85, "NotePane");
  RoundRectify(NotePane, VDARK, VDARK, VDARK, 2, NULL, NULL, NULL, 0.015);
  EchoWrap(NotePane, echo, 1, 0, 0);
  InstallWin(NotePane);

  HandyWin = MakeWindow(display, locator, NotePane, 951, 6, 218, 100, "HandyWin");
  params = HandyPaletter(HandyWin, Root, BobWin, echo);
  EchoWrap(HandyWin, echo, 1, 0, 0);
  Maker(HandyWin, 5, 60, -25, -5);
  InstallWin(HandyWin);

  /*** Create Sub windows ***/

  /*** E D I T  W I N D O W S ***/

  set_font("arch", 24, 8);
  w = string_width("Add Color")+4;
  xx = 8+w+10;
  addcolor = (struct Window *)MakeWindow(Root->display, Root->input, NotePane,
					 8, 24, w, 18, "Addcolor Menu");
  StringWin(addcolor, "Add Color", 0, 0, "arch", 24, 8, 90, 88, 95, 0);
  Messenger(addcolor, DO, HandyWin, ADDCOLOR, NULL, 1);
  EchoWrap(addcolor, echo, 1, 0, 0);
  InstallWin(addcolor);

  w = string_width("Load")+4;
  loadcolor = (struct Window *)MakeWindow(Root->display, Root->input, NotePane,
					  xx, 24, w, 18, "Loadcolor Menu");
  StringWin(loadcolor, "Load", 0, 0, "arch", 24, 8, 90, 88, 95, 0);
  Messenger(loadcolor, DO, HandyWin, LOAD, NULL, 1);
  EchoWrap(loadcolor, echo, 1, 0, 0);
  InstallWin(loadcolor);
  xx += w+10;

  w = string_width("Save")+4;
  savecolor = (struct Window *)MakeWindow(Root->display, Root->input, NotePane,
					  xx, 24, w, 18, "Savecolor Menu");
  StringWin(savecolor, "Save", 0, 0, "arch", 24, 8, 90, 88, 95, 0);
  Messenger(savecolor, DO, HandyWin, SAVE, NULL, 1);
  EchoWrap(savecolor, echo, 1, 0, 0);
  InstallWin(savecolor);
  xx += w+10;

  /* default windows */
  DragBar = MakeWindow(display, locator, NotePane, 0, 0, 20, 20, "DragBar");
  RoundRectify(DragBar, DRAG_BAR_COLOR, 0, 0, 0, 0, .01);
  MoverII(DragBar, NotePane, 0);
  EchoWrap(DragBar, echo, 1, 0, 0);
  Maker(DragBar, 5, 5, -25, 20);
  InstallWin(DragBar);

  CloserWin = MakeWindow(display, locator, NotePane, 0, 0, 20, 20, "CloserWin");
  RoundRectify(CloserWin, CLOSER_COLOR, 0, 0, 0, 0, .01);
  StringWin(CloserWin, "X", 2, -2, "arch", 24, 8, 0, 0, 0, 0);
  Closer(CloserWin, NotePane);
  EchoWrap(CloserWin, echo, 1, 0, 0);
  Maker(CloserWin, -20, 5, -5, 20);
  InstallWin(CloserWin);

  SizerWin = MakeWindow(display, locator, NotePane, 0, 0, 15, 15, "SizerWin");
  MakeIcon(SizerWin, ICONSDIR/resize_icon", 8, 82);
   ResizerPlus(SizerWin, NotePane, 80, 80, 1300, 1100);
   EchoWrap(SizerWin, echo, 7, 0, 0);
   Maker(SizerWin, -20, -20, -5, -5);
   InstallWin(SizerWin);

   /***  Transform it  ***/
   /*** We don't want to see the NotePane at first ***/

   TransformCoords(NotePane, NotePane->relx, NotePane->rely,
		   NotePane->width, NotePane->height);
 }



struct HandyPaletteStruct *HandyPaletter(W, Root, BobWin, echo)     /* v 1.0b */
struct Window *W, *Root, *BobWin;
struct curstruct *echo;
{
  int HandyPalette();
  struct ColorStruct *clrstruct;
  struct HandyPaletteStruct *params;

  params = (struct HandyPaletteStruct *) malloc(sizeof(struct HandyPaletteStruct));
  params->cursor = echo;
  params->BobWin = BobWin;
  params->Root = Root;
  params->numcolors = 0;
  params->current_color = -1;

  clrstruct = (struct ColorStruct *) malloc (sizeof(struct ColorStruct)*500);
  params->colors = clrstruct;

  AttachCommand(W, DO, HandyPalette, (char *)params);
  AttachCommand(W, DRAW, HandyPalette, (char *)params);
  AttachCommand(W, ADDCOLOR, HandyPalette, (char *)params);
  AttachCommand(W, DELETECOLOR, HandyPalette, (char *)params);
  AttachCommand(W, SAVE, HandyPalette, (char *)params);
  AttachCommand(W, LOAD, HandyPalette, (char *)params);

  return(params);
}



HandyPalette(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  int x, y, button;
  struct curstruct *echo;
  struct pickstruct *pick;
  struct HandyPaletteStruct *params;
  struct clipstruct *cstruct;
  float cx1, cx2, cy1, cy2;
  int screen, I, row, size, num, curr;
  float x1, x2, y1, y2;
  struct ColorStruct *color;

  params = (struct HandyPaletteStruct *)stuff;
  screen = (W->display)->fildes;
  color = params->colors;
  num = params->numcolors;
  curr = params->current_color;

  switch (id) {
  case (DRAW):
    cstruct = (struct clipstruct *)data;
    cx1 = cstruct->x1;  cy1 = cstruct->y1;
    cx2 = cstruct->x2;  cy2 = cstruct->y2;
    clip_rectangle(screen, cx1, cx2, cy1, cy2);

    interior_style(screen, 1, 1);
    perimeter_color(screen, 0.0, 0.0, 0.0);
    fill_color(screen, 0.5, 0.5, 0.5);
    rectangle(screen, (float) W->x1, (float) W->y1,
	      (float) W->x2, (float) W->y2);

    size = get_size(W, num);

    row = W->width / size;

    for (I = 0; I < num; I++) {
      x1 = ((float) W->x1 + (I % row) * size + 2);
      x2 = ((float) W->x1 + (I % row) * size + size - 2);
      y1 = ((float) W->y1 + (I / row) * size + 2);
      y2 = ((float) W->y1 + (I / row) * size + size - 2);

      if (I == curr) {
	fill_color(screen, 1.0, 1.0, 1.0);
	rectangle(screen, x1 - 2, y1 - 2, x2 + 2, y2 + 2);
      }
      fill_color(screen, (color+I)->R / 255.0,
		 (color+I)->G / 255.0, (color+I)->B / 255.0);
      rectangle(screen, x1, y1, x2, y2);
    }

    clip_rectangle(screen, 0.0, 2047.0, 0.0, 1023.0);
    make_picture_current(screen);
    break;
  case (DO):
    echo = params->cursor;
    pick = (struct pickstruct *)data;
    x = pick->x;
    y = pick->y;
    button = pick->button;

    switch (button) {
    case (JUSTDOWN):
      curr = get_color_index(W, num, x, y);
      if (curr >= 0 && (curr != params->current_color)) {
	CurrentHue = (color+curr)->Hue;
	CurrentSat = (color+curr)->Sat;
	CurrentVal = (color+curr)->Val;
	CurrentRed = (color+curr)->R;
	CurrentGreen = (color+curr)->G;
	CurrentBlue = (color+curr)->B;
	params->current_color = curr;
	UpdateWin(W);
	UpdateWin(params->BobWin);
	if (set_new_color != NULL)
	  (*set_new_color)();
      }
      break;
    case (BEENUP):
      break;
    case (JUSTUP):
      break;
    case (BEENDOWN):
      break;
    }
    break;
  case (ADDCOLOR):
    printf("Adding (%d, %d, %d) to list.\n",
	   CurrentRed, CurrentGreen, CurrentBlue);
    (color+(num))->Hue = CurrentHue;
    (color+(num))->Sat = CurrentSat;
    (color+(num))->Val = CurrentVal;
    (color+(num))->R = CurrentRed;
    (color+(num))->G = CurrentGreen;
    (color+(num))->B = CurrentBlue;
    curr = num;
    num += 1;
/*    params->colors = color;*/
    params->numcolors = num;
    params->current_color = curr;
    UpdateWin(W);
    break;
  case (DELETECOLOR):
    delete_color(color, num, curr);
    UpdateWin(W);
    break;
  case (SAVE):
    SavePalette(W->parent, params);
    break;
  case (LOAD):
    LoadPalette(W->parent, params);
    break;

  }

  return(0);
}



delete_color(colors, numcolors, current_color)     /* v 1.0b */
     struct ColorStruct *colors;
     int current_color, numcolors;
{
  int I;

  for (I = current_color; I < numcolors; I++)
    {
      (colors+I)->R = (colors+(I+1))->R;
      (colors+I)->G = (colors+(I+1))->G;
      (colors+I)->B = (colors+(I+1))->B;
      (colors+I)->Hue = (colors+(I+1))->Hue;
      (colors+I)->Sat = (colors+(I+1))->Sat;
      (colors+I)->Val = (colors+(I+1))->Val;
    }

  numcolors -= 1;
}



int get_color_index(W, numcolors, x, y)     /* v 1.0b */
     struct Window *W;
     int numcolors;
     int x, y;
{
  int size, row, I;

  size = get_size(W, numcolors);

  row = W->width / size;

  I = ((y - W->y1) / size * row) +
    ((x - W->x1) / size);

  if (I >= numcolors) return (-1);
  /*  else if ((y - W->y1) > size * (I / row)) return (-1);*/
  else if ((x - W->x1) > size * row) return (-1);
  else return (I);
}



int get_size(W, numcolors)     /* v 1.0b */
     struct Window *W;
     int numcolors;
{
  int size;

  if (numcolors * 60 * 60 < W->width * (W->height - 60))
    size = 60;
  else if (numcolors * 50 * 50 < W->width * (W->height - 50))
    size = 50;
  else if (numcolors * 40 * 40 < W->width * (W->height - 40))
    size = 40;
  else if (numcolors * 30 * 30 < W->width * (W->height - 30))
    size = 30;
  else if (numcolors * 25 * 25 < W->width * (W->height - 25))
    size = 25;
  else if (numcolors * 20 * 20 < W->width * (W->height - 20))
    size = 20;
  else if (numcolors * 15 * 15 < W->width * (W->height - 15))
    size = 15;
  else
    size = 10;

  return (size);
}



SavePalette(W, params)     /* v 1.0b */
     struct Window *W;
     struct HandyPaletteStruct *params;
{
  struct ColorStruct *color;
  FILE *fp, *fopen();
  char string[100];
  int num;

  color = params->colors;
  if (GetString(W->parent, E_lastx, E_lasty, "Enter filename. (full path)", string, 50))
    {
      if (!(fp = fopen(string, "w")))
	{
	  printf("Can't open %s for output text\n", string);
	}
      else
	{
	  /* save info */
	  fprintf(fp, "%d colors\n", params->numcolors);
	  for (num = 0; num < params->numcolors; num++)
	    fprintf(fp, "%d %d %d %f %f %f\n", (color+num)->R, (color+num)->G, (color+num)->B,
		    (color+num)->Hue, (color+num)->Sat, (color+num)->Val);

	  fclose(fp);
	}
    }
}



LoadPalette(W, params)     /* v 1.0b */
     struct Window *W;
     struct HandyPaletteStruct *params;
{
  FILE *fp, *fp2, *fopen();
  struct ColorStruct *color;
  char string[100];
  int num, i, numread;


  color = params->colors;
  if (GetString(W->parent, E_lastx, E_lasty, "Enter object filename. (full path)", string, 50))
    {
      if ( !(fp = fopen(string, "r")) ) {
	printf("Can't open %s for input text. Sorry.\n", string);
      }
      else {
	/* info */

	fscanf(fp, "%d colors\n", &num);

	for (i = params->numcolors; i< num+params->numcolors; i++) {
	  fgets(string, 50, fp);

	  numread = (sscanf(string, "%d %d %d %f %f %f\n", &((color+i)->R), &((color+i)->G), &((color+i)->B),
			 &((color+i)->Hue), &((color+i)->Sat), &((color+i)->Val)));
	  if (numread < 6) {
	    (color+i)->Hue = 0.5;
	    (color+i)->Sat = 0.0;
	    (color+i)->Val = 0.0;
	  }
	  printf("%d %d %d %f %f %f\n", ((color+i)->R), ((color+i)->G), ((color+i)->B),
			 ((color+i)->Hue), ((color+i)->Sat), ((color+i)->Val));
	}
	params->numcolors += num;
	fclose(fp);
	UpdateWin(W);
      }
    }
}

