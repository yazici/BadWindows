/*--------------------------------------------------------------------------
  This program makes a look-up table of maximum Chroma values for every
  Value and Hue values in the Munsell data. The table provides max chroma
  values for 50 steps in Hue and 30 steps in Value, thus 1500 max Chroma
---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "munsell.h"

#define TRUE  1
#define FALSE 0




MakeMaxChromaTable (filename)
char *filename;
{
  float H, V, maxC, MaxChromas [50][30];
  int h, v, hcount = 0, vcount = 0;
  FILE *fp, *testfile;

  for (h = 0 ; h < 50 ; h++)
    for (v = 0 ; v < 30 ; v++)
      MaxChromas [h][v] = 0.000;

/*  for (h = 0 ; h < 50 ; h++) {
    for (v = 0 ; v < 30 ; v++)
      printf("%7.1f", MaxChromas [h][v]);
    printf("\n");
  }
*/

  printf("\nBefore LOOP!");

  for (H = 0.0, h = 0 ; H < 100.0 ; H += 2.0, h++) {
    hcount++;
    for (V = 0.0, v = 0  ; V < 10.0 ; V += 0.3333, v++) {
      FindMaxChroma (H, V, &maxC);
      MaxChromas [h][v] = maxC;

      if (h % 10 == 0)
	printf(".");

      vcount++;
    }
  }

  printf("\nAfter Loop");

  testfile = fopen ("maxchroma.text", "w");
  for (h = 0 ; h < 50 ; h++) {
    for (v = 0 ; v < 30 ; v++)
      fprintf(testfile, "%7.1f", MaxChromas [h][v]);
    printf("\n");
  }
  fclose (testfile);

  for (h = 0 ; h < 50 ; h++) {
    for (v = 0 ; v < 30 ; v++)
      printf("%7.1f", MaxChromas [h][v]);
    printf("\n");
  }
  
  printf("\n\n\n");

  if ((fp = fopen (filename, "w")) == NULL) {
    printf("\nCouldn't open or create file %s", filename);
    exit(1);
  }

  for (h = 0 ; h < 50 ; h++) 
    for (v = 0 ; v < 30 ; v++) {
      /*printf("\nWriting Maxchromas [%d] [%d]  %6.1f  h %d  v %d  size %d", h, v, MaxChromas [h][v], h, v , sizeof (float));*/
      fwrite (&MaxChromas [h][v], sizeof(float), 1, fp);
    }
  fclose(fp);

  if ((fp = fopen (filename, "r")) == NULL) {
    printf("\nCouldn't RE-open file %s for read check", filename);
    exit(1);
  }


  printf( "\n\n\n");

  for (h = 0 ; h < 50 ; h++) {
    for (v = 0 ; v < 30 ; v++) {
      fread (&MaxChromas [h][v], sizeof(float), 1, fp);
      printf("%7.1f", MaxChromas [h][v]);
    }
    printf("\n");
  }
  fclose(fp);

  for (h = 0 ; h < 50 ; h++) {
    for (v = 0 ; v < 30 ; v++)
      printf("%7.1f", MaxChromas [h][v]);
    printf("\n");
  }

  return (0);
}






FindMaxChroma (H, V, maxC)
float H, V, *maxC;
{
  float C = 26.0;
  int test = FALSE, r, g, b;

  printf( "H %.3f   V %.3f", H, V);

  if (V < 1.0) {
    *maxC = 0.0;
    printf("$");
    return;
  }

  while (test == FALSE) {

    test = (int) munrgb ( H, V, C, &r, &g, &b);
    
    C *= 0.99;
    if (C < 0.0001) test = 0;

    printf("-");
  }

  *maxC = C;
  return;
}










