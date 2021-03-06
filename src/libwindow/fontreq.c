/*************************************************************

  Tim Kukulski

  Summer 1990

  Visible Language Workshop


  Font Requestor

*************************************************************/
#include <stdio.h>
#include <math.h>
#include <starbase.c.h>
#include <windows.c.h>
#include <structures.h>

#define RFONTDIR "font/rects"
#define VLWFONTDIR "font/new_bitmaps"
#define POINTSIZELIST "font/#pointsizes#"

#define I_BEIGE          150,140,130
#define I_BLACK            0,  0,  0
#define I_EXTRA_WHITE    255,255,255
#define I_GRAPHITE        50, 50, 50
#define I_WHITE          250,250,250
#define I_PUTTY          120,120,115
#define I_GRAY50         128,128,128


#define NEWVERSION       0
/************************************************************************

  Instructions For Use:

  call early in your program,
  (just as you'd call MakeYesNo)

             MakeFontRequester(root,echo,   <---- just like in MakeWindow
                               font,size,   <---- just like in stringwin
			       x,y,         <---- x and y coordinates of
			                          the requester
			       c1,c2,c3,    <---- background color
			       b1,b2,b3,    <---- button color
			       t1,t2,t3,    <---- text color
			       h1,h2,h3);   <---- indicator color
       OR...

             DelayedMakeFontRequester( <with the same arguments> )            *DKY- 8Oct90*

	   { The difference is that DelayedMakeFontRequester() won't make the font requester until
	     it is first used -- thus saving time when starting your program, but causing a delay
	     the first time you request a font requester be put up. }

  and then when you want the requester, call-

char *GetFont(size)   returns a pointer to a string in the form <fontname>.<pointsize>

       int size      the size of the font to be returned
	             call with size = -1 to get the pointsize requester

int ChangeStringWinFont(W,id,data,stuff)    changes the font for a stringwin, preserving pointsize
                                            if pointsize is available in the font selected.
				       **** pass it (char *)Window

int ChangeStringWinFontB(W,id,data,stuff)
					    changes the font in a stringwin, ignoring pointsize
				       **** pass it (char *)Window

************************************************************************

  those darned Globals that these things need...
 ... structures too
**********************************************************************/

struct Window *FontReqWin = NULL, *SizeReqWin;
char *fontrequesterreturnvalue;
int sizerequesterreturnvalue;

int FontRequesterMade = NULL, UsingDelayedFontRequester = NULL;	/* *DKY- 8Oct90* */

/**********************************************************************

  Access Routines

**********************************************************************/

char *GetFont(size)							  /* v 0.01 */
     int size;
{
  char *string = NULL;
  char line[100];
  int sscanf();

  if( UsingDelayedFontRequester)					  /* *DKY- 8Oct90* */
    MakeDelayedFontRequester();

  fontrequesterreturnvalue = (char *)size;	/* to pass the pointsize to the routine */
  Query(FontReqWin);
  
  if((fontrequesterreturnvalue == NULL)||(fontrequesterreturnvalue == (char *)size)) return(NULL);
  sscanf(fontrequesterreturnvalue,"%[^.]%*c%d",line,&size);
  if(!BETWEEN(size,1,1000)) return(NULL);
  if(line == NULL) return(NULL);
  string = fontrequesterreturnvalue;
  fontrequesterreturnvalue = NULL;
  return(string);
}


char *
GetFontAt( root, x, y, size)	/* Same as GetFont, but you give it a location to put up the requestor. *DKY- 4Jun91* */
     struct Window *root;
     int x, y, size;
{
  TransformCoords( FontReqWin, x, y, FontReqWin->width, FontReqWin->height);
  FontReqWin->parent = root;
  FontReqWin->display = root->display;
  ChangeDisplay( FontReqWin);
  return( (char *)GetFont( size));
}


char *GetJustFont()     /* v 0.01 */
{
  char *string = NULL;

  if( UsingDelayedFontRequester)					  /* *DKY- 8Oct90* */
    MakeDelayedFontRequester();

  fontrequesterreturnvalue = NULL;
  Query(FontReqWin);
  if(fontrequesterreturnvalue == NULL) return(NULL);
  string = fontrequesterreturnvalue;
  fontrequesterreturnvalue = NULL;
  return(string);
}

int ChangeStringWinFont(W,id,data,stuff)			/* change stringwin font */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct stringstruct *mstruct;
  struct Window *M;
  char *strdup();
  char *string;
  int size;

  M = (struct Window *)stuff;
  mstruct=(struct stringstruct *)GetStuff(M,CHANGEFONT);
  if((string = GetFont(mstruct->size)) == NULL) return (-1);
  SendChangeFont(M,string,STRINGWIN);
  UpdateWin(M);
  return(0);
}

