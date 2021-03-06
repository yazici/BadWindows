#include <stdio.h>
#include <starbase.c.h>
#include <windows.c.h>
#include <structures.h>



/* kind INT (0) is integer,  kind FLOAT (1) is float */
/* orient HORIZONTAL (0) is horizontal, orient VERTICAL (1) is vertical */

EzSlider(W, orient, red, green, blue, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue, kind, number, max, min)     /* v 1.0b */
     struct Window *W;
     int orient;
     int red, green, blue, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue;
     char *number;
     char *max, *min;
{
  struct ezslidestruct *params;
  int EzSlideDraw(), EzSlideDo();

  params = (struct ezslidestruct *)malloc(sizeof(struct ezslidestruct));
  params->orient = orient;
  params->red = red;
  params->green = green;
  params->blue = blue;
  params->shadow = shadow;
  params->shadowred = shadowred;
  params->shadowgreen = shadowgreen;
  params->shadowblue = shadowblue;
  params->barred = barred;
  params->bargreen = bargreen;
  params->barblue = barblue;
  params->number = number;
  params->max = max;
  params->min = min;
  params->kind = kind;
  params->oldIntValue = 0;
  params->oldFloatValue = 0.0;

  AttachCommand(W, DRAW, EzSlideDraw, (char *)params);
  AttachCommand(W, UPDATE, EzSlideDraw, (char *)params);
  AttachCommand(W, DO, EzSlideDo, (char *)params);
  AttachCommand(W, NEWVALUE, EzSlideDo, (char *)params);
  AttachCommand(W, NEWCOLOR, EzSlideDo, (char *)params);  /* S.Librande 8/22/91 */
  AttachCommand(W, NEW_NUMBER, EzSlideDo, (char *)params);  /* BS 9/25/91 */
}



EzSlideDraw(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct ezslidestruct *params;
  int x1, x2, y1, y2, screen, shadow, t, x, y;
  int barred, bargreen, barblue;
  float fx1, fx2, fy1, fy2;
  float cx1, cx2, cy1, cy2;
  float red, green, blue;
  struct clipstruct *cstruct;
  int *number, *max, kind, *min;
  float *fnumber, *fmax, ratio, *fmin;

  screen = (W->display)->fildes;

  switch(id)
    {
    case (DRAW):
      cstruct = (struct clipstruct *)data;
      cx1 = cstruct->x1;
      cy1 = cstruct->y1;
      cx2 = cstruct->x2;
      cy2 = cstruct->y2;
      break;
    case (UPDATE):
      cx1 = (float)W->x1;
      cy1 = (float)W->y1;
      cx2 = (float)W->x2;
      cy2 = (float)W->y2;
      break;
    }	
  clip_rectangle(screen, cx1, cx2, cy1, cy2);

  params = (struct ezslidestruct *)stuff;
  shadow = params->shadow;
  kind = params->kind;

  max = NULL;
  min = NULL;
  fmax = NULL;
  fmin = NULL;
  number = NULL;
  fnumber = NULL;
  if (kind) fnumber = (float *)params->number, fmax = (float *)params->max, fmin = (float *)params->min;
  else number = (int *)params->number, max = (int *)params->max, min = (int *)params->min;

  red = ((float)(params->red)/255.0);
  green = ((float)(params->green)/255.0);
  blue = ((float)(params->blue)/255.0);
  fill_color(screen, red, green, blue);

  red = ((float)(params->shadowred)/255.0);
  green = ((float)(params->shadowgreen)/255.0);
  blue = ((float)(params->shadowblue)/255.0);
  line_color(screen, red, green, blue);
  perimeter_color(screen, red, green, blue);
  interior_style(screen, INT_SOLID, TRUE);

  x1 = W->x1; y1 = W->y1; x2 = W->x2; y2 = W->y2;

  if (x1>x2) {t = x2; x2 = x1; x1 = t;}
  if (y1>y2) {t = y2; y2 = y1; y1 = t;}

  x2 -= (float)shadow; y2 -= (float)shadow;

  fx1=x1; fx2=x2;
  fy1=y1; fy2=y2;

  rectangle(screen, fx1, fy1, fx2, fy2);

  red = ((float)(params->barred)/255.0);
  green = ((float)(params->bargreen)/255.0);
  blue = ((float)(params->barblue)/255.0);
  fill_color(screen, red, green, blue);

  if (kind)
    ratio = (*fnumber-*fmin)/(*fmax-*fmin);
  else ratio = ((float)*number - (float)*min)/((float)*max-(float)*min);

  if (params->orient)
    {
      float ratline;
      ratline = fy2-ratio*(fy2-fy1-3.0)-2.0;
      rectangle(screen, fx1, ratline, fx2, ratline+4.0);
    }
  else
    {
      float ratline;
      ratline = fx1+ratio*(fx2-fx1-3.0)-2.0;
      rectangle(screen, ratline, fy1, ratline+4.0, fy2);
    }

  for (t=0; t<shadow; t++)
    {
      fx1 += 1.0;
      fy1 += 1.0;
      fy2 += 1.0;
      fx2 += 1.0;

      move2d(screen, fx1, fy2);
      draw2d(screen, fx2, fy2);
      move2d(screen, fx2, fy1);
      draw2d(screen, fx2, fy2);
    }

  clip_rectangle((W->display)->fildes, (float)0.0, (float)((W->display)->width-1), 
		 0.0, (float)((W->display)->height-1));
  make_picture_current((W->display)->fildes);
}



