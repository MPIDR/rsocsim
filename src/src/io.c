//#include "defs.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
#define H_WIFE i1
#define H_HUSBAND i2
#define H_W_PRIOR i3
#define H_H_PRIOR i4
*/

/** file desc for intermediate post segment results 
    Tue Oct 22 12:35:04 PDT 2002 
**/

extern FILE *fd_out_pop_seg, *fd_out_mar_seg, *fd_out_xtra_seg,
    *fd_out_otx_seg;

extern FILE *fd_pop, *fd_mar, *fd_xtra, *fd_out_pop, *fd_out_mar, *fd_out_xtra;
extern FILE *fd_rn, *fd_out_otx, *fd_out_otx_seg;
extern FILE *fd_pyr, *fd_stat, *fd_log;

void logmsg(char * frmt, char * msg, int where)
{

  /** This would be better if it could be treated as a pintf as it is
      msg must be passed as a string and this requires sprintf before this
      can be called **/
  /** Fri Dec  6 16:30:59 PST 2002
   ** over time, all screen output should be funelled through this
   ** it will write a line to the screen if where >=1 and to the log_file
   **/

  fprintf(fd_log, frmt, msg);
  fflush(fd_log); /*flush the buffer */
  if (where)
  {
    printf(frmt, msg);
  }
  return;
}

/***************************

int make_random_socsim_NEVERUSED (){
  /****************************************
   * This will check for existance of file random.socsim. if it finds it
   * it will read it and create a linked list of random uniforms. If not
   * it will write 100K random uniforms to the file and then open it 
   * and read it and create the linked list
   ***************************************
  FILE *fd_random_socsim;
  struct stat bla; 
  int fexists,i,command_line_ceed;
  double runif;
  fexists = (stat("random.socsim",&bla)==0);
  
  command_line_ceed= ceed;

  if (! fexists){
    
    logmsg("random.socsim file not found ... creating it now\n","",1);
    ceed = RECYCLE_SEED;
    if((fd_random_socsim = fopen("random.socsim","w")) == NULL){
      perror("Can't open file: random.socsim\n");
    }
    
    for (i=1;i<=100000;i++){
      runif=real_rrandom();
      fprintf(fd_random_socsim,"%.18f\n",runif);
    }
    fclose(fd_random_socsim);
    logmsg("random.socsim written\n","",1);
    
  } /* random.socsim not found and therefore created */

/* read from random.socsim and create linked list 

  if((fd_random_socsim = fopen("random.socsim","r")) == NULL){
    perror("Can't open file: random.socsim for reading...?\n");
  }
  logmsg("reading random numbers from  file\n","",1);
  first_random_number = NEW(struct random_number);
  int junk = fscanf(fd_random_socsim,"%lf",&runif);
  first_random_number->value=runif;
  current_random_number=first_random_number;
  while(fscanf(fd_random_socsim, "%lf",&runif) != EOF){
    current_random_number->next = NEW(struct random_number);
    current_random_number=current_random_number->next;
    current_random_number->value=runif;
  }
  current_random_number->next=first_random_number;
  current_random_number=first_random_number;
  logmsg("random number table read from file\n","",1);
  for (i=1;i<=command_line_ceed; i++){
    current_random_number=current_random_number->next;
  }
  }  ***/
/***********************************************************************/
void initialize_person(struct person *pnew)

