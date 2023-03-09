#include <stdio.h>
#include <math.h>
/* %W% %G% */

#define cfree free

/** if ENHANCED is def'ed AND enhanced.h and enhanced.c point to files that 
implement "enhancements" then all the special modification in the
enhanced.c file will come into play. if ENHANCED is NOT def'ed then
plain old socsim should compile
#define ENHANCED 
**/
#ifdef ENHANCED
#include "enhancement.h"
#else
#define ENHANCEMENT_NAME "STANDARD-UNENHANCED-VERSION"
#endif
/** extra_data struct includes a value (float) and a pointer
    if EXTRA_PTR is not defined in enhancement.h then we set it
    to char and assume that it will go unused
***/
#ifndef EXTRA_PTR_DECLARATION
#define EXTRA_PTR_DECLARATION char *xptr
#endif
/** 
    The "hooks" for enhancement are forward declared here. These are
    what link plain old socsim to whatever special features are in
    the enhancement.c file. Obviously additional hooks can be added easily

**/
void enhance_defaults();
void enhance_read_initial_pop();
void enhance_birth();
void enhance_death();
void enhance_divorce();
int enhance_check_spouse();
void enhance_marriage_queue();
void enhance_new_marriage();
double enhance_usedeathmult();
double enhance_usefertmult();
void enhance_transit_before();
void enhance_transit_after();
void enhance_write_extra();
double enhance_couple_score();
/* Macros */
#define ABS(a) (((a) > 0) ? (a) : -(a))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define NEW(type) ((type *)malloc(sizeof(type)))

/* #define MOD1200(x) (x) % 1200 + 1200 * ((int)((x) - (x) % 1200) / (x))*/

/* bounds for static structures */
#define MAXPARITY 10
#define MAXGROUPS 60
#define MAXKT 2400
#define HASHSIZE 10000
#define NUMBER_OF_GROUPS (MAXGROUPS - 1)
/* events */
#define E_BIRTH 0
#define E_MARRIAGE 1
#define E_DIVORCE 2
#define E_DEATH 3
/* used in transit() to determine destination group */
#define NONTRANSIT 3
#define TRANSIT1 4
#define TRANSIT2 5
#define TRANSIT3 6
#define TRANSIT4 7
#define TRANSIT5 8
#define TRANSIT6 9
#define TRANSIT7 10
#define TRANSIT8 11
#define TRANSIT9 12
#define TRANSIT10 13
#define TRANSIT11 14
#define TRANSIT12 15
#define TRANSIT13 16
#define TRANSIT14 17
#define TRANSIT15 18
#define TRANSIT16 19

#define TRANSIT17 20
#define TRANSIT18 21
#define TRANSIT19 22
#define TRANSIT20 23
#define TRANSIT21 24
#define TRANSIT22 25
#define TRANSIT23 26
#define TRANSIT24 27
#define TRANSIT25 28
#define TRANSIT26 29
#define TRANSIT27 30
#define TRANSIT28 31
#define TRANSIT29 32
#define TRANSIT30 33
#define TRANSIT31 34
#define TRANSIT32 35
#define TRANSIT33 36
#define TRANSIT34 37
#define TRANSIT35 38
#define TRANSIT36 39
#define TRANSIT37 40
#define TRANSIT38 41
#define TRANSIT39 42
#define TRANSIT40 43
#define TRANSIT41 44
#define TRANSIT42 45
#define TRANSIT43 46
#define TRANSIT44 47
#define TRANSIT45 48
#define TRANSIT46 49
#define TRANSIT47 50
#define TRANSIT48 51
#define TRANSIT49 52
#define TRANSIT50 53
#define TRANSIT51 54
#define TRANSIT52 55
#define TRANSIT53 56
#define TRANSIT54 57
#define TRANSIT55 58
#define TRANSIT56 59
#define TRANSIT57 60
#define TRANSIT58 61
#define TRANSIT59 62
#define TRANSIT60 63

#define NUMEVENTS 64
#define E_NULL 65
/*#define E_TRANSIT 23   never used*/

/* options for child_inherits_group */
#define FROM_MOTHER -10
#define FROM_FATHER -11
#define FROM_SAME_SEX_PARENT -12
#define FROM_OPPOSITE_SEX_PARENT -13

