/************************************
*                                   *
* NOTES.C                           *
*                                   *
* by Michael J. Bauer               *
*    Laura  H. Robin                *
* date: 21 Nov  1989                *
* version 1.0B                      *
*                                   *
*************************************

*******************************************************************************
*                                                                             *
* NOTES.C creates a color palette with some sketching tools (e.g. pencil,     *
* marker, line width, translucency) and a tool to save your sketch in a       *
* window.                                                                     *
*                                                                             *
*                                                                             *
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <starbase.c.h>
#include <windows.c.h>
#include <structures.h>
#include <palette.h>
#include <sketch.h>

extern unsigned char *ctable,*mcontrol;
extern int CurrentRed, CurrentGreen, CurrentBlue;
extern char *CurrentFont;
extern int CurrentFontSize;
extern struct curstruct *pulldown;

static char line_table[] = "/vlw/data/100.tab";

struct NoteDrawstuffStruct {
  struct Window *FixW, *pallette;
  struct curstruct *echo, *pencil, *marker, *brush;
  struct curstruct *eraser, *spray, *hand, *mini;
  int  trans;
  int  back_red, back_green, back_blue;
  float  thick;
  int  wd, ht;
  int  draw_mode, curs_type;
  unsigned char *r_bank, *g_bank, *b_bank;
  unsigned char *r_paper, *g_paper, *b_paper;
  int pattern_wd, pattern_ht;
  unsigned char *r_pattern, *g_pattern, *b_pattern;
  FILE *dribble;
};

/*====================================================================================
  Notes() is a function which creates a canvas, a palette and all related drawing
  functions.  One function is to make your sketch into a window. 

  This function assumes that you have made a button to invoke the canvas and palette.  The
  code to open the canvas from this button is left up to you.  This gives you the option
  of putting this button into a popup menu, or making it a button on the surface -- each
  of which require different opening routines.  The closing-code is built in to this routine.

/*====================================================================================*/

struct Window *Notes(pointer, CanvasParent, ToolkitParent, x,y,wd,ht, IconW)
     struct curstruct *pointer;
     struct Window *CanvasParent;   /* Parent of canvas */
     struct Window *ToolkitParent; /* Parent of Toolkit - if set to NULL, assumes canvas as parent - DS 9/91 */
     int x, y, wd, ht;                     /* Position and dimensions of the canvas */
     struct Window *IconW;                 /* Window which canvas & palette are invoked from */
{
  struct Window *NoteToolkit, *RandWin, *NoteCanvas;
  struct Window *MarkWin, *PenWin;
  static float maxthick=40.0, minthick=0.01;
  static int maxtrans=255, mintrans=0;
  struct NoteDrawstuffStruct *params, *Notify();
  int OpenAll();
  
  /* Create the clear overlay window, install all do/draw functions on it */
  NoteCanvas = (struct Window *)MakeWindow(CanvasParent->display, CanvasParent->input, CanvasParent, x, y, wd, ht, "NoteCanvas");

  /* The note interface window, with title */                
  if (ToolkitParent == NULL)
    NoteToolkit=(struct Window *)MakeWindow(NoteCanvas->display, NoteCanvas->input, NoteCanvas, 0, 0, 381, 230, "NoteWin");
  else
    NoteToolkit=(struct Window *)MakeWindow(ToolkitParent->display, ToolkitParent->input, ToolkitParent, 0, 0, 381, 230, "NoteWin");

  Notify(NoteCanvas,NoteToolkit);
  InstallWin(NoteCanvas);

/* Grab all pertinent struct info */
  params = (struct NoteDrawstuffStruct *)GetStuff(NoteCanvas, DRAW);

/* Piles and piles of subwindows...  here goes nothing! */
  EchoWrap(NoteToolkit, pointer, 1, 0, 0);
  Rectify(NoteToolkit, 95, 95, 95, 1, 0, 0, 0, 0);
  StringWin(NoteToolkit, "SketchTool", 3, 0, "latin", 24, 8, 0, 0, 0, 0);
  InstallWin(NoteToolkit);
  
/* The drag bar.  */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 100, 3, 315, 15, "Drag Bar");
  Mover(RandWin, NoteToolkit);
  EchoWrap(RandWin, pointer, 1, 0, 0);
  Rectify(RandWin, 127, 127, 127, 1, 0, 0, 0, 0);
  InstallWin(RandWin);


