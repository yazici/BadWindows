#include <starbase.c.h>
#include <stdio.h>
#include <structures.h>
#include <windows.c.h>



StringWin(W, string, x, y, font, size, bits, red, green, blue, t)     
     struct Window *W;
     char *string, *font;
     int size, bits, red, green, blue, t, x, y;
{
  struct stringstruct *params;
  int DrawAString();

  params = (struct stringstruct *) malloc(sizeof(struct stringstruct));
  params->font = font;

  params->string = string;

  params->size = size;
  params->bits = bits;
  params->x = x;
  params->y = y;
  params->red = red;
  params->green = green;
  params->blue = blue;
  params->t = t;
  params->char_space = 0;
  params->word_space = 0;
  AttachCommand(W, DRAW, DrawAString, params);
  AttachCommand(W, ATTRIBUTES, DrawAString, params);
  AttachCommand(W, CHANGECOLOR, DrawAString, params);
  AttachCommand(W, CHANGETRANS, DrawAString, params);
  AttachCommand(W, CHANGEFONT, DrawAString, params);
  AttachCommand(W, CHANGESTRING, DrawAString, params);
  AttachCommand(W, FREESTRING, DrawAString, params); /* *DKY-30Aug90* */
  AttachCommand(W, CHANGE_X_POS, DrawAString, params); /* *DKY-13May90* */
  AttachCommand(W, CHANGE_Y_POS, DrawAString, params); /* *DKY-13May90* */
  AttachCommand(W, SET_SIZE, DrawAString, params); /* *DKY-13May90* */
  AttachCommand(W, GET_STRINGWIN_COLOR, DrawAString, params); /* *DKY-15May90* */
  AttachCommand(W, SETCHARSPACE, DrawAString, params);
  AttachCommand(W, SETWORDSPACE, DrawAString, params);
}

/* StringWin with parameters for character and word spacing ** G. Colby 9/5/91 */ 
StringWinWithSpacing(W, string, x, y, font, size, bits, red, green, blue, t, char_space, word_space)     /* v 1.0b */
     struct Window *W;
     char *string, *font;
     int size, bits, red, green, blue, t, x, y, char_space, word_space;
{
  struct stringstruct *params;
  int DrawAString();

  params = (struct stringstruct *) malloc(sizeof(struct stringstruct));
  params->font = font;

  params->string = string;

  params->size = size;
  params->bits = bits;
  params->x = x;
  params->y = y;
  params->red = red;
  params->green = green;
  params->blue = blue;
  params->t = t;
  params->char_space = char_space;
  params->word_space = word_space;
  AttachCommand(W, DRAW, DrawAString, params);
  AttachCommand(W, ATTRIBUTES, DrawAString, params);
  AttachCommand(W, CHANGECOLOR, DrawAString, params);
  AttachCommand(W, CHANGETRANS, DrawAString, params);
  AttachCommand(W, CHANGEFONT, DrawAString, params);
  AttachCommand(W, CHANGESTRING, DrawAString, params);
  AttachCommand(W, FREESTRING, DrawAString, params); /* *DKY-30Aug90* */
  AttachCommand(W, CHANGE_X_POS, DrawAString, params); /* *DKY-13May90* */
  AttachCommand(W, CHANGE_Y_POS, DrawAString, params); /* *DKY-13May90* */
  AttachCommand(W, SET_SIZE, DrawAString, params); /* *DKY-13May90* */
  AttachCommand(W, GET_STRINGWIN_COLOR, DrawAString, params); /* *DKY-15May90* */
  AttachCommand(W, SETCHARSPACE, DrawAString, params);
  AttachCommand(W, SETWORDSPACE, DrawAString, params);
}



