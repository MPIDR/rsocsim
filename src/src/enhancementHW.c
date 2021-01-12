#include "defs.h"
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <math.h>

/**  This file contains all the functions that get called if
     ENHANCED is def'ed. In general, all customizations should be
     done in this file.  Throughout socsim code, hooks are embedded
     that call functions with names line enahnce_x. IFF ENHANCED is
     defined.  Those enhance_xxx functions live in this file AND must
     be forward declared in defs.h.

     Other functions -- such as those which the enahnce_xx functions
     might call also live here. If they are not referenced elsewhere
     they need not be forward declared.=?

     All enhance_ functions in this file should be 
     forward declared in defs.h as noted there

**/

/** Generally you will want to set up the "extra_variables" that 
you're going to keep track of by defining for example:


#define INCOME 0
#define PIETY 1
#define EDUCATION 2
**/
/** make sure you set SIZE_OF_EXTRA to the number of variables that
  are to be tracked in the extra_variables in the "extra" structure which setup for all persons. The size_of_extra variable gets set to this value
 in enhance_defin()

#define SIZE_OF_EXTRA 3
**/
#define SIZE_OF_EXTRA 5
#define REJECT_COUPLE 0
#define ACCEPT_COUPLE 1
#define MIN_AGE_AT_MARRIAGE 60
/** It's nice to declare functions here which are only called from
    this file -- as noted the enhanced_xxx hook functions need to be
    declared in defs.h
***/

void generate_extra();  /* a function creates a blank extra_data structure
			 called for initial population and at birth*/

