/*****************************************************************************
 *
 * NAME
 *    cpanel.c
 * 
 * CONTENTS
 *    This file contains the interface needed for control panels
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <windows.c.h>
#include <structures.h>
#include <cpanels.h>

extern int CloseWindow();

int __x_foo1()
{
}

CPANEL_STRUCT * MakeControlPanel(parent, pointer, startx, starty, 
				 sizex, sizey, title)
struct Window * parent;
struct curstruct * pointer;
int startx, starty, sizex, sizey;
char * title;
{

  struct Window * base, * title_bar, * close_box;
  CPANEL_STRUCT * retval;

  base = MakeWindow(parent->display, parent->input,  parent, startx, starty, 
		      sizex, sizey, title);
  Rectify(base, CPANEL_BKGND, 1, CPANEL_BLACK, CPANEL_SHADOW);
  EchoWrap(base, pointer, JUSTUP|JUSTDOWN, 0, 0);

  title_bar = MakeWindow(parent->display, parent->input, base,
			 CPANEL_WINDOW_TAB, CPANEL_WINDOW_TAB,
			 sizex-(3*CPANEL_WINDOW_TAB)-CPANEL_TITLE_BAR_HEIGHT,
			 CPANEL_TITLE_BAR_HEIGHT, "Title bar");
  Rectify(title_bar, CPANEL_DKBLUE, 1, CPANEL_BLACK, 0);
  StringWin(title_bar, title,  CPANEL_TITLE_XTAB, CPANEL_TITLE_YTAB,
	    CPANEL_FONT, CPANEL_PTSIZE_LARGE, CPANEL_FONTBITS,
	    CPANEL_TEXT_ECHO, 0);
  Mover(title_bar, base);
  EchoWrap(title_bar, pointer, JUSTUP|JUSTDOWN, 0, 0);
  InstallWin(title_bar);
  
  close_box = MakeWindow(parent->display, parent->input, base,
		    sizex-(CPANEL_WINDOW_TAB+CPANEL_TITLE_BAR_HEIGHT), 
			    CPANEL_WINDOW_TAB,
			    CPANEL_TITLE_BAR_HEIGHT,  CPANEL_TITLE_BAR_HEIGHT,
			    "Close button");
  Rectify(close_box, CPANEL_BLACK, 0, CPANEL_BLACK, 0);
  FunctionCaller(close_box, DO, CloseWindow, (char *) base, JUSTDOWN);
  EchoWrap(close_box, pointer, JUSTUP|JUSTDOWN, 0, 0);
  InstallWin(close_box);

  retval = (CPANEL_STRUCT *) malloc(sizeof(CPANEL_STRUCT));
  retval->base = base;
  retval->title_bar = title_bar;
  retval->close_box = close_box;

}