DrawAString(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct stringstruct *mstruct, *newstruct;
  struct clipstruct *cstruct;
  int screen, cx1, cx2, cy1, cy2;
  int *colors, *trans;
  int *temp;

  if (id==DRAW)
    {
      mstruct = (struct stringstruct *)stuff;
      cstruct = (struct clipstruct *)data;

      set_font(mstruct->font, mstruct->size, mstruct->bits);
      set_monitor(W->display->bits, W->display->fildes);
      set_font_color(mstruct->red, mstruct->green, mstruct->blue, mstruct->t);
      set_text_clip_rectangle(cstruct->x1, cstruct->y1, cstruct->x2, cstruct->y2);
      set_char_space(mstruct->char_space);
      set_word_space(mstruct->word_space);
      if (mstruct->string)
	{
	  int xx, yy;
	  
	  if (mstruct->x == CENTERED)
	    xx = (W->width - string_width(mstruct->string))/2;
	  else if (mstruct->x < 0)
	    xx = W->width + mstruct->x - string_width(mstruct->string);
	  else xx = mstruct->x;
	  
	  if (mstruct->y == CENTERED)
	    yy = W->height/2 - string_height(mstruct->string)/2;
	  else if (mstruct->y < 0)
	    yy = W->height + mstruct->y - string_height(mstruct->string);
	  else yy = mstruct->y;

	  lockdev(W->display->fildes);
	  display_string(mstruct->string, W->x1+xx, W->y1+yy);
	  unlockdev(W->display->fildes);
	}
      set_char_space( 0);	/* *DKY-18Mar90-17:29* */
      set_word_space( 0);	/* *DKY-18Mar90-17:29* */
    }

  else if (id == ATTRIBUTES)
    {
      mstruct = (struct stringstruct *)stuff;
      mstruct->string = data;
    }

  else if (id == CHANGESTRING)
    {
      mstruct = (struct stringstruct *)stuff;
      mstruct->string = data;
    }

  else if (id == FREESTRING)	/* optional argument is a new string *DKY-30Aug90* */
    {
      mstruct = (struct stringstruct *)stuff;
      
      if( mstruct->string)
	free( mstruct->string);
      mstruct->string = data;
    }

  else if (id == CHANGECOLOR)
    {
      CloseWindow(W);
      mstruct = (struct stringstruct *)stuff;
      colors = (int *)data;	/* I changed this to take an *int, 12/5/89, dsmall */
      mstruct->red = (colors[0]);
      mstruct->green = (colors[1]);
      mstruct->blue = (colors[2]);
      OpenWindow(W);
    }

  else if (id == CHANGETRANS)
    {
      mstruct = (struct stringstruct *)stuff;
      trans = (int *)data;
      mstruct->t = *trans;
    }

  else if (id == SETCHARSPACE)
    {
      mstruct = (struct stringstruct *)stuff;
      temp = (int *)data;
      mstruct->char_space = *temp;
    }

  else if (id == SETWORDSPACE)
    {
      mstruct = (struct stringstruct *)stuff;
      temp = (int *)data;
      mstruct->word_space = *temp;
    }

  else if (id == CHANGEFONT)
    {
      mstruct = (struct stringstruct *)stuff;
      newstruct = (struct stringstruct *)data;
      mstruct->font = newstruct->font;
      mstruct->size = newstruct->size;
      mstruct->bits = newstruct->bits;
    }

  else if (id == CHANGE_X_POS)	/* *DKY-13May90* */
    {
      mstruct = (struct stringstruct *)stuff;
      temp = (int *)data;
      mstruct->x = *temp;
    }

  else if (id == CHANGE_Y_POS)	/* *DKY-13May90* */
    {
      mstruct = (struct stringstruct *)stuff;
      temp = (int *)data;
      mstruct->y = *temp;
    }

  else if (id == SET_SIZE)	/* *DKY-13May90* */
    {
      mstruct = (struct stringstruct *)stuff;
      temp = (int *)data;
      mstruct->size = *temp;
    }

  else if (id == GET_STRINGWIN_COLOR) {	/* *DKY-15May90* */
    int **colors;
    /* data is a pointer to an array of 3 integers */
    colors = (int **)data;
    mstruct = (struct stringstruct *)stuff;

    *colors = (int *)&(mstruct->red);
    *(colors+1) = (int *)&(mstruct->green);
    *(colors+2) = (int *)&(mstruct->blue);
  }

  return (0);
}



RStringWin(W, string, x, y, parms)     /* v 1.0b */
     struct Window *W;
     char *string;
     int x, y;
     struct stringstruct *parms;
{
  struct rstringstruct *rparms;
  int DrawRString();

  rparms = (struct rstringstruct *) malloc(sizeof(struct rstringstruct));
  rparms->string = string;
  rparms->parms = parms;
  rparms->x = x;
  rparms->y = y;
  AttachCommand(W, DRAW, DrawRString, rparms);
  AttachCommand(W, CHANGE_X_POS, DrawRString, rparms);
  AttachCommand(W, CHANGE_Y_POS, DrawRString, rparms);
}



DrawRString(W, id, data, stuff)     /* v 1.0b */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  int *temp;

  switch( id)
    {
    case DRAW:
      {
	struct stringstruct *mstruct;
	struct rstringstruct *rparms;
	struct clipstruct *cstruct;
	int screen, cx1, cx2, cy1, cy2;

	rparms = (struct rstringstruct *)stuff;
	mstruct = rparms->parms;
	cstruct = (struct clipstruct *)data;

	lockdev(W->display->fildes);
	set_font(mstruct->font, mstruct->size, mstruct->bits);
	set_monitor(W->display->bits, W->display->fildes);
	set_font_color(mstruct->red, mstruct->green, mstruct->blue, mstruct->t);
	set_text_clip_rectangle(cstruct->x1, cstruct->y1, cstruct->x2, cstruct->y2);
	display_string(rparms->string, W->x1+rparms->x, W->y1+rparms->y);
	unlockdev(W->display->fildes);
	
      }
      break;

    case CHANGE_X_POS:
      {
	struct rstringstruct *rparms;

	rparms = (struct rstringstruct *)stuff;

	temp = (int *)data;
	rparms->x = *temp;
      }
      break;
      

    case CHANGE_Y_POS:
      {
	struct rstringstruct *rparms;

	rparms = (struct rstringstruct *)stuff;

	temp = (int *)data;
	rparms->y = *temp;
      }
      break;

    default:
      printf( "WARNING: DrawRString() - unknown message id: %d\n", id);
    }
}

