{
  /***
      This will plug a bunch of NULL and zero values into a 
      person struct. It is nec because solaris and no doubt other
      versions of gcc do not initialize newly malloc'ed structures in
      happy ways 
  ***/
  /*
    p->extra=NULL; 
    p->mother=NULL;
    p->father=NULL;
    p->e_sib_mom=NULL;
    p->e_sib_dad=NULL;
    p->tmult=1.0;
    p->dmult=1.0;
    p->fmult=0;
    p->lborn=NULL;
    p->last_marriage=NULL;
  */

  pnew->person_id = 0;
  pnew->sex = 99; /* initialize to something silly */
  pnew->group = 0;
  pnew->birthdate = 0;
  pnew->next_event = 0;
  pnew->mqueue_month = 0;
  pnew->mother = NULL;
  pnew->father = NULL;
  pnew->e_sib_mom = NULL;
  pnew->e_sib_dad = NULL;
  pnew->lborn = NULL;
  pnew->last_marriage = NULL;
  pnew->mstatus = MS_NULL;
  pnew->fmult = 1.0;
  pnew->dmult = 1.0;
  pnew->tmult = 1.0;
  pnew->down = NULL;
  pnew->extra = NULL;
  pnew->ltrans = NULL;
}
/************************************************************************/
int sanity_check_person(struct person *p)
{
  /*
This is called from read_initial_pop (and perhaps elsewhere latere)
after reading a person from the initial pop file and just before
installing. It will do basic sanity checking for valid ranges of
inputs
*/
  char logstring[256];
  int result = 0;
  if (p->person_id <= 0)
  {
    sprintf(logstring, "\nInvalid person id read: %d", p->person_id);
    logmsg("%s\n", logstring, 1);
    result++;
  }
  if (p->sex != MALE && p->sex != FEMALE)
  {
    sprintf(logstring, "\nPerson %d in init pop file has invalid sex %d",
            p->person_id, p->sex);
    logmsg("%s\n", logstring, 1);
    result++;
  }
  if (p->group <= 0 || p->group > MAXGROUPS)
  {
    sprintf(logstring, "\nPerson %d in init pop file has invalid group %d",
            p->person_id, p->group);
    logmsg("%s\n", logstring, 1);
    result++;
  }
  if (p->birthdate <= 0)
  {
    sprintf(logstring, "\nPerson %d in init pop file has invalid birthdate %d",
            p->person_id, p->birthdate);
    logmsg("%s\n", logstring, 1);
    result++;
  }
  return (result);
}
int read_initial_pop(FILE *fd)
{
  struct person *p, *prev_p;
  char line[200];
  int imother, ifather, iesibmom, iesibdad, ilborn, ilastmar;
  int i;
  int row = 0;

  for (i = 0; i < HASHSIZE; i++)
  {
    hashtab[i] = NULL;
  }
  numgroups_in_ipop = 1; /*must be at least one group*/
  prev_p = NULL;
  p = NEW(struct person);

  initialize_person(p); /* set everything to zero /NULL */
  person0 = p;
  /*set fgets arg to 150 when conversion flags in write_person were increased used to be 110 */
  while (fgets(line, 150, fd) != NULL)
  {
    if (sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %lf",
               &p->person_id, &p->sex, &p->group, &p->next_event,
               &p->birthdate,
               &imother, &ifather, &iesibmom, &iesibdad, &ilborn,
               &ilastmar, &p->mstatus, &p->deathdate, &p->fmult) == 14)
    {
      if ((p->sex == FEMALE) && (p->fmult == 0))
      {
        p->fmult = fertmult();
      }
      if ((p->sex == FEMALE) && (p->fmult > 100))
      {
        printf("warning--fmult > 100. Is the format correct?\n");
      }
      /*
	  printf("id %d fmult %f\n", p->person_id, p->fmult);;
	*/
    }
    else if (sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d",
                    &p->person_id, &p->sex, &p->group, &p->next_event,
                    &p->birthdate, &imother, &ifather,
                    &iesibmom, &iesibdad,
                    &ilborn, &ilastmar, &p->mstatus,
                    &p->deathdate) == 13)
    {
      if (p->sex == FEMALE)
      {
        p->fmult = fertmult();
        /*
	    printf("id %d fmult %f\n", p->person_id, p->fmult);;
	  */
      }
      else
      {
        p->fmult = 0;
      }
    }
    else
    {
      perror("population file: invalid input");
    }

    if (p->mstatus == 0)
    {
      p->mstatus = 1;
    }

    if (prev_p != NULL)
    {
      prev_p->down = p;
    }
    p->down = NULL;
    p->NEXT_ON_MQUEUE = NULL;
    if (sanity_check_person(p) != 0)
    {
      stop("dying after reading bad person from init opop");//exit(-1);
    }
    prev_p = p;
    /* increment group counter if nec */
    numgroups_in_ipop = MAX(p->group, numgroups_in_ipop);
    if (install(PERSON_TAG, p->person_id, p, imother, ifather, iesibmom,
                iesibdad, ilborn, ilastmar) == NULL)
    {
      warning("hash table problems");
    }