int ChangeStringWinFontB(W,id,data,stuff)			/* change stringwin font and size */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct Window *M;
  char *string;
  int size;

  M = (struct Window *)stuff;
  if((string = GetFont(-1)) == NULL) return (-1);
  SendChangeFont(M,string,STRINGWIN);
  UpdateWin(M);
  return(0);
}

int ChangeStringWinSize(W,id,data,stuff)			/* change font size in a StringWin */
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct stringstruct *mstruct;
  struct Window *M, *FindRoot();
  char *strdup();
  char *string;
  int size;

  M = (struct Window *)stuff;
  mstruct=(struct stringstruct *)GetStuff(M,CHANGEFONT);
  if((mstruct->size = GetPointSize(FindRoot(W),W->x2, W->y1,mstruct->font)) == 0) return (-1);
  SendMessage(M,CHANGEFONT,(char *)mstruct);
  size=mstruct->size;
  UpdateWin(M);
  return(size);
}

int MChangeStringWinSize(W)					   /* to be called from menus */
     struct Window *W;
{
  int ChangeStringWinSize();
  return(ChangeStringWinSize(W,NULL,NULL,W));
}
int MChangeStringWinFont(W)					   /* to be called from menus */
     struct Window *W;
{
  int ChangeStringWinFont();
  return(ChangeStringWinFont(W,NULL,NULL,W));
}

int MChangeStringWinBoth(W)				   /* to be called from menus */
     struct Window *W;
{
  int ChangeStringWinFontB();
  return(ChangeStringWinFontB(W,NULL,NULL,W));
}


/**********************************************************************

  Window Creation

**********************************************************************/

MakeFontRequester(root, echo, font, size, x, y,  c1, c2, c3,  b1, b2, b3,  t1, t2, t3,  h1, h2, h3)
     struct Window *root;
     struct curstruct *echo;
     int x, y, size, c1, c2, c3, b1, b2, b3, t1, t2, t3, h1, h2, h3;
     char *font;
{
  struct Window *W;
  struct Window *MakeCancelThingy();
  struct OutDev *display;
  struct InDev  *locator;
  int CloseWindow(), FreeString();

  if( FontReqWin) 		/* *DKY-15Feb91* */
    return( 0);
    
  display = root->display;
  locator = root->input;
  MakeSizeRequester(root,echo,font,size,x,y,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3);
  FontReqWin = (struct Window *)MakeWindow(display, locator, root,x,y, 200, 35, "FontReqWin");
  MoverII(FontReqWin,FontReqWin);
  Myify(FontReqWin, c1, c2, c3);
  StringWin(FontReqWin,"Select Family",CENTERED,3,font,24,8,t1,t2,t3,0);
  InstallWin(FontReqWin);
  MakeFamilies(FontReqWin, echo, font, 5,170, size ,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3);

  W = (struct Window *) MakeCancelThingy(FontReqWin, echo,font,0,FontReqWin->width,c1,c2,c3,t1,t2,t3);
  FunctionCaller(W,DO,FreeString,fontrequesterreturnvalue,/*JUSTUP*/JUSTDOWN);
  FunctionCaller(W,DO,CloseWindow,FontReqWin,/*JUSTUP*/JUSTDOWN);
  EchoWrap(W,echo,/*JUSTUP*/JUSTDOWN,0,0);
  InstallWin(W);
  StretchWin(FontReqWin,W);
 
  EchoWrap(FontReqWin,echo,JUSTDOWN/*|BEENDOWN|JUSTUP*/,0,0);
  UpdateSizes(FontReqWin);
  UnstallWin(FontReqWin);

  FontRequesterMade = 1;	/* *DKY- 8Oct90* */
}