#define RECYCLE_SEED 5678
/* RECYCLE_SEED is used to create a reusable file of random uniforms
   called socsim.random -- this file is created whenever the random_recycling
   option is in effect but the socsim.random file is not found.
   Wed Nov 12 09:14:23 PST 2003
*/

/* forward declarations */

/* Tue Jan 31 14:54:57 PST 2006
   commented char *malloc(); declaration b/c it caused
   complie failure in x86_64. */

/* uncomment for some compilers */
/*char *malloc();*/

/* from random.c */
int irandom();
double rrandom();
double real_rrandom();
double normal();
double fertmult();
double flog();

/* from events.c */
int date_and_event(struct person*);

/* from load.c */
//char *index_to_sex[2];
/*char *index_to_event[8]; */
//char *index_to_event[NUMEVENTS + 1];
//char *index_to_mstatus[6];

/* from xevents.c */
struct person *random_spouse(struct person *p);
struct person *random_spouse2(struct person *p);
struct person *find_random_father(struct person*);
double score1(struct person*, struct person*);
double score2(struct person*, struct person*);
double score3(struct person*, struct person*);

/* from utils.c */
double ident(struct person*, struct age_block*);
double twofold(struct person*, struct age_block*);
double usefertmult(struct person*, struct age_block*);
double usetransmult(struct person*, struct age_block*);
double usedeathmult(struct person*, struct age_block*);
double usenullmult(struct person*, struct age_block*);
//double lookup_cohab_prob();
double table_lookup( int age,struct age_table*);
float get_extra( struct person*, int);
void put_extra(struct person*,int,float);
void write_person(struct person*, FILE*);
void write_marriage(struct marriage*, FILE*);
int get_marity(struct person*);
int get_parity(struct person*);
/* from io.c */
/*int make_random_socsim(); OBSOLETE */
void initialize_person();
void write_popfiles();
struct nlist
{                     /* hash table entry, a la K and R    */
  int uniontag;       /* save there are 6 integers parked  */
  struct nlist *next; /* here to save a second pass in the */
  int val;            /* opop read                         */
  int i1, i2, i3, i4, i5, i6;
  union {
    struct person *p;
    struct marriage *m;
  } u;
};

#define PP u.p
#define MM u.m

#define H_MOTHER i1
#define H_FATHER i2
#define H_E_SIB_MOM i3
#define H_E_SIB_DAD i4
#define H_LBORN i5
#define H_LASTMAR i6

struct nlist *install(int , int , void * , int, int , int, int, int, int);
struct nlist *lookup( int , int);
static struct nlist *hashtab[HASHSIZE];
/** end of stuff from io.c **/
/* CONSTANTS */
#define PI 3.141592654

/* booleans */
#define FALSE 0
#define TRUE 1

/* marriage_eval */
#define PREFERENCE 1
#define DISTRIBUTION 2
/* dynamic data structures */
/*** 
  defined already in egcc
#define NULL 0

****/

/* maximum lifespan */
/*
#define MAXUMONTHS 1440
#define MAXUYEARS 120
*/

#define MAXUMONTHS 2400
#define MAXUYEARS 200

/*
const int MAXUYEARS = 120;
const int MAXUMONTHS = MAXUYEARS * 12;
*/

/* sex */
#define MALE 0
#define FEMALE 1
#define NUMSEXES 2

/* marital status */
#define MS_NULL 0
#define SINGLE 1
#define DIVORCED 2
#define WIDOWED 3
#define MARRIED 4
#define COHABITING 5
#define NUMMARSTATUS 6

/* Lee Carter rates information */
#define AX 0
#define BX 1
#define K_VAL 2
#define K_MEAN 3
#define K_STD_DEV 4
#define K_START_YEAR 5
#define NUMLC 6

/* Hash table tags */
#define PERSON_TAG 0
#define MARRIAGE_TAG 1

/* File status tags */
#define CLOSED 0
#define OPEN 1

/*
#define SINGLE_LOOKING 6
#define DIVORCED_LOOKING 7
#define MARRIED_LOOKING 8
#define WIDOWED_LOOKING 9
*/

/* reason marriage (union)  ends */
#define DIVORCE 2
#define DEATH 3
#define BREAKCOHAB 4
#define MARAFTERCOHAB 5
#define COCOHABDIES 6
#define M_NULL 16