/* MAKE a new window.  Makes current sketch into a window. */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 273, 165, 45, 15, "Sketch2Win");
  Messenger(RandWin, DO, NoteCanvas, MAKEWINDOW, NULL, 7);
  EchoWrap(RandWin, pointer, 1, 0, 0);
  Rectify(RandWin, 127, 127, 127, 1, 0, 0, 0, 0);
  StringWin(RandWin, "Object!", 0, 0, "latin", 16, 8, 0, 0, 0, 0);
  InstallWin(RandWin);

/* CLEAR button.  Clears all notes off the screen, nukes dribble file */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 273, 190, 45, 15, "Clear");
  Messenger(RandWin, DO, NoteCanvas, CLEAR, NULL, 7);
  EchoWrap(RandWin, pointer, 1, 0, 0);
  Rectify(RandWin, 127, 127, 127, 1, 0, 0, 0, 0);
  StringWin(RandWin, "Clear", 3, 0, "latin", 16, 8, 0, 0, 0, 0);
  InstallWin(RandWin);

/* EXIT window.  Cans dribble file, reiconifies note windows. */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 273, 205, 45, 15, "Exit");
  Messenger(RandWin, DO, NoteCanvas, EXIT, NULL, JUSTUP);
  EchoWrap(RandWin, pointer, JUSTDOWN | JUSTUP, 0, 0);
  Rectify(RandWin, 127, 127, 127, 1, 0, 0, 0, 0);
  StringWin(RandWin, "Exit", 3, 0, "latin", 16, 8, 0, 0, 0, 0);
  InstallWin(RandWin);

/* TOOLS header */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 333, 160, 39, 15, "Tools");
  EchoWrap(RandWin, pointer, 1, 0, 0);
  StringWin(RandWin, "Tools", 0, 0, "latin", 16, 8, 0, 0, 0, 0);
  InstallWin(RandWin);
  
/* PENCIL selector.  Draw with a pencil. */
  PenWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 333, 175, 45, 15, "Pencil");

/* MARKER selector.  Scribble with a marker */
  MarkWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 333, 190, 45, 15, "Marker");

  Messenger(PenWin, DO, NoteCanvas, SWITCHTOPENCIL, NULL, JUSTDOWN);
  Messenger(PenWin, DO, PenWin, TURN_ON, NULL, JUSTDOWN);
  Messenger(PenWin, DO, MarkWin, TURN_OFF, NULL, JUSTDOWN);
  EchoWrap(PenWin, pointer, JUSTDOWN, 0, 0);
  Rectify(PenWin, 127, 127, 127, 1, 0, 0, 0, 0);
  StringWin(PenWin, "Pencil", 0, 0, "latin", 16, 8, 0, 0, 0, 0);
  IndicatorWin(PenWin, ON, 255, 0, 0, 2);
  InstallWin(PenWin);

  Messenger(MarkWin, DO, NoteCanvas, SWITCHTOMARKER, NULL, JUSTDOWN);
  Messenger(MarkWin, DO, MarkWin, TURN_ON, NULL, JUSTDOWN);
  Messenger(MarkWin, DO, PenWin, TURN_OFF, NULL, JUSTDOWN);
  EchoWrap(MarkWin, pointer, JUSTDOWN, 0, 0);
  Rectify(MarkWin, 127, 127, 127, 1, 0, 0, 0, 0);
  StringWin(MarkWin, "Marker", 0, 0, "latin", 16, 8, 0, 0, 0, 0);
  IndicatorWin(MarkWin, OFF, 255, 0, 0, 2);
  InstallWin(MarkWin);

/* The color palette. The array 'color' is defined in palette.h.  The color
   scheme was color-matched by hand from a set of paint chips by Suguru.  I
   was told it's a Japanese palette.  I dunno.  It looks neat.  */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 3, 22, 15*25, 15*9, "Color RandWin");
  SuguruPalette(RandWin, 15);
  EchoWrap(RandWin, pointer, 1, 0, 0);
  InstallWin(RandWin);
  
  /* TRANSPARENCY title. */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 3, 190, 100, 15, "Transparent Text");
  EchoWrap(RandWin, pointer, 1, 0, 0);
  StringWin(RandWin, "Transparency", 0, 0, "latin", 16, 8, 0, 0, 0, 0);
  InstallWin(RandWin);

