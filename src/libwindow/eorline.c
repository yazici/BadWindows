#include <starbase.c.h>
#include <stdio.h>

static int ux1, uy1, ux2, uy2,
           dot_mode;
static float repeat_length = 100.0;

/*
     dot_mode can be set to one of the following:
        SOLID               0
	DASH                1
	DOT                 2
	DASH_DOT            3
	DASH_DOT_DOT        4
	LONG_DASH           5
	CENTER_DASH         6
	CENTER_DASH_DASH    7
*/	

SetLineDot_Mode(d)     /* v 1.0b */
{
  dot_mode = d;
}

SetLineRepeat_Length(r)
     float r;
{
  repeat_length = r;
}  


EorSolidLine(screen, x1, y1, x2, y2)     /* v 1.0b */
     int screen, x1, y1, x2, y2;
{
  drawing_mode(screen, 6);
  line_color(screen, 1.0, 1.0, 1.0);
  move2d (screen, (float)x1, (float)y1); 
  draw2d (screen, (float)x2, (float)y2);
  drawing_mode(screen, 3);
  make_picture_current(screen);
  line_type(screen, 0);
}


EorLine(screen, x1, y1, x2, y2)     /* v 1.0b */
     int screen, x1, y1, x2, y2;
{
  if (dot_mode)
    {
      line_type(screen, dot_mode);
      line_repeat_length(screen, repeat_length);
    }

  move2d(screen, (float)x1, (float)y1);
  draw2d(screen, (float)x2, (float)y2);
  line_type(screen, 0);
}



StartEorLine(screen, x1, y1, x2, y2)     /* v 1.0b */
     int screen, x1, y1, x2, y2;
{
  int xs, ys, xe, ye;

  xs = x1;   ys = y1;
  xe = x2;   ye = y2;

  ux1 = xs;
  uy1 = ys;
  ux2 = xe;
  uy2 = ye;

  line_color(screen, 1.0, 1.0, 1.0);
  drawing_mode(screen, 6);
  EorLine(screen, xs, ys, xe, ye);
  drawing_mode(screen, 3);
  make_picture_current(screen);
}



UpdateEorLine(screen, x1, y1, x2, y2)     /* v 1.0b */
     int screen, x1, y1, x2, y2;
{
  int xs, ys, xe, ye;

  xs = x1;   ys = y1;
  xe = x2;   ye = y2;

  if ((xs != ux1) || (ys != uy1) || (xe!=ux2) || (ye!=uy2))
    {
      drawing_mode(screen, 6);
      EorLine(screen, ux1, uy1, ux2, uy2);
      make_picture_current(screen); 
      EorLine(screen, xs, ys, xe, ye);
      drawing_mode(screen, 3);
      make_picture_current(screen);
      ux1 = xs;
      uy1 = ys;
      ux2 = xe;
      uy2 = ye;
    }
}



EndEorLine(screen)     /* v 1.0b */
     int screen;
{
  drawing_mode(screen, 6);
  EorLine(screen, ux1, uy1, ux2, uy2);
  drawing_mode(screen, 3);
  make_picture_current(screen);
}



DrawEorLine(screen, x1, y1, x2, y2)     /* v 1.0b */
     int screen, x1, y1, x2, y2;
{
  line_color(screen, 1.0, 1.0, 1.0);
  drawing_mode(screen, 6);
  EorLine(screen, x1, y1, x2, y2);
  drawing_mode(screen, 3);
  make_picture_current(screen);
}  

