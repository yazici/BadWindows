#include<stdio.h>
#include<dq.h>
#include "cie.h"

/*Stephanie Houde 7/13/86
        This function takes rgb color coordinates and converts them to cie
        space color coordinates.
        cie boundaries look like this:
                0.0 <= x <= 1.0
                0.0 <= y <= 1.0
                0.0 <= Y <= 100
        if the cie values are out of bounds, -1 is returned             */

/*neutral point - illuminant D 6500*/
/*#define Nx 0.313*/
/*#define Ny 0.329*/

/*neutral point - illuminant C*/
#define Nx 0.310
#define Ny 0.316

#ifdef(0)
/*inverted Matrix from ciergb transformation constants This values independant of neutral point illuminant choice*/
/*They depend on the graphic monitor primaries*/
#define I1 0.47624
#define I2 0.29902
#define I3 0.17473
#define I4 0.26239
#define I5 0.65607
#define I6 0.08154
#define I7 0.01972
#define I8 0.17067
#define I9 0.90861
#endif

/*same as above, computed with corrected transformation determinant*/
#define I1 0.49799
#define I2 0.28959
#define I3 0.19342
#define I4 0.27437
#define I5 0.63537
#define I6 0.09026
#define I7 0.02061
#define I8 0.15560
#define I9 1.00578


rgbcie(r,g,b,x,y,Y)
int     r,g,b; /*RGB color values*/
float   *x,*y,*Y;  /*CIE specification */
{

        register float  xc,zc,t;    /*CIE chromaticity coefficients*/

	xc = ((I1)*(float)r + (I2)*(float)g + (I3)*(float)b);

        *Y = ((I4)*(float)r + (I5)*(float)g + (I6)*(float)b);

	zc = ((I7)*(float)r + (I8)*(float)g + (I9)*(float)b);

         t = ( xc + *Y + zc );
        if (t == 0.0) *y = Ny;
        else *y = (*Y/t);
        if (*Y == 0.0) *x = Nx;
        else *x = ((xc)*(*y))/(*Y);

        *Y *= (100.0)/(255.0); 

        if((*x<0.0)||(*x>1.0)||(*y<0.0)||(*y>1.0)||(*Y<0.0)||(*Y>100)) 
	  return(-1);

        return( 0 );
 
}


compute_i_coef ()
{
  printf("\nI1 %f", RX*TR);
  printf("\nI2 %f", GX*TG);
  printf("\nI3 %f", BX*TB);
  printf("\nI4 %f", RY*TR);
  printf("\nI5 %f", GY*TG);
  printf("\nI6 %f", BY*TB);
  printf("\nI7 %f", RZ*TR);
  printf("\nI8 %f", GZ*TG);
  printf("\nI9 %f", BZ*TB);
}