/* for consanguinity tests */
#define FATHER 0
#define MOTHER 1
#define FATHERS_MOTHER 2
#define MOTHERS_MOTHER 3
#define FATHERS_FATHER 4
#define MOTHERS_FATHER 5

/* printing, i/o */
#define DEBUGGING 0
#define READING 1

#define CHECK_SEX 0
#define CHECK_MSTATUS 1

/** file desc for intermediate post segment results **/

FILE *fd_out_pop_seg, *fd_out_mar_seg, *fd_out_xtra_seg;

FILE *fd_pop, *fd_mar, *fd_xtra, *fd_out_pop, *fd_out_mar, *fd_out_xtra;
FILE  *fd_pyr, *fd_stat, *fd_log, *fd_otx, *fd_out_otx, *fd_out_otx_seg; //*fd_rn,
FILE *fd_random, *fd_allrandom; /** temp file for random number gen debugging **/
/* structures and access to structures, and the globals */

/** struct and pointers for random_recycling feature  
struct random_number {
  double value;
  struct random_number *next ;
};

struct random_number *current_random_number;
struct random_number *first_random_number;
**/

double marriage_agetarget[MAXGROUPS][2 * MAXUYEARS];
int marriage_ageobs[MAXGROUPS][2 * MAXUYEARS];
int marriage_tots[MAXGROUPS];

struct stat_block
{
  int upper_age;
  int width;
  double lambda;
  int pml;
  int events;
  struct stat_block *previous;
  struct stat_block *next;
};

struct age_block
{
  int upper_age;
  int width;
  double lambda;
  double mu;
  double modified_lambda;
  struct age_block *previous;
  struct age_block *next;
  double (*mult)(struct person*, struct age_block*);
};

struct age_table
{
  int upper_age;
  double prob;
  struct age_table *next;
};

struct lc_rates
{
  int ck[NUMLC]; /* checklist--is everything present */
  double val;
  double mean;
  double std_dev;
  int start_year;
  struct age_block *ax_bx; /* ax's are lambdas. bx's are mu's */
};

struct table_entry
{ /* hash table entry, a la K and R    */
  struct table_entry *next;
  int key; /* up to 6 values read               */
  int i1, i2, i3, i4, i5, i6;
};

/* other global vars */
/* define array of reals for parameters -- to be used in enhancement.c
   load() knows how to read parameter0..9 and will stick the values in this 
   array.  But will not complain if none are present */

double parameter0, parameter1, parameter2, parameter3,
    parameter4, parameter5, parameter6, parameter7, parameter8,
    parameter9, parameter10;

struct person *person0; /* first person read */
struct person *last_person;
int last_person_id;         /* last number used */
struct person *debug_p;     /*in case one needs to track a person */
struct marriage *marriage0; /* oldest marriage */
struct marriage *last_marriage;
int last_marriage_id;       /*last number used */
int time_waiting[NUMSEXES]; /*person months on mqueue*/
int firstyear;              /* map sim to real time */
struct age_block *rate_set[MAXGROUPS][NUMEVENTS][NUMSEXES][NUMMARSTATUS];
double rate_factors[MAXGROUPS][NUMEVENTS][NUMSEXES][NUMMARSTATUS];
int duration_specific[MAXGROUPS][NUMEVENTS][NUMSEXES][NUMMARSTATUS];
struct age_block *birth_rate_set[MAXGROUPS][NUMMARSTATUS][MAXPARITY];
double birth_rate_factors[MAXGROUPS][NUMMARSTATUS];
struct age_table *cohab_probs[MAXGROUPS][NUMSEXES];
struct lc_rates *lc_rate_set[MAXGROUPS][NUMSEXES];
double *kt_vals[MAXGROUPS][NUMSEXES];
double *ax_vals[MAXGROUPS][NUMSEXES];
double *bx_vals[MAXGROUPS][NUMSEXES];
double read_kt_vals[MAXGROUPS][NUMSEXES][MAXKT];
/* arrays of MAXGROUPS*NUMSEXES pointers to doubles */
int e_index, max_e_index, current_e_index, epsilon;
struct age_table *lc_epsilon[MAXGROUPS][NUMSEXES][MAXKT];
int kword;

int compatibility_mode = 1;

