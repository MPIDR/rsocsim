/* %W% %G% */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

/* PCG32 Implementation */
typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;
static pcg32_random_t global_rng_state = {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL};

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

/* Global variable 'ceed' is used by other files (e.g. events.cpp) */
long long ceed = 12345678; 

/* Function to initialize the RNG */
void socsim_seed(long long seed) {
    ceed = seed;
    global_rng_state.state = 0U;
    global_rng_state.inc = (seed << 1u) | 1u;
    pcg32_random_r(&global_rng_state);
    global_rng_state.state += 0x853c49e6748fea9bULL;
    pcg32_random_r(&global_rng_state);
}

int irandom()
{
    /* Ensure we use the PCG generator. 
       Note: If 'ceed' is modified directly by external code after initialization,
       it won't affect the RNG state unless socsim_seed is called. 
       This is a behavior change but a necessary one for better RNG. */
    return (int)pcg32_random_r(&global_rng_state);
}

/***********************************************************************/
double
rrandom()
{
  /* Return a double in [0, 1) */
  return ldexp(pcg32_random_r(&global_rng_state), -32);
}


/************************************************************************/

double
real_rrandom()
{
    return rrandom();
}

double
normal()
{
    double theta, r, w;

    theta = rrandom() * 2 * 3.14159265358979323846;
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

double flog(double x)
{
    return cycle(x,1) - cycle(x, 2)/2
    + cycle(x, 3)/3 - cycle(x,4)/4
    + cycle(x,5)/5;
}
