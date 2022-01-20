/* %W% %G% */
#include <stdio.h>
#include <math.h>
//#include "defs.h"
#include <stdlib.h>
// #include <stdint.h>

/*
Thu Sep 20 14:10:43 PDT 2012
NO these must be const long int
#define MODULUS  65536
#define MULTIPLIER 25173
#define INCREMENT 13849
*/

/*const long int MODULUS= 65536;     // m */
/*const long int MULTIPLIER = 25173; // a */
/*const long int  INCREMENT = 13849; // c */

const long long MODULUS= 2147483648 ;     /*  m */
const long long MULTIPLIER = 1103515245; /* a */
const long int  INCREMENT = 12345; /*c */

double cycle(double,int);
//long int ceed=12345678;


int irandom()
{
    #if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) //check if this is Windows, because win has no random(), only rand()
        //ceed = rand(); // rand() is not very good randomness.
        ceed = ((MULTIPLIER * ceed) + INCREMENT) % MODULUS; //implement our own https://en.wikipedia.org/wiki/Linear_congruential_generator ... leads to errors
    #else
    //Linux has random() which gives proper random-numbers:
        ceed = random();
    #endif
    printf("\nrand_max: %ld",RAND_MAX);

    return (int) ceed;
}
/***********************************************************************/
double
rrandom()
{

  double u;
  /***********************************************************************
   * This just calls the builtin random() function. Initialization is
   * done in event.c.  In case you don't care for your compiler's
   * random() you cann call real_rrandom() instead real_rrandom
   * implements what wikipedia says is gcc's cheapest/simplest random
   * number generator.
   ************************************************************************/
    #if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) //check if this is Windows, because win has no random(), only rand()
        u = real_rrandom();
        //u=rand()/(double) (RAND_MAX+1);//rand()/(double) RAND_MAX; // real_rrandom();  
        //the +1 is important, otherwise u will sometimes (every about 32600 times) be exactly 1.0!
    #else
    //Linux has random() which gives proper random-numbers:
        u= random()/(double) RAND_MAX;  /** using system function !!!! **/
    #endif
  /* u=real_rrandom(); */
  
  /*fprintf(fd_allrandom,"%36.30f\n",u);*/
  return(u);
}


/************************************************************************/

double
real_rrandom()
{
  /* Thu Sep 20 14:07:32 PDT 2012 This is NOT used by default. The
     code is here in case we encounter a compiler that does not do
     random() well.
  */


    double t;
    t = (double) MODULUS;

    ceed = ((MULTIPLIER * ceed) + INCREMENT) % MODULUS;
	;
    /*
    printf("rand: %e\n", ((double) ceed)/t);
    */

    return ((double) ceed)/t;

}

double
normal()
{
    double theta, r, w;

    theta = rrandom() * 2 * PI;
    w = rrandom();
    while (w == 0)
	w = rrandom();
    r = sqrt(-2. * log(w));
    return  r * sin(theta);
}

double
fertmult()
{
    double u;

    u = rrandom();

    return  ((-1.764 + 1.995 * u) * u + 2.178) * u;
    /*
    return 1;
    */
}

double flog(double x)
{
    return cycle(x,1) - cycle(x, 2)/2
    + cycle(x, 3)/3 - cycle(x,4)/4
    + cycle(x,5)/5;
    /*
    - cycle(x,6)/6
    + cycle(x,7)/7 - cycle(x,8)/8
    + cycle(x,9)/9 - cycle(x,10)/10
    + cycle(x,11)/11 - cycle(x,12)/12
    + cycle(x,13)/13 - cycle(x,14)/14
    + cycle(x,15)/15 - cycle(x,16)/16;
    */
}

double cycle(double x,int p)
{
    double y;
    int i;

    y = 1;
    for (i = 1; i <= p; i++) {
	y*=x;
    }
    return y;
}

/** good RNG from https://www.pcg-random.org/download.html needs the #include <stdint.h> at the top:
// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
**/