EzSlideDo(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  static new = 0;
  struct messenger *mstruct;
  struct ezslidestruct *params;
  float *fnum=0, *fmax = 0, ratio, *fmin;
  int x, y, button, kind, *num=0, *max = 0, *min = 0, dx, dy;
  int *colors;
  struct pickstruct *pick;

  params = (struct ezslidestruct *)stuff;
  kind = params->kind;
  if (kind) fnum = (float *)params->number, fmax = (float *)params->max, fmin = (float *)params->min;
  else num = (int *)params->number, max = (int *)params->max, min = (int *)params->min;

  pick = (struct pickstruct *)data;
  x = pick->x;
  y = pick->y;

  dx = MAX(x, W->x1);
  dy = MAX(y, W->y1);
  dx = MIN(dx, W->x2);
  dy = MIN(dy, W->y2);
  button = pick->button;

  switch (id)
    {
    case (DO):
      switch (button)
	{
	case (JUSTDOWN):
	  Dispatch(W, TRAP, MakeTrap(W, BEENUP), NULL);
	  new = 1;

	case (JUSTUP):
	case (BEENDOWN):

	  if (new)
	    {
	      if (params->orient) /* for DYoung, so his sliders will work */
		{		/* but this'll break some stuff */
		  ratio = (float)(W->y2-dy)/(float)(W->height-params->shadow-1);  
		}
	      else
		{
		  ratio = (float)(dx-W->x1)/(float)(W->width-params->shadow-1);
		}

	      if (kind)
		{
		  *fnum = *fmin+ratio*(*fmax-*fmin);
		  if (*fmax > *fmin)
		    {
		      *fnum = MAX(*fnum, *fmin);
		      *fnum = MIN(*fnum, *fmax);
		    }
		  else {
		    *fnum = MIN(*fnum, *fmin);
		    *fnum = MAX(*fnum, *fmax);
		  }
		}
	      else
		{
		  *num = ((float)*min + ratio*((float)*max-(float)*min));
		  if (*max > *min)
		    {
		      *num = MAX(*num, *min);
		      *num = MIN(*num, *max);
		    }
		  else
		    {
		      *num = MIN(*num, *min);
		      *num = MAX(*num, *max);
		    }
		}
	      /* UpdateWin(W); */
	      /* I added the following IF stuff so that the slider wouldn't update
	       * if you don't move it, but still have the pen down.
	       * *DKY-24Jan91*
	       */
	      /* I changed the following to update anyway if double buffering -BOB 26May92 */

	      if (!(W->display->bits&DBUFFER)) {
		if (kind) {
		  if( params->oldFloatValue != (float)(*fnum))
		    UpdateWin( W);
		  params->oldFloatValue = (float)(*fnum);
		}
		else {
		  if( params->oldIntValue != (int)(*num))
		    UpdateWin( W);
		  params->oldIntValue = (int)(*num);
		}
	      }
	      else UpdateWin(W);

	      if (id == JUSTUP) new = 0;
	    }
	  break;
	}
    break;
      
    case (NEWVALUE):
      if (kind) {
	float *newval;
	newval = (float *) data;
	*fnum = *newval;
      }
      else {
	int *newval;
	newval = (int *) data;
	*num = *newval;
      }
      break;
      
    case (NEW_NUMBER):
      params->number = data;
      break;

    case (NEWCOLOR):  /* S.Librande  8/22/91 */
      colors = (int *)data;
      params = (struct ezslidestruct *)stuff;
      
      params->red = *(colors);
      params->green = *(colors+1);
      params->blue = *(colors+2);
      break;
      
    default:
      printf("EzSlider: unknown message %5d\n",id);
    }
  return(0);
}
/*************************************************************************/


/* kind INT (0) is integer,  kind FLOAT (1) is float */
/* orient HORIZONTAL (0) is horizontal, orient VERTICAL (1) is vertical */


ClearEzSlider(W, orient, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue, kind, number, max, min)	/* v 1.0b */
     struct Window *W;
     int orient;
     int shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue;
     char *number;
     char *max, *min;
{
  struct ezslidestruct *params;
  int ClearEzSlideDraw(), ClearEzSlideDo();

  params = (struct ezslidestruct *)malloc(sizeof(struct ezslidestruct));
  params->orient = orient;
  params->shadow = shadow;
  params->shadowred = shadowred;
  params->shadowgreen = shadowgreen;
  params->shadowblue = shadowblue;
  params->barred = barred;
  params->bargreen = bargreen;
  params->barblue = barblue;
  params->number = number;
  params->max = max;
  params->min = min;
  params->kind = kind;
  params->oldIntValue = 0;
  params->oldFloatValue = 0.0;

  AttachCommand(W, DRAW, ClearEzSlideDraw, (char *)params);
  AttachCommand(W, DO, ClearEzSlideDo, (char *)params);
  AttachCommand(W, NEWVALUE, ClearEzSlideDo, (char *)params);
  AttachCommand(W, NEW_NUMBER, ClearEzSlideDo, (char *)params);
  AttachCommand(W, UPDATE, ClearEzSlideDraw, (char *)params);
}