MakeFamilies(root,echo,displayfont,x0,wd, pointsize, c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3)
     struct Window *root;
     struct curstruct *echo;
     char *displayfont;
     int x0,wd,pointsize;
     int c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3;
{
  struct Window *W;
  struct OutDev *display;
  struct InDev  *locator;
  char **button_list, **GetFamilyList();
  int i=0;

  display = root->display;
  locator = root->input;

  button_list=GetFamilyList();

  while(*(button_list+i))
      MakeFontButton(root,echo,displayfont,x0,wd,*(button_list+i++),pointsize,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3);

  free(button_list);
 return(0);

}
struct Window *MakeStringButton(root,echo,x,wd,font,size,string,wr,wg,wb,fr,fg,fb)
     struct Window *root;
     struct curstruct *echo;
     int x,wd;
     int wr,wg,wb,fr,fg,fb;
     char *font, *string;
{
  struct Window *W;
  struct OutDev *display;
  struct InDev  *locator;
  char *path;
  int ht;
  int rht;

  rht = root->y2 - root->y1 +1;
  display = root->display;
  locator = root->input;
  ht = size *3 /2;
  
  string = strdup(string);
  
  W = (struct Window *)MakeWindow(display,locator,root,x,rht,wd,ht,string);
  Myify(W,wr,wg,wb);
  StringWin(W,string,5,1,font,size,8,fr,fg,fb,0);
  return(W);
}


MakeFontButton(root,echo,displayfont,x,wd,font,size,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3)
     struct Window *root;
     struct curstruct *echo;
     int x,wd;
     char *displayfont, *font;
     int size;
     int c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3;
{
  struct Window *W, *M, *MakeFlavors();
  char *family, *GetFamily();
  int Query(), MoveRight(), SiblingMessenger();

  family=GetFamily(font);
  W = (struct Window *)MakeStringButton(root,echo,x,wd,font,size,family,b1,b2,b3,t1,t2,t3);
  free(family);
  M = (struct Window *)MakeFlavors(root,echo, displayfont, root->x2,root->y1, font,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3);
  IndicatorWin(W, OFF, h1,h2,h3, 3);

  SiblingMessenger(W,DO,W,TURN_OFF,NULL,JUSTDOWN|BEENDOWN|JUSTUP);
  Messenger(W,DO,W,TURN_ON,NULL,JUSTDOWN|BEENDOWN|JUSTUP);

  AttachCommand(W,DO,MoveRight,(char *)M);
  FunctionCaller(W, DO, Query, M, JUSTUP);
  EchoWrap(W,echo,JUSTDOWN|BEENDOWN|JUSTUP,0,0);
  InstallWin(W);
  StretchWin(root,W);
}

int MoveRight(W,id,data,stuff)
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct Window *M;
  M = (struct Window *)stuff;
  TransformCoords(M, W->parent->x2+5, W->parent->y1,M->width,M->height); 
  return(0);
}


struct Window *MakeFlavors(root,echo,displayfont,x,y,font,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3)
     struct Window *root;
     struct curstruct *echo;
     int x,y;
     char *displayfont, *font;
     int c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3;
{
  struct Window *W, *Button;
  struct Window *MakeLoadButton();
  struct Window *MakeCancelThingy();
  extern struct Window *Root1;
  struct OutDev *display;
  struct InDev  *locator;
  int ReadButtons();
  char *strdup();
  char *family, *GetFamily();
  int CloseWindow();

  display = root->display;
  locator = root->input;

  family=GetFamily(font);
  W = MakeWindow(display, locator, Root1, x,y, 300, 35,family);
  Myify(W,c1,c2,c3);
  StringWin(W,family,CENTERED,3,font,24,8,t1,t2,t3,0);
  
  InstallWin(W);

  MakeFlavorButtons(W,echo,5,190,font,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3);

  Button = (struct Window *)MakeLoadButton(W,echo,displayfont,0,W->width /2, c1,c2,c3,t1,t2,t3);
  AttachCommand(Button,DO,ReadButtons,W);
  EchoWrap(Button,echo,JUSTUP,0,0);
  InstallWin(Button);

  Button = (struct Window *)MakeCancelThingy(W,echo,displayfont,W->width /2 , W->width /2,c1,c2,c3,t1,t2,t3);
  ChildrenMessenger(Button,DO,FontReqWin,TURN_OFF,NULL,JUSTUP);
  ChildrenMessenger(Button,DO,Button->parent,TURN_OFF,NULL,JUSTUP);
  FunctionCaller(Button,DO,CloseWindow,W,JUSTUP);
  EchoWrap(Button,echo,JUSTUP,0,0);
  InstallWin(Button);

  StretchWin(W,Button);

  UnstallWin(W);
  EchoWrap(W,echo,0,0,0);
  return(W);
}

int FreeString(string)
     char *string;
{
  if(string != NULL)
    free(string);
  string = NULL;
  return(0);
}