double patriDist();
double consanguinity_score();
/*************************************************************************/
void enhance_defaults()
     /** gets called from events.c just after the rate_file (ie .sup) is
	 processed. This is when you make sure that the right things
	 happen regardless of the .sup file's contents. A good place 
	 to force exit if required parameters are not set.
     **/
{
  char logstring[256];  
 
  size_of_extra = SIZE_OF_EXTRA;
  sprintf(logstring,
	  "%s : size_of_xtra is %d",ENHANCEMENT_NAME,
	  size_of_extra);
  
  logmsg("%s\n",logstring,1);
  

 /** Announce on screen and log that this is not your father's socsim**/
  logmsg("\n\nHEADS UP!! this is an enhanced socsim(%s)\n\n",
	 ENHANCEMENT_NAME,1);
  logmsg("**** %s  overiding defaults***\n",ENHANCEMENT_NAME,1);
  logmsg("\n--------------------------------------------------------\n");
  logmsg(" parameter0 : proportion of alleles of type1; if zero then proprotion is 1/position where position = 0.. %d + epsilon\n",(ALLELES -1));
  logmsg(" parameter0 : 2 = init pop will have 0 for all A and 1 for all B allels (for K&Barnes paper replications)\n");
  logmsg(" parameter1 : 0 = generational endogamy prohibited; 1 = allowed\n");
  logmsg(" parameter2 : -1= ALL incest taboos disabled \n");
  logmsg(" parameter2 : 0= incest taboo on sib-sib;parent-child;gpar-gchild;cousin\n");
  logmsg(" parameter2 : 1= cousin incest checks disabled\n");
  logmsg(" parameter2 : 2= MCCM preferred PCCM discouraged; all else equal\n");
  logmsg(" parameter2 : 2.1= MCCM preferred PCCM discouraged then groom mothers pat ancestor=bride fathers pat anc\n");
  logmsg(" parameter2 : 3= PCCM preferred MCCM discouraged above all else\n");
  logmsg(" parameter2 : 3.1= PCCM preferred MCCM discouraged; then grooms fath pat anc=brides moth pat anc \n");
  logmsg(" parameter3 : 1= first born female; subsequent male\n");
  logmsg(" parameter3>10 : target population for each group; dmult will rise/fall if pop <> parameter3 value\n");
  logmsg(" parameter4 : 1 enables consanguinity enhancement with early marr\n");
  logmsg(" parameter4 : 2 enables consanguinity enhancement + disallows male widow remarriage\n");
  logmsg(" parameter4 : X.1 ==> wife takes husband's group on marriage\n");
  
  sprintf(logstring,
	  "Minimum age at marriage for either sex is %d months\n",
	  MIN_AGE_AT_MARRIAGE);
        logmsg("%s\n",logstring,1);

  logmsg("\n--------------------------------------------------------\n");
  /** change config settings and announce ***/


  /*  
  read_xtra_file = 0;

  logmsg("%s: read_xtra_file=0\n",ENHANCEMENT_NAME,1);
  */

  /*******  Recite parameter selections and warn user  ****/


  /*************************parameter0*********************/

  int p0case=0;
  p0case= (int) parameter0 *10;
  switch(p0case){
  case 0:
    logmsg("parameter0 set to 0 thus initial population will have prob of allele ==1 vary accross sites\n",0,1);
    break;

  case 20:
    logmsg("parameter0 set to 2 so all alleles in init pop will be unique per Krawacz and Barnes\n");
    break;

  default:
    
    if(parameter0 >= 0 && parameter0 < 1){   
      sprintf(logstring, 
	      "parameter0 is set to %lf thus all sites will have p=%lf of being intereting\n",parameter0,parameter0);
      
      logmsg("%s\n",logstring,1);
    }else{
      sprintf(logstring, 
	      "parameter0 is set to a value, %lf, that I do not understand. Probably we will all crash and die",parameter0);
      logmsg("%s\n",logstring,1);
    }
  }

  /******************** parameter1 ********************/
  int p1case=0;
  p1case= (int) (parameter1 * 10);
  switch(p1case){

  case 0:
    sprintf(logstring,"\nparameter1 == %f marriages and mating constrained generational endogamy\n",parameter1 );
    logmsg("%s\n",logstring, 1);
    break;
  case 10:
    sprintf(logstring,"\nparameter1 == %f generational endogamy IS PERMITTED\n",
	    parameter1);
    logmsg("%s\n",logstring,1);
    break;
  default:
    sprintf(logstring, 
	    "parameter1 is set to a value, %lf, that I DO NOT UNDERSTAND.",
	    parameter1);
    logmsg("%s\n",logstring,1);
    
  }

  
  /********************parameter2 ********************/
  int p2case = (int) (parameter2 * 10);
  switch(p2case){
    
  case -10:
  logmsg("\nparameter2 == -1 Anything goes NO incest checking;no cousin pref\n",1);
  break;
  case 10:
    
  logmsg("\nparameter2 ==1 Only sib-sib/parent-child prohibitted on marriage or mating\n",1);
  break;
  case 20:
  logmsg("\nparameter2 ==2 MCCM pref PCCM discouraged; indiff to 2+ cousins\n",1);
  break;
  case 21:
    
    logmsg("\nparameter2 ==2.1 MCCM pref PCCM discouraged points for patrilines\n",1);
    break;
    
  case 30:
    
    logmsg("\nparameter2 ==3 PCCM pref MCCM discouraged indiff to 2+ cousins\n",1);
    break;
    
  case 31:
    
    logmsg("\nparameter2 ==3.1 PCCM pref MCCM discouraged points for patrilines\n",1);
    break;
  default:
    sprintf(logstring, 
	    "parameter2 is set to a value, %lf, that I DO NOT UNDERSTAND. ",
	    parameter2);
    logmsg("%s\n",logstring,1);

  }
    
  /*************************parameter3*************************/
  int p3case=0;
  p3case= (int) (parameter3 * 10);

  switch(p3case){
    
  case 10:
    logmsg("\nparameter3==1 so each woman will have girl then boys prop_male ignored\n",1);
    break;
    
  default:
    if(parameter3 > 10){
      sprintf(logstring,"\nparameter3 == %g so this will be taken as target pop for each group\n birth and death multipliera will rise and fall accordingly \n",
	      parameter3);
      logmsg("%s\n",logstring,1);
    }else{
      sprintf(logstring, 
	      "parameter3 is set to a value, %lf, that I DO NOT UNDERSTAND. ",
	      parameter3);
      logmsg("%s\n",logstring,1);
      
    }
  }
  /*************************parameter4*************************/
  int p4case=0;
  p4case= (int) (parameter4 * 10);
  switch(p4case){
  case 0: case 1:
    logmsg("parameter 4 = 0 (default) consang enhancement disabled\n");
    break;
  case 10: case 11:
    logmsg("parameter 4 = 1 young brides only accept close cousins\n");
    break;

  case 20: case 21:
    logmsg("parameter 4 = 2 young brides only accept close cousins AND male widows do NOT remarry\n");
    break;
    
  default:
    sprintf(logstring, 
	    "parameter4 is set to a value, %lf, that I DO NOT UNDERSTAND. ",
	    parameter4);
    logmsg("%s\n",logstring,1);
    
  }
  
  /*check parameter4 for X.1 condition*/
  p4case= (int) (parameter4 * 10) % 10;
  switch(p4case){
  case 1:
    sprintf(logstring, 
	    "parameter4 = %lf ; the X.1 makes wives transition to husband's group on marriage\n",
	    parameter4);
    logmsg("%s\n",logstring,1);
    break;
  default:
    sprintf(logstring, 
	    "parameter4 = %lf ; Wives do NOT have grp transition on marriage\n",
	    parameter4);
    logmsg("%s\n",logstring,1);
    
  }
}