#ifdef ENHANCED
    enhance_read_initial_pop(p);
#endif

    p = NEW(struct person);
    initialize_person(p);
    row++; /* count number of rows read */
    /*printf("%s\n", line); */
  }

  last_person = prev_p;
  free(p);
  return (row);
  /*
      dump_table();
    */
}

void read_marlist(FILE *fd)
{
  struct marriage *m, *prev_m;
  struct nlist *np;
  char line[200];

  int iwife, ihusband, iwifes_prior, ihusbands_prior;

  prev_m = NULL;
  m = NEW(struct marriage);
  marriage0 = m;

  while (fgets(line, 110, fd) != NULL)
  {
    if (sscanf(line, "%d %d %d %d %d %d %d %d",
               &m->marr_id, &iwife, &ihusband,
               &m->date_start, &m->date_end, &m->reason_end,
               &iwifes_prior, &ihusbands_prior) != 8)
    {
      perror("marriage file: invalid input");
    }

    /*
      printf("%s\n", line);
    */

    last_event_date = MAX(last_event_date, m->date_start);
    last_event_date = MAX(last_event_date, m->date_end);

    install(MARRIAGE_TAG, m->marr_id, m, 0, 0, 0, 0, 0, 0);

    if (prev_m != NULL)
    {
      prev_m->down = m;
    }

    /* The omar file is in id==temporal order, so given a marriage it
     * is always possible to find the pointer to a previous marriage.
     * There's no need to store id numbers for future reconciliation.
     */
    np = lookup(PERSON_TAG, iwife);
    if (np != NULL)
    {
      m->wife = np->PP;
    }
    else
    {
      m->wife = NULL;
      printf("wife id %d gives null pointer back\n", iwife);
    }

    np = lookup(PERSON_TAG, ihusband);
    if (np != NULL)
    {
      m->husband = np->PP;
    }
    else
    {
      m->husband = NULL;
      printf("husband id %d gives null pointer back\n", ihusband);
    }

    np = lookup(MARRIAGE_TAG, iwifes_prior);
    if (np != NULL)
    {
      m->wifes_prior = np->MM;
    }
    else
    {
      m->wifes_prior = NULL;
    }

    np = lookup(MARRIAGE_TAG, ihusbands_prior);
    if (np != NULL)
    {
      m->husbands_prior = np->MM;
    }
    else
    {
      m->husbands_prior = NULL;
    }

    m->down = NULL;
    prev_m = m;
    m = NEW(struct marriage);
  }
  last_marriage = prev_m;
  free(m);
  /** if this simulation begins with no marriage file and has zero 
      marriage rates than attempts to write the marriage file will 
      segfault unless marriage0 = NULL
  **/

  if (last_marriage == NULL)
  {
    marriage0 = NULL;
  }
}