ClearEzSlideDraw(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct ezslidestruct *params;
  int x1, x2, y1, y2, screen, shadow, t, x, y;
  int barred, bargreen, barblue;
  float fx1, fx2, fy1, fy2;
  float cx1, cx2, cy1, cy2;
  float red, green, blue;
  struct clipstruct *cstruct;
  int *number, *max, kind, *min;
  float *fnumber, *fmax, ratio, *fmin;

  switch ( id)
    {
    case UPDATE:
      UpdateWin( W->parent);
      break;
    case DRAW:
      {

	screen = (W->display)->fildes;
	cstruct = (struct clipstruct *)data;
	cx1 = cstruct->x1;
	cy1 = cstruct->y1;
	cx2 = cstruct->x2;
	cy2 = cstruct->y2;
	clip_rectangle(screen, cx1, cx2, cy1, cy2);

	params = (struct ezslidestruct *)stuff;
	shadow = params->shadow;
	kind = params->kind;

	max = NULL;
	min = NULL;
	fmax = NULL;
	fmin = NULL;
	number = NULL;
	fnumber = NULL;

	if (kind) fnumber = (float *)params->number, fmax = (float *)params->max, fmin = (float *)params->min;
	else number = (int *)params->number, max = (int *)params->max, min = (int *)params->min;

	red = ((float)(params->shadowred)/255.0);
	green = ((float)(params->shadowgreen)/255.0);
	blue = ((float)(params->shadowblue)/255.0);
	line_color(screen, red, green, blue);
	perimeter_color(screen, red, green, blue);
	interior_style(screen, INT_SOLID, TRUE);

	x1 = W->x1; y1 = W->y1; x2 = W->x2; y2 = W->y2;

	if (x1>x2) {t = x2; x2 = x1; x1 = t;}
	if (y1>y2) {t = y2; y2 = y1; y1 = t;}

	x2 -= (float)shadow; y2 -= (float)shadow;

	fx1=x1; fx2=x2;
	fy1=y1; fy2=y2;

	/*******************************/

	red = ((float)(params->barred)/255.0);
	green = ((float)(params->bargreen)/255.0);
	blue = ((float)(params->barblue)/255.0);
	fill_color(screen, red, green, blue);

	if (kind)
	  ratio = (*fnumber-*fmin)/(*fmax-*fmin);
	else ratio = ((float)*number - (float)*min)/((float)*max-(float)*min);

	if (params->orient)
	  {
	    float ratline;
	    ratline = fy2-ratio*(fy2-fy1-3.0)-2.0;
	    rectangle(screen, fx1, ratline, fx2, ratline+4.0);
	  }
	else
	  {
	    float ratline;
	    ratline = fx1+ratio*(fx2-fx1-3.0)-2.0;
	    rectangle(screen, ratline, fy1, ratline+4.0, fy2);
	  }

	for (t=0; t<shadow; t++)
	  {
	    fx1 += 1.0;
	    fy1 += 1.0;
	    fy2 += 1.0;
	    fx2 += 1.0;

	    move2d(screen, fx1, fy2);
	    draw2d(screen, fx2, fy2);
	    move2d(screen, fx2, fy1);
	    draw2d(screen, fx2, fy2);
	  }

	clip_rectangle((W->display)->fildes, (float)0.0, (float)((W->display)->width-1), 
		       0.0, (float)((W->display)->height-1));
	make_picture_current((W->display)->fildes);
      }
    }
}





ClearEzSlideDo(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  static new = 0;
  struct messenger *mstruct;
  struct ezslidestruct *params;
  float *fnum=0, *fmax = 0, ratio, *fmin;
  int x, y, button, kind, *num=0, *max = 0, *min = 0, dx, dy;
  struct pickstruct *pick;

  params = (struct ezslidestruct *)stuff;
  kind = params->kind;
  if (kind) fnum = (float *)params->number, fmax = (float *)params->max, fmin = (float *)params->min;
  else num = (int *)params->number, max = (int *)params->max, min = (int *)params->min;


  switch (id)
    {
    case (DO):
      
      pick = (struct pickstruct *)data;
      x = pick->x;
      y = pick->y;
      
      dx = MAX(x, W->x1);
      dy = MAX(y, W->y1);
      dx = MIN(dx, W->x2);
      dy = MIN(dy, W->y2);
      button = pick->button;
      switch (button)
	{
	case (JUSTDOWN):
	  Dispatch(W, TRAP, MakeTrap(W, BEENUP), NULL);
	  new = 1;
	  
	case (JUSTUP):
	case (BEENDOWN):
	  
	  if (new)
	    {
	      if (params->orient)
		  ratio = (float)(W->y2-dy)/(float)(W->height-params->shadow -1);
	      else
		  ratio = (float)(dx-W->x1)/(float)(W->width-params->shadow -1);
	      if (kind)
		{
		  *fnum = *fmin+ratio*(*fmax-*fmin);
		  if (*fmax > *fmin)
		    {
		      *fnum = MAX(*fnum, *fmin);
		      *fnum = MIN(*fnum, *fmax);
		    }
		  else {
		    *fnum = MIN(*fnum, *fmin);
		    *fnum = MAX(*fnum, *fmax);
		  }
		}
	      else
		{
		  *num = ((float)*min + ratio*((float)*max-(float)*min));
		  if (*max > *min)
		    {
		      *num = MAX(*num, *min);
		      *num = MIN(*num, *max);
		    }
		  else
		    {
		      *num = MIN(*num, *min);
		      *num = MAX(*num, *max);
		    }
		}
	      /* I added the following IF stuff so that the slider wouldn't update
	       * if you don't move it, but still have the pen down.
	       * *DKY-24Jan91*
	       */
	      /* I changed the following to update anyway if double buffering -BOB 26May92 */
	      
	      if (!(W->display->bits&DBUFFER)) {
		if(kind) {
		  if( params->oldFloatValue != (float)(*fnum))
		    UpdateWin( W->parent);
		  params->oldFloatValue = (float)(*fnum);
		}
		else {
		  if( params->oldIntValue != (int)(*num))
		    UpdateWin( W->parent);
		  params->oldIntValue = (int)(*num);
		}
	      }
	      else UpdateWin(W);

	      if (id == JUSTUP) new = 0;
	    }
	  break;
	}
      break;

    case (NEWVALUE):
      {
	if (kind) {
	  float *newval;
	  newval = (float *) data;
	  *fnum = *newval;
	}
	else {
	  int *newval;
	  newval = (int *) data;
	  *num = *newval;
	}
	break;
      }
      
    case (NEW_NUMBER):		/* *DKY-23Jan91* */
      params->number = data;
      break;
    }

  return(0);
}


/*************************************************************************/
  


/* kind 0 is integer,  kind 1 is float, orient 0 is horizontal, orient 1 is vertical */


