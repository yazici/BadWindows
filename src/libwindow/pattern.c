

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





/************************************
*                                   *
* PATTERN.C                         *
*                                   *
* by Michael J. Bauer               *
* (mjbauer@media-lab.media.mit.edu  *
*  mjbauer@athena.mit.edu)          *
* date:  1 June 1989                *
* version:  1.0                     *
*                                   *
******************************************************************************
*                                                                            *
* This is a window function much like Rectify.  However, instead of keeping  *
* a single-color background, it makes and maintains a patterned background.  *
*                                                                            *
* It's arguements are:                                                       *
* W       (struct Window *)       The window to patternify                   *
* pattern (struct curstruct *)    The pattern to be used.  Max size 540x1024 *
*                                                                            *
* The first complete pattern is placed such that its upper left corner is in *
* the upper left corner of the screen.  The pattern tiles until the screen   *
* is filled.                                                                 *
*                                                                            *
* If you want the pattern tiles to start relative to the window, instead     *
* of the upper left corner send the PATTERN_RELATIVE message.  -SL  2/14/92  *
*****************************************************************************/

#include <stdio.h>
#include <starbase.c.h>
#include <string.h>
#include <windows.c.h>
#include <structures.h>

Patternify(W,pattern)
struct Window *W;
struct curstruct *pattern;
{

/*****************************************************************************
*                                                                            *
* Internal variables:                                                        *
* DoPattern() (int)                        The DO/DRAW functions             *
* params      (struct DoPatternStruct *)   The parameters to pass to DO/DRAW *
*                                                                            *
*****************************************************************************/

  int DoPattern();
  struct DoPatternStruct *params;

/* Load the parameter structure */

  params = (struct DoPatternStruct *) malloc(sizeof(struct DoPatternStruct));
  params->pattern = pattern;
  params->isRelative = FALSE;

/* Drag in appropriate commands */

  AttachCommand(W,DRAW,DoPattern,(char *)params);
  AttachCommand(W,PATTERN_RELATIVE,DoPattern,(char *)params);
}


/*************************************************
* This takes the standard do-function arguements *
*************************************************/