/************************************************************************/

void enhance_read_initial_pop(p)
     struct person *p; 

     /** Gets called just after a person, p is read from the initial
	 pop file.  A good time to generate variables wouldn't you say
     **/

     /** The initial population will have random dna here is where they shall get it**/
{
  /* create p->extra and set "value" or MGENERATION =0 */
  generate_extra(p);

  if( read_xtra_file == 0){
    /* create random set of genes */
    int i,j;
    
    for(i=0;i<ALLELES;i++){
      for(j=0;j<2;j++){
	
	if(parameter0 == 0){
	  p->extra->dna[j][i]=  (rrandom() < ((double)(i+1)/102))? 1 : 0;

	}else{
	  if(parameter0 >0 && parameter0 <1){
	    
	    p->extra->dna[j][i]=  (rrandom() < ((double) parameter0)) ? 1 : 0;
	  }else{
	    if(parameter0 == 2){
	      /* Krawczak & Barnes replication --*/
	      p->extra->dna[j][i]= (float) j ;
	    }
	  }
	}
	/* add serial number to each allele*/
	p->extra->dna[j][i]+= (float) p->person_id/10000;

      }
    }
  }
  
}



/**************************************************************************/
void enhance_birth(mom,child)
     struct person *mom;
     struct person *child;
  {
    /** called at end of xevents:birth args: mother and child this is a
	good time to generate extra variables for the child and to
	modify anything that happens to moms at birth

	NOTE this happens **before** mother and child have next events
	generated AND before male children are installed in marriage queue
	iff a one queue system is happening
    ***/

    if(parameter3 ==1){
      /* this is for girl-boy twins baseline runs*/
      child->sex =(child->e_sib_mom == NULL)? FEMALE : MALE;
      if (child->sex == FEMALE) {
	child->fmult = alpha * mom->fmult + (1 - alpha) * fertmult();
	child->fmult = 2.5 * exp(beta*log(child->fmult/2.5));
      } else {
	child->fmult = 0;
      }

    }
    char logstring[256];

      generate_extra(child);
      put_extra(child,MGENERATION,get_extra(mom,MGENERATION)+1);
      
      put_extra(child,MFOUNDER, 
		get_extra(mom,MFOUNDER)==0? (float) mom->person_id:
		get_extra(mom,MFOUNDER));
      
    
    /** assign genes **/
    int i,pick=0;
    
    /* this is miosis -- one allele from dad one allele from mom*/
    for(i=0;i<ALLELES;i++){
      /* dad genes go in dna[0], moms in dna[1]*/
      if(child->father == NULL){
	/* Dad snuck in from another planet*/
	/*child->extra->dna[0][i]=  (rrandom() <.5)? 0 : 1;*/
	logmsg("\nenhance_birth: immaculate conception problem writing population filse then exiting...\n","",1);
	logmsg("\n\n Simulation failing population files will be written for diagnostic purposes..sorry  is random_father enabled?\n","",1);
	write_popfiles(0);
	exit(0);
	
      }else{
	pick=(rrandom() <.5)? 0 : 1;
	child->extra->dna[0][i]=child->father->extra->dna[pick][i];
      }
      /* get mom's contribution*/
      pick=(rrandom() <.5)? 0 : 1;
      child->extra->dna[1][i]=mom->extra->dna[pick][i];
    }
  
    put_extra(child,PGENERATION,get_extra(child->father,PGENERATION)+1);

    put_extra(child,PFOUNDER, 
	      get_extra(child->father,PFOUNDER)==0? 
	      (float) child->father->person_id:
	      get_extra(child->father,PFOUNDER));

    double parentdist=0;
    if(child->father->mother != NULL &&
       child->mother->father != NULL &&
       child->father->father != NULL &&
       child->mother->mother != NULL ){
      if(parameter2== 2 || parameter2==2.1) {
	parentdist=patriDist(child->father->mother,child->mother->father);
      }else if (parameter2 ==3 || parameter2 == 3.1){
	parentdist=patriDist(child->father->father,child->mother->mother);
      }else{
	parentdist=patriDist(child->father,child->mother);
      }
    }
    put_extra(child,COMMANCEST,parentdist);
      

}