MakeFlavorButtons(root,echo,x,wd,font,c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3)
     struct Window *root;
     struct curstruct *echo;
     int x,wd;
     char *font;
     int c1,c2,c3,b1,b2,b3,t1,t2,t3,h1,h2,h3;
{
  struct Window *W, *MakeStringButton();
  extern struct Window *Root1;
  struct OutDev *display;
  struct InDev  *locator;
  int i=0;
  char *family, *GetFamily();
  char **button_list, **GetFlavorList();

  display = root->display;
  locator = root->input;

  family = GetFamily(font);
  button_list=GetFlavorList(family);
  free(family);
  
  while(*(button_list+i) != NULL)
    {
      W = MakeStringButton(root,echo,5,275,*(button_list +i),16,*(button_list +i),b1,b2,b3, t1,t2,t3);
      IndicatorWin(W, ((i == 0) ? ON: OFF), h1,h2,h3, 3);  /* the first one is on, the rest off */

      SiblingMessenger(W,DO,W,TURN_OFF,NULL,JUSTDOWN|BEENDOWN|JUSTUP);
      Messenger(W,DO,W,TURN_ON,NULL,JUSTDOWN|BEENDOWN|JUSTUP);
      EchoWrap(W,echo,JUSTDOWN|BEENDOWN|JUSTUP,0,0);
      InstallWin(W);
      TransformCoords(root,root->x1,root->y1,root->width,W->y2-root->y1);
/*      StretchWin(root,W);*/
      i++;
    }
  free(button_list);
 return(0);
}



struct Window *MakeLoadButton(root,echo,font,x1,wd,b1,b2,b3,t1,t2,t3)
     struct Window *root;
     struct curstruct *echo;
     char *font;
     int x1,wd;
     int b1,b2,b3,t1,t2,t3;
{
  struct Window *W;
  struct OutDev *display;
  struct InDev  *locator;
  int CloseWindow();
  int ht;

  display = root->display;
  locator = root->input;

  ht = root->y2 - root->y1;

  W = (struct Window*)MakeWindow(display,locator,root, x1,ht,wd,35,"Load");
  Maker(W,x1,-35,x1+wd-1,-0);
  Myify(W,b1,b2,b3);
  StringWin(W,"LOAD",CENTERED,4,font,24,8,t1,t2,t3,0);
  return(W);
}



struct Window *MakeCancelThingy(root,echo,font,x1,wd,b1,b2,b3,t1,t2,t3)
     struct Window *root;
     struct curstruct *echo;
     char *font;
     int x1, wd;
     int b1,b2,b3,t1,t2,t3;
{
  struct Window *W;
  struct OutDev *display;
  struct InDev  *locator;
  int CloseWindow();
  int ht;

  display = root->display;
  locator = root->input;

  ht = root->y2 - root->y1;

  W = (struct Window*)MakeWindow(display,locator,root, x1,ht,wd,40,"Cancel");
  Maker(W,x1,-35,x1+wd-1,-0);
  Myify(W,b1,b2,b3);
  StringWin(W,"CANCEL",CENTERED,4,font,24,8,t1,t2,t3,0);
  return(W);
}

/********************************************************************

  window utilities

********************************************************************/

int SendChangeFont(W,string,windowtype)
     struct Window *W;             /*  a window */
     char *string;                 /*  a string in the form <fontname>.<pointsize> */
     int windowtype;
{
  struct stringstruct *mstruct;
  struct parastruct *pstruct;
  char line[80];
  char *strdup();
  int DrawAString(), DrawAParagraph();
  int size;
  if(windowtype == STRINGWIN){
    mstruct=(struct stringstruct *)GetStuff(W,CHANGEFONT);
    sscanf(string,"%[^.]%*c%d",line,&size);
    if(!BETWEEN(size,1,1000))
      return(0);
    mstruct->font = strdup(line);
    mstruct->size = size;
    SendMessage(W,CHANGEFONT,(char *)mstruct);
  }
  if(windowtype == PARAGRAPHWIN){
    pstruct=(struct parastruct *)GetStuff(W,CHANGEFONT);
    sscanf(string,"%[^.]%*c%d",line,&size);
    if(!BETWEEN(size,1,1000))
      return(0);
    pstruct->font = strdup(line);
    pstruct->size = size;
    SendMessage(W,CHANGEFONT,(char *)pstruct);
  }
  return(0);
}


Myify(W,r,g,b)
     struct Window *W;
     int r,g,b;
{
  /*    BevelRectify(W, 23, 5, r,g,b,0, I_BLACK 0);  */
  /*  RoundRectify(W, r,g,b,1,I_BLACK,0.03);  */
  Rectify(W,r,g,b,1,I_BLACK,1);
}