int read_xtra(FILE *fd, int pop_rows)
    /***********************************************************************
  Tue Feb  7 12:27:20 PST 2006  The goal now is to be able to read
  .opox without knowing what is in it. The first element in each row
  has to be a person id but beyond that -- everything is just floats
  that will be added onto the end of the egos_extra link list. 
***********************************************************************/
{
  char line[10]; /*might need to increase this*/
  struct nlist *np;
  int id, val, indx, row;
  float rval;
  struct person *p;

  /* this will not work if size_of_extra is zero -- must be set
     either in .sup or in enhancement code */
  if (size_of_extra <= 0)
  {
    char logstring[256];
    sprintf(logstring, "size_of_xtra is %d yet you want me to read an .opox file.  Set size_of_xtra to number of xtra variables in .sup or in more likely in enhancement code",
            size_of_extra);
    stop("%s\n", logstring, 1);// exit(-1);
  }

  row = 1;
  while ((!feof(fd)) && (row <= pop_rows))
  {

    if (fscanf(fd, "%d", &id) != 1)
    {
      char logstring[256];
      stop(logstring, "can't find person id in line %d of .opox file", row);
      logmsg("%s\n", logstring, 1);
      //exit(-1);
    }
    np = lookup(PERSON_TAG, id);

    if (np == NULL)
    {
      char logstring[256];
      stop(logstring, "Error reading opox person id %d gives null pointer back", id);
      logmsg("%s\n", logstring, 1);
      //exit(-1);
    }
    p = np->PP;

    for (indx = 0; indx < (size_of_extra); indx++)
    {
      if (fscanf(fd, "%f", &rval) != 1)
      {
        char logstring[256];

        stop(logstring,
                "bad input reading  %d'th col pid=%d of opox",
                indx, id);
        logmsg("%s\n", logstring, 1);
        //exit(-1);
      }
      put_extra(p, indx, rval);
    }
#ifdef ENHANCED
    enhance_read_extra(fd, p);
#endif

    row++;
    if (!feof(fd))
    {
      char *junk = fgets(line, 1, fd); /* stupid way to toss 
					 the rest of the line*/
    }
  }
  if (row != pop_rows)
  {
    char logstring[256];
    sprintf(logstring, "WARNING: Read %d from .opop and %d from .opox",
            pop_rows, row);
    logmsg("%s\n", logstring, 1);
  }
  return (row);
}

/********************************************************************/

void read_otx(FILE *fd)
{
  struct transition *tx, *newer = NULL;
  char line[200];
  int pid, seq;
  struct nlist *np;

  while (fgets(line, 110, fd) != NULL)
  {
    tx = NEW(struct transition);
    if (sscanf(line, "%d %d %d %d %d",
               &pid, &tx->date, &tx->fromg, &tx->tog, &seq) != 5)
    {
      perror("otx file: invalid input");
      stop("otx file: invalid input");//exit;
    }

    tx->prior = NULL;
    if (seq == 0)
    {
      np = lookup(PERSON_TAG, pid);
      np->PP->ltrans = tx;
      newer = tx;
    }
    else
    {
      newer->prior = tx;
      newer = tx;
    }
  }
}
/**********************************************************************/

void delete_hash_table()
{
  struct nlist *np, *npp;
  int i;

  for (i = 0; i < HASHSIZE; i++)
  {
    np = hashtab[i];
    while (np != NULL)
    {
      npp = np;
      np = np->next;
      free(npp);
    }
  }
}


