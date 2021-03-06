/*
 * added mode to Command structure!
 */


#ifndef _WINDOWS_C_H
#define _WINDOWS_C_H 
#include <X11/Xlib.h>

#include <malloc.h>
#include "messages.h"

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX(A,B)  ((A) > (B) ? (A) : (B))
#define MIN(A,B)  ((A) < (B) ? (A) : (B))
#define UpdateSizes(W) TransformCoords(W,W->relx,W->rely,W->width,W->height);

/***  These are the structures for output and input devices  ***/
#define MAX_CONTEXT_DEPTH 8
#define X             0
#define Y             1

/* 
 * All the display function will be 
 *   func(fb_idx, fb, ... )
 *     int fb_idx;
 *     fb_type fb;
 */

typedef char * FrameBuffer;
typedef char * Locator;

struct OutDev {
  int         bits;
  int         width, height;
  int         viswidth, visheight;
  FrameBuffer frameBuffer;
  unsigned char *addrR, *addrG, *addrB;
};

struct InDev {
  int fildes;
  int x_ratio, y_ratio;
  int width, height;
  int buttons;
  Locator locator;
};

/***  WINDOW STRUCTURES  ***/

struct Window {
  int stat;                      /*   Is the window open/closed?           */
  int relx, rely;                /*   position of window within parent     */
  int width, height;             /*   size of window in pixels             */
  int x1, y1, x2, y2;            /*   window's actual screen coordinates   */
  struct OutDev *display;        /*   screen file descriptor               */
  struct InDev *input;           /*   mouse or tablet file descriptor      */
  struct List *mask;             /*   list of overlapping windows          */
  struct Window *parent;         /*   parent window                        */
  struct List *port;             /*   pointer to children windows          */
  struct Command *commands;      /*   window functions                     */
  char *tag;                     /*   identifying string                   */
};

struct Command {
  int id, numfuncs;              /* integer message id */
  int mode;
  int (**functions)();
  char **arguments; 
  struct Command *next;          /* pointer to next command */
};

struct Stencil {
  struct Window *id; 
  int x1, y1, x2, y2;
  struct Stencil *Behind;
  struct Stencil *Front;
};

struct List {
  struct Window *id;
  struct List *Behind, *Front;
};

struct messsage {
  int id;
  char *data;
};

struct loopstruct {
  int button, trapstat;
  struct Window *Last, *trap;
};


/*** PARAMETER STRUCTURES FOR WINDOWS ***/

struct pickstruct {
  int x, y, button, bnum, character;
  float p;
};

struct trapstruct {
  struct Window *window;
  int until;
};

struct clipstruct {
  int x1, y1, x2, y2;
};

struct Thing {
  struct Thing *Front, *Behind;
  char *id;
};


struct Window *MakeWindow();
struct Command *GetCommand();
struct Thing *MakeThingList(), *AskThing();
char *CopyString();
struct OutDev *GR_OpenScreen();
struct InDev *GR_OpenLocator();

#endif