ReadButtons(W,id,data,stuff)
     struct Window *W;
     int id;
     char *data,*stuff;
{

  struct Window *M, *P, *tempwin, *FindRoot();
  struct List *Current, *L, *L2;
  char *strdup();
  char line[80];
  int ReadIndicator();
  int size;
  P = (struct Window *)stuff;
  M = P;
  L = P->port;

  if(L == NULL)
    {
      printf("L == NULL\n");
      return(NULL);
    }

  CopyList(L,&L2);

  for (Current = L2->Front; Current != L2 ; Current = Current->Front)
    {
      tempwin = Current->id;
      if(ReadIndicator(tempwin) == 1)
	break;
    }

  FreeList(L2);

  if(fontrequesterreturnvalue == NULL)
    {
      fontrequesterreturnvalue=strdup(tempwin->tag);
      sprintf(line,"%s/%s.rfnt",bw_file(RFONTDIR),fontrequesterreturnvalue);
      if(CHECKFILE(line))
	{
	  SendChildrenMessage(FontReqWin,TURN_OFF,NULL);
	  CloseWindow(FontReqWin);
	  CloseWindow(M);
	  return(1);
	}
    }
  else  /* then it has a number stored in it by my cheap hack */
    {
      size = (int)fontrequesterreturnvalue;
      fontrequesterreturnvalue=strdup(tempwin->tag);

      sprintf(line,"%s/%s.%d",bw_file(VLWFONTDIR),fontrequesterreturnvalue,size);
      while  (! CHECKFILE(line))
	{
	  size = 0;
	  if ((size = GetPointSize(FindRoot(W),W->x1, W->y2 + 10, fontrequesterreturnvalue)) == 0) return (0);
	  sprintf(line,"%s/%s.%d",bw_file(VLWFONTDIR),fontrequesterreturnvalue,size);
	}
      free(fontrequesterreturnvalue);
      sprintf(line,"%s.%d",fontrequesterreturnvalue,size);
      fontrequesterreturnvalue=strdup(line);

      SendChildrenMessage(FontReqWin,TURN_OFF,NULL);
      CloseWindow(FontReqWin);
      CloseWindow(M);
      return(size);
    }
  return(0);
}

int GetPointSize(Root,x,y,font)
     struct Window *Root;
     int x,y;
     char *font;
{
  struct OutDev *display;
  struct listwinstruct *newstruct;
  int size;

  if( UsingDelayedFontRequester)	/* *DKY- 8Oct90* */
    MakeDelayedFontRequester();

  display = Root->display;
  SizeReqWin->parent = Root;
  newstruct = (struct listwinstruct *)malloc(sizeof(struct listwinstruct));
  SizeReqWin->display = display;
  ChangeDisplay(SizeReqWin);
  sizerequesterreturnvalue = 0;
  newstruct->listbase = font;
  SendMessage(SizeReqWin,NEWKEY,(char *)newstruct);
  free(newstruct); 
  TransformCoords(SizeReqWin, x, y, SizeReqWin->width, SizeReqWin->height);
  Query(SizeReqWin);
  size =  sizerequesterreturnvalue;
  return(size);
}

MakeSizeRequester(root, echo, font, size, x,y,  c1, c2, c3,  b1, b2, b3,  t1, t2, t3,  h1, h2, h3)
     struct Window *root;
     struct curstruct *echo;
     int x, y, size, c1, c2, c3, b1, b2, b3, t1, t2, t3, h1, h2, h3;
     char *font;
{
  struct Window *W;
  struct Window *MakeCancelThingy();
  struct OutDev *display;
  struct InDev  *locater;
  int CloseWindow();

  display = root->display;
  locater = root->input;

  SizeReqWin = MakeWindow(display, locater, root, 0,0, 100, 70, "SizeReqWin");
  Myify(SizeReqWin, c1, c2, c3);
  StringWin(SizeReqWin,"Size",CENTERED,3,font,24,8,t1,t2,t3,0);
  PointSizeify(SizeReqWin, root, echo, font, size, 8, 5, 35,
	       c1, c2, c3, b1, b2, b3, t1, t2, t3, h1, h2, h3);
  EchoWrap(SizeReqWin,echo,JUSTUP,0,0);
  InstallWin(SizeReqWin);

  W = (struct Window *) MakeCancelThingy(SizeReqWin, echo,font,0,SizeReqWin->width,c1,c2,c3,t1,t2,t3);
  FunctionCaller(W,DO,CloseWindow,SizeReqWin,JUSTUP);
  EchoWrap(W,echo,JUSTUP,0,0);
  InstallWin(W);
  StretchWin(SizeReqWin,W);

  UpdateSizes(SizeReqWin);
  UnstallWin(SizeReqWin);
}

