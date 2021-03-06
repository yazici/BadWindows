#include <stdio.h>
#include <math.h>
#include <starbase.c.h>
#include <windows.c.h>
#include <structures.h>
#include <ctype.h>

extern struct Window *Palette;
extern int NextColorIndex;

float Shmax = 378.0, Shmin = 0.0, Shgap = 18.0;
float Ssmax = 1.05, Ssmin = 0.0, Ssgap = .05;    
float Svmax = 1.05, Svmin = 0.0, Svgap = .05;    
float Shue,Ssat,Sval;

int Srmax = 268, Srmin = 0, Srgap = 13; 
int Sgmax = 268, Sgmin = 0, Sggap = 13;
int Sbmax = 268, Sbmin = 0, Sbgap = 13, colormode = 0;
int Sred,Sgreen,Sblue, Ured, Ugreen, Ublue;

struct Window *hlabel, *hslider, *vslider,*sslider;
struct Window *rlabel, *rslider, *gslider,*bslider;
struct Window *NumRed, *NumGreen, *NumBlue, *upad;


struct Window *MakeColorSlider(Root,echo,x,y)
struct Window *Root;
struct curstruct *echo;
int x,y;
{
  struct Window *base, *pad, *hs, *vs, *ss,*t, *numpad;
  struct OutDev *display;
  struct InDev *locator;
  struct rangestruct *rstruct;
  int BufUpdateWin(),CsliderDo(),ToggleRGBMode();
  int ConvertHSVVals(), ConvertRGBVals(), SetCsliderRGB();
#ifdef USA
  int PaintUndo();
#endif

  display = Root->display;
  locator = Root->input;

   base = MakeWindow(display,locator,Root,x,y,144,335,"color slider");
   Rectify(base,34,34,34,1,0,0,0,2);
   MoverII(base,base,24);
   EchoWrap(base,echo,7,0,0); 
 
   pad = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
   MoverII(pad,base,24);
   EchoWrap(pad,echo,7,0,0); 
   AttachCommand(pad,DRAW,CsliderDo,NULL);
   Maker(pad,3,6,-5,40);
   InstallWin(pad);

   upad = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
#ifdef USA
  FunctionCaller(upad,DO,PaintUndo(),NULL,JUSTDOWN);
#else
  MoverII(upad,base,24);
#endif

  EchoWrap(upad,echo,7,0,0); 
  RemoteRectify(upad, &Ured, &Ugreen, &Ublue, 1,0,0,0,0);
   Maker(upad,3,42,-5,60);
   InstallWin(upad);

  numpad = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  Rectify(numpad,200,200,200,1,0,0,0,0);
  StringWin(numpad,"rgb: ",4,0,"Swiss",10,8,0,0,0,0);
  EchoWrap(numpad,echo,0,0,0); 
  Maker(numpad,3,62,-5,78);
  InstallWin(numpad);

  NumRed = (struct Window *)MakeWindow(Root->display,Root->input,numpad, 0,0,10,10,"portal");
  Rectify(NumRed,200,200,200,0,0,0,0,0);
  Register(NumRed,1,&CurrentRed,0,0,0,"Swiss",10,8,0,0,0,0);
  FunctionCaller(NumRed,DO,SetCsliderRGB,pad,JUSTDOWN);
  EchoWrap(NumRed,echo,1,0,0); 
  Maker(NumRed,30,1,60,-1);
  InstallWin(NumRed);

  NumGreen = (struct Window *)MakeWindow(Root->display,Root->input,numpad, 0,0,10,10,"portal");
  Rectify(NumGreen,200,200,200,0,0,0,0,0);
  Register(NumGreen,1,&CurrentGreen,0,0,0,"Swiss",10,8,0,0,0,0);
  FunctionCaller(NumGreen,DO,SetCsliderRGB,pad,JUSTDOWN);
  EchoWrap(NumGreen,echo,1,0,0); 
  Maker(NumGreen,65,1,95,-1);
  InstallWin(NumGreen);

  NumBlue = (struct Window *)MakeWindow(Root->display,Root->input,numpad, 0,0,10,10,"portal");
  Rectify(NumBlue,200,200,200,0,0,0,0,0);
  Register(NumBlue,1,&CurrentBlue,0,0,0,"Swiss",10,8,0,0,0,0);
  FunctionCaller(NumBlue,DO,SetCsliderRGB,pad,JUSTDOWN);
  EchoWrap(NumBlue,echo,1,0,0); 
  Maker(NumBlue,100,1,135,-1);
  InstallWin(NumBlue);

   t = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
   StringWin(t,"Hue",2,0,"Swiss",16,8,170,170,170,0);
   StringWin(t,"Sat",49,0,"Swiss",16,8,170,170,170,0);
   StringWin(t,"Val",93,0,"Swiss",16,8,170,170,170,0);
   FunctionCaller(t,DO,ToggleRGBMode,1,JUSTDOWN);
   EchoWrap(t,echo,1,0,1);
   Maker(t,3,80,-5,96);
   InstallWin(t);
   hlabel = t;

  NextColorIndex = CurrentIndex;

  hs = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  AreaSlider(hs,1,50,50,45,1,0,0,0,100,100,110,1,&Shue,&Shmax,&Shmin,&Shgap);
  FunctionCaller(hs,DO,ConvertHSVVals,1,7);
  FunctionCaller(hs,DO,ConvertRGBVals,0,7);
  FunctionCaller(hs,DO,BufUpdateWin,pad,7);
  EchoWrap(hs,echo,7,0,1);
  Maker(hs,3,100,43,-5);
  InstallWin(hs);
  hslider = hs;
  
  ss = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  AreaSlider(ss,1,50,50,45,1,0,0,0,100,100,110,1,&Ssat,&Ssmax,&Ssmin,&Ssgap);
  FunctionCaller(ss,DO,ConvertHSVVals,1,7);
  FunctionCaller(ss,DO,ConvertRGBVals,0,7);
  FunctionCaller(ss,DO,BufUpdateWin,pad,7);
  EchoWrap(ss,echo,7,0,1);
  Maker(ss,47,100,87,-5);
  InstallWin(ss);
  sslider = ss;

  vs = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  AreaSlider(vs,1,50,50,45,1,0,0,0,100,100,110,1,&Sval,&Svmax,&Svmin,&Svgap);
  FunctionCaller(vs,DO,ConvertHSVVals,1,7);
  FunctionCaller(vs,DO,ConvertRGBVals,0,7);
  FunctionCaller(vs,DO,BufUpdateWin,pad,7);
  EchoWrap(vs,echo,7,0,1);
  Maker(vs,91,100,131,-5);
  InstallWin(vs);
  vslider = vs;


   t = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
   StringWin(t,"Red",3,0,"Swiss",16,8,170,170,170,0);
   StringWin(t,"Green",42,0,"Swiss",16,8,170,170,170,0);
   StringWin(t,"Blue",93,0,"Swiss",16,8,170,170,170,0);
   FunctionCaller(t,DO,ToggleRGBMode,0,JUSTDOWN);
   EchoWrap(t,echo,1,0,1);
   Maker(t,3,80,-5,96);
  InstallWin(t);
  rlabel = t;
  
  hs = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  AreaSlider(hs,1,50,50,45,1,0,0,0,100,100,110,0,&Sred,&Srmax,&Srmin,&Srgap);
  FunctionCaller(hs,DO,ConvertRGBVals,1,7);
  FunctionCaller(hs,DO,ConvertHSVVals,0,7);
  FunctionCaller(hs,DO,BufUpdateWin,pad,7);
  EchoWrap(hs,echo,7,0,1);
  Maker(hs,3,100,43,-5);
  InstallWin(hs);
  rslider = hs;

  ss = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  AreaSlider(ss,1,50,50,45,1,0,0,0,100,100,110,0,&Sgreen,&Sgmax,&Sgmin,&Sggap);
  FunctionCaller(ss,DO,ConvertRGBVals,1,7);
  FunctionCaller(ss,DO,ConvertHSVVals,0,7);
  FunctionCaller(ss,DO,BufUpdateWin,pad,7);
  EchoWrap(ss,echo,7,0,1);
  Maker(ss,47,100,87,-5);
  InstallWin(ss);
  gslider = ss;

  vs = (struct Window *)MakeWindow(Root->display,Root->input,base, 0,0,10,10,"portal");
  AreaSlider(vs,1,50,50,45,1,0,0,0,100,100,110,0,&Sblue,&Sbmax,&Sbmin,&Sbgap);
  FunctionCaller(vs,DO,ConvertRGBVals,1,7);
  FunctionCaller(vs,DO,ConvertHSVVals,0,7);
  FunctionCaller(vs,DO,BufUpdateWin,pad,7);
  EchoWrap(vs,echo,7,0,1);
  Maker(vs,91,100,131,-5);
  bslider = vs;
  InstallWin(vs);
 
  UpdateSizes(base);

  UnstallWin(t);
  UnstallWin(hs);
  UnstallWin(ss);
  UnstallWin(vs);
  return (base);
}