int birth_target[MAXGROUPS];
int transit_target[MAXGROUPS][MAXGROUPS];
double birth_adjust[MAXGROUPS];
double transit_adjust[MAXGROUPS][MAXGROUPS];
int last_event_date;
int size_of_pop[MAXGROUPS];
int crnt_month_events[NUMEVENTS];
double alpha; /* inheritance of fmult */
double betaT;  /* inheritance of fmult */
double bint;  /* minimum interval between births */
double prop_males;
int hhmigration;
int hetfert;
/* httrans added carlm 11/1/99*/
int hettrans;
double endogamy;
int take_census;
int fixed_epsilon, random_epsilon;
int read_xtra_file;
long int ceed;
long int original_seed = -4;
//static char randstate[2048]; /* for gcc's random number generator *///not needed here?!?!
int current_month;
int stop_month;
int current_segment;
int num_segments;
/*int random_recycling; OBSOLETE */
int size_of_extra; /* number of floats in extra default 0, read in .sup*/
int numgroups;

int numgroups_in_ipop; /*set in read_initial_pop*/

int duration_of_segment;
int write_output;         /* write out .oxxx files after current segment */
int child_inherits_group; /* from_mother,from_fater,from_same_sex_parent
			     from_opposite_sex_parent */

int random_father;         /* find random dads for unmarried mothers */
int random_father_min_age; /*min age of randomly selected fathers*/

float agedif_marriage_mean[MAXGROUPS];
float agedif_marriage_sd[MAXGROUPS];
int marriage_eval;
int marriage_queues;

int last_kt_index[MAXGROUPS][NUMSEXES];
int current_kt_index[MAXGROUPS][NUMSEXES];

char pop_file_name[1024], mar_file_name[1024], xtra_file_name[1024];
char pop_out_name[1024], mar_out_name[1024], xtra_out_name[1024];
char pyr_file_name[1024], stat_file_name[1024], prefix_out_name[1024];
char log_file_name[1024], otx_file_name[1024], otx_out_name[1024];

char rate_file_name[1024];

char pop_out_name_seg[1024], mar_out_name_seg[1024], xtra_out_name_seg[1024];
char pyr_file_name_seg[1024], stat_file_name_seg[1024], otx_out_name_seg[1024];

long current_offset; /* keep place in main file */
int current_lineno;  /* keep place in main file */
int current_fstatus; /* is there a placeholder in file */
FILE *current_fp;    /* saved file pointer */
int use_lc_rates;    /* use lee-carter mortality */
int read_ax_or_bx;   /* which to read */

double marriage_peak_age; /*used in score3 read from .sup */
double marriage_slope_ratio;
int marriage_agedif_max;
int marriage_agedif_min;

int option_marriage_after_childbirth = 0;

/* some ad hoc stuff */

#define A_SELF 0
#define A_SPOU 1
#define A_BKID 2
#define A_SKID 3
#define A_BSONBKID 4
#define A_BDAUBKID 5
#define A_BSONSKID 6
#define A_BDAUSKID 7
#define A_SSONBKID 8
#define A_SDAUBKID 9
#define A_NUMCAT 10

#define C_SELF 0
#define C_SPOU 1
#define C_BDAD 2
#define C_BMOM 3
#define C_SDAD 4
#define C_SMOM 5
#define C_BPARBKID 6
#define C_BDADBKID 7
#define C_BMOMBKID 8
#define C_BDADBDAD 9
#define C_BDADBMOM 10
#define C_BMOMBDAD 11
#define C_BMOMBMOM 12
#define C_BDADSDAD 13
#define C_BDADSMOM 14
#define C_BMOMSDAD 15
#define C_BMOMSMOM 16
#define C_SDADBDAD 17
#define C_SDADBMOM 18
#define C_SMOMBDAD 19
#define C_SMOMBMOM 20
#define C_BDADBDADBDAD 21
#define C_BDADBDADBMOM 22
#define C_BDADBMOMBDAD 23
#define C_BDADBMOMBMOM 24
#define C_BMOMBDADBDAD 25
#define C_BMOMBDADBMOM 26
#define C_BMOMBMOMBDAD 27
#define C_BMOMBMOMBMOM 28
#define C_BDADBPARBKID 29
#define C_BDADBDADBKID 30
#define C_BDADBMOMBKID 31
#define C_BMOMBPARBKID 32
#define C_BMOMBDADBKID 33
#define C_BMOMBMOMBKID 34
#define C_BDADBDADBPARBKID 35
#define C_BDADBMOMBPARBKID 36
#define C_BMOMBDADBPARBKID 37
#define C_BMOMBMOMBPARBKID 38
#define C_NUMCAT 39