/******************************************************************/
void enhance_death(p)
     struct person *p;
{  
  /** This gets called at the end of  xevents:death good time to
      fiddle any extra variables or whatever
  **/
  
  return;
}

/******************************************************************/

void enhance_divorce(p,spouse)
  struct person *p;
  struct person *spouse;
{

}

/******************************************************************/

void enhance_marriage_queue(p)
  struct person *p;
{

  /** this gets called from xevents:marriage IFF a spouse is not found
      and the person winds up on the marriage queue. 
  **/
      
}
/************************************************************************/
int  enhance_check_spouse(p, suitor)
     struct person *p, *suitor;
{
     /* This gets called from marriage_allowable BEFORE the incest,agediff
	and whatever other checks are inflicted on suitors before marriage
	is permitted.  
	
	THIS IS SETUP TO PRE_EMPT ALL OTHER CHECKS IF THIS RETURNS 0
	THEN THE MATCH WILL BE DENIED AND NO OTHER CHECKS WILL BE
	PERFORMED. However, if it returns 1 then the other checks
	*WILL* be performed
	
	return 1=marriage is OK 0= permision denied*/

  /* the final call is to allow the match so each check routine
     should reject or do nothing*/

    /*Thu Oct 31 12:27:18 PDT 2013 enforce minimum age at marriage
      on both sexes -- even though it will generally only matter
      for males */
    if( (current_month - p->birthdate) <
	MIN_AGE_AT_MARRIAGE) {return(REJECT_COUPLE);};
    if( (current_month - suitor->birthdate) <
	MIN_AGE_AT_MARRIAGE) {return(REJECT_COUPLE);};


  if(parameter1 == 0 && /* generational endog prohibitd param1==0 */
     (get_extra(p,MGENERATION) != get_extra(suitor,MGENERATION))){
    return(0); 
  }
  
  /** parameter4 **/
  double cscore=0;
  struct person *w, *m;

  int p4case=0;
  if (p->sex == FEMALE){
    w = p;
    m = suitor;
  } else {
    m = p;
    w = suitor;
  }
    

  p4case= (int) (parameter4 * 10);
  switch(p4case){
  case 0: case 1:
    break;
  case 10: case 11:
   
 /*    if(parameter4 == 1){ */
    /* consanguinity enhancement plan */

    cscore=consanguinity_score(m,w);
    
    if( ((current_month - w->birthdate) < (7*12)) && (cscore < 2)){
      return(REJECT_COUPLE);
    }
    if( ((current_month - w->birthdate) < (10*12)) && (cscore < 2)){
      return(REJECT_COUPLE);
    }
    if( ((current_month - w->birthdate) < (13*12)) && (cscore < 2 )){
      return(REJECT_COUPLE);
    }
    
    if( ((current_month - w->birthdate) < (15*12)) && (cscore < (1+1/4))){
      return(REJECT_COUPLE);
    }
    break;
  case 20: case 21:
    if( m->mstatus == WIDOWED){return(REJECT_COUPLE);}
    break;

  default:
    /* couple is ok but allow for subsequent checks so pass on without
       returning */
    break;
  }

  
  return(ACCEPT_COUPLE);
 }
  
/************************************************************************/
int enhance_check_endogamy(p,suitor)
     struct person *p,*suitor;

     
{
     /*This is called in place of check_endogamy so if you are not messing
       with groups at the point where the working marriage queue is constructed
       then leave the call to check_endogamy alone*/

  struct person *w, *m;
  
  int p4case=0;
  if (p->sex == FEMALE){
    w = p;
    m = suitor;
  } else {
    m = p;
    w = suitor;
  }
  

  
  