AreaSlider(W, orient, red, green, blue, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue, kind, number, max, min, area)     /* v 1.0b */
     struct Window *W;
     int orient;
     int red, green, blue, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue;
     char *number;
     char *max, *min, *area;
{
  struct ezslidestruct *params;
  int AreaSlideDraw(), AreaSlideDo();
  int ClearEzSlideDraw(), ClearEzSlideDo();

  params = (struct ezslidestruct *)malloc(sizeof(struct ezslidestruct));
  params->orient = orient;
  params->red = red;
  params->green = green;
  params->blue = blue;
  params->shadow = shadow;
  params->shadowred = shadowred;
  params->shadowgreen = shadowgreen;
  params->shadowblue = shadowblue;
  params->barred = barred;
  params->bargreen = bargreen;
  params->barblue = barblue;
  params->number = number;
  params->max = max;
  params->min = min;
  params->kind = kind;
  params->area = area;

  AttachCommand(W, DO, AreaSlideDo, (char *)params);
  AttachCommand(W, DRAW, AreaSlideDraw, (char *)params);
  AttachCommand(W, NEW_NUMBER, AreaSlideDo, (char *)params); /* added BS 1/23/91 */
  AttachCommand(W, NEWCOLOR, AreaSlideDo, (char *)params);
  AttachCommand(W, NEWBARCOLOR, AreaSlideDo, (char *)params);
  AttachCommand(W, SET_SHADOW, AreaSlideDo, (char *)params);
}



AreaSlideDraw(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct ezslidestruct *params;
  int x1, x2, y1, y2, screen, shadow, t, x, y;
  int barred, bargreen, barblue;
  float fx1, fx2, fy1, fy2;
  float cx1, cx2, cy1, cy2;
  float red, green, blue;
  struct clipstruct *cstruct;
  int *number, *max, kind, *min, *area;
  float *fnumber, *fmax, ratio, *fmin, *farea, aratio;

  screen = (W->display)->fildes;
  cstruct = (struct clipstruct *)data;
  cx1 = cstruct->x1;  cy1 = cstruct->y1;
  cx2 = cstruct->x2;  cy2 = cstruct->y2;
  clip_rectangle(screen, cx1, cx2, cy1, cy2);

  params = (struct ezslidestruct *)stuff;
  shadow = params->shadow;
  kind = params->kind;

  max = NULL;
  min = NULL;
  fmax = NULL;
  fmin = NULL;
  number = NULL;
  fnumber = NULL;
  area = NULL;
  farea = NULL;
  if (kind)
    {
      fnumber = (float *)params->number;
      fmax = (float *)params->max;
      fmin = (float *)params->min;
      farea = (float *)params->area;
    }
  else
    {
      number = (int *)params->number;
      max = (int *)params->max;
      min = (int *)params->min;
      area = (int *)params->area;
    }

  red = ((float)(params->red)/255.0);
  green = ((float)(params->green)/255.0);
  blue = ((float)(params->blue)/255.0);
  fill_color(screen, red, green, blue);

  red = ((float)(params->shadowred)/255.0);
  green = ((float)(params->shadowgreen)/255.0);
  blue = ((float)(params->shadowblue)/255.0);
  line_color(screen, red, green, blue);
  perimeter_color(screen, red, green, blue);
  interior_style(screen, INT_SOLID, TRUE);

  GetPoints(W, &x1, &y1, &x2, &y2);

  if (x1>x2) {t = x2; x2 = x1; x1 = t;}
  if (y1>y2) {t = y2; y2 = y1; y1 = t;}

  x2 -= (float)shadow; y2 -= (float)shadow;

  fx1=x1; fx2=x2;
  fy1=y1; fy2=y2;

  rectangle(screen, fx1, fy1, fx2, fy2);

  red = ((float)(params->barred)/255.0);
  green = ((float)(params->bargreen)/255.0);
  blue = ((float)(params->barblue)/255.0);
  fill_color(screen, red, green, blue);

  if (kind)
    {
      float div;
      if (!(*fmax-*fmin)) div = 1.0;
      else div = *fmax-*fmin;
      aratio = (*farea)/div;
      ratio = (*fnumber-*fmin)/div;
    }
  else
    {
      float div;
      if (!(*max-*min)) div = 1.0;
      else div = (float)(*max-*min);
      aratio = ((float)*area)/div;
      ratio = ((float)*number - (float)*min)/div;
    }

  if (params->orient)
    {
      float ratline, aratline;
      ratline = fy1+ratio*(fy2-fy1+1);
      ratline = MAX(ratline, fy1);
      aratline = ratline+aratio*(fy2-fy1+1.0);
      aratline = MIN(aratline, fy2-1.0);
      aratline -= ratline;
      rectangle(screen, fx1, ratline, fx2, ratline+aratline);
    }
  else
    {
      float ratline, aratline;
      ratline = fx1+ratio*(fx2-fx1+1.0);
      ratline = MAX(ratline, fx1);
      aratline = ratline+aratio*(fx2-fx1+1.0);
      aratline = MIN(aratline, fx2-1.0);
      aratline -= ratline;
      rectangle(screen, ratline, fy1, ratline+aratline, fy2);
    }


  for (t=0; t<shadow; t++)
    {
      fx1 += 1.0;
      fy1 += 1.0;
      fy2 += 1.0;
      fx2 += 1.0;

      move2d(screen, fx1, fy2);
      draw2d(screen, fx2, fy2);
      move2d(screen, fx2, fy1);
      draw2d(screen, fx2, fy2);
    }

  clip_rectangle((W->display)->fildes, (float)0.0, (float)((W->display)->width-1), 
		 0.0, (float)((W->display)->height-1));
  make_picture_current((W->display)->fildes);
}