/* TRANSPARENCY slider.  How thick the (paint/ink/goo) is that you're
   drawing with.  */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 3, 205, 255, 15, "Transparent Slider");
  EzSlider(RandWin, HORIZONTAL, 127, 127, 127, 0, 0, 0, 0, 0, 0, 255, INT,
	   (char *)&(params->trans), (char *)&maxtrans, (char *)&mintrans);
  EchoWrap(RandWin, pointer, 5, 0, 0);
  InstallWin(RandWin);

/* LINE WIDTH title. */
  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 3, 160, 100, 15, "Width Text");
  EchoWrap(RandWin, pointer, 1, 0, 0);
  StringWin(RandWin, "Line Width", 0, 0, "latin", 16, 8, 0, 0, 0, 0);
  InstallWin(RandWin);

/* LINE WIDTH slider.  How thick you want your lines.  Depending on what tool 
   you're using, the change in thickness may not be obvious (i.e. pencil) */

  RandWin=(struct Window *)MakeWindow(NoteToolkit->display, NoteToolkit->input, NoteToolkit, 3, 175, 255, 15, "Width Slider");
  EzSlider(RandWin, HORIZONTAL, 127, 127, 127, 0, 0, 0, 0, 0, 0, 255, FLOAT,
	   (char *)&(params->thick), (char *)&maxthick, (char *)&minthick);
  EchoWrap(RandWin, pointer, 5, 0, 0);
  InstallWin(RandWin);

  UnstallWin(NoteCanvas);
  UnstallWin(NoteToolkit); /* want to open this when canvas */
  return(NoteCanvas);
}

/*===========================================================
  PALETTE STUFF
  ===========================================================*/

SuguruPalette(W, size)
     struct Window *W;
     int size;
{
  struct PaletteStruct *fd;
  int PaletteWin();

  if ((fd = (struct PaletteStruct *) malloc (sizeof (struct PaletteStruct))) == 0)
    printf ("error mallocing PaletteStruct\n");
  
  fd->selected = 24;
  fd->size = size;
  
  AttachCommand(W,DRAW,PaletteWin,fd);
  AttachCommand(W,DO,PaletteWin,fd);
  
}


PaletteWin(W,id,data,stuff)
struct Window *W;
int id;
char *data,*stuff;
{
  struct clipstruct *cstruct;
  struct PaletteStruct *params;
  struct pickstruct *pick;
  int button, x, y;
  int cx1,cx2,cy1,cy2;
  int screen;
  int r, g, b, t;
  int i, j;
  int size, selected;
  static unsigned char grey[10] = {0, 31, 63, 95, 127, 159, 191, 223, 255};

  screen = (W->display)->fildes;
  params = (struct PaletteStruct *)stuff;

  size = params->size;
  selected = params->selected;

  switch (id) {
  case (DRAW):
    cstruct = (struct clipstruct *)data;
    cx1 = cstruct->x1;  cy1 = cstruct->y1;
    cx2 = cstruct->x2;  cy2 = cstruct->y2;
    clip_rectangle(screen, 0.0, 2047.0, 0.0, 1023.0);

    interior_style(screen, TRUE, TRUE);
    perimeter_color(screen, 0.0, 0.0, 0.0);
    
    for (i=0; i<25; i++)
      for (j=0; j<9; j++)
	{
	  if (i!=24)
	    /* Draws color squares */
	    fill_color(screen, (colors[j][i][0]) / 255.0,
		       (colors[j][i][1]) / 255.0,
		       (colors[j][i][2]) / 255.0);
	  else
	    /* Gray scale squares instead */
	    fill_color(screen, 
		       grey[j] / 255.0, grey[j] / 255.0, grey[j] / 255.0);

	  rectangle(screen, (float) (W->x1 + i*size),
		    (float) (W->y1 + j * size),
		    (float) (W->x1 + i*size + size),
		    (float) (W->y1 + j * size + size));
	}
    
    i = selected%25;
    j = (int) (selected/25);

    interior_style(screen, FALSE, TRUE);
    perimeter_color(screen, 1.0, 1.0, 1.0); 
    
    rectangle(screen, (float) (W->x1 + i*size),
	      (float) (W->y1 + j * size),
	      (float) (W->x1 + i*size + size),
	      (float) (W->y1 + j * size + size));

    if (i!=24) {		
      CurrentRed = colors[j][i][0];
      CurrentGreen = colors[j][i][1];
      CurrentBlue = colors[j][i][2];
    }
    else 
      CurrentRed = CurrentGreen = CurrentBlue = grey[j];

    clip_rectangle(screen, 0.0, 2047.0, 0.0, 1023.0);
    make_picture_current(screen);
    break;
  case (DO):
    pick = (struct pickstruct *)data;
    button = pick->button;
    x = pick->x - W->x1;
    y = pick->y - W->y1;

    switch (button) {
    case (JUSTDOWN):  
      i = (int) (x / size);
      j = (int) (y / size);
      params->selected = i + j*25;

      UpdateWin(W);
      break;
    case (BEENUP): 
      break;
    case (JUSTUP): 
      break;
    case (BEENDOWN):
      break;
    }
    break;
  default:
    printf("Unknown message: %d\n", id);
    break;
  }
  return(0);
}