  /** enforce group directionality **/
  /** if p6 is 1 then make sure MCCMs males always marry up 
      and if PCCM then males alternate depending on who dad married
  **/
  if(m->mother->father != NULL){
    /** need to know mom's birth group **/
    int p6case=0;
    p6case= (int) (parameter6 * parameter2* 10);
    switch(p6case){
    case 20: case 21: 
      /** MCCM case all up and around **/
      
    if ((m->group +1 == w->group)||
	((m->group == numgroups)&&(w->group == 1))){
      return(ACCEPT_COUPLE);
    }else{
      return(REJECT_COUPLE);
    }
    break;
    case 30: case 31:
      if((m->mother->father->group > m->group) /*dad married up*/
	 ||(m->group == numgroups && m->mother->father->group ==1)){
	if(m->group -1 == w->group){
	  return(ACCEPT_COUPLE);
	}
      }
      
      if((m->mother->father->group < m->group) /*dad married down*/
	 ||( m->group == 1 && m->mother->father->group == numgroups)){
	if(m->group +1 == w->group){
	  return(ACCEPT_COUPLE);
	}
      }
      return(REJECT_COUPLE);
      break;
    default:
      break;
    }
  }  
  
  return(check_endogamy(p,suitor));

}
/******************************************************************/

void enhance_new_marriage(p, spouse)
struct person *p, *spouse;
{
  
  

  /** This gets called after a successful marriage search -- that is 
      it is called IN PLACE of new_marriage().
      
      as usual, if you don't want to mess with the actions of mew_marriage
than be sure not to disturb the call to new_marriage()

  **/
  /*
    printf( "wifeage: %f husbage: %f cscore %f\n",
	  (double)(current_month - mar->wife->birthdate)/12,
	  (double)(current_month - mar->husband->birthdate)/12,
	 consanguinity_score(mar->husband,mar->wife));
  */
  
  new_marriage(p,spouse); /* return; */

  struct marriage *mar;
  struct person *h, *w;
  
  if (p->sex == FEMALE) {
    h = spouse;
    w = p;
  } else {
    h = p;
    w = spouse;
  }
  
  int p4case=0, eventnum =0, m=0;
  p4case= (int) (parameter4 * 10) % 10; /* ONLY checking for x.1 */
  switch(p4case){
  case 1:
    /*  case 1: case 11: case 21: */
    /* make wife transit to husb group IF she's not already in it */
    if(w->group != h->group){
      /* cancel scheduled event for wife and then execute transit event 
       h&w each got a new event in new_marriage */
      queue_delete(w, EVENT_QUEUE);
   
      eventnum =  NONTRANSIT + h->group;
      /* don't install the event in the queue that confuses things
       just execute it*/
      w->next_event = eventnum;
      transit(w);
      /* new event was assigned in transit() so don't do that again */
    }
    break;
  default:
    break;
  }
  
  return;
}


/******************************************************************/
/*
 
all mortrate age_blocks will have ->mult point to this INSTEAD
of usedeathmult(). This must therefore do at least what usedeathmult()
does which is to 
    return(p->dumlt * crnt_block->lambda); 


  HEADS UP dmult is stored exponentiated, so probably you wanna
  log it; fiddle it; exp it; and return then return it
      
*/
double enhance_usefertmult(p, crnt_block)
     struct person *p;
     struct age_block * crnt_block;
{
  /** this gets assigned to crnt_block->mult INSTEAD of usefertmult
      SO if you want fmult et al to work, you need to make sure that 
      this takes care of plain vanila usefertmult's job -- probably by
      calling it.
  **/
  /* the null option is to *just* call usefertmult */
  /* return(usefertmult(p, crnt_block)); */

  if(parameter3>10) {
    double popratio=0;
    popratio = ((double)size_of_pop[p->group] / parameter3);
    popratio = (popratio > 2 )?  2 : popratio;
    /*    popratio = (popratio < .5)? .5: popratio;*/
    /* 1000 seems a bit extreme but for pop <100 it is required */
    popratio = (popratio < .85)? popratio/1000 : popratio;
    
    return(usefertmult(p, crnt_block)/popratio);
  }
      
  return(usefertmult(p, crnt_block)); 
}
double enhance_usedeathmult(p, crnt_block)
     struct person *p;
     struct age_block * crnt_block;
{
  /* called INSTEAD of usedeathmult so one must make sure that
     the business of plain vanilla usedeathmult is carried out hereby */
  
  /*return(usedeathmult(p, crnt_block));*/
  
  /* this is for keeping the population from growing or shrinking
     dmult will rise if population is high and fall if pop is low
     If parameter3 >10 it will be taken to be the target population
     for each group
  */