int a_tally[NUMSEXES][A_NUMCAT];
int c_tally[NUMSEXES][C_NUMCAT];

/* definitions for event queue management */

struct queue_element
{
  int num;
  struct person *first, *last; /*last is experimental 1/2014*/
};

struct queue_element event_queue[MAXUMONTHS];

struct queue_element marriage_queue[NUMSEXES];

struct person
{
  int person_id;
  int sex;
  int group;
  int birthdate;
  int deathdate;
  int next_event;
  int mqueue_month; /*used to track wait on mqueue */
  struct person *mother;
  struct person *father;
  struct person *e_sib_mom;
  struct person *e_sib_dad;
  struct person *lborn;
  struct marriage *last_marriage;
  int mstatus;
  double fmult; /*fert mult*/
  double tmult; /*transit mult not built in*/
  double dmult; /*death mult not built in*/
  struct person *down;
  struct person *(*pref)(person*);
  double (*score)(struct person*, struct person*);
  int pointer_type[2];
  union {
    struct person *next_on_mqueue;
    struct queue_element *mqueue;
  } u_marriage_queue;
  union {
    struct person *next_person;
    struct queue_element *month;
  } u_event_queue;
  struct extra_data *extra;
  struct transition *ltrans; /*track transition history*/
};

#define NEXT_PERSON u_event_queue.next_person
#define MONTH u_event_queue.month

#define NEXT_ON_MQUEUE u_marriage_queue.next_on_mqueue
#define MQUEUE u_marriage_queue.mqueue

#define EVENT_QUEUE 0
#define MARRIAGE_QUEUE 1

#define NEXT_ELEMENT(p) \
  ((q_type) == EVENT_QUEUE) ? (p)->NEXT_PERSON : (p)->NEXT_ON_MQUEUE
#define NEXT_NODE(p) \
  ((q_type) == EVENT_QUEUE) ? (p)->MONTH : (p)->MQUEUE

#define SET_NEXT_ELEMENT(p, q) \
  ((q_type) == EVENT_QUEUE) ? ((p)->NEXT_PERSON = (q)) : ((p)->NEXT_ON_MQUEUE = (q))
#define SET_NEXT_NODE(p, e) \
  ((q_type) == EVENT_QUEUE) ? ((p)->MONTH = (e)) : ((p)->MQUEUE = (e))

struct marriage
{
  int marr_id;
  struct person *wife;
  struct person *husband;
  int date_start;
  int date_end;
  int reason_end;
  struct marriage *husbands_prior;
  struct marriage *wifes_prior;
  struct marriage *down;
};

struct transition
{
  int date;
  int fromg;
  int tog;
  struct transition *prior;
};

/* left over from Ken's AIDS project 
   generalized now 

struct extra {
    int parity;
    int marity;
    int prev_group;
    int migration_date;
    int prev_marital_status;
    int factor;
    int mqueue_month;
  int birth_group;
  float swet_weight;
  float tmult;
};
*/
/* extra is the stuff of .opox -- stuff like socio-economic
   variables that can affect rates go here. It is a linked
   list of floats utils.c will have get and put functions
   for easy manipulation
   
   For a start, we'll use extra to track the most recent group migration and
   the month in which a person joined the marriage queue. These variables will
   NEVER be read in so the read_extra function will need to be careful about 
   this. 

*/

/* Now some definitions created for Ken's Thai AIDS project 
   These should be ignorable by all others
*/
#define FACTOR 5
#define SWET_WEIGHT 6
#define TMULT 7

struct extra_data
{
  float value;
  EXTRA_PTR_DECLARATION;
  struct extra_data *next;
};

#define PTR_NULL 0
#define PTR_N 1
#define PTR_Q 2

/* debugging hooks for some experiments */





/* explicit function declarations: */
void install_in_order(struct person *p, struct queue_element *e, int q_type);
void transit(struct person * q);
void assemble_household( struct person * p);
void population_pyramid(FILE * fd_pyr);
FILE *open_write(char*);
void print_segment_info();
void logmsg(const char * frmt, const char * msg, int where);
void initialize_segment_vars();
void initialize_marriage_targets();
void new_events_for_all();
void queue_delete(struct person *p, int q_type);
void print_rate_table(struct age_block *r);