AreaSlideDo(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  static int new = 0;
  static difx=0, dify=0;
  struct messenger *mstruct;
  struct message *trap;
  struct ezslidestruct *params;
  float *fnum=0, *fmax = 0, *fmin=0, *farea=0;
  int x, y, button, kind, *num=0, *max = 0, *min = 0, *area = 0, dx, dy;
  struct pickstruct *pick;
  float aratio, ratio, rat, newpos, div;

  params = (struct ezslidestruct *)stuff;
  kind = params->kind;
  if (kind)
    {
      fnum = (float *)params->number;
      fmax = (float *)params->max;
      fmin = (float *)params->min;
      farea = (float *)params->area;

      if (!(*fmax-*fmin)) div = 1.0;
      else div = *fmax-*fmin;
      aratio = (*farea)/div;
      ratio = (*fnum-*fmin)/div;
    }

  else
    {
      num = (int *)params->number;
      max = (int *)params->max;
      min = (int *)params->min;
      area = (int *)params->area;
      if (!(*max-*min)) div = 1.0;
      else div = (float)(*max-*min);

      aratio = ((float)*area)/div;
      ratio = (float)(*num - *min)/div;
    }
  switch( id)
    {
    case DO:
      {
	pick = (struct pickstruct *)data;
	x = pick->x;
	y = pick->y;

	dx = MAX(x, W->x1);
	dy = MAX(y, W->y1);
	dx = MIN(dx, W->x2);
	dy = MIN(dy, W->y2);
	button = pick->button;
	switch (button) {
	case (JUSTDOWN):
	  Dispatch(W, TRAP, MakeTrap(W, BEENUP), NULL);
	  new = 1;
	  if (new)
	    {
	      if (params->orient)
		rat = (float)(dy-W->y1)/(float)(W->height-params->shadow);
	      else
		rat = (float)(dx-W->x1)/(float)(W->width-params->shadow);

	      if (rat < ratio) newpos = rat;
	      else if (rat >ratio+aratio)
		{
		  if (rat+aratio > 1.0)
		    {
		      newpos = 1.0-aratio;
		      difx = (float)dx-(float)W->x1-newpos*(float)(W->width-params->shadow);
		      dify = (float)dy-(float)W->y1-newpos*(float)(W->height-params->shadow);
		    }
		  else newpos = rat;
		}
	      else    {
		newpos = ratio;
		difx = (float)dx-(float)W->x1-newpos*(float)(W->width-params->shadow);
		dify = (float)dy-(float)W->y1-newpos*(float)(W->height-params->shadow);
	      }

	      if (kind)
		{
		  *fnum = *fmin+newpos*(*fmax-*fmin);
		  *fnum = MAX(*fnum, *fmin);
		  *fnum = MIN(*fnum, *fmax);
		}
	      else
		{
		  *num = (int)(((float)*min + newpos*((float)*max-(float)*min))+.5);
		  *num = MAX(*num, *min);
		  *num = MIN(*num, *max);
		}

	      UpdateWin(W);
	    }
	  break;
	case (BEENDOWN):
	  if (new)
	    {
	      dx -= difx;
	      dy -= dify;

	      if (params->orient)
		rat = (float)(dy-W->y1)/(float)(W->height-params->shadow);
	      else
		rat = (float)(dx-W->x1)/(float)(W->width-params->shadow);

	      if (rat <= ratio) newpos = rat;
	      else
		{
		  if (rat+aratio > 1.0)
		    {
		      newpos = 1.0-aratio;
		    }
		  else newpos = rat;
		}
	      if (kind)
		{
		  *fnum = *fmin+newpos*(*fmax-*fmin);
		  *fnum = MAX(*fnum, *fmin);
		  *fnum = MIN(*fnum, *fmax);
		}
	      else
		{
		  *num = (int)(((float)*min + newpos*((float)*max-(float)*min))+.5);
		  *num = MAX(*num, *min);
		  *num = MIN(*num, *max);
		}

	      /* UpdateWin(W); */
	      /* I added the following IF stuff so that the slider wouldn't update
	       * if you don't move it, but still have the pen down.
	       * *DKY-24Jan91*
	       */
	      /* I changed the following to update anyway if double buffering -BOB 26May92 */

	      if (!(W->display->bits&DBUFFER)) {
		if( kind) {
		  if( params->oldFloatValue != (float)(*fnum))
		    UpdateWin( W);
		  params->oldFloatValue = (float)(*fnum);
		}
		else {
		  if( params->oldIntValue != (int)(*num))
		    UpdateWin( W);
		params->oldIntValue = (int)(*num);
		}
	      }
	      else UpdateWin(W);

	    }
	  break;
	case (BEENUP):
	  break;
	case (JUSTUP):
	  if (new)
	    {
	      new = 0;
	      difx = dify = 0;
	    }
	  break;

	}
      }
      break;

    case NEWCOLOR:		/* *DKY- 2Nov90* */
      {
	int *colors;
	colors = (int *)data;
	params->red = *(colors);
	params->green = *(colors+1);
	params->blue = *(colors+2);
      }
      break;

    case NEWBARCOLOR:		/* *DKY- 2Nov90* */
      {
	int *colors;
	colors = (int *)data;
	params->barred = *(colors);
	params->bargreen = *(colors+1);
	params->barblue = *(colors+2);
      }
      break;

    case SET_SHADOW:		/* *DKY- 2Nov90* */
      params->shadow = (int)data;
      break;
      
    case NEW_NUMBER:		/* BS - 1/23/91 */
      params->number = data;
      break;
      
    default:
      printf( "AreaSlideDo: unknown id %d\n", id);
   }
  return(0);
}

/****************************************************************/
/*
 * IndentedAreaSlider()
 *
 *   This is just about identical code to AreaSlider except that it makes an area slider where
 * the region indicating the visible region is indented from the background, so it's easier to
 * tell the slider bar from it's background.
 *   There are also new arguments:
 *           indent -- which is the number of pixels to indent the bar.
 *           barborder(red,green,blue) -- the color of the bar's border.
 *
 * *DKY-24Jul90*
 */