  if((parameter3>10) && (crnt_block->upper_age < 45*12)){
    /* Only use dmult for children under 45)*/
    double popratio=0;
    popratio=(double)size_of_pop[p->group] / parameter3;
    popratio = (popratio > 2 )?  2: popratio;
    popratio = (popratio < .5)? popratio*.5: popratio;
    
    return(usedeathmult(p,crnt_block)*popratio);
  }
  return(usedeathmult(p,crnt_block));
  
}



/******************************************************************/


void generate_extra(p)
     struct person *p;
     /* 
	this fills up one extra struct full of variables
	it should probably be called from enhance_birth_event_child() 
	
	from read_initial_pop() (via enhance_generate_extra_all()
      */
{
  put_extra(p,MGENERATION,(float) 0);
  put_extra(p,PGENERATION,(float) 0);
  put_extra(p,MFOUNDER,(float) 0);
  put_extra(p,PFOUNDER,(float) 0);
  put_extra(p,COMMANCEST,(float) -1);
  /*  p->extra->dna[0]= (char *) malloc(ALLELES * sizeof(char));
  p->extra->dna[1]= (char *) malloc(ALLELES * sizeof(char));
  */  
  p->extra->dna[0]= (float *) malloc(ALLELES * sizeof(float));
  p->extra->dna[1]= (float *) malloc(ALLELES * sizeof(float));

}

/******************************************************************/

void enhance_transit_before(q,tl)
     struct person *q;
     struct transit_list *tl;
     /* tl is created by assemble_household function called just before this
      */
{

}

/********************************************************/

void enhance_transit_after(q)
     struct perons *q;
{

  /*************************************************************************/  
}
void enhance_write_extra(fd,p)
     FILE *fd;
     struct person *p;
{
  /*********************************************************
   *This gets called after write_extra has written p's stuff BUT before
   *the fput("\n") that is before the line end is written.
   *this will add stuff as required but NOT put the \n 
   *******************************************************************/

  int i=0, j=0;
  for(j=0;j<2;j++){
    for(i=0;i<ALLELES;i++){
      fprintf(fd,"%f ",p->extra->dna[j][i]);
    }
  }
}

/************************************************/
void
enhance_read_extra(fd,p)
  /**********************************************
   *This gets called from read_xtra immediately after a person's
   *record is processed but before advancing to the next row. This
   *must read what needs to be read and no more --so it does NOT pass
   *to the next row.
   ************************/
  FILE *fd;
  struct person *p;
{
    int i,j;
    char allele;
    for(j=0;j<=1;j++){
      for(i=0;i<ALLELES;i++){
	if(fscanf(fd,"%s", &allele) ==1){
	  p->extra->dna[j][i] = (float) allele;
	}else{
	  char logstring[256];
	  
	  sprintf(logstring,
		  "screwup reading dna of person %d",
		  p->person_id);
	  logmsg("%s\n",logstring,1);
	  exit(-1);
	}
      }
    }
  }
/********************************************************************/
double
patriDist(p,q)
     struct person *p, *q;
     /* This will count back along father's-father's-fathers...
	until the first common ancestor is found. It will then return
	the sum of the generational links back.
	identical person = 0; sibs=2;father-daughter=1;first cousins=4
	
	Will stop at maxdist. If no common anc found, will return -1

	This is used to reckond closeness in enhance_couple_score
     */

{
  struct person *panc, *qanc;
  int pdist=0,qdist=0,maxdist=10;

  panc=p;
  qanc=q;
  /*Relies on the fact that fathers have lower person_ids than their children*/
  while(panc != qanc && pdist < maxdist && qdist< maxdist &&
	panc->father != NULL && qanc->father !=NULL){
    while(pdist <= maxdist && panc->father !=NULL && 
	  panc->person_id > qanc->person_id){
      pdist++;
      panc=panc->father;
    }
    while(qdist <= maxdist && qanc->father != NULL && 
	  qanc->person_id > panc->person_id){
      qdist++;
      qanc=qanc->father;
    }
    /*exit loop if panc==qanc == ancestor found  OR
      if search depth exceeded either b/c maxdist OR
	because we don't have maxdist generation yet in sim
    */
  }
    
  if(panc==qanc){
    /* common ancestor found*/
    return((double) pdist+qdist);
  }else{
    /* search ended without identifying commons ancestor */
    return((double) -1);
  }
}
  /***************************************************/