PointSizeify(W, root, echo, displayfont, size, bits, x, y, c1, c2, c3,  b1, b2, b3,  t1, t2, t3,  h1, h2, h3)
     struct Window *W, *root;
     struct curstruct *echo;
     char *displayfont;
     int size, bits, x, y, c1, c2, c3, b1, b2, b3, t1, t2, t3, h1, h2, h3;
{
  int PointSizeWin();
  struct listwinstruct *params;
  
  if ((params = (struct listwinstruct *) malloc(sizeof(struct listwinstruct))) == NULL)
    printf("Error mallocing listwinstruct\n");

  params->Root = root;
  params->echo = echo;
  
  params->list = NULL;
  params->listbase = NULL;
  
  params->no_of_items = 0;

  params->displayfont = strdup(displayfont);
  params->size = size;
  params->bits = bits;
  
  params->x = x, params->y = y;

  params->c1 = c1,  params->c2 = c2, params->c3 = c3;
  params->b1 = b1,  params->b2 = b2, params->b3 = b3;
  params->t1 = t1,  params->t2 = t2, params->t3 = t3;
  params->h1 = h1,  params->h2 = h2, params->h3 = h3;
  
  AttachCommand(W,DRAW,PointSizeWin,params);
  AttachCommand(W,DO,PointSizeWin,params);
  AttachCommand(W,NEWKEY,PointSizeWin,params);
}

int PointSizeWin(W,id,data,stuff)
     struct Window *W;
     int id;
     char *data, *stuff;
{
  struct clipstruct *cstruct;
  struct listwinstruct *params, *newstruct;
  struct pickstruct *pick;
  struct curstruct *echo;

  float cx1,cx2,cy1,cy2;

  int screen;
  int x, y, button;
  int xx;
  int size;
  int i, j;
  char **GetPointSizeList();
  char *pointer;
  
  screen = (W->display)->fildes;
  params = (struct listwinstruct *)stuff;
  size = params->size * 3 / 2;
  switch (id)
    {
    case (DRAW):
      cstruct = (struct clipstruct *)data;
      cx1 = cstruct->x1;  cy1 = cstruct->y1;
      cx2 = cstruct->x2;  cy2 = cstruct->y2;
      clip_rectangle(screen,cx1,cx2,cy1,cy2);

      set_font(params->displayfont, params->size, params->bits);
      set_monitor(W->display->bits, W->display->fildes);
      set_font_color(params->t1,params->t2,params->t3,0);
      set_text_clip_rectangle(cstruct->x1, cstruct->y1, cstruct->x2, cstruct->y2);
      set_char_space(0);
      set_word_space(0);

      fill_color(screen, FCON(params->b1), FCON(params->b2), FCON(params->b3));
      line_color(screen, 0.0, 0.0, 0.0);
      interior_style(screen, INT_SOLID, TRUE);
      for(i=0; i <= params->no_of_items ; i++)
	{
	  rectangle(screen, (float)(W->x1+params->x), (float)(W->y1+params->y+size*i), 
		    (float)(W->x2 - 5), (float)(W->y1+params->y+size*(i+1)));
	  
	  make_picture_current(screen);	/* *DKY- 4Oct90* */
	  xx = W->x1+W->width/2 - string_width(*(params->list+i))/2;
	  display_string(params->list[i], xx, W->y1+params->y+size*i+size/4);
	  make_picture_current(screen);	/* *DKY- 4Oct90* */
	}
      clip_rectangle(screen, 0.0, 2047.0, 0.0, 1023.0);
      make_picture_current(screen);
      break;

    case (DO):
      pick = (struct pickstruct *)data;
      button = pick->button;
      switch (button)
	{
	case (JUSTDOWN):  
	  break;
	case (BEENUP):    
	  break;
	case (JUSTUP):
	  if( BETWEEN((y = (pick->y - params->y - W->y1)/size),0,params->no_of_items))
	    {
	      sscanf(*(params->list + y),"%d",&sizerequesterreturnvalue);
	      CloseWindow(W);
	      return(0);
	    }
	  break;
	case (BEENDOWN):  
	  break;
	}
      break;
    case (NEWKEY):
      newstruct = (struct listwinstruct *)data;
      params->listbase = newstruct->listbase;
      if(params->list)						  /* clears old list */
	{
	  for(i = 0; params->list [i] ; i++)
	    free(params->list[i]);
	  free(params->list);
	}
      params->list = GetPointSizeList(params->listbase);		  /* gets new list */
      for(i = 0; *(params->list + i) != NULL ; i++);			  /* counts items */
      i--;
      
      InstallWin(W);
      UpdateSizes(W);
      TransformCoords(W, 0,0, W->width, W->height - params->no_of_items*size + i*size);	/* sizes window */
      UpdateSizes(W);
      UnstallWin(W);
      params->no_of_items = i;
      for(i = 0; i < params->no_of_items ; i++)				  /*  sorts the list  */
	for(j = i; j <= params->no_of_items ; j++)
	  if(atoi(*(params->list+j)) < atoi(*(params->list+i)))
	    SWAP(*(params->list+j),*(params->list+i),pointer);
      break;
    }
  return(0);
}
/********************************************************************

  string and file routines

********************************************************************/

