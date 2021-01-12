#include "defs.h"
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/file.h>
#include <math.h>

/** these lines must be included in enhancement.h
#define ENHANCED 1
#define ENHANCEMENT_NAME "EmilioZ_HIV_orphans_0"
**/


/**  Mon Sep 28 06:40:14 PDT 2009
     This enhancement is for Emillio Z's project. It will introduce
     corelation between HIV status and that of spouse.  The plan is:

     1) set hettrans= !0 in .sup
     2) generate tmult 
     3) on marriage check HIV status of spouse if pos (and spouse's is
     negative, set tmult to hettrans before spouse gets new events
     4) on transit to HIV positive status if spouse is negative, set
     tmult to hettrans and generate new events.


     This file contains all the functions that get called if
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
#define SIZE_OF_EXTRA 0

/** It's nice to declare functions here which are only called from
    this file -- as noted the enhanced_xxx hook functions need to be
    declared in defs.h
***/
int hiv_status(p)
     struct person *p; 
     
     /* called only from enhancement.c returns 1 if perosn is HIV positive
	-- for now this just means membership in group 2 */
{
  if(p->group == 2){
    return(1);
  }else{
    return(0);
  }
}

void generate_extra();  /* a function that draws random values for a
			   person's extra struct */
		       

/*************************************************************************/
void enhance_defaults()
     /** gets called from events.c just after the rate_file (ie .sup) is
	 processed. This is when you make sure that the right things
	 happen regardless of the .sup file's contents
     **/
{
  char logstring[256];

  /** Announce on screen and log that this is not your father's socsim**/
  logmsg("\n\nHEADS UP!! this is an enhanced socsim(%s)\n\n",
	 ENHANCEMENT_NAME,1);
  logmsg("**** %s  overiding defaults***\n",ENHANCEMENT_NAME,1);

  /** change config settings and announce ***/

  read_xtra_file = 0;
  /*  logmsg("%s: read_xtra_file=0\n",ENHANCEMENT_NAME,1); */
  size_of_extra = SIZE_OF_EXTRA;
  /*  logmsg("%s: size_of_xtra is \n",ENHANCEMENT_NAME,1); */
  
  sprintf(logstring," : %f\n", parameter0);

  sprintf(logstring, "inter-spousal HIV transmission multiplier (parameter0)=%f\n",parameter0);
  logmsg("%s",logstring,1);

  sprintf(logstring, "parameter1 %f\n",parameter1);
  logmsg("%s",logstring,1);
  sprintf(logstring, "parameter2 %f\n",parameter2);
  logmsg("%s",logstring,1);
  sprintf(logstring,"parameter3 %f\n",parameter3);
  logmsg("%s",logstring,1);
  sprintf(logstring, "parameter4 %f\n",parameter4);
  logmsg("%s",logstring,1);
  sprintf(logstring, "parameter5 %f\n",parameter5);
  logmsg("%s",logstring,1);

}

/************************************************************************/

void enhance_read_initial_pop(p)
     struct person *p; 

     /** Gets called just after a person, p is read from the initial
	 pop file.  A good time to generate variables wouldn't you say
     **/
 
{
  
  generate_extra(p);
}


