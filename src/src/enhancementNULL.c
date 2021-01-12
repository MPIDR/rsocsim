#include "defs.h"
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

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
#define SIZE_OF_EXTRA 0

/** It's nice to declare functions here which are only called from
    this file -- as noted the enhanced_xxx hook functions need to be
    declared in defs.h
***/

void generate_extra();  /* a function that draws random values for a
			   person's extra struct */
		       

/*************************************************************************/
void enhance_defaults()
     /** gets called from events.c just after the rate_file (ie .sup) is
	 processed. This is when you make sure that the right things
	 happen regardless of the .sup file's contents
     **/
{
  /** Announce on screen and log that this is not your father's socsim**/
  logmsg("\n\nHEADS UP!! this is an enhanced socsim(%s)\n\n",
	 ENHANCEMENT_NAME,1);
  logmsg("**** %s  overiding defaults***\n",ENHANCEMENT_NAME,1);

  /** change config settings and announce ***/

  read_xtra_file = 0;
  logmsg("%s: read_xtra_file=0\n",ENHANCEMENT_NAME,1);
  size_of_extra = SIZE_OF_EXTRA;
  logmsg("%s: size_of_xtra is 5\n",ENHANCEMENT_NAME,1);
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

/************************************************************************/
int  enhance_check_spouse(p, suitor)
     struct person *p, *suitor;
{
     /* This gets called from marriage_allowable before the incest,agediff
	and whatever other checks are inflicted on suitors before marriage
	is permitted.  return 1=marriage is OK 0= permision denied*/
  return(1); /*default 1 = do NOT reject */

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
struct person *p, *spouse;

{
  /** This gets called after a successful marriage -- that is at the
      end of xevents:new_marriage() after a new marriage structure
      has been created and filled in
  **/
  new_marriage(p,spouse);  /* return; */  
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
  */
  age_score= score3(p,suitor);
  return(age_score);
}/* end of stub*/


/****************************************************************/

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
   return(usefertmult(p, crnt_block)); 
}

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

}

/******************************************************************/
int enhance_check_endogamy(p,suitor)
     struct person *p,*suitor;

     
{
     /*This is called in place of check_endogamy so if you are not messing
       with groups at the point where the working marriage queue is constructed
       then leave the call to check_endogamy alone*/

  
  return(check_endogamy(p,suitor));

}
/******************************************************************/


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
  
  return(check_incest(p1,p2));
}