/************ this is the version without its own temporary files ***********/

char **GetFamilyList()
{
  char ***arrays, ***GetFontArrays();
  int i=0,entry=0;
  char *GetFamily(), *strdup();
  char *pointer, *pointer2, *item;
  char **familylist, **rfontlist;
  char line[100], extention[20], *family;
  
  familylist = (char **)malloc(100*sizeof(char *));
  rfontlist = *((arrays = GetFontArrays())+2);

  while(rfontlist[i])
    {
      if(sscanf(rfontlist[i++],"%[^.]%s",line,extention) != 2)
	continue;
      if(strcmp(extention,".rfnt"))
	continue;
      if(!entry)
	familylist[entry++] = strdup(line);	    /* the item is added to the list */
      else if(strcmp( (pointer = GetFamily(familylist[entry-1])),(pointer2 = GetFamily(line))))  /* if original */
	{
	  familylist[entry++] = strdup(line);	    /* the item is added to the list */
	  free(pointer);
	  free(pointer2);
	}
    }
  *(familylist + entry)= NULL;				    /* put a NULL at the end of the list */
  free(arrays);
  return(familylist);
}
  
  
char **GetFlavorList(family)
     char *family;
{
  char ***arrays, ***GetFontArrays();
  char **flavorlist, **namelist;
  char *GetFamily(), *strdup();
  char *pointer = NULL, *item;
  char *temp;
  char line[100], extention[10];
  int i=0,entry=0;
  
  namelist = *((arrays = GetFontArrays())+2);
  flavorlist = (char **)malloc(100*sizeof(char *));

  while(namelist[i])
    {
      if(sscanf(namelist[i++],"%[^.]%s",line,extention) != 2)
	continue;
      if(strcmp(extention,".rfnt"))
	continue;
      temp = GetFamily(line);
      if(strcmp(temp,family) == 0)			    /* if same family */
	*(flavorlist+entry++) = strdup(line);	    /* the item is added to the list */
      free(temp);
    }
  *(flavorlist + entry)= NULL;				    /* put a NULL at the end of the list */
  free(arrays);
  return(flavorlist);
}

char **GetPointSizeList(font)
     char *font;
{
  char **namelist, **returnlist;
  char ***arrays, ***GetFontArrays();
  int *sizelist;
  int i,j;

  arrays = GetFontArrays();
  namelist = arrays[0], sizelist = (int *)arrays[1];
  returnlist = (char **)malloc(50*sizeof(char*));
  
  i = 0, j = 0;
  while(namelist[i])
    {
      if(strcmp(font,namelist[i++]) == 0)                          /* if this is the font we want */
	{
	  returnlist[j]=(char*)malloc(5*sizeof(char));
	  sprintf(returnlist[j],"%d",sizelist[i-1]);
	  j++;
	}
    }
  returnlist[j] = NULL;    /* put a null at the end of the list */
  free(arrays);
  return(returnlist);          /* return the list */
}
#if 0