double
enhance_couple_score(p, suitor)
/*** This *replaces* score3 when ENHANCED is defined. The default
     version must therefore call score3.  Score3 reckons the couple's
     score based solely on age difference prefs as defined in the .sup
     file.  So IF you want the age diff to matter you have to handle
     it here -- perhaps by calling score3
     
     Higher numbers are better; the suitor who produces the highest
     score will get the spouse.
**/
     
     struct person *p, *suitor;
{
  
  
  double age_score=0, cscore=0, score=0;
  /* The next two lines effectively noop this function so that
     socsim's behavior is not "enhanced" w.r.t scoring potential mates 
     
     age_score= score3(p,suitor);
     return(age_score);
  /*} end of stub*/
  if(parameter2 != 0 && parameter2 != 1){ 
    struct person *w, *m;
    if (p->sex == FEMALE){
      w = p;
      m = suitor;
    } else {
      m = p;
      w = suitor;
    }
    cscore=consanguinity_score(m,w);
  }
  age_score= score3(p,suitor);
  score=age_score +  cscore;
  return(score);
  
}

/************************************************************************/
double
consanguinity_score(m, w) 
  
     struct person *w, *m;
{ 
  /* This will do all of the consanguinity scoring for prospective
     p and suitor it is to be called from enhance_couple_score
  */
  double score = 0;
  enum prents {mom, pop};
  int wgp[2][2], mgp[2][2];
  
  /*indicators for cousininness*/
  int PCCM=0,MCCM=0,PpcM=0,MpcM=0;
  /*distance scores for 2nd+ cousins*/
  double PCCMdist=0,MCCMdist=0,PpcMdist=0,MpcMdist=0;

  /* parameter2 ==1 or 0 we need go no further we are indifferent to
     cousinliness; parma2 == 1/0 differ only in incest taboo. 0 is the
     standard socsim taboo (against cousins and closer; 1 allows
     cousins but nothing closer neither need go looking for cousins*/
  
  if(parameter2 ==1 || parameter2==0){return(0);}
  
  if(w->mother==NULL || w->father==NULL || m->mother==NULL || m->father==NULL){
    /*none of this works for the founding generation*/
    /*     return(age_score);*/
    return(0);
  }
  
  /* no parent-child or sibling-sibling matches that's just yucky
      should be eliminated in (enhance)check_incest but one can't be
      too careful*/
  if (w->mother == m->mother ||
      w->father == m->father ||
      w->father == m         ||
      m->mother == w ){
    return(-1000);
  }
  
  if(
     /* we must have GPs in order to reckon cousinhood*/
     w->mother->mother==NULL||w->mother->father==NULL||
     w->father->father==NULL||w->father->mother==NULL||
     m->mother->mother==NULL||m->mother->father==NULL||
     m->father->father==NULL||m->father->mother==NULL){
    /*return(age_score);*/
    return(0);
  }
  
  /*
  wgp[mom][mom]=w->mother->mother->person_id;
  wgp[mom][pop]=w->mother->father->person_id;
  wgp[pop][pop]=w->father->father->person_id;
  wgp[pop][mom]=w->father->mother->person_id;
  
  mgp[mom][mom]=m->mother->mother->person_id;
  mgp[mom][pop]=m->mother->father->person_id;
  mgp[pop][pop]=m->father->father->person_id;
  mgp[pop][mom]=m->father->mother->person_id;
  */
  
  /** we need to distinguish MBD and FZD marriages which are cross
      cousin Maternal and Paternal respectively . Paralell cousin
       marriages may also be considered **/
  
   /* Here are the cross cousin marriages*/
   /* init pop doesn't count*/   
  /*
  MCCM=(wgp[pop][pop]==mgp[mom][pop]) || (wgp[pop][mom]==mgp[mom][mom]);
  
  PCCM=(mgp[pop][pop]==wgp[mom][pop]) || (mgp[pop][mom]==wgp[mom][mom]);
  /* Here are the paralell cousin marriages*/
  /* these are not distinguishable from sibling by way of GP
  
  PpcM= (wgp[pop][pop]==mgp[pop][pop]) + (wgp[pop][mom]==mgp[pop][mom]);
  
  MpcM= (mgp[mom][pop]==wgp[mom][pop]) +(mgp[mom][mom]==wgp[mom][mom]);
  */

  MCCM= (w->father->father == m->mother->father)||
    (w->father->mother == m->mother->mother);