/**************************************************************************/
void enhance_birth(mom,child)
  struct person *mom;
  struct person *child;
  {
  /** called at end of xevents:birth args: mother and child this is a
      good time to generate extra variables for the child and to
      modify anything that happens to moms at birht
  ***/

  generate_extra(child);
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

/******************************************************************/

void enhance_new_marriage(mar)
     struct marriage *mar;
{
  /** This gets called after a successful marriage -- that is at the
      end of xevents:new_marriage() after a new marriage structure
      has been created and filled in  BUT BEFORE new events are generated for the spouses
  **/
  
  /** Mon Sep 28 07:08:49 PDT 2009
      On marriage check for HIV status and tweak tmult of all current
      spouses before calling event lottary thereon **/

  /** assuming that polygamy only happens on one side -- multiple
      wives OR multiple husbands but not both **/
  struct marriage *pmar;
  char logstring[256];
 if( hiv_status(mar->wife) && ! hiv_status(mar->husband)){
    /* wife pos, husb negative */
    mar->husband->tmult=parameter0;
    
    /*
    sprintf(logstring,"husband :HIV-  wife:HIV+ tmult=%f\n",
    mar->husband->tmult);
    logmsg("%s",logstring,1);
    */


  }
  
  if( hiv_status(mar->husband) && ! hiv_status(mar->wife)){
    /* husb pos, wife negative */
    mar->wife->tmult=parameter0;
    /*
    sprintf(logstring,"husband :HIV+  wife:HIV- tmult=%f\n",
    mar->wife->tmult);
    logmsg("%s",logstring,1); 
    */
  }
    

}

/************************************************************************/
void enhance_transit_before(p,tl)
     struct person *p;
     struct transit_list *tl;
{
  /** this gets a pointer to the egp that is transitioning and also to
      the transit list. First element of tl is ego, the rest are all
      members of ego's household spouses, children and etc tl is ONLY
      the migrant UNLESS hhmigration >0  **/

}
/************************************************************************/
void enhance_transit_after(p)
     struct person *p;

{
  /** This gets called AFTER ego and perhaps her household has already
      had their transition events i.e. group membership has been
      changed AND NEW EVENTS HAVE BEEN GENERATED AS REQUIRED **/

  /** for the present purpose, we need to verify that ego has
      transited to group 2  AND then we  need to tweak the tmult of
      all who have sex with him/her. **/

  /* is this a 1->2 transit */
  int destgroup,month;
  char logstring[256];

  /*printf("%s\n","Visiting enhance_transit_after"); */

  destgroup = p->group;

  if (destgroup != 2) {
        return;
  }
  if (p-> last_marriage != NULL){
    struct marriage *mar;
    mar = p->last_marriage;
    if(p->sex == MALE){
      while(mar != NULL) {
	if(mar->reason_end == M_NULL){
	  if (hiv_status(mar->wife) == 0){
	    mar->wife->tmult=parameter0;

	    /*
	      sprintf(logstring, 
	      "transit of H( %d ) -> tmult bump for W (%d)\n",
	      p->person_id,mar->wife->person_id);
	      logmsg("%s",logstring,1);
	    */
	    /** generate new event **/
	    queue_delete(mar->wife, EVENT_QUEUE);
	    month = date_and_event(mar->wife);
	    month %= MAXUMONTHS;
	    install_in_order(mar->wife, event_queue + month, EVENT_QUEUE);
	    /** event scheduled */
	  }
	}
	mar=mar->husbands_prior;
      }
    }else{
      if(p->sex == FEMALE){
	while(mar != NULL) {
	  if(mar->reason_end == M_NULL){
	    if (hiv_status(mar->husband) == 0){
	      mar->husband->tmult=parameter0;
	      /*
		sprintf(logstring, 
		"transit of W( %d ) -> tmult bump for H (%d)\n",
		p->person_id,mar->husband->person_id);
		logmsg("%s",logstring,1);
	      */
	    /** generate new event **/
	    queue_delete(mar->husband, EVENT_QUEUE);
	    month = date_and_event(mar->husband);
	    month %= MAXUMONTHS;
	    install_in_order(mar->husband, event_queue + month, EVENT_QUEUE);
	    /** event scheduled */



	    }
	  }
	  mar=mar->wifes_prior;
	}
      }
    }
  }
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

double enhance_usedeathmult(p, crnt_block)
     struct person *p;
     struct age_block * crnt_block;
{
  
  if(p->dmult == 1){
    return(crnt_block->lambda);
  } else{


    return(p->dmult * crnt_block->lambda); 
    
  }
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

}

/******************************************************************/

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

}
/************************************************************************/
void enhance_write_extra(fd,p)
     FILE *fd;
     struct person *p;
{
  /*********************************************************
   *This gets called after write_extra has written p's stuff BUT before
   *the fput("\n") that is before the line end is written.
   *this will add stuff as required but NOT put the \n 
   *******************************************************************/

}