MakeTempFile()
{
  char line[300];
  printf("Font Requestor: Hang On... \n");
  printf("                Making Work Files...\n");
  sprintf(line,"ls -1 %s | sed '/^\\./d' | sed  >%s 's/.*\\///'",bw_file(VLWFONTDIR),bw_file(POINTSIZELIST));
  system(line);       /* strip path 
  printf("                One more...\n");
  sprintf(line,"ls -1 %s/*.rfnt | sed '/\\..*\\./d' | sed 's/.*\\///' | sed > tempfile 's/\\.rfnt//'",bw_file(RFONTDIR));
  system(line);  
  printf("                ...Done.\n");
}

char **GetFamilyList()
{
  FILE *fopen(), *fp;
  int i=0;
  char *GetFamily(), *strdup();
  char *pointer = NULL, *pointer2 = NULL;
  char **familylist;
  char line[300], family[100], previousfamily[100];

  familylist = (char **)malloc(50*sizeof(char *));

  MakeTempFile();

  fp = fopen("tempfile","r");
  fscanf(fp,"%s",line);					 /* get the first item */

  *familylist=strdup(line);

  for (i=1; (fscanf(fp,"%s",line) != EOF) ;)
    {
      if(strcmp((pointer = GetFamily(*(familylist+i-1))) ,(pointer2 = GetFamily(line))) != 0)
	{                                               /* if this IS an original item: */
	  *(familylist+i++) = strdup(line);             /* the item is added to the list */
	}
      free(pointer);
      free(pointer2);                                        /* free the space made by GetFamily */
    }
  *(familylist + i)= NULL;                                  /* put a NULL at the end of the list */
  fclose(fp);
  return(familylist);
}
char**GetFlavorList(family)
     char *family;
{
  FILE *fopen(), *fp;
  char *GetFamily();
  char *strdup();
  char line[300];
  char *pointer = NULL;
  char **flavorlist;
  int i;

  flavorlist = (char **)malloc(70*sizeof(char *));

  fp = fopen("tempfile","r");

  for (i=0; (fscanf(fp,"%s",line) != EOF) ;)
    {
      if(strcmp(family,(pointer = GetFamily(line))) == 0)   /* if this is the family we want */
	*(flavorlist+i++)=strdup(line);                     /* add a copy of the line to the flavorlist */
      free(pointer);                                        /* free the spce made by GetFamily */
    }
  *(flavorlist + i)= NULL;     /* put a null at the end of the list */
  fclose(fp);                  /* close the file */
  return(flavorlist);          /* return the list */
}


/*
  
char **GetPointSizeList(font)
     char *font;
{
  FILE *fopen(), *fp;
  char *strdup(), *fgets();
  char **sizelist;
  char line[300],name[80], size[5];
  int i;

  sizelist = (char **)malloc(70*sizeof(char *));
  fp = fopen(POINTSIZELIST,"r");
  i = 0;
  while(fgets(line, 299, fp) != NULL)
    {
      sscanf(line,"%[^.]%*c%s",name,size);
      if(strcmp(font,name) == 0)                         
	{
	  sizelist[i]=strdup(size);                    
	  i++;
	}
    }
  sizelist[i] = NULL;   
  fclose(fp);                
  return(sizelist);         
}

*/
#endif
char *GetFamily(font)
     char *font;
{
  char *family;
  int position, strcspn();

  position = strcspn(font,"_");            /* finds length of string wihtout "_" */
  family = (char *)malloc(position +2);
  strncpy(family,font,position);
  *(family + position) = '\0';
  return(family);
}

/* *DKY- 8Oct90* */
struct Window *FR_root;
struct curstruct *FR_echo;
int FR_x, FR_y, FR_size, FR_c1, FR_c2, FR_c3, FR_b1, FR_b2, FR_b3, FR_t1, FR_t2, FR_t3, FR_h1, FR_h2, FR_h3;
char *FR_font;

DelayedMakeFontRequester( root, echo, font, size, x, y,  c1, c2, c3,  b1, b2, b3,  t1, t2, t3,  h1, h2, h3) /* *DKY- 8Oct90* */
     struct Window *root;
     struct curstruct *echo;
     int x, y, size, c1, c2, c3, b1, b2, b3, t1, t2, t3, h1, h2, h3;
     char *font;
{
  FR_root = root;
  FR_echo = echo;
  FR_x = x; FR_y = y; FR_size = size;
  FR_c1 = c1; FR_c2 = c2; FR_c3 = c3;
  FR_b1 = b1; FR_b2 = b2; FR_b3 = b3;
  FR_t1 = t1; FR_t2 = t2; FR_t3 = t3;
  FR_h1= h1; FR_h2 = h2; FR_h3 = h3;
  FR_font = font;
  FontRequesterMade = NULL;
  UsingDelayedFontRequester = 1;
}


MakeDelayedFontRequester()	/* *DKY- 8Oct90* */
{
  if( !FontRequesterMade) {
    MakeFontRequester( FR_root, FR_echo, FR_font, FR_size, FR_x, FR_y,
		      FR_c1, FR_c2, FR_c3,  FR_b1, FR_b2, FR_b3,  FR_t1, FR_t2, FR_t3,  FR_h1, FR_h2, FR_h3);
    FontRequesterMade = 1;
  }
}