  PCCM= (m->father->father==w->mother->father)||
    (m->father->mother==w->mother->mother);
    
  if(parameter2 == 2||parameter2==2.1){
    /*MCCM */
    score=  2*MCCM - 10*PCCM ;
    if(parameter2 == 2.1){
      /* distant relatives also preferred*/
      MCCMdist=patriDist(m->mother,w->father);
      score += (double)(!MCCM)*(1+1/MCCMdist);
    }
  }
  if(parameter2 == 3 || parameter2==3.1){
    /*PCCM only no matching ancestors*/
    score=  2*PCCM -10*MCCM ;
    if(parameter2==3.1){
      PCCMdist= patriDist(m->father,w->mother);
      score+=  (double)(!PCCM)*(1+1/PCCMdist);
    }
  }
  return(score);
}
 
/************************************************************************/
/********************************************************************/
int
enhance_check_incest(p1, p2)
     struct person * p1, *p2;
{



  /*This is called INSTEAD of check_incest. If you are not interested in 
    modifying the way socsim checks for incest among potential spouses then
    do not remove the call to check_incest below.
    
    If you do want to modify incest rules, then returning 0 means
    marriage is VETOED. returning 1 means it can happen as long as the
    other checks are satisfied.
  */
    
/*  return(check_incest(p1,p2)); */

  int par2 = (int) (parameter2*10);
  switch(par2){

  case -10:
    /*NO incest checking at all*/
    return(ACCEPT_COUPLE);
    break;
  
  case 0:
     /* parameter2==0 ==> the usual socsim incest check NO cousins */
    return(check_incest(p1,p2)); 
    break;

  case 10: case 20: case 21: case 30: case 31:

    /* this gets called from random_father -- we need to wory about
       dads and daughters */
    /* {return(check_incest(p1,p2));}*/
   /* no sib-sib; no parent-child; but cousin is ok*/
    
    /** Nothing to say about initial pop members or to be safe their
	descendents to 2 generations**/
    if ((p1->mother == NULL) || (p2->mother == NULL) ||
	(p1->father == NULL) || (p2->father == NULL)){
      return(ACCEPT_COUPLE);
    }
    
    
    /*brother-sister marriages are not ok*/
    if ((p1->mother == p2->mother)||(p1->father == p2->father)){
      /*    logmsg("%s\n","rejecting siblings incest",1);*/
      return(REJECT_COUPLE) ; /* 0== NO*/
    }
    
    /*father-dauhter or mother-son not ok*/
    if((p1->mother == p2) || (p1->father == p2) || (p2->mother == p1) ||
       (p2->father == p1)){
      return(REJECT_COUPLE);  /* 0== NO*/
    }
    
  
    /*grandfather -granddaughter is probably not so great either but
      it seems unlikely enough to ignore but no cousin reckoning
    is possible until there be grand parents*/
    if((p1->mother->mother==NULL)|| (p2->mother->mother==NULL)||
       (p1->mother->father==NULL)|| (p2->mother->father==NULL)||
       (p1->father->mother==NULL) || (p2->father->mother==NULL)||
       (p1->father->father==NULL) || (p2->father->father==NULL)){
      return(ACCEPT_COUPLE); 
    }
    if(p1->mother->father == p2 || p1->father->father==p2 ||
       p2->mother->father == p1 || p2->father->father==p1){
      return(REJECT_COUPLE); /* 0 = NO*/
    }
    
    
    return(ACCEPT_COUPLE); /* marriage is ok by me */

    break;
  default:
    logmsg("\n%s\n",
	   "Nonesensical parameter2 value snuck by better crash\n",
	   1);
    exit -1;
  }
  /*by default return 1 for marriage ok assume it will be
    incest checked elsewhere*/
 
}
/***********************************************************************/
/********************************************************************/
int
enhance_check_agedif(p1,p2)
     struct person * p1, *p2;
{
  /** this just checks age difference husband - wife  accepts with 1
      if within bounds otherwiser rejects with 0 
  **/

  /** This will call the default age difference function. Leave it alone
      if you do not want to mess with age difference based spouse preferences
  **/
  

return(check_agedif(p1,p2));

  /*
  struct person  *husb, *wife;

  if(p1->sex == MALE){
    husb=p1;
    wife=p2;
  }else{
    wife=p1;
    husb=p2;
  }
**/
}




