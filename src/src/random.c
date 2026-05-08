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

static long int socsim_next_legacy_random_state()
{
    /* TODO: Replace the legacy package-owned LCG with PCG32 while keeping
       compatibility handling explicit for reproducible historical runs. */
    ceed = ((MULTIPLIER * ceed) + INCREMENT) % MODULUS;
    return ceed;
}


int irandom()
{
    socsim_next_legacy_random_state();
    return (int) ceed;
}
/***********************************************************************/
double
rrandom()
{

  double u;
    /***********************************************************************
     * Use the package-owned generator on every platform so the same seed
     * produces the same stream across operating systems.
     ************************************************************************/
        u = real_rrandom();
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

	(void) socsim_next_legacy_random_state();
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