void fix_pop_pointers()
{
  struct person *p, *pp;
  struct marriage *mm;
  struct nlist *np, *npp;

  last_person_id = 0;
  for (p = person0; p != NULL; p = p->down)
  {

    last_event_date = MAX(last_event_date, p->birthdate);
    last_event_date = MAX(last_event_date, p->deathdate);
    if (p->deathdate == 0)
    {
      size_of_pop[0]++;
      size_of_pop[p->group]++;
    }
    last_person_id = MAX(last_person_id, p->person_id);

    np = lookup(PERSON_TAG, p->person_id);
    if (p != np->PP)
      perror("got the wrong person out of the hash table");

    npp = lookup(PERSON_TAG, np->H_MOTHER);
    if (npp != NULL)
    {
      p->mother = npp->PP;
      /* printf("mother lookup: %d\n", npp->PP->person_id);  */
    }
    else
    {
      p->mother = NULL;
      /* printf("mother lookup: NULL\n");  */
    }

    npp = lookup(PERSON_TAG, np->H_FATHER);
    if (npp != NULL)
    {
      p->father = npp->PP;
      /* printf("father lookup: %d\n", npp->PP->person_id);  */
    }
    else
    {
      p->father = NULL;
      /* printf("father lookup: NULL\n");  */
    }

    npp = lookup(PERSON_TAG, np->H_E_SIB_MOM);
    if (npp != NULL)
    {
      p->e_sib_mom = npp->PP;
      /* printf("e_sib_mom lookup: %d\n", npp->PP->person_id);  */
    }
    else
    {
      p->e_sib_mom = NULL;
      /* printf("e_sib_mom lookup: NULL\n");  */
    }

    npp = lookup(PERSON_TAG, np->H_E_SIB_DAD);
    if (npp != NULL)
    {
      p->e_sib_dad = npp->PP;
      /* printf("e_sib_dad lookup: %d\n", npp->PP->person_id);  */
    }
    else
    {
      p->e_sib_dad = NULL;
      /* printf("e_sib_dad lookup: NULL\n");  */
    }

    npp = lookup(PERSON_TAG, np->H_LBORN);
    if (npp != NULL)
    {
      p->lborn = npp->PP;
      /* printf("lborn lookup: %d\n", npp->PP->person_id);  */
    }
    else
    {
      p->lborn = NULL;
      /* printf("lborn lookup: NULL\n");  */
    }

    npp = lookup(MARRIAGE_TAG, np->H_LASTMAR);
    if (npp != NULL)
    {
      p->last_marriage = npp->MM;
      /* printf("last_marriage lookup: %d\n", npp->MM->marr_id);  */
    }
    else
    {
      p->last_marriage = NULL;
      /* printf("last_marriage lookup: NULL\n");  */
    }

    p->pointer_type[MARRIAGE_QUEUE] = PTR_NULL;
    p->pref = random_spouse2;
    p->score = score3;
#ifdef ENHANCED
    p->score = enhance_couple_score;
#endif
    /*
	p->egos_extra = NEW(struct extra);
	p->egos_extra->parity = 0;
	p->egos_extra->marity = 0;
	p->egos_extra->prev_group = p->group;
	p->egos_extra->migration_date = 0;
      */
    /*
	p->egos_extra->factor = (irandom() % 21) - 10;
      */
    /*
	p->egos_extra->prev_marital_status = MS_NULL;
      */
  }

  /* can't fix the parity until all the
     * kids have their pointers fixed up
			     * Do marity here, too.
			     */
  /*
    for (p = person0; p != NULL; p = p->down) {
	if (p->sex == FEMALE) {
	    if ((pp = p->lborn) != NULL) {
		p->egos_extra->parity = 1;
		while ((pp = pp->e_sib_mom) != NULL) {
		    p->egos_extra->parity++;
		}
	    }
	}
    */

  /*
	if ((mm = p->last_marriage) != NULL) {
	    if ((mm->reason_end == MARAFTERCOHAB) ||
		 (mm->reason_end == BREAKCOHAB) ||
		 (mm->reason_end == COCOHABDIES)) {
		p->egos_extra->marity = 0;
	    } else {
		if (p->mstatus != COHABITING) {
		    p->egos_extra->marity = 1;
		} else {
		     p->egos_extra->marity = 0;
		}
	    }

	    while ((mm = (p->sex == FEMALE)
			 ? mm->wifes_prior
			 : mm->husbands_prior) != NULL) {
		if ((mm->reason_end != MARAFTERCOHAB) &&
		    (mm->reason_end != BREAKCOHAB) &&
		    (mm->reason_end != COCOHABDIES)) {
		    p->egos_extra->marity++;
		}
	    }
	}
    */
  /*
    if (p->mstatus == COHABITING) {
      mm = p->last_marriage;
      while ((mm = (p->sex == FEMALE)
	      ? mm->wifes_prior
	      : mm->husbands_prior) != NULL) {
	if (mm->reason_end == DEATH) {
	  p->egos_extra->prev_marital_status = WIDOWED;
	  break;
	}
	else if (mm->reason_end == DIVORCE) {
	  p->egos_extra->prev_marital_status = DIVORCED;
	  break;
	}
      }
    */
  /* if here and NULL, then no prev (legal) marriage */

  /*
	    if (p->egos_extra->prev_marital_status == MS_NULL) {
		p->egos_extra->prev_marital_status = SINGLE;
	    }

    */
  /*
	    printf("discovered marital status is %s\n",
		index_to_mstatus[p->egos_extra->prev_marital_status]);
			    */
  /*	}
	}
    */
  /*
    dump_table();
    np = lookup(PERSON_TAG, 11047);
    debug_p = np->PP;
    */
}