CsliderDo(W,id,data,stuff)
struct Window *W;
int id;
char *data, *stuff;
{
  struct pickstruct *p;
  struct clipstruct *cstruct;
  int cx1,cy1,cx2,cy2,screen,n,r,g,b,stat,done;
  float fstep,hstep,vstep,sstep,f,h,v,s,cl[3];
  struct rangestruct *rs;

  rs = (struct rangestruct *)stuff;
  screen = W->display->fildes;

  switch(id) {
    case(DRAW): 
      {
	cstruct = (struct clipstruct *)data;
	cx1 = cstruct->x1;
	cy1 = cstruct->y1;
	cx2 = cstruct->x2;
	cy2 = cstruct->y2;
	
	clip_rectangle(screen, (float)cx1, (float)cx2, (float)cy1, (float)cy2);
	perimeter_color(screen,0.0,0.0,0.0);
	screen = W->display->fildes;
	interior_style(screen,INT_SOLID,TRUE);

	cl[0] = FCON(CurrentRed);	cl[1] = FCON(CurrentGreen);	cl[2] = FCON(CurrentBlue);
	if ((W->display->bits&BUFTYPE) != CRX8)
	  fill_color(screen,cl[0],cl[1],cl[2]);
	else fill_color_index(screen,CurrentIndex);

	rectangle(screen,(float)(W->x1),(float)W->y1,(float)(W->x2),(float)W->y2);
	clip_rectangle(screen, (float)0.0, (float)((W->display)->width-1),
		       0.0, (float)((W->display)->height-1));
	make_picture_current(screen);

 	UpdateWin(upad); /* keep undo color refreshed */
     }
      break;
	
  }

  return(0);
}