DoPattern(W,id,data,stuff)
     struct Window *W;
     int id;
     char *data,*stuff;
{

/****************************************************************************
*                                                                           *
* Internal variables:                                                       *
* The standard button, cx1, cy1, cx2, cy2, cstruct, pick                    *
* fildes (int)    The ever-present file descriptor                          *
* pat_wd (int)    The width of the pattern                                  *
* pat_ht (int)    The height of the pattern                                 *
* x1     (int)    The upper-left coordinates of where the master pattern is *
* y1     (int)      kept off-screen                                         *
* i      (int)    Width and height of the pattern to copy (figured by       *
* j      (int)      assorted routines)                                      *
* nextx1 (int)    How far off the upper left corner the piece of pattern to *
* nexty1 (int)      be replaced is (figured by assorted routines)           *
*                                                                           *
* offx1  (int)    The offset off the pattern.  This changes depending on    *
* offy1  (int)      whether the pattern is absolutely placed, or is         *
* offx2  (int)      relative to its window.    -SL 2/14/92                  *
* offy2  (int)                                                              *
****************************************************************************/

  int button;
  struct pickstruct *pick;
  struct clipstruct *cstruct;
  int cx1,cx2,cy1,cy2;

  int fildes;
  int pat_wd, pat_ht;
  int x1=1408, y1=0, i, j, nextx1, nexty1;
  int offx1,offy1,
      offx2,offy2;

  struct curstruct *pattern;
  struct DoPatternStruct *params;

  params = (struct DoPatternStruct *)stuff;   /* unpacking the windows stuff */
  pattern = params->pattern;
  fildes = (W->display)->fildes;

  switch (id) {
  case (DRAW):
    cstruct = (struct clipstruct *)data;
    cx1 = cstruct->x1;  cy1 = cstruct->y1;
    cx2 = cstruct->x2;  cy2 = cstruct->y2;
    clip_rectangle(fildes, 0.0, 2047.0, 0.0, 1023.0);

/* This segment of code draws the master curstruct in the frame buffer */

    (W->display)->control[0x0003] = 0x04;    /* these two lines set up the */
    (W->display)->control[0x40bf] = 7;       /* draw mode of frame buffer  */
    if (W->display->bits == 24)
      shape_write24(W->display->control,pattern->source,
		    pattern->dx,pattern->dy,
		    W->display->buffer,W->display->width,
		    W->display->height,x1,y1,pattern->rule,
		    0, 0, 2048, 1024);
    else if (W->display->bits == 32)
      {
	/* Have to do a starbase command before frame buffer access will work.  Don't ask me why -BS 4/90 */
      interior_style(W->display->fildes,INT_SOLID,FALSE);
      rectangle(W->display->fildes,(float)(1280),(float)(0),(float)(1280+pattern->dx-1),(float)(pattern->dy-1));
      make_picture_current(W->display->fildes);

      shape_write32(W->display->control,pattern->source,
		    pattern->dx,pattern->dy,
		    W->display->buffer,W->display->width,
		    W->display->height,x1,y1,pattern->rule,
		    0,0, 2048, 1024);
      }
    else if (W->display->bits == 4)
      shape_write24to4(pattern->source,pattern->dx,pattern->dy,
		       W->display->buffer,W->display->width,
		       W->display->height,x1,y1,pattern->rule,
		       0,0, 2048, 1024);

/* This segment takes the four possible cases of the area cx1, cy1, cx2, cy2 
   (especially the area's edges) vs the size of the pattern unit being used,
   and fills according to whichever it finds. */

/* Random useful variables */

    pat_wd = pattern->dx;   /* width of one pattern unit */
    pat_ht = pattern->dy;   /* height of one pattern unit */

/* If the pattern is absolutely placed, based on the upper left corner of the
   screen, then the offsets are calculated from the clipstruct.
   Otherwise, if the offsets are relative to the window's upper left corner,
   then the offsets are calculated by transforming the clipstruct into the
   window's coordinate system. */

    if (params->isRelative==FALSE) {
      offx1 = cx1%pat_wd;
      offx2 = cx2%pat_wd;
      offy1 = cy1%pat_ht;
      offy2 = cy2%pat_ht;
    }
    else {
      offx1 = (cx1 - W->x1)%pat_wd;  /* transform clipstruct into W's coord system */
      offx2 = (cx2 - W->x1)%pat_wd;
      offy1 = (cy1 - W->y1)%pat_ht;
      offy2 = (cy2 - W->y1)%pat_ht;
    }
    
    if (offx1==0) nextx1 = 0; /* How far cx1 is from the side of */
    else nextx1=pat_wd-offx1; /* the next pattern unit right of it */

    if (offy1==0) nexty1 = 0; /* How far cy1 is from the top of */
    else nexty1=pat_ht-offy1; /* the next pattern unit below it */

    
/* If the area to be filled is completely within one pattern unit */

    (W->display)->control[0x40bf] = 7;       /* draw mode of frame buffer  */
    if((cy2-offy2)==(cy1-offy1) &&
       (cx2-offx2)==(cx1-offx1))
      dcblock_move(fildes, x1+offx1, y1+offy1,
		   cx2-cx1+1, cy2-cy1+1, cx1, cy1);

/* If the area is long and skinny -- top and bottom edges are in the same
   row of pattern units, but left and right edges are not in the same unit */

    else if ((cy2-offy2)==(cy1-offy1))
      {

/* Fill all the subareas that completely cut a pattern unit (possibly none) */

	for(i=(cx1+nextx1); i<(cx2-offx2); i+=pat_wd)
	    dcblock_move(fildes, x1, y1+offy1, pat_wd, cy2-cy1+1, i, cy1);

/* Fill the incompletely cut end units (one or both may not exist) */

	dcblock_move(fildes, x1+offx1, y1+offy1,
		     nextx1, cy2-cy1+1, cx1, cy1);
	dcblock_move(fildes, x1, y1+offy1, 
		     offx2+1, cy2-cy1+1, cx2-offx2, cy1);
      }

/* If the area is tall and skinny -- left and right edges are in the same
   column of pattern units, but top and bottom edges not in the same unit */

    else if ((cx2-offx2)==(cx1-offx1))
      {

/* Fill all the subareas that completely cut a pattern unit (possibly none) */

	for(j=(cy1+nexty1); j<(cy2-offy2); j+=pat_ht)
	    dcblock_move(fildes, x1+offx1, y1, cx2-cx1+1, pat_ht, cx1, j);

/* Fill the incompletely cut end units (one or both may not exist) */

	dcblock_move(fildes, x1+offx1, y1+offy1,
		     cx2-cx1+1, nexty1, cx1, cy1);
	dcblock_move(fildes, x1+offx1, y1,
		     cx2-cx1+1, offy2+1, cx1, cy2-offy2);
      }

/* Failing all the weird cases, this is where the top edge is not in the same
   row of pattern units as the bottom edge, and the left edge is not in the
   same column of pattern units as the right edge. */

    else
      {

/* Fill in the units that were only partially covered by a corner */

	dcblock_move(fildes, x1+pat_wd-nextx1, y1+pat_ht-nexty1,
		     nextx1, nexty1, cx1, cy1);
	dcblock_move(fildes, x1+pat_wd-nextx1, y1,
		     nextx1, offy2+1, cx1, cy2-offy2);
	dcblock_move(fildes, x1, y1+pat_ht-nexty1,
		     offx2+1, nexty1, cx2-offx2, cy1);
	dcblock_move(fildes, x1, y1,
		     offx2+1, offy2+1,
		     cx2-offx2, cy2-offy2);

/* Fill in the units that were covered by the left and right edges */

	for (j=(cy1+nexty1); j<(cy2-offy2); j+=pat_ht)
	  {
	    dcblock_move(fildes, x1+pat_wd-nextx1, y1, nextx1, pat_ht, cx1, j);
	    dcblock_move(fildes, x1, y1,
			 offx2+1, pat_ht, cx2-offx2, j);
	  }

/* Fill in the rest of the area (top, bottom, and full middle units) */

	for (i=(cx1+nextx1); i<(cx2-offx2); i+=pat_wd)
	  {

	    /* Top edge */
	    dcblock_move(fildes, x1, y1+pat_ht-nexty1, pat_wd, nexty1, i, cy1);

	    /* Center units */
	    for (j=(cy1+nexty1); j<(cy2-offy2); j+=pat_ht)
	      dcblock_move(fildes, x1, y1, pat_wd, pat_ht, i, j);

	    /* Bottom edge */
	    dcblock_move(fildes, x1, y1, pat_wd, offy2+1, i, cy2-offy2);
	  }
      }


    clip_rectangle(fildes, 0.0, 2047.0, 0.0, 1023.0);
    make_picture_current(fildes);
    break;
  case (DO):
    pick = (struct pickstruct *)data;
    button = pick->button;

    switch (button) {
    case (JUSTDOWN):  
      break;
    case (BEENUP): 
      break;
    case (JUSTUP): 
      break;
    case (BEENDOWN):
      break;
    }
    break;
  case (PATTERN_RELATIVE):
    if ((int)data == 0)
      params->isRelative = FALSE;
    else
      params->isRelative = TRUE;
    break;
    
  default:
    printf("Unknown id: %d\n", id);
    break;
  }
  
  return(0);
}