/* assuming some kind of uniform distribution I'll just use a mod
 * function, slightly thinned out */

/* Tue Aug 11 11:28:41 PDT 2009 changing "int val" to
   unsigned long long int val in order to solve the surprising integer
   overflow on personid 46431. 

   Tue Mar  2 10:30:32 PST 2010 refined so that hash() now returns an int.
   this was causing a seg fault in lookup on 32bit machines when the return()
   was sending back a long long int.  

*/

int hash(int val)
/*long long int val;*/
{
  /*return (((long long int) (val * val)) % 10000); */
  int result;

  /*  result= (int) (val * val) % 10000; */
  /** think about it... pids are just consequtive integers
      we don't need a fancy hash routine **/
  result = (int)val % 10000;
  return (result);
}

/* find the value in the table: it will give the index of the head of
 * the list*/

struct nlist * lookup(int uniontag, int val) 
{
  struct nlist *np;

  if (val == 0)
    return NULL;
  for (np = hashtab[hash(val)]; np != NULL; np = np->next)
  {
    if ((val == np->val) && (uniontag == np->uniontag))
    {
      return np;
    }
  }
  return NULL; /* not found */
}

struct nlist * install(int uniontag, int val, void * p, int i1, int i2, int i3, int i4, int i5, int i6) 
{
  struct nlist *np, *temp;
  int hashval;

  /*printf("uniontag %d value %d\n", uniontag, val); */

  if ((np = lookup(uniontag, val)) == NULL)
  { /* not found */
    np = NEW(struct nlist);
    hashval = hash(val);
    temp = hashtab[hashval];
    np->next = temp;
    np->val = val;
    np->i1 = i1;
    np->i2 = i2;
    np->i3 = i3;
    np->i4 = i4;
    np->i5 = i5;
    np->i6 = i6;
    np->uniontag = uniontag;
    if (uniontag == PERSON_TAG)
    {
      np->PP = (struct person *)p;
    }
    else
    {
      np->MM = (struct marriage *)p;
    }
    hashtab[hashval] = np;
  }
  return np;
}

int delete_deprecated(int uniontag, int val)
{
  struct nlist *np, *temp;
  int hashval;

  hashval = hash(val);

  if ((np = lookup(uniontag, val)) == NULL)
  { /* not found */
    printf("value not present val:  %d\n", val);
    return -1;
  }

  np = hashtab[hashval];
  if ((np->val == val) && (np->uniontag == uniontag))
  {
    hashtab[hashval] = np->next;
    free(np);
    return 1;
  }
  else
  {
    while (np->next != NULL)
    {
      if (np->next->val == val)
      {
        temp = np->next;
        np->next = temp->next;
        free(temp);
        return 1;
      }
      else
        np = np->next;
    }
  }
  return 1;
}

void dump_table()
{
  int i;
  struct nlist *np;

  for (i = 0; i < HASHSIZE; i++)
  {
    if (hashtab[i] == NULL)
      continue;
    else
    {
      printf("i: %d  ", i);
      for (np = hashtab[i]; np != NULL; np = np->next)
      {
        if (np->uniontag == PERSON_TAG)
        {
          printf("..P %d", np->PP->person_id);
        }
        else
        {
          printf("..M %d", np->MM->marr_id);
        }
      }
      printf("\n");
    }
  }
}

void write_population(FILE *fd)
{
  logmsg("writing population (.opop) ..\n", "", 1);

  struct person *p;
  p = person0;
  while (p != NULL)
  {
    write_person(p, fd);
    p = p->down;
  }
  fclose(fd);
}

