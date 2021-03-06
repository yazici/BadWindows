

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





/*
 *
 *	Munsell to CIE conversion  (Hopefully undamaged!!)
 *
 *	Author	Walter Bender
 *	Date	31 July 1986
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <munsell.h>

static float   munciedata[NUMV][NUMH][NUMC][2];
static char    mundata[] = "data/muncie.data";
static char    mundata2[] = "data/muncie.data";
static int     munsellinit = 0;

#define CAPY(v)\
		C1*(v)-\
		C2*pow((v),2.0)+\
		C3*pow((v),3.0)-\
		C4*pow((v),4.0)+\
		C5*pow((v),5.0)

#define lerp(a, p0, p1, p)\
  p[0] = p0[0] + (p1[0]-p0[0])*a;\
  p[1] = p0[1] + (p1[1]-p0[1])*a
  
/*
 *	muncie( H, V, C, x, y, Y )
 *	float	H, V, C, *Y, *x, *y;
 *
 *	H is Munsell HUE, ranging from 0 to 100
 *	V is Munsell Value, ranging from 0 to 12
 *	C is Munsell Chroma, ranging from 0 to 26
 *
 *	Y is CIE Luminance, ranging from about 0 to 100
 *	x is CIE x ranging from about 0 to 1
 *	y is CIE y ranging from about 0 to 1
 *
 *	Note that if the HVC coordinate is oobounds, muncie returns BADHVC
 *      If there is no data for a HVC triplet, muncie returns BADCIE
 */

muncie( H, V, C, x, y, Y )
float	H, V, C, *Y, *x, *y;
{
	register int i, j, k;
	int	h[2], v[2], c[2];
	float	ha, va, ca;
	float	p[8][2], ph[4][2], pv[2][2], phvc[2];

	H *= 0.4;	/* go from 0-100 to 0-40 */
	while( H < 0.0 ) H += 40.0;
	while( H > 40.0 )H -= 40.0;
	h[0] = floor( (double)H );
	h[1] = ceil( (double)H );
	ha = H-h[0];
	if( h[1] >= NUMH ) h[1] = 0;

	C /= 2.0;
	c[0] = floor( (double)C );
	ca = C-c[0];
	c[1] = c[0]+1;
	if( c[0] < 0 ) return( BADHVC );
	if( c[0] > NUMC ) c[0] = c[1] = NUMC;
	if( c[1] > NUMC ) c[1] = NUMC;

	v[0] = v[1] = floor( (double)V ); /* value 1 is table entry 0 */
	v[0]--;
	va = V-v[1];
	if( v[0] < 0 ) return( BADHVC );
	if( v[0] >= NUMV ) return( BADHVC );
	if( v[1] >= NUMV ) v[1] = NUMV-1;

#ifdef DEBUG
	fprintf( stderr, "HVC (%f %f %f), %d-%d, %d-%d, %d-%d, a %f %f %f\n",
		H,V,C,h[0],h[1],v[0],v[1],c[0],c[1] );
#endif

	/* get corners of cube surrounding point in HVC space */
	for( i = 0 ; i < 2 ; i++ )
	for( j = 0 ; j < 2 ; j++ )
	for( k = 0 ; k < 2 ; k++ ) {
/*	  printf("calling munaccess(%d, %d, %d...\n", h[i], v[j], c[k]);*/
	  if( munaccess( h[i], v[j], c[k], &p[((i<<2)+(j<<1)+k)][0], 
			&p[((i<<2)+(j<<1)+k)][1]) < 0 )
	    return( BADCIE );
	}

	/* interpolate hue */
	lerp( ha, p[0], p[4], ph[0] );
	lerp( ha, p[1], p[5], ph[1] );
	lerp( ha, p[2], p[6], ph[2] );
	lerp( ha, p[3], p[7], ph[3] );

	/* interpolate value */
	lerp( va, ph[0], ph[2], pv[0] );
	lerp( va, ph[1], ph[3], pv[1] );

	/* interpolate chroma */
	lerp( ca, pv[0], pv[1], phvc );

	*x = phvc[littlex];
	*y = phvc[littley];
	*Y = CAPY((double)V);
	return( 0 );
}

munaccess( h, v, c, lx, ly)
int	h, v, c;
float	*lx, *ly;
{
  FILE *munfp;
  char data_path[40];

  if( munsellinit == 0 ) {
    if( (munfp = fopen( bw_file(mundata), "r" )) <= 0 ) {
      if( (munfp = fopen( bw_file(mundata2), "r" )) <= 0 ) {
	printf("Please enter path for munsell data file:");
	sscanf("%s", data_path);
	if( (munfp = fopen( data_path, "r" )) < 0 ) {
	  fprintf( stderr, "muncie: I couldn't open %s\n",
		  data_path);
	  return( -1 );
	}
      }
    }
    fread( munciedata, sizeof(float), NUMMUN, munfp );
    fclose( munfp );
    munsellinit = 1;
  }

      if( c == 0 ) {
	  *lx = Nx;
	  *ly = Ny;
	}
	else {
	  if( (*lx = munciedata[v][h][c][littlex]) == 0.0 )
	    return( BADCIE );
	  if( (*ly = munciedata[v][h][c][littley]) == 0.0 )
	    return( BADCIE );
	}
  return( 0 );
}


/*****
lerp( a, p0, p1, p )
float	a, p0[2], p1[2], p[2];
{
	register int	i;
		
	for( i = 0 ; i < 2 ; i++ )
		p[i] = p0[i] + (p1[i]-p0[i])*a;
	return( 0 );
}
******/