IndentedAreaSlider(W, orient, red, green, blue, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue, barborderred, barbordergreen, barborderblue, kind, indent, number, max, min, area)     /* v 1.0b */
     struct Window *W;
     int orient, kind, indent;
     int red, green, blue, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue;
     int barborderred, barbordergreen, barborderblue;
     char *number;
     char *max, *min, *area;
{
  struct indentedezslidestruct *params;
  int IndentedAreaSlideDraw(), IndentedAreaSlideDo();

  params = (struct indentedezslidestruct *)malloc(sizeof(struct indentedezslidestruct));
  params->orient = orient;
  params->red = red;
  params->green = green;
  params->blue = blue;
  params->shadow = shadow;
  params->shadowred = shadowred;
  params->shadowgreen = shadowgreen;
  params->shadowblue = shadowblue;
  params->barred = barred;
  params->bargreen = bargreen;
  params->barblue = barblue;
  params->barborderred = barborderred;
  params->barbordergreen = barbordergreen;
  params->barborderblue = barborderblue;
  params->number = number;
  params->max = max;
  params->min = min;
  params->kind = kind;
  params->area = area;
  params->indent = indent;
  params->oldIntValue = 0;
  params->oldFloatValue = 0.0;
  
  AttachCommand(W, DO, IndentedAreaSlideDo, (char *)params);
  AttachCommand(W, DRAW, IndentedAreaSlideDraw, (char *)params);
  AttachCommand(W, CHANGE_BAR_BORDER_COLOR, IndentedAreaSlideDraw, (char *)params);
  AttachCommand(W, CHANGE_BAR_AND_BORDER_COLOR, IndentedAreaSlideDraw, (char *)params);
  AttachCommand(W, CHANGE_BAR_COLOR, IndentedAreaSlideDraw, (char *)params);
}



IndentedAreaSlideDraw(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct indentedezslidestruct *params;
  int x1, x2, y1, y2, screen, shadow, t, x, y;
  int barred, bargreen, barblue;
  float fx1, fx2, fy1, fy2;
  float cx1, cx2, cy1, cy2;
  float red, green, blue;
  struct clipstruct *cstruct;
  int *number, *max, kind, *min, *area;
  float *fnumber, *fmax, ratio, *fmin, *farea, aratio;
  int indent;
  
  screen = (W->display)->fildes;
  params = (struct indentedezslidestruct *)stuff;

  switch ( id)
    {
    case (CHANGE_BAR_COLOR):
      {
	int *colors;
	colors = (int *)data;
	params->barred = (colors[0]);
	params->bargreen = (colors[1]);
	params->barblue = (colors[2]);

      }
      break;
      
    case (CHANGE_BAR_BORDER_COLOR):
      {
	int *colors;
	colors = (int *)data;
	params->barborderred = (colors[0]);
	params->barbordergreen = (colors[1]);
	params->barborderblue = (colors[2]);

      }
      break;
      
    case (CHANGE_BAR_AND_BORDER_COLOR):
      {
	int *colors;
	colors = (int *)data;
	params->barborderred = (colors[0]);
	params->barbordergreen = (colors[1]);
	params->barborderblue = (colors[2]);
	params->barred = (colors[0]);
	params->bargreen = (colors[1]);
	params->barblue = (colors[2]);
      }
      break;
      
    case (DRAW):
      {
	cstruct = (struct clipstruct *)data;
	cx1 = cstruct->x1;  cy1 = cstruct->y1;
	cx2 = cstruct->x2;  cy2 = cstruct->y2;
	clip_rectangle(screen, cx1, cx2, cy1, cy2);

	shadow = params->shadow;
	kind = params->kind;
	indent = params->indent;
  
	max = NULL;
	min = NULL;
	fmax = NULL;
	fmin = NULL;
	number = NULL;
	fnumber = NULL;
	area = NULL;
	farea = NULL;
	if (kind)
	  {
	    fnumber = (float *)params->number;
	    fmax = (float *)params->max;
	    fmin = (float *)params->min;
	    farea = (float *)params->area;
	  }
	else
	  {
	    number = (int *)params->number;
	    max = (int *)params->max;
	    min = (int *)params->min;
	    area = (int *)params->area;
	  }

	red = ((float)(params->red)/255.0);
	green = ((float)(params->green)/255.0);
	blue = ((float)(params->blue)/255.0);
	fill_color(screen, red, green, blue);

	red = ((float)(params->shadowred)/255.0);
	green = ((float)(params->shadowgreen)/255.0);
	blue = ((float)(params->shadowblue)/255.0);
	line_color(screen, red, green, blue);
	perimeter_color(screen, red, green, blue);
	interior_style(screen, INT_SOLID, TRUE);

	GetPoints(W, &x1, &y1, &x2, &y2);

	if (x1>x2) {t = x2; x2 = x1; x1 = t;}
	if (y1>y2) {t = y2; y2 = y1; y1 = t;}

	x2 -= (float)shadow; y2 -= (float)shadow;

	fx1=x1; fx2=x2;
	fy1=y1; fy2=y2;

	rectangle(screen, fx1, fy1, fx2, fy2);

	red = ((float)(params->barred)/255.0);
	green = ((float)(params->bargreen)/255.0);
	blue = ((float)(params->barblue)/255.0);
	fill_color(screen, red, green, blue);

	/* *DKY-24Jul90-01:33* */
	red = ((float)(params->barborderred)/255.0);
	green = ((float)(params->barbordergreen)/255.0);
	blue = ((float)(params->barborderblue)/255.0);
	line_color(screen, red, green, blue);
	perimeter_color(screen, red, green, blue);

	if (kind)
	  {
	    float div;
	    if (!(*fmax-*fmin)) div = 1.0;
	    else div = *fmax-*fmin;
	    aratio = (*farea)/div;
	    ratio = (*fnumber-*fmin)/div;
	  }
	else
	  {
	    float div;
	    if (!(*max-*min)) div = 1.0;
	    else div = (float)(*max-*min);
	    aratio = ((float)*area)/div;
	    ratio = ((float)*number - (float)*min)/div;
	  }

	if (params->orient)
	  {
	    float ratline, aratline;
	    ratline = fy1+ratio*(fy2-fy1+1);
	    ratline = MAX(ratline, fy1);
	    aratline = ratline+aratio*(fy2-fy1+1.0);
	    aratline = MIN(aratline, fy2-1.0);
	    aratline -= ratline;
	    rectangle(screen, fx1 + indent, ratline + 1, fx2 - indent, ratline+aratline);
	  }
	else
	  {
	    float ratline, aratline;
	    ratline = fx1+ratio*(fx2-fx1+1.0);
	    ratline = MAX(ratline, fx1);
	    aratline = ratline+aratio*(fx2-fx1+1.0);
	    aratline = MIN(aratline, fx2-1.0);
	    aratline -= ratline;
	    rectangle(screen, ratline + 1, fy1 + indent, ratline+aratline, fy2 - indent);
	  }


	for (t=0; t<shadow; t++)
	  {
	    fx1 += 1.0;
	    fy1 += 1.0;
	    fy2 += 1.0;
	    fx2 += 1.0;

	    move2d(screen, fx1, fy2);
	    draw2d(screen, fx2, fy2);
	    move2d(screen, fx2, fy1);
	    draw2d(screen, fx2, fy2);
	  }

	clip_rectangle((W->display)->fildes, (float)0.0, (float)((W->display)->width-1), 
		       0.0, (float)((W->display)->height-1));
	make_picture_current((W->display)->fildes);
      }
      break;
    }
}



