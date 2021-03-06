

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

/******************************************************************************/
/* VIDEO.C                                                                    */
/******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <starbase.c.h>
#include <windows.c.h>
#include <structures.h>


struct curstruct *LoadSuguruFirstFrame(fname)     /* v 1.0b */
     char *fname;
{
  struct curstruct *icon = NULL;
  int val = FALSE;

  icon = (struct curstruct *)malloc(sizeof(struct curstruct));

  if (!(val = LoadSuguruFrameBitMap(fname, &(icon->dx), &(icon->dy), &(icon->source)))) {
    printf("Yow! Could not find file %s.\n", fname);
    free(icon);
    icon = NULL;
  }
  else {
    icon->rule = redclear;
    icon->hotdx = 0;
    icon->hotdy = 0;
  }

  return(icon);
}


struct video_struct *LoadSuguruVideo(filename)
     char *filename;
{
  struct video_struct *vid = NULL;
  FILE *fopen(), *fp;
  int i, size;


  /* Load in a video file of the format "width height numframesCR" --> banks */
  vid = (struct video_struct *)malloc(sizeof(struct video_struct));

  if ((fp = fopen(filename, "r")) == NULL) {
    free(vid);
    return(NULL);
  }

  fscanf(fp, "%d %d %d\n", &(vid->width), &(vid->height), &(vid->numframes));
  printf("\nLoading %s, size %d x %d, numframes %d.",filename,vid->width,vid->height,vid->numframes);

  if ((vid->frames = (unsigned char **)malloc((vid->numframes)*(sizeof(unsigned char *)))) == NULL)
    printf("\nBad malloc in LoadVideo.");

  size = vid->width*vid->height*3;

  for (i = 0; i < vid->numframes; i++) {

    /* malloc a buffer for the video data */
    if ((*((vid->frames)+i) = (unsigned char *)malloc(size * (sizeof(unsigned char)))) == NULL)
      printf("\nError in mallocing video data buffer for %s.", filename);

    fread(*(vid->frames+i), sizeof(unsigned char), size, fp);
    printf("\nLoaded frame number %d.", i+1);
  }

  vid->current_frame = 0;


  fclose(fp);
  return(vid);
}


int PlayVideo(display, control, vid, x, y, playframes)
     struct video_struct *vid;
     int display, x, y, playframes;  /* <-- number of frames to play */
     unsigned char *control;
{
  int i,j, width, height;
  unsigned char *data;

  width = vid->width;  height = vid->height;

  for (j=0; j<playframes; j++) {
    data = *(vid->frames+vid->current_frame);
    bank_switch(display,2,0);
    dcblock_write(display,x,y,width,height,data,FALSE);
    bank_switch(display,1,0);
    dcblock_write(display,x,y,width,height,data+height*width,FALSE);
    bank_switch(display,0,0);
    dcblock_write(display,x,y,width,height,data+height*width*2,FALSE);
    control [0x40bf] = 7;

    if (vid->current_frame < ((vid->numframes)-1)) vid->current_frame++;
    else vid->current_frame = 0;
  }
}