/*============================================================
  DRAW NOTES STUFF
  ============================================================*/

struct NoteDrawstuffStruct *Notify(W,P)
     struct Window *W;
     struct Window *P;  /* added by grace to open pallette */
{
  struct NoteDrawstuffStruct *fd;
  struct curstruct *pencil, *marker, *brush, *eraser, *spray, *hand, *mini;
  int NoteWin();
  float thick = 38.0;
  static FILE *dribble;

  if ((dribble=fopen(".dribble", "w")) == NULL)
    printf("Error opening .dribble.  Check your write permissions!\n");
  read_short_table (bw_file(line_table));
  aa_set_fb (W->display->fildes,(W->display)->bits, (W->display)->buffer, (W->display)->control,W->display->width,W->display->height);

  pencil = (struct curstruct *) LoadIcon("icons/pencil_cursor",8,50);
  pencil->hotdx = 0;
  pencil->hotdy = 22;
  marker = (struct curstruct *) LoadIcon("icons/marker_cursor",8,249);
  marker->hotdx = 0;
  marker->hotdy = 22;
  spray = (struct curstruct *) LoadIcon("icons/spray_cursor",8,100); 
  spray->hotdy = 2;
  brush = (struct curstruct *) LoadIcon("icons/brush_cursor",8,249);
  brush->hotdx = 0;
  brush->hotdy = 22;
  hand = (struct curstruct *) LoadIcon("icons/grab_cursor",8,100);
  eraser = (struct curstruct *) LoadIcon("icons/eraser_cursor",8,249);
  eraser->hotdx = 0;
  eraser->hotdy = 22;
  mini = (struct curstruct *) LoadIcon("icons/dot.pix",24,0);

  if ((fd = (struct NoteDrawstuffStruct *) malloc (sizeof (struct NoteDrawstuffStruct))) == 0)
    printf ("error mallocing NoteDrawstuffStruct\n");

  fd->pallette = P; /* added by grace to open pallette */
  fd->pencil = pencil;
  fd->marker = marker;
  fd->brush = brush;
  fd->eraser = eraser;
  fd->spray = spray;
  fd->hand = hand;
  fd->mini = mini;
  fd->trans = 240;
  fd->draw_mode = PENCIL;
  fd->curs_type = MAXIMAL;
  fd->thick = thick;
  fd->dribble = dribble;

  AttachCommand(W,DO,NoteWin,fd); 
  AttachCommand(W,DRAW,NoteWin,fd);
  AttachCommand(W,SWITCHTOMARKER,NoteWin,fd);
  AttachCommand(W,SWITCHTOPENCIL,NoteWin,fd);
  AttachCommand(W,LOAD,NoteWin,fd);
  AttachCommand(W,SAVE,NoteWin,fd);
  AttachCommand(W,MAKEWINDOW,NoteWin,fd);
  AttachCommand(W,EXIT,NoteWin,fd);
  AttachCommand(W,CLEAR,NoteWin,fd);
  AttachCommand(W,OPEN,NoteWin,fd);
  return (fd);
}