IndentedAreaSlideDo(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  static int new = 0;
  static difx=0, dify=0;
  struct messenger *mstruct;
  struct message *trap;
  struct indentedezslidestruct *params;
  float *fnum=0, *fmax = 0, *fmin=0, *farea=0;
  int x, y, button, kind, *num=0, *max = 0, *min = 0, *area = 0, dx, dy;
  struct pickstruct *pick;
  float aratio, ratio, rat, newpos, div;

  params = (struct indentedezslidestruct *)stuff;
  kind = params->kind;
  if (kind)
    {
      fnum = (float *)params->number;
      fmax = (float *)params->max;
      fmin = (float *)params->min;
      farea = (float *)params->area;

      if (!(*fmax-*fmin)) div = 1.0;
      else div = *fmax-*fmin;
      aratio = (*farea)/div;
      ratio = (*fnum-*fmin)/div;
    }

  else
    {
      num = (int *)params->number;
      max = (int *)params->max;
      min = (int *)params->min;
      area = (int *)params->area;
      if (!(*max-*min)) div = 1.0;
      else div = (float)(*max-*min);

      aratio = ((float)*area)/div;
      ratio = (float)(*num - *min)/div;
    }
  if (id==DO)
    {
      pick = (struct pickstruct *)data;
      x = pick->x;
      y = pick->y;

      dx = MAX(x, W->x1);
      dy = MAX(y, W->y1);
      dx = MIN(dx, W->x2);
      dy = MIN(dy, W->y2);
      button = pick->button;
      switch (button) {
      case (JUSTDOWN):
	Dispatch(W, TRAP, MakeTrap(W, BEENUP), NULL);
	new = 1;
	if (new)
	  {
	    if (params->orient)
	      rat = (float)(dy-W->y1)/(float)(W->height-params->shadow);
	    else
	      rat = (float)(dx-W->x1)/(float)(W->width-params->shadow);

	    if (rat < ratio) newpos = rat;
	    else if (rat >ratio+aratio)
	      {
		if (rat+aratio > 1.0)
		  {
		    newpos = 1.0-aratio;
		    difx = (float)dx-(float)W->x1-newpos*(float)(W->width-params->shadow);
		    dify = (float)dy-(float)W->y1-newpos*(float)(W->height-params->shadow);
		  }
		else newpos = rat;
	      }
	    else    {
	      newpos = ratio;
	      difx = (float)dx-(float)W->x1-newpos*(float)(W->width-params->shadow);
	      dify = (float)dy-(float)W->y1-newpos*(float)(W->height-params->shadow);
	    }

	    if (kind)
	      {
		*fnum = *fmin+newpos*(*fmax-*fmin);
		*fnum = MAX(*fnum, *fmin);
		*fnum = MIN(*fnum, *fmax);
	      }
	    else
	      {
		*num = (int)(((float)*min + newpos*((float)*max-(float)*min))+.5);
		*num = MAX(*num, *min);
		*num = MIN(*num, *max);
	      }

	    UpdateWin(W);
	  }
	break;
         case (BEENDOWN):
	   if (new)
	     {
	       dx -= difx;
	       dy -= dify;

	       if (params->orient)
		 rat = (float)(dy-W->y1)/(float)(W->height-params->shadow);
	       else
		 rat = (float)(dx-W->x1)/(float)(W->width-params->shadow);

	       if (rat <= ratio) newpos = rat;
	       else
		 {
		   if (rat+aratio > 1.0)
		     {
		       newpos = 1.0-aratio;
		     }
		   else newpos = rat;
		 }
	       if (kind)
		 {
		   *fnum = *fmin+newpos*(*fmax-*fmin);
		   *fnum = MAX(*fnum, *fmin);
		   *fnum = MIN(*fnum, *fmax);
		 }
	       else
		 {
		   *num = (int)(((float)*min + newpos*((float)*max-(float)*min))+.5);
		   *num = MAX(*num, *min);
		   *num = MIN(*num, *max);
		 }

	       /* I added the following IF stuff so that the slider wouldn't update
		* if you don't move it, but still have the pen down.
		* *DKY-24Jan91*
		*/
	       /* I changed the following to update anyway if double buffering -BOB 26May92 */
	       
	       if (!(W->display->bits&DBUFFER)) {
		 if( kind) {
		   if( params->oldFloatValue != (float)(*fnum))
		     UpdateWin( W);
		   params->oldFloatValue = (float)(*fnum);
		 }
		 else {
		   if( params->oldIntValue != (int)(*num))
		     UpdateWin( W);
		   params->oldIntValue = (int)(*num);
		 }
	       }
	       else  UpdateWin(W); 
	     }
	break;
      case (BEENUP):
	break;
      case (JUSTUP):
	if (new)
	  {
	    new = 0;
	    difx = dify = 0;
	  }
	break;

      }
    }

  return(0);
}

  



#if 0

/***********************************************************************
 * DKY- 4Dec92* *DKY- 4Dec92* *DKY- 4Dec92*
 **********************************************************************
 */


