/* NULL enhancement.h file -- this is a good skeleton to start with.
   you may not need to fiddle with it at all but it must exist as
   enhancement.h in order to compile a modified socsim
*/

#ifdef ENHANCED
#define ENHANCEMENT_NAME "Hardy-Weinberg and marriage markets"
/** EXTRA_PTR_DECLARATION is glommed into the struct extra_data
    declaration.  the extra_data struct is a node of a linked list
    that holds individual person level data that is not part of the
    basic vanilla socism
**/


/* zeroth (first) node of extra_data, value = GENERATION */
#define MGENERATION 0
#define PGENERATION 1
#define MFOUNDER 2
#define PFOUNDER 3
#define COMMANCEST 4
#define ALLELES  100
/*#define EXTRA_PTR_DECLARATION char *dna[2]*/
#define EXTRA_PTR_DECLARATION float *dna[2]

/*
void enhance_generate_extra_all();
void enhance_generate_extra();
void enhance_defaults();
double enhance_usedeathmult();
*/


#endif