NoteWin(W,id,data,stuff)
struct Window *W;
int id;
char *data,*stuff;
{
  struct Window *pallette; /* new for opening pallette */
  struct clipstruct *cstruct;
  struct NoteDrawstuffStruct *params;
  int cx1,cx2,cy1,cy2;
  int screen;
  struct pickstruct *pick;
  float p, fx, fy;
  int x, y, button;
  int r, g, b, t;
  float wd, wid;
  int flag, draw_flag = 1;
  FILE *fp;
  static FILE *dribble;
  char pathstr[35], string[100];
  struct trapstruct T;
  static int new = 0;
  screen = (W->display)->fildes;
  params = (struct NoteDrawstuffStruct *)stuff;

  dribble = params->dribble;

  switch (id) {
  case (DRAW):
    cstruct = (struct clipstruct *)data;
    cx1 = cstruct->x1;  cy1 = cstruct->y1;
    cx2 = cstruct->x2;  cy2 = cstruct->y2;
    clip_rectangle(screen, 0.0, 2047.0, 0.0, 1023.0);

    DrawMask(W, cx1, cy1, cx2, cy2);
  aa_set_fb (W->display->fildes,(W->display)->bits, (W->display)->buffer, (W->display)->control,W->display->width,W->display->height);
    make_picture_current(screen);
    break;
  case (DO):
    pick = (struct pickstruct *)data;
    x = pick->x;
    y = pick->y;
    p = pick->p;
    button = pick->button;

    fx = (float)x;
    fy = (float)y;

    r = CurrentRed;
    g = CurrentGreen;
    b = CurrentBlue;
    t = params->trans;
    wd = params->thick * p;

    W->display->control [3] = 4;
    W->display->control [0x40bf] = 7;
    W->display->control [0x40ef] = 0x33;
    drawing_mode(W->display->fildes, 3);
    make_picture_current(W->display->fildes);
    
    switch (button) {
    case (JUSTDOWN):
      T.window = W,T.until = BEENUP, Dispatch(W,TRAP,&T,NULL),new = 1;

      aa_set_clip (0, 0, 1279, 1023);
      aa_set_fb (W->display->fildes,(W->display)->bits, (W->display)->buffer, (W->display)->control,W->display->width,W->display->height);
      
      aa_set_color (r, g, b);
      write_set_color(r, g, b);
      aa_st_trans (t);
      write_set_trans(t);
      if (params->draw_mode == MARKER)
	{
	  wid = wd/2.0;
	  aa_move (fx, fy, wid);
	  write_move (fx, fy, wid);
	}
      else if (params->draw_mode == PENCIL)
	{
	  wid = wd/16.0;
	  aa_move (fx, fy, wid);
	  write_move (fx, fy, wid);
	}
      break;
    case (BEENUP):
      break;
    case (JUSTUP):
      new = 0;
      break;
    case (BEENDOWN):
      EchoOff(W->display,1);
      if (new) {
	make_picture_current (screen);
	(W->display)->control [3] = 4;
	switch (params->draw_mode) {
	case (MARKER):
	  {
	    wid = wd/2.0;
	    aa_draw (fx, fy, wid);
	    write_draw (fx, fy, wid);
	  }
	  break;
	case (PENCIL):
	  {
	    wid = wd/16.0;
	    aa_draw (fx, fy, wid);
	    write_draw (fx, fy, wid);
	  }
	  break;
	}
      }
      break;
    }
    if (params->curs_type == MAXIMAL)
      switch (params->draw_mode) {
      case (PENCIL):
	UpdateEcho(x,y,W->display,params->pencil);
	break;
      case (MARKER):
	UpdateEcho(x,y,W->display,params->marker);
	break;
      }
    else
      UpdateEcho(x,y,W->display,params->mini);
    break;
  case (SWITCHTOPENCIL):
    params->draw_mode = PENCIL;
    break;
  case (SWITCHTOMARKER):
    params->draw_mode = MARKER;
    break;
  case (EXIT):
    fclose(dribble);
    system("rm .dribble");
    if (W->stat) CloseWindow(W);
    if ((params->pallette)->stat) CloseWindow(params->pallette);
    break;
  case (MAKEWINDOW):
    GetString(W->parent, 10, 60,"Name this object:", pathstr, 35);
    if(strlen(pathstr))
      {
	fclose(dribble);
	sprintf(string, "cp .dribble %s", pathstr);
	system(string);
	MakeScribbleWindow(W, params->hand, pathstr);
	system("rm .dribble");
	if ((dribble=fopen(".dribble", "w")) == NULL)
	  printf("Error opening .dribble.  Check your write permissions!\n");
      }
    break;
  case (CLEAR):
    fclose(dribble);
    system("rm .dribble");
    OpenWindow(W);
    if ((dribble=fopen(".dribble", "w")) == NULL)
      printf("Error opening .dribble.  Check your write permissions!\n");
    break;
  case (OPEN):
    printf("Opening notation stuff\n");
    if ((dribble=fopen(".dribble", "w")) == NULL)
      printf("Error opening .dribble.  Check your write permissions!\n");
    if (!(W->stat)) OpenWindow(W);
    if (!((params->pallette)->stat)) OpenWindow(params->pallette);
    break;
  default:
    printf("Unknown message: %d\n", id);
    break;
  }
  return(0);
}