ToggleRGBMode(mode)
{
  if (mode) {
    CloseWindow(hlabel);
    CloseWindow(hslider);
    CloseWindow(sslider);
    CloseWindow(vslider);

    OpenWindow(rlabel);
    OpenWindow(rslider);
    OpenWindow(gslider);
    OpenWindow(bslider);
    colormode = 1;
  }
  else {
    colormode = 0;
    CloseWindow(rlabel);
    CloseWindow(rslider);
    CloseWindow(gslider);
    CloseWindow(bslider);

    OpenWindow(hlabel);
    OpenWindow(hslider);
    OpenWindow(sslider);
    OpenWindow(vslider);
  }
}



ConvertHSVVals(mode)
int mode;
{
  int stat;
  float cl[3];
  if (mode) {
    CurrentHue = 360.0 - Shue;
    CurrentSat = 1.0 - Ssat;
    CurrentVal = 1.0 - Sval;

    hsv2rgb(CurrentHue,CurrentSat,CurrentVal,&CurrentRed,&CurrentGreen,&CurrentBlue);
    cl[0] = FCON(CurrentRed);	cl[1] = FCON(CurrentGreen);	cl[2] = FCON(CurrentBlue);
    ColorTable[CurrentIndex][0] = CurrentRed;
    ColorTable[CurrentIndex][1] = CurrentGreen;
    ColorTable[CurrentIndex][2] = CurrentBlue;
    define_color_table(hslider->display->fildes,CurrentIndex,1,cl);
    ChooserColorUpdate(CurrentPaletteIndex,CurrentSubIndex,CurrentRed,CurrentGreen,CurrentBlue,
		       CurrentHue,CurrentSat,CurrentVal);
  }
  else {
    Shue = 360.0 - CurrentHue;
    Ssat = 1.0 - CurrentSat;
    Sval = 1.0 - CurrentVal;
  }
  return (0);
}


ConvertRGBVals(mode)
int mode;
{
  int stat;
  float cl[3];
  if (mode) {
    CurrentRed =   255 - Sred;
    CurrentGreen = 255 - Sgreen;
    CurrentBlue =  255 - Sblue;
    ColorTable[CurrentIndex][0] = CurrentRed;
    ColorTable[CurrentIndex][1] = CurrentGreen;
    ColorTable[CurrentIndex][2] = CurrentBlue;

    rgb2hsv(CurrentRed,CurrentGreen,CurrentBlue,&CurrentHue,&CurrentSat,&CurrentVal);
    cl[0] = FCON(CurrentRed);	cl[1] = FCON(CurrentGreen);	cl[2] = FCON(CurrentBlue);
    define_color_table(hslider->display->fildes,CurrentIndex,1,cl);
    ChooserColorUpdate(CurrentPaletteIndex,CurrentSubIndex,CurrentRed,CurrentGreen,CurrentBlue,
		       CurrentHue,CurrentSat,CurrentVal);
  }
  else {
    Sred   = 255 - CurrentRed;
    Sgreen = 255 - CurrentGreen;
    Sblue  = 255 - CurrentBlue;
  }

  BufUpdateWin(NumRed);
  BufUpdateWin(NumGreen);
  BufUpdateWin(NumBlue);
  return (0);
}

