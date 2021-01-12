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


/***
    Enhancements for the Spring 2007 Demog 212 Socsim Easter Egg Hunt:

    Per the Cutler & Deaton paper,  this simulation will be set up to
    do the following:

    0) Income reduces mort bottom 5% LE at all ages 25% less than top 5%
       
    1) Education reduces mortality categorical variable not linear.

    
    2) Early Life factors: children of poor moms will have higher
       mortality BUT only late in life.

    3) Month of birth Oct - Dec .6 year longer e(50)

    4) Race Blacks, White -- continuous pigmentation children inherit

        kid<- (mom+dad)/2 + .5*(mom - dad)*ifelse(runif(1000)> .5, runif(1000), - runif(1000))
       
    5) BMI is Ushaped BUT mort falls with height for const. BMI


***/

#define INCOME 0
#define PIGMENTATION 1
#define EDUCATION 2
#define HEIGHT 3
#define MASS 4
#define DMULT 5
#define GENEROSITY 6
#define VIRTUE 7
#define TOBACCO 8
#define EXERCISE 9
#define SIZE_OF_EXTRA 10
#define MAXDMULT 2
#define MINDMULT .01

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
      has been created and filled in
  **/
  
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

    double logdmult;
    double mominc ;
      
  
    logdmult = log(p->dmult);
    mominc = (p->mother == NULL)?40000:get_extra(p->mother,INCOME);


    
    /* low mom's income increases mortality after 50 */
    if (mominc < 35635 ) {
      
            
      /*      logdmult += ((crnt_block ->upper_age /12) > 50 )? 0.1 : -0.06; */

      /* logdmult += ((crnt_block ->upper_age /12) > 50 &&
	 (crnt_block ->upper_age /12) < 75)? 0.2 : -0.06; */
      
      logdmult += ((crnt_block ->upper_age /12) > 55 )? 0.25 : -0.5; 

      return exp(logdmult) * crnt_block->lambda;
    
    } else {
    
      
      return(p->dmult * crnt_block->lambda); 
    
    }
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
  int ivalue;
  float rvalue;
  float  mominc;
  float momed;
  int  birthorder;
  float pigment, mpig, fpig;

  momed=(p->mother == NULL)?3:get_extra(p->mother,EDUCATION);
  mominc=(p->mother == NULL)?40000:get_extra(p->mother,INCOME);
  birthorder=(p->mother == NULL)?1:get_parity(p->mother);
  mpig = (p->mother == NULL)? rrandom() : get_extra(p->mother, PIGMENTATION);
  fpig = (p->father == NULL)? rrandom() : get_extra(p->father, PIGMENTATION);

  pigment = (mpig + fpig)/2 ;
  pigment += (rrandom() > .5)? .5*(mpig - fpig)*rrandom() :
    -.5*(mpig - fpig)*rrandom() ;
  
  put_extra(p,PIGMENTATION,pigment);
  /*  rvalue= exp(normal()) *500;
  rvalue= (rvalue > 5000)? 5000 : rvalue;
  */

  rvalue = exp(normal()*.4)*40000 + 5000;
  put_extra(p,INCOME,rvalue);

  /**
   quantile(inc, probs=c(.05,.25 , .75,.95))
         5%      25%      75%      95%
      25814.91 35634.84 57434.01 81717.25

  **/
  int inc_qtile = 0;
  inc_qtile = (rvalue < 25814) ? 0:
    (rvalue < 35635 ) ? 1:
    (rvalue < 57434 ) ? 2:
    (rvalue < 81717)  ? 3 : 4;
 

  /*rvalue=(rrandom() >.66)?1:0;*/


  rvalue=  1.86 + normal() * .33;
  rvalue= (rvalue < 1.5) ? 1.5 : rvalue;
  rvalue= (rvalue > 2.2) ? 2.2 : rvalue;
  put_extra(p,HEIGHT,rvalue);

  rvalue=rrandom();
  ivalue=(rvalue<.2) ? 1 :
    (rvalue < .3) ? 2 :
    (rvalue < .4) ? 3 :
    (rvalue < .7) ? 4 :
    (rvalue < .8) ? 5 :
    (rvalue < .93) ? 6 : 7;

  put_extra(p,EDUCATION,(float) ivalue);

  rvalue= 68 + normal()*8;
  put_extra(p,MASS,rvalue);


  /*
 p->dmult=  (get_extra(p,INCOME)==1)? mominc/1000: -1*mominc/1000;

  p->dmult=  (get_extra(p,PIETY)==1)? mominc/1000: -1*mominc/1000;
  */

  /*  p->dmult = p->dmult + (get_extra(p,EDUCATION) == 1)* .25 +
    (get_extra(p,EDUCATION) == 7)* (-.25);
  */



  p ->dmult = 1;
  /* own income: bottom 5% should have 25% higher mort than top 5%
     at all ages */

  p ->dmult *= (inc_qtile == 0)? 1.125 :
    (inc_qtile == 1)? 1.05 :
    (inc_qtile == 3)? .95  :
    (inc_qtile == 4)? .875 : 1.0;

  p ->dmult += get_extra(p,EDUCATION) == 1 ? .25 :
    get_extra(p,EDUCATION) >  5 ? -.25 : 0 ;

  /** effect of momed only occurs late in life -- non proportional
      doesn'tw ork in dmult  **/

  float bmi;
  bmi = get_extra(p,MASS)/(get_extra(p,HEIGHT)*get_extra(p,HEIGHT));

  p->dmult= (bmi < 18.5)? p->dmult - .25 : 
    (bmi < 24.9)? p->dmult :
    (bmi < 30  )? p->dmult + .15 : p->dmult + .5;
    
  /*
  p->dmult = (birthorder == 4)? p->dmult*1.5: p->dmult;
  */
  

  p->dmult += (pigment - .5)*.2 ;
    
  /**
     the usedeathmult function does not exponentiate (don't trust me on that
     things chage) but if dmult is to function as X*Beta in a Cox prop haz
     model, then it should be exponentiated before it gets used in 
     usedeathmult
  **/
  /* born between oct...dec -- good news */
  if (((current_month % 12) == 0) || ((current_month % 12) > 9)) {
    p->dmult *= .92;
  }

  /** center dmult near 1 **/
  p->dmult += -.1;

  

  rvalue=rrandom();
  ivalue = (rvalue<.2) ? 0 :
    (rvalue < .3) ? 0 :
    (rvalue < .4) ? 0 :
    (rvalue < .7) ? 1 :
    (rvalue < .8) ? 2 :
    (rvalue < .93)? 3 : 4;
  
  if(p->dmult > 2){
    ivalue = 3;
  }

  put_extra(p,TOBACCO, (float) ivalue);

  rvalue = rrandom();
  ivalue = (rvalue<.2) ? 0 :
    (rvalue < .3) ? 10 :
    (rvalue < .4) ? 20 :
    (rvalue < .7) ? 30 :
    (rvalue < .75) ? 60 :
    (rvalue < .93)? 90 : 120;

  put_extra(p,EXERCISE, (float) ivalue);

  rvalue= normal()*10 + p->dmult;
  put_extra(p,VIRTUE,rvalue);
  rvalue = fabs(log(p->person_id));
  put_extra(p,GENEROSITY,rvalue);

  
  p->dmult = exp(p->dmult);

  
  put_extra(p,DMULT,p->dmult);
}

/******************************************************************/

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