ClearEzSlider(W, orient, shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue, kind, number, max, min) /* v 1.0b */
     struct Window *W;
     int orient;
     int shadow, shadowred, shadowgreen, shadowblue, barred, bargreen, barblue;
     char *number;
     char *max, *min;
{
  struct ezslidestruct *params;
  int ClearEzSlideDraw(), ClearEzSlideDo();

  params = (struct ezslidestruct *)malloc(sizeof(struct ezslidestruct));
  params->orient = orient;
  params->shadow = shadow;
  params->shadowred = shadowred;
  params->shadowgreen = shadowgreen;
  params->shadowblue = shadowblue;
  params->barred = barred;
  params->bargreen = bargreen;
  params->barblue = barblue;
  params->number = number;
  params->max = max;
  params->min = min;
  params->kind = kind;

  AttachCommand(W, DRAW, ClearEzSlideDraw, (char *)params);
  AttachCommand(W, DO, ClearEzSlideDo, (char *)params);
}



ClearEzSlideDraw(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct ezslidestruct *params;
  int x1, x2, y1, y2, screen, shadow, t, x, y;
  int barred, bargreen, barblue;
  float fx1, fx2, fy1, fy2;
  float cx1, cx2, cy1, cy2;
  float red, green, blue;
  struct clipstruct *cstruct;
  int *number, *max, kind, *min;
  float *fnumber, *fmax, ratio, *fmin;

  screen = (W->display)->fildes;
  cstruct = (struct clipstruct *)data;
  cx1 = cstruct->x1;
  cy1 = cstruct->y1;
  cx2 = cstruct->x2;
  cy2 = cstruct->y2;
  clip_rectangle(screen, cx1, cx2, cy1, cy2);

  params = (struct ezslidestruct *)stuff;
  shadow = params->shadow;
  kind = params->kind;

  max = NULL;
  min = NULL;
  fmax = NULL;
  fmin = NULL;
  number = NULL;
  fnumber = NULL;
  if (kind) fnumber = (float *)params->number, fmax = (float *)params->max, fmin = (float *)params->min;
  else number = (int *)params->number, max = (int *)params->max, min = (int *)params->min;

  red = ((float)(params->shadowred)/255.0);
  green = ((float)(params->shadowgreen)/255.0);
  blue = ((float)(params->shadowblue)/255.0);
  line_color(screen, red, green, blue);
  perimeter_color(screen, red, green, blue);
  interior_style(screen, INT_SOLID, TRUE);

  x1 = W->x1; y1 = W->y1; x2 = W->x2; y2 = W->y2;

  if (x1>x2) {t = x2; x2 = x1; x1 = t;}
  if (y1>y2) {t = y2; y2 = y1; y1 = t;}

  x2 -= (float)shadow; y2 -= (float)shadow;

  fx1=x1; fx2=x2;
  fy1=y1; fy2=y2;

  /*******************************/

  red = ((float)(params->barred)/255.0);
  green = ((float)(params->bargreen)/255.0);
  blue = ((float)(params->barblue)/255.0);
  fill_color(screen, red, green, blue);

  if (kind)
    ratio = (*fnumber-*fmin)/(*fmax-*fmin);
  else ratio = ((float)*number - (float)*min)/((float)*max-(float)*min);

  if (params->orient)
    {
      float ratline;
      ratline = fy2-ratio*(fy2-fy1-3.0)-2.0;
      rectangle(screen, fx1, ratline, fx2, ratline+4.0);
    }
  else
    {
      float ratline;
      ratline = fx1+ratio*(fx2-fx1-3.0)-2.0;
      rectangle(screen, ratline, fy1, ratline+4.0, fy2);
    }

  for (t=0; t<shadow; t++)
    {
      fx1 += 1.0;
      fy1 += 1.0;
      fy2 += 1.0;
      fx2 += 1.0;

      move2d(screen, fx1, fy2);
      draw2d(screen, fx2, fy2);
      move2d(screen, fx2, fy1);
      draw2d(screen, fx2, fy2);
    }

  clip_rectangle((W->display)->fildes, (float)0.0, (float)((W->display)->width-1), 
                 0.0, (float)((W->display)->height-1));
  make_picture_current((W->display)->fildes);
}





ClearEzSlideDo(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  static new = 0;
  struct messenger *mstruct;
  struct ezslidestruct *params;
  float *fnum=0, *fmax = 0, ratio, *fmin;
  int x, y, button, kind, *num=0, *max = 0, *min = 0, dx, dy;
  struct pickstruct *pick;

  params = (struct ezslidestruct *)stuff;
  kind = params->kind;
  if (kind) fnum = (float *)params->number, fmax = (float *)params->max, fmin = (float *)params->min;
  else num = (int *)params->number, max = (int *)params->max, min = (int *)params->min;

  pick = (struct pickstruct *)data;
  x = pick->x;
  y = pick->y;

  dx = MAX(x, W->x1);
  dy = MAX(y, W->y1);
  dx = MIN(dx, W->x2);
  dy = MIN(dy, W->y2);
  button = pick->button;
  switch (button)
    {
    case (JUSTDOWN):
      Dispatch(W, TRAP, MakeTrap(W, BEENUP), NULL);
      new = 1;

    case (JUSTUP):
    case (BEENDOWN):

      if (new)
        {
          if (params->orient)
            {
              ratio = (float)(W->y2-dy)/(float)(W->height-params->shadow);
            }
          else
            {
              ratio = (float)(dx-W->x1)/(float)(W->width-params->shadow);
            }

          if (kind)
            {
              *fnum = *fmin+ratio*(*fmax-*fmin);
              *fnum = MAX(*fnum, *fmin);
              *fnum = MIN(*fnum, *fmax);
            }
          else
            {
              *num = ((float)*min + ratio*((float)*max-(float)*min));
              *num = MAX(*num, *min);
              *num = MIN(*num, *max);
            }
          UpdateWin( W->parent);  /* *DKY-26Apr90-16:58* */
          if (id == JUSTUP) new = 0;
        }
      break;

    }
  return(0);
}

#endif