NewCsliderColor()
{
  rgb2hsv(CurrentRed,CurrentGreen,CurrentBlue,&CurrentHue,&CurrentSat,&CurrentVal);
  ConvertHSVVals(0);
  ConvertRGBVals(0);
  
  /* remove the following line if you aren't compiling with animate */
  NewCelUndoStat(6,CurrentRed, CurrentGreen, CurrentBlue, CurrentPaletteIndex, CurrentSubIndex, CurrentIndex); 
}



rgb2hsv(r,g,b,H,S,V)
int r,g,b;
float *H,*S,*V;
{
  int max, min;
  float rc,gc,bc;
  float h,s,v;

  max = MAX(r,MAX(g,b));
  min = MIN(r,MIN(g,b));
  v = ((float)max)/255.0;

  if (max)
    {
      s = ((float)(max-min))/(float)max;
    }
  else s = 0.0;

  if (s == 0) h = 500.0;
  else {
    rc = ((float)(max-r))/((float)(max-min));
    gc = ((float)(max-g))/((float)(max-min));
    bc = ((float)(max-b))/((float)(max-min));

    if (r == max) h = bc-gc;
    else if (g == max) h = 2.0+rc-bc;
    else if (b == max) h = 4.0+gc-rc;

    h = h*60.0;
    if (h<0.0) h = h+360.0;
      
  }
  
  *H = h;
  *S = s;
  *V = v;
}


hsv2rgb(h,s,v,R,G,B)
float h,s,v;
int *R,*G,*B;
{
  int i;
  float f,p,q,t;

  if (s == 0.0)
    {
      if (h == 500.0)
	*R = *G = *B = (int) (v*255.0);
    }
  else {
    if (h == 360.0) h = 0.0;
    h = h/60.0;

    i = (int)h;
    f = h - i;
    p = v*(1.0 -s);
    q = v*(1.0-(s*f));
    t = v*(1.0 -(s*(1.0-f)));
    switch (i)
      {
      case (0):
	*R = (int)(v*255.0);
	*G = (int)(t*255.0);
	*B = (int)(p*255.0);
	break;
      case (1):
	*R = (int)(q*255.0);
	*G = (int)(v*255.0);
	*B = (int)(p*255.0);
	break;
      case (2):
	*R = (int)(p*255.0);
	*G = (int)(v*255.0);
	*B = (int)(t*255.0);
	break;
      case (3):
	*R = (int)(p*255.0);
	*G = (int)(q*255.0);
	*B = (int)(v*255.0);
	break;
      case (4):
	*R = (int)(t*255.0);
	*G = (int)(p*255.0);
	*B = (int)(v*255.0);
	break;
      case (5):
	*R = (int)(v*255.0);
	*G = (int)(p*255.0);
	*B = (int)(q*255.0);
	break;
      default:
	printf("case ERROR in hsv convert hsv = %f %f %f\n",h,s,v);
	break;
      }
  }

  *R = MIN(MAX(0,*R), 255);
  *G = MIN(MAX(0,*G), 255);
  *B = MIN(MAX(0,*B), 255);
}


SetUndoColorBox(r,g,b)
   int r,g,b;
{
   float cl[3];
   
   Ured = r;
   Ugreen = g;
   Ublue = b;
   ColorTable[255][0] = r;
   ColorTable[255][1] = g;
   ColorTable[255][2] = b;
   cl[0] = FCON(r);
   cl[1] = FCON(g);
   cl[2] = FCON(b);
   
   define_color_table(upad->display->fildes,255,1,cl);
   if (upad->parent->stat) BufUpdateWin(upad);
}


SetCsliderRGB(W)
   struct Window *W;
{
   ConvertRGBVals(0);
   ConvertRGBVals(1);
   ConvertHSVVals(0);
   BufUpdateWin(W->parent);
   return(0);
}