void write_person(struct person *p, FILE *fd)
{
  /* Mon Apr 12 17:08:38 PDT 2010
     adding space between elements in person output otherwise they run together
     when population is big*/
  /*    fprintf(fd, "%10d%3d%3d%3d%7d%11d%11d%11d%11d%11d%7d%3d%7d%10lf\n",*/
  fprintf(fd, "%12d%3d%5d%5d%8d%13d%13d%13d%13d%13d%8d%3d%8d%10lf\n",
          p->person_id, p->sex, p->group, p->next_event, p->birthdate,
          (p->mother == NULL) ? 0 : p->mother->person_id,
          (p->father == NULL) ? 0 : p->father->person_id,
          (p->e_sib_mom == NULL) ? 0 : p->e_sib_mom->person_id,
          (p->e_sib_dad == NULL) ? 0 : p->e_sib_dad->person_id,
          (p->lborn == NULL) ? 0 : p->lborn->person_id,
          (p->last_marriage == NULL) ? 0 : p->last_marriage->marr_id,
          p->mstatus, p->deathdate,
          p->fmult);
  /*
    fprintf( stdout, "%d %d %d\n", p->sex, p->egos_extra->paternal,
	    p->egos_extra->maternal);
    */
}

write_marriages(FILE *fd)
{
  struct marriage *m;
  m = marriage0;
  logmsg("writing marriages (.omar)..\n", "", 1);

  while (m != NULL)
  {
    write_marriage(m, fd);
    m = m->down;
  }
  fclose(fd);
}

void write_marriage(marriage *m, FILE *fd)
{
  /* chokes on zero'th marriage rec unclear why
     if block added by carlm 5/22/01 to avoid seg fault when writing 
      marriage file with very small population
      
      Wed Aug  9 11:31:35 PDT 2006 probably the issue above had to
      do with small pops with NO marriages and uninitialized marraige0
      pointer.  On the off chance that a marriage file will have a marriage id 
      of zero, the if block is hereby commented out.
      

 */
  /*  if (m->marr_id !=0){*/
  /*Mon Apr 12 17:10:07 PDT 2010
    adding space here too*/
  /*    fprintf(fd,"%6d%11d%11d%7d%7d%3d%7d%7d \n",*/
  fprintf(fd, "%7d%13d%13d%8d%8d%4d%8d%8d \n",
          m->marr_id, m->wife->person_id, m->husband->person_id,
          m->date_start, m->date_end, m->reason_end,
          (m->wifes_prior == NULL) ? 0 : m->wifes_prior->marr_id,
          (m->husbands_prior == NULL) ? 0 : m->husbands_prior->marr_id);
  /*}*/
}

/*  11/1/99 -- carlm : */
/*  modifications for writing out wachters SWET .opox file see */
/*  read_xtra for corresponding changes on input side */

void write_xtra(FILE *fd)
{
  struct person *p;
  int indx;
  float wvalue;

  logmsg("writing extra (.opox) \n", "", 1);

  p = person0;

  while (p != NULL)
  {
    fprintf(fd, "%6d ", p->person_id);
    if (p->extra != NULL)
    {
      for (indx = 0; indx < size_of_extra; indx++)
      {
        wvalue = get_extra(p, indx);
        fprintf(fd, "%f ", wvalue);
      }

      /*
	  fprintf(fd,"%6d %4d %7d %f %f\n", p->person_id, 
	  p->egos_extra->prev_group,
	  p->egos_extra->migration_date, 
	  p->egos_extra->swet_weight,
	  p->egos_extra->tmult);
	*/
#ifdef ENHANCED
      enhance_write_extra(fd, p);
#endif
      fprintf(fd, "\n", "");
    }
    else
    {
      logmsg("%s\n", "skipping row in .opox for want of data in extra", 0);
    }
    p = p->down;
  }
  fclose(fd);
}

/**********************************************/
void write_otx(FILE *fd)
    /**Wed Jun 23 04:47:33 PDT 2010
     write transition history file. There is no escaping the need to
     be able to determine when group transitions occurred.
     should not be called unless transition history is enabled.
  **/
{
  struct person *p;
  int prior;
  struct transition *ctrans, *ftrans;
  p = person0;

  while (p != NULL)
  {

    ctrans = p->ltrans;
    prior = 0;
    while (ctrans != NULL)
    {
      fprintf(fd, "%9d %8d %6d %6d  %4d\n", p->person_id,
              ctrans->date, ctrans->fromg, ctrans->tog,
              prior);
      ctrans = ctrans->prior;
      prior--;
    }
    p = p->down;
  }
}

/************************************/

FILE *open_write(char *fname)
{
  /*********************************************
   ** opens fname for write on fd or complains
   ** called from prepare_output_files
   *********************************************/
  char emsg[1024];
  FILE *fd;
  
  fprintf(fd_pyr,"-------------OK: open_write: open file: %d", fname);
  if ((fd = fopen(fname, "w")) == NULL)
  {
    strcpy(emsg, "Error in open_write: can't open file:");
    fprintf(fd_pyr, "-------------Error in open_write: can*t open file.");
    strcat(emsg, fname);
    perror(emsg);
  }
  

  return fd;
}

void prepare_output_files(int seg)
{
  /**************************************************
 ** Tue Oct 22 12:45:45 PDT 2002
 ** this will open all of the necessary files for writing 
 ** output. Called with seg=0 it will open the final files. If called
 ** with a seg=n it will open .opopn, omarn & etc for output after
 ** a segment completes. This code used to be in main when only
 ** final popuation tables were writable.
 ** after running this subroutine, all output filenames should be defined
 ** and all file descriptors should be nonnull
 ***********************************************/

  char segnum[6];

  if (seg == 0)
  {
    strcat(pop_out_name, ".opop");
    strcat(mar_out_name, ".omar");
    strcat(xtra_out_name, ".opox");
    strcat(otx_out_name, ".otx");
  }

  strcpy(pop_out_name_seg, pop_out_name);
  strcpy(mar_out_name_seg, mar_out_name);
  strcpy(xtra_out_name_seg, xtra_out_name);
  strcpy(otx_out_name_seg, otx_out_name);

  if (seg > 0)
  {
    sprintf(segnum, "%d", seg);
    strcat(pop_out_name_seg, segnum);
    strcat(mar_out_name_seg, segnum);
    strcat(xtra_out_name_seg, segnum);
    strcat(otx_out_name_seg, segnum);

    fd_out_pop_seg = open_write(pop_out_name_seg);
    fd_out_mar_seg = open_write(mar_out_name_seg);
    fd_out_xtra_seg = open_write(xtra_out_name_seg);
    fd_out_otx_seg = open_write(otx_out_name_seg);
  }
  else
  {

    fd_out_pop = open_write(pop_out_name);
    fd_out_mar = open_write(mar_out_name);
    fd_out_xtra = open_write(xtra_out_name);
    fd_out_otx = open_write(otx_out_name);
  }
}

/**  **/
void write_popfiles(int intermediate)
{
  /** writes all population files at end of sim or where ever **/

  if (intermediate)
  {
    write_population(fd_out_pop_seg);
    fclose(fd_out_pop_seg);
    logmsg("writing marriage list (.omar) ..\n", "", 1);
    write_marriages(fd_out_mar_seg);
    fclose(fd_out_mar_seg);
    if (size_of_extra > 0)
    {
      logmsg("writing extra file (.opox) ..\n", "", 1);
      write_xtra(fd_out_xtra_seg);
      fclose(fd_out_xtra_seg);
    }
    if (numgroups > 1)
    {
      logmsg("writing transition history file (.otx) ..\n", "", 1);
      write_otx(fd_out_otx);
    }
  }
  else
  {
    write_population(fd_out_pop);
    write_marriages(fd_out_mar);
    if (size_of_extra > 0)
    {
      write_xtra(fd_out_xtra);
    }
    if (numgroups > 1)
    {
      logmsg("writing transition hist (.otx)..\n", "", 1);
      write_otx(fd_out_otx);
    }
  }
}
