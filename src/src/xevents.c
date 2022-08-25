/* %W% %G% */
//#include "defs.h"
#include <math.h>
#include <stdlib.h>

struct mqueue_w
{
  struct person *mq_person;
  double score;
  struct mqueue_w *next;
};

struct mqueue_w *mq_w = NULL;
int mq_count = 0;

struct transit_list
{
  struct person *migrant;
  struct transit_list *next;
};

struct transit_list *tl;
struct transit_list *tl0;

void new_marriage( struct person*, struct person*);
int marriage_allowable(struct person *,struct person *);
int check_agedif(struct person *, struct person *);
void create_working_mqueue(struct person*);
void destroy_working_mqueue();

void add_minor_children(struct person *);
int check_endogamy(struct person *, struct person *);
int check_incest(struct person *, struct person *);

void birth(struct person *p)
{
  struct person *child;
  int m;

  /* create child, set up pointers */
  child = NEW(struct person);
  child->extra = NULL; /* best to initialize to NULL */
  child->ltrans = NULL;
  child->dmult = 1.0;
  child->tmult = 1.0;
  child->person_id = ++last_person_id;
  last_person->down = child;
  last_person = child;
  child->down = NULL;

  child->sex = (rrandom() >= prop_males) ? FEMALE : MALE;

  child->birthdate = current_month;
  child->mother = p;
  child->father = NULL;
  /*but if there is a father (i.e., current husband, cohabitant).. */
  /*M_NULL as check for reson_end includes current cohabitations */
  if (p->last_marriage != NULL)
  {
    if (p->last_marriage->reason_end == M_NULL)
    {
      child->father = p->last_marriage->husband;
    }
  }

  /** Tue Oct  2 05:55:15 PDT 2012
	If child has no father by virtue of mother's marital/cohab state
	then IF randomdad is in effect we find him one **/

  if ((child->father == NULL) && (random_father == TRUE))
  {
    child->father = find_random_father(child);
  }
  /* Tue Oct 29 13:33:56 PST 2002
       determine group inheritance according to value of child_inherits_group
    */

  /* by default child get's mom's group */

  switch (child_inherits_group)
  {
  case FROM_MOTHER:
    child->group = p->group;
    break;
  case FROM_FATHER:
    child->group = (child->father != NULL) ? child->father->group : p->group;
    break;
  case FROM_SAME_SEX_PARENT:
    if (child->sex == MALE)
    {
      child->group = (child->father != NULL) ? child->father->group : p->group;
    }
    else
    {
      child->group = p->group;
    }
    break;
  case FROM_OPPOSITE_SEX_PARENT:
    if (child->sex == FEMALE)
    {
      child->group = (child->father != NULL) ? child->father->group : p->group;
    }
    else
    {
      child->group = p->group;
    }
    break;
  default:
    child->group = child_inherits_group;
  }

  child->e_sib_mom = p->lborn;
  child->e_sib_dad = (child->father == NULL) ? NULL : child->father->lborn;
  child->lborn = NULL;
  child->last_marriage = NULL;
  child->mstatus = SINGLE;
  child->deathdate = 0;
  if (child->sex == FEMALE)
  {
    child->fmult = alpha * p->fmult + (1 - alpha) * fertmult();
    child->fmult = 2.5 * exp(betaT * log(child->fmult / 2.5));
  }
  else
  {
    child->fmult = 0;
  }
  child->pointer_type[MARRIAGE_QUEUE] = PTR_NULL;
  child->NEXT_ON_MQUEUE = NULL;
  child->pref = random_spouse2;

  child->score = score3;
#ifdef ENHANCED
  child->score = enhance_couple_score;
#endif

  /* Here is where socio demographic variables for the new kid
       should be added.  don't malloc if you're not going to use it*/
  /*
    child->extra = NEW(struct extra_data);
    
    */
  /*
    printf("new person: id %d fmult %f\n", child->person_id, child->fmult);
    */
  /*also update the parent-to-child pointers, etc */
  p->lborn = child;
  /*
    p->egos_extra->parity++;
    */
  if (child->father != NULL)
  {
    child->father->lborn = child;
  }

#ifdef ENHANCED
  enhance_birth(p, child);

#endif

  /* event for the child */
  m = date_and_event(child);
  m %= MAXUMONTHS;
  install_in_order(child, event_queue + m, EVENT_QUEUE);

  /* update population tallies */
  size_of_pop[0]++;
  size_of_pop[child->group]++;
  /* new event for the mother */
  m = date_and_event(p);
  m %= MAXUMONTHS;
  install_in_order(p, event_queue + m, EVENT_QUEUE);

  crnt_month_events[E_BIRTH]++;

  if (marriage_queues == 1 && child->sex == MALE)
  {
    /*every unmarried male is on the marriage queue*/
    install_in_order(child, marriage_queue + child->sex, MARRIAGE_QUEUE);
    child->mqueue_month = current_month;
  }
}

/*************************************************************************/
void death(struct person *p)
{
  struct person *spouse;
  int m;

  /*
    printf("death for person %d\n", p->person_id);
    */
  size_of_pop[0]--;
  size_of_pop[p->group]--;
  p->deathdate = current_month;
  if (p->NEXT_ON_MQUEUE != NULL)
  {
    /* delete dead person from marriage queue */
    queue_delete(p, MARRIAGE_QUEUE);
  }
  /** update marriage records generate new events for all spouses
	12/1/09 this was only being done for last marriage and was
	resulting in dead spice being dequeued...segfault **/

  if (p->last_marriage != NULL)
  {
    /** follow marriage chain deque and re-event all spouses **/
    struct marriage *cmar;
    cmar = p->last_marriage;
    while (cmar != NULL)
    {
      /*if (p->last_marriage->reason_end == M_NULL) {*/
      if (cmar->reason_end == M_NULL)
      {
        if (p->sex == MALE)
        {
          /*spouse = p->last_marriage->wife;*/
          spouse = cmar->wife;
        }
        else
        {
          /*spouse = p->last_marriage->husband;*/
          spouse = cmar->husband;
        }

        /* p->last_marriage->date_end = current_month;*/
        cmar->date_end = current_month;
        /*delete dead person's spouse from event queue*/
        queue_delete(spouse, EVENT_QUEUE);

        if (p->mstatus == MARRIED)
        {
          /** heads up -- in polygyny this could be troublesome. could
		  ego cohab with one "spouse" and be married to another?
		  mstatus rules and that could be troublesome **/
          /*p->last_marriage->reason_end = DEATH;*/
          cmar->reason_end = DEATH;
          spouse->mstatus = WIDOWED;
          if (marriage_queues == 1 && spouse->sex == MALE)
          {
            /* every unmarried male is in the marraige queue*/
            install_in_order(spouse, marriage_queue + MALE, MARRIAGE_QUEUE);
            spouse->mqueue_month = current_month;
          }
        }
        else
        {
          /*p->last_marriage->reason_end = COCOHABDIES;*/
          cmar->reason_end = COCOHABDIES;

          /*
		spouse->mstatus = spouse->egos_extra->prev_marital_status;
	      */
          /**Mon Apr 19 09:01:25 PDT 2010 when deceased is a cohab,
		 co-cohab needs to have mstatus set to status before cohab
		 began.  This is complicated if there is polygamy especially 
		 marital + cohab simultaneously. **/

          spouse->mstatus = get_previous_mstatus(spouse);
        }

        m = date_and_event(spouse);
        m %= MAXUMONTHS;
        install_in_order(spouse, event_queue + m, EVENT_QUEUE);
        /*
	    printf("spouse: next event person %d at month %d\n",
		spouse->person_id, m);
	    */
      }
      cmar = (p->sex == MALE) ? cmar->husbands_prior : cmar->wifes_prior;
    }
  }
  crnt_month_events[E_DEATH]++;
  /*cfree(p);*/
#ifdef ENHANCED
  enhance_death(p);
#endif
}

void divorce(struct person *p)
{
  struct person *spouse;
  int m;

  /*
    printf("divorce for person %d\n", p->person_id);
    */

  if (p->sex == MALE)
  {
    spouse = p->last_marriage->wife;
  }
  else
  {
    spouse = p->last_marriage->husband;
  }

  p->last_marriage->date_end = current_month;
  queue_delete(spouse, EVENT_QUEUE);

  if (p->mstatus == MARRIED)
  {
    p->last_marriage->reason_end = DIVORCE;
    p->mstatus = DIVORCED;
    spouse->mstatus = DIVORCED;
  }
  else
  {
    p->last_marriage->reason_end = BREAKCOHAB;
    /*p->mstatus = p->egos_extra->prev_marital_status; 
	spouse->mstatus = spouse->egos_extra->prev_marital_status;
	*/
    p->mstatus = get_previous_mstatus(p);
    spouse->mstatus = get_previous_mstatus(spouse);
  }

  /*
    printf("delete divorcing person's spouse from event queue\n");
    */
  /*
		New events for divorcer and spouse
		*/
  m = date_and_event(p);
  m %= MAXUMONTHS;
  install_in_order(p, event_queue + m, EVENT_QUEUE);

  m = date_and_event(spouse);
  m %= MAXUMONTHS;
  install_in_order(spouse, event_queue + m, EVENT_QUEUE);
  /*
    printf("spouse: next event person %d at month %d\n",
	spouse->person_id, m);
    */
  crnt_month_events[E_DIVORCE]++;
  if (marriage_queues == 1)
  {
    /* if one-queues system male divorcee goes back on marriage queue*/
    struct person *malespouse;
    malespouse = p->sex == MALE ? p : spouse;
    install_in_order(malespouse, marriage_queue + MALE, MARRIAGE_QUEUE);
    malespouse->mqueue_month = current_month;
  }

#ifdef ENHANCED
  enhance_divorce(p, spouse);
#endif
}

void marriage(struct person *p)
{
  struct person *spouse;
  int m;

  /*
    printf("person id %d, sex %d group %d\n", p->person_id, p->sex, p->group);
    printf("about to create mqueue\n");
    */

  /**one marriage queue routine: unmarried males go straight to the
       queue and wait.  No search on the part of males; no waiting on
       the part of females... we hope BUT this means that marriage(p)
       should never be called if p is male**/

  if (marriage_queues == 1 && (p->sex == MALE))
  {
    /* error one-queue implies that this should not be called for males*/
    // logmsg("Error: marriage_queues==1 yet marriage() called for male person %d", //jimtom
    // p->person_id,1);
    stop("wefa marriage ");//exit(-1);
  }

  if (p->mstatus == COHABITING)
  {
    spouse = (p->sex == FEMALE) ? p->last_marriage->husband : p->last_marriage->wife;
    queue_delete(spouse, EVENT_QUEUE);
#ifdef ENHANCED
    enhance_new_marriage(p, spouse);
#else
    new_marriage(p, spouse);
#endif
  }
  else
  {
    /*not cohabiting so find a spouse */
    create_working_mqueue(p);

    /* p->pref is a function random_spouse2() */
    spouse = (*(p->pref))(p);

    //int mq_count_tmp = mq_count;
    destroy_working_mqueue();

    if (spouse != NULL)
    {

      /*
	    printf("doing a marriage\n");
	    */
      queue_delete(spouse, MARRIAGE_QUEUE);
      queue_delete(spouse, EVENT_QUEUE);
      /*doing this counting at end of month
	    time_waiting[spouse->sex] +=
		current_month - spouse->mqueue_month + 1;
	    */
      spouse->mqueue_month = 0;
#ifdef ENHANCED
      enhance_new_marriage(p, spouse);
#else
      new_marriage(p, spouse);
#endif

      crnt_month_events[E_MARRIAGE]++;
    }
    else
    {
      /* spouse not found Add person to marriage queue */

      if (p->NEXT_ON_MQUEUE == NULL)
      {
        /* a person having a failed marriage attempt *might* already
	       be on the queue in which case we don't do it again*/
        install_in_order(p, marriage_queue + p->sex, MARRIAGE_QUEUE);
        p->mqueue_month = current_month;
      }
      /* We should only put men on the queue if one-queue is on,
	     females just pass through with a noopt and get a new
	     event  we'll barf out a warning at the end if
	  there are women on the marriage queue and m_eval==distribution**/
      m = date_and_event(p);
      m %= MAXUMONTHS;
      install_in_order(p, event_queue + m, EVENT_QUEUE);
      /* crnt_month_events[E_MARRIAGE]++;*/
#ifdef ENHANCED
      enhance_marriage_queue(p);
#endif
    } /*spouse not found*/
  }   /*not cohabitting */
      /*
    printf("exiting marriage proc\n");
    */
}

/************************************************************************/
void new_marriage( struct person *p, struct person *spouse)
{

  struct marriage *mar;
  struct person *h, *w;
  double prob;
  int m;
  int cohab = FALSE;

  /*
    printf("going to the create a new marriage husband: %d, wife %d\n", 
	h->person_id, w->person_id);
    */

  /* legalize the cohabitiation */
  if (p->mstatus == COHABITING)
  {
    p->last_marriage->date_end = current_month;
    p->last_marriage->reason_end = MARAFTERCOHAB;
  }
  else if (cohab_probs[p->group][p->sex] != NULL)
  {
    prob = table_lookup(current_month - p->birthdate,
                        cohab_probs[p->group][p->sex]);
    /*
	printf("looking up cohab prob for %d age %d %lf\n",
	    p->person_id, current_month - p->birthdate, prob);
	*/
    if (rrandom() < prob)
      cohab = TRUE;
  }
  else if (cohab_probs[spouse->group][spouse->sex] != NULL)
  {
    prob = table_lookup(current_month - spouse->birthdate,
                        cohab_probs[spouse->group][spouse->sex]);
    /*
	printf("looking up cohab prob for spouse %d age %d %lf\n",
	    spouse->person_id, current_month - spouse->birthdate, prob);
	*/
    if (rrandom() < prob)
      cohab = TRUE;
  }

  if (p->sex == FEMALE)
  {
    h = spouse;
    w = p;
  }
  else
  {
    h = p;
    w = spouse;
  }

  mar = NEW(struct marriage);

  mar->down = NULL;
  if (last_marriage != NULL)
  {
    last_marriage->down = mar;
    mar->marr_id = last_marriage->marr_id + 1;
  }
  else
  {
    marriage0 = mar;
    mar->marr_id = 1;
  }

  last_marriage = mar;

  mar->husband = h;
  mar->wife = w;
  mar->husbands_prior = h->last_marriage;
  mar->wifes_prior = w->last_marriage;
  mar->date_start = current_month;
  mar->date_end = 0;
  mar->reason_end = M_NULL;

  h->last_marriage = mar;
  w->last_marriage = mar;

  if (cohab)
  {
    /*     see get_previous_mstatus() 
	h->egos_extra->prev_marital_status = h->mstatus;
	w->egos_extra->prev_marital_status = w->mstatus;
      */
    h->mstatus = COHABITING;
    w->mstatus = COHABITING;
  }
  else
  {
    h->mstatus = MARRIED;
    w->mstatus = MARRIED;
  }

  if (!cohab)
  {
    /*
	h->egos_extra->marity++;
	w->egos_extra->marity++;
      */
  }

  if (marriage_eval == DISTRIBUTION)
  {
    /**keep track of ages at marriage for pref calcluations
	 ignoring issue of mixed marriages. We'll just count them from her perspective
      **/
    int agediff = 0, i = 0;
    agediff = (int)(((current_month - h->birthdate) -
                     (current_month - w->birthdate)) /
                    12);
    i = MAXUYEARS + agediff;
    marriage_ageobs[w->group][i] = (marriage_ageobs[w->group][i]) + 1;
    marriage_tots[w->group] = (marriage_tots[w->group]) + 1;
    /** that's it for tracking marriage ages **/
  }

  m = date_and_event(h);
  m %= MAXUMONTHS;
  install_in_order(h, event_queue + m, EVENT_QUEUE);

  m = date_and_event(w);
  m %= MAXUMONTHS;
  install_in_order(w, event_queue + m, EVENT_QUEUE);

  /*
    printf("difference: %d\n", h->birthdate - w->birthdate); 
    fflush();
 
    */
}

/********************************************************************/

struct person * random_spouse2( struct person *p)
{
  /* pointed to by p->pref  and thus called from marriage() to
     produce a spouse  called after mq_w is created 

     NOTE that working marriage queue, mq_w  contains ONLY max scoring AND
     marriage_allowable() spouses SO this only needs to choose a random 
     spouse from among the optimal*/

  struct mqueue_w *mq;
  struct person *spouse;
  int nth;

  spouse = NULL;

  /*
    printf("mqueue_count %d \n", mq_count);
    */

  if (mq_count == 0)
  {
    return spouse; /** which is to say NULL **/
  }

  if (mq_count == 1)
  {
    /* only one person in marriage queue if not rejectable accept*/
    /*if (marriage_allowable(p, mq_w->mq_person)==1)  
	all on the mq_w are vetted and optimal*/
    return mq_w->mq_person;
  }

  if (mq_count > 0)
  {

    mq = mq_w;

    /* choose a random nth on the queue */
    nth = (int)((rrandom()-0.0000001) * mq_count);

    /* count to random n*/
    while (--nth >= 0)
    {
      mq = mq->next;
    }
    spouse = mq->mq_person;
  }

  return spouse;
}

/**********************************/

void create_working_mqueue(struct person *p)
/* called from marriage() just before p->pref() is invoked to find a
   spouse from the current marriage_queue of opposite sex potential
   spouses. This creates a linked list, called mq_w which is a GLOBAL
   variable containing a link list of potential marriage partners who
   HAVE BEEN SCREENED by marriage_allowable() AND who have the maximum
   scores from p->score() (typically score3)
   
   So p is indifferent between ALL who are on the working_queue.
   
   destroy_working_queue must be called to break down
   mq_w after it has served its purpose */

{

  struct queue_element *op;
  struct mqueue_w *mq, *mq0, *mq_max;
  struct person *current_person;
  double s;
  int i;
  i=0;
  /* create "working" mqueue */

  /*    printf("\nentering create working mqueue\n");  */

  /* choose appropriate marriage_queue one for each sex */

  op = marriage_queue + (1 - p->sex);
  mq_count = op->num;
  current_person = op->first;

  /* inspect the marriage_queue until a potential spouse who passes
    the incest tests in marriage_allowable() function is found. Then
    malloc a queue element and make that the head of the working
    marriage queue mq_w This used to disallow negative scoring spouses
    but not any more 
    Thu May 13 11:59:27 PDT 2010 if ((s =(*(p->score))(p, current_person)) >= 0) {
    */

   /* 
    tomt: this degrades the performance by a lot. In bigg populations, most of the time is spent here.
    change: do not check all males on the marriage-queue, but only a random block of maybe 1000 males.
    first to check whether it improves performance: the 500 first
  */

  while ((mq_count-- > 0) && (mq_w == NULL) && (i < 5000))
  {
    i++;
    if (marriage_allowable(p, current_person) == 1)
    {
      mq_w = NEW(struct mqueue_w);
      mq_w->mq_person = current_person;
      mq_w->score = (*(p->score))(p, current_person);
      mq_w->next = NULL;
    }
    /* current_person gets incremented even if the lucky winner
	 has been found  reflected in i=2 in next loop*/
    current_person = current_person->NEXT_ON_MQUEUE;
  }

  mq0 = mq_w;
  mq_max = mq_w;

  /** march through the rest of the marriage queue of potential
	spouses getting a score for each from the p->score() function;

	If a score is >= max  verify  compatibility and skip
	it if the new potential spouse is not marriage_allowable(). HOWEVER
	if the the new potential mate has a score equal to or > the
	current max, it goes on working queue. If the new pot-mate
	has a lower score than current max, then skip it.  Also make
	mq_max point to the new element if it's score justifies
	it **/

  /*
    printf("Ego age:%d\n",(current_month-p->birthdate)/12);
    if(mq_max != NULL){
    printf("First queuester id:%d age: %d score:%lf mq_count:%d\n",
	   mq_max->mq_person->person_id,
	   mq_max->score,
	   (current_month-mq_max->mq_person->birthdate)/12,
	   mq_count);
	   }*/
  /*for (i = 2; i <= mq_count; i++) {*/
  while ((mq_count-- > 0) && (i<3000000))
  {
    i++;
    if ((s = (*(p->score))(p, current_person)) >= mq_max->score)
    {
      /*printf("good s:%lf\n",s);*/
      /* current score is at least as good as  mq_max */
      if (marriage_allowable(p, current_person) == 1)
      {
        /** and marriage is allowable **/
        mq = NEW(struct mqueue_w);
        mq->mq_person = current_person;
        mq->score = s;
        /*printf("adding qster w score:%lf\n",s);*/
        if (s > mq_max->score)
        {
          mq_max = mq;
        }
        mq->next = NULL;
        mq0->next = mq;
        mq0 = mq;
      }
    }
    /*else{
	printf("not adding qster w/ s: %lf \n", s);
	}	*/
    current_person = current_person->NEXT_ON_MQUEUE;
  }

  /** rejigger the working queue so that it hold ONLY people with
	score=max are on it.  This means deleting all links above
	mq_max since mq_max and all links below it in the list have
	max score**/

  if (mq_max != NULL)
  {

    /* this depends on mq_w being sorted in ascending score order -which
	 should be the case b/c only better suitors are added to the queue*/
    mq0 = mq_w;
    while ((mq0 = mq_w) != mq_max)
    {
      mq_w = mq0->next;
      /* */ free(mq0);
    }
  }
  /* fix up count */
  mq_count = 0;
  mq0 = mq_w;
  while (mq0 != NULL)
  {
    /*printf("%d) age:%d  scr:%lf\n",
	     mq_count,
	     (current_month-mq0->mq_person->birthdate)/12,
	     mq0->score);*/
    mq0 = mq0->next;
    mq_count++;
  }

  /*    
      if(mq_max != NULL){
      printf("Max qster score:%lf (age:%d) qlength:%d\n",mq_max->score,
	     (current_month-mq_max->mq_person->birthdate)/12,
	     mq_count);
    }	
    */
  /*
      printf("exiting create_working_mqueue\n");
    */
}

void destroy_working_mqueue()
{

  struct mqueue_w *mq, *mq2;

  /* get rid of "working" mqueue */
  mq = mq_w;

  while (mq != NULL)
  {
    mq2 = mq;
    mq = mq->next;
    free(mq2);
  }
  /*
    */

  mq_w = NULL;
  mq_count = 0;
}

struct person * random_spouse(struct person *p)
{
  struct queue_element *op;
  struct person *spouse;

  op = marriage_queue + (1 - p->sex);

  if (op->num != 0)
  {
    int nth;
    spouse = op->first;
    nth = 1;
    if (op->num > 1)
    {
      nth = (int)(rrandom() * op->num);
      while (--nth >= 0)
      {
        spouse = spouse->NEXT_ON_MQUEUE;
      }
    }
  }
  else
  {
    spouse = NULL;
  }

  return spouse;
}
/****************************************************************************/

int marriage_allowable(struct person *p1,struct person *p2)
{
  /** This will call all of the functions that can disallow a union
      outright This includes incest, past marriage to same partner, age
      difference restrictions, and whatever else builds up over time

      this gets called also from random_father to disallow
      particularly loathsome matings.
  **/

  /*insert possible overall enhancement hook this can pre-empt all other checks*/
#ifdef ENHANCED
  if (enhance_check_spouse(p1, p2) == 0)
  {
    return (0); /*permission denied*/
  }
#endif

#ifdef ENHANCED
  if ((marriage_eval == DISTRIBUTION) || enhance_check_agedif(p1, p2) == 1)
  {
    if (enhance_check_endogamy(p1, p2) == 1)
    {
      if (enhance_check_incest(p1, p2) == 1)
      {
        return (1);
      }
    }
  }
#else
  if ((marriage_eval == DISTRIBUTION) || check_agedif(p1, p2) == 1)
  {
    /*skip check_agedif if DISTRIBUTION in effect*/
    if (check_endogamy(p1, p2) == 1)
    {
      if (check_incest(p1, p2) == 1)
      {
        return (1);
      }
    }
  }
#endif
  /** union is rejected **/
  return (0);
}

/********************************************************************/
int check_agedif(struct person *p1, struct person *p2)
{
  /** this just checks age difference husband - wife  accepts with 1
      if within bounds otherwiser rejects with 0 
  **/
  struct person *husb, *wife;

  if (p1->sex == MALE)
  {
    husb = p1;
    wife = p2;
  }
  else
  {
    wife = p1;
    husb = p2;
  }

  /* headsup younger ==> larger birthdate opp of age */
  int agedif;
  agedif = wife->birthdate - husb->birthdate;

  if ((agedif >= marriage_agedif_min) &&
      (agedif <= marriage_agedif_max))
  {
    /** acceptable **/
    return (1);
  }
  else
  {
    /**sorry**/
    return (0);
  }
}

/********************************************************************/
int check_incest(struct person *p1,struct person *p2)
{
  /** This function used to be called "check" but that was too stupid
      a name for a function now it does what the name implies. It is
      called from marriage_allowable() and returns 1 if the marriage
      IS allowable and 0 if it is NOT
  **/
  struct person *p1_side[6];
  struct person *p2_side[6];
  int i1, i2;

  if ((p1->mother == NULL) || (p2->mother == NULL))
  {
    return 1;
  }

  /* note that this system rejects parents, aunts, uncles,
   * siblings and first cousins */

  p1_side[FATHER] = p1->father;
  if (p1->father != NULL)
  {
    p1_side[FATHERS_MOTHER] = p1->father->mother;
    p1_side[FATHERS_FATHER] = p1->father->father;
  }
  else
  {
    p1_side[FATHERS_MOTHER] = NULL;
    p1_side[FATHERS_FATHER] = NULL;
  }
  p1_side[MOTHER] = p1->mother;
  p1_side[MOTHERS_MOTHER] = p1->mother->mother;
  p1_side[MOTHERS_FATHER] = p1->mother->father;

  p2_side[FATHER] = p2->father;
  if (p2->father != NULL)
  {
    p2_side[FATHERS_MOTHER] = p2->father->mother;
    p2_side[FATHERS_FATHER] = p2->father->father;
  }
  else
  {
    p2_side[FATHERS_MOTHER] = NULL;
    p2_side[FATHERS_FATHER] = NULL;
  }
  p2_side[MOTHER] = p2->mother;
  p2_side[MOTHERS_MOTHER] = p2->mother->mother;
  p2_side[MOTHERS_FATHER] = p2->mother->father;

  for (i1 = 0; i1 <= 5; i1++)
  {
    for (i2 = 0; i2 <= 5; i2++)
    {
      if ((p1_side[i1] == p2_side[i2]) && (p1_side[i1] != NULL))
      {
        /*
	  printf("rejecting this union %d %d\n",
	  p1->person_id, p2->person_id);
	*/
        return 0;
      }
    }
  }

  /*Wed May 12 15:13:27 PDT 2010
    let's also reject previously married couples */

  if (p1->last_marriage != NULL && p1->last_marriage != NULL)
  {
    struct marriage *lmar;
    struct person *other;

    if (p1->sex == MALE)
    {
      lmar = p1->last_marriage;
      other = p2;
    }
    else
    {
      lmar = p2->last_marriage;
      other = p1;
    }

    while (lmar != NULL)
    {

      if (lmar->wife == other)
      {
        return 0;
      }
      lmar = lmar->husbands_prior;
    }
  }
  return 1;
}

/**************************************************************************/

double score3(struct person *p, struct person *suitor)
    /** default marriage scoring routine. assigns a numerical score to the
    happy couple. p will ultimately choose the max scorer. Since the
    ordinal rank is all that matters, the user chooses the peak
    marriage age and the ratio of the slopes of two lines which
    descend there from. A 2 means that matches on the left of the peak
    (wife older than husband) decline in desirability twic as fast as
    age differences where the husband is older. 

    Negative numbers were once  tossed out in create_working_mqueue but no
    longer so negative values from this function are ok. Still for simplicity,
    let's try to keep them positive
**/


{
  struct person *w, *m;
  double x;
  //double bonus = 1;
  double maxS = 2;
  double rightSlope = .0005; /* oldest can man 
				marry newborn girl theoreticlly*/
  /*   read from .sup or defaults in events.c
       double marriage_peak_age = 36;
       double marriage_slope_ratio =2;
  */
  double score;

  if (p->sex == FEMALE)
  {
    w = p;
    m = suitor;
  }
  else
  {
    m = p;
    w = suitor;
  }
  /***There are TWO distinct marriage scoring algorithms here. The
	newer one triggered by marriage_eval == DISTRIBUTION
	mininimizes the diffs between observed and target
	distributions of age differences (groom-bride). The other,
	(PREFERENCE) scores based on a fixed preference scheme using
	lines and slopes and a peak and two age cutoffs. **/
  if (marriage_eval == DISTRIBUTION)
  {

    double result;
    int i = 0, agediff = 0;
    agediff = (int)(((current_month - m->birthdate) -
                     (current_month - w->birthdate)) /
                    12);
    i = MAXUYEARS + agediff; /*agediff can be >|< zero */
    /* result =(1.017*marriage_agetarget[w->group][i])- */
    result = (marriage_agetarget[w->group][i]) -
             (double)((double)marriage_ageobs[w->group][i]) /
                 ((double)(1 + marriage_tots[w->group]));
    /*
      printf("---agedif:%d  targ:%g obs:%g result:%g\n",
	     agediff,
	     marriage_agetarget[w->group][i],
	     marriage_ageobs[w->group][i],
	     result);
      */

    return (result);
  }
  else if (marriage_eval == PREFERENCE)
  {
    /* lower birthdate == older */
    x = (double)(w->birthdate - m->birthdate);

    if (x >= marriage_peak_age)
    {
      /*male older */
      score = maxS - rightSlope * ABS(x - marriage_peak_age);
    }
    else
    {
      /*female older */
      score = maxS - (rightSlope * marriage_slope_ratio) *
                         ABS(x - marriage_peak_age);
    }
    /*printf("agedif:%f  score:%f\n",x,score);*/
    /* a bonus if suitors have been married same number of times 
	 if(get_marity(w) == get_marity(p)) bonus=1.05;
	 return score * bonus;
      */

    return score;
    /*
       */
  } /*end of typical scoring algorithm*/
  stop("reached end of score3 without returning score... should be impossible ");
  return -1;
}

/********************************************************************/
int check_endogamy(struct person *p, struct person *suitor)
{
  /*
    Note that if the user set endogamy directive is no -1,0,1 this 
    function will call rrandom and thus it matters if this gets called
    extra times.  Potential mates should be checked once per marriage attempt
  */

  /** used to be called score2() this is called only from
      marriage_allowable() and it rejects those whose group status are
      not what they should be.  Like all functions called from
      marriage_allowable, this will return a 1 if the marriage is OK
      and a 0 if it is NOT **/

  /**Thu Apr 29 10:56:30 PDT 2010
     endogamy == 1 --> all marriages are to be within group
     endogamy == -1 --> all marriages are to be across groups
     endogamy == 0 --> marriages are random as far as groups go
     
     1> endogamy >0 --> prob/prop of ingroup suitors permitted
     0 > endogamy > -1  prob/prop of outgroup suitors permitted
  **/
  if (endogamy == 0)
  {
    /** skip this business entirely if endogamy=0 **/
    return (1); /*match is ok*/
  }

  if (endogamy == 1)
  {
    /* only ingroup marriages allowed*/
    if (p->group == suitor->group)
    {
      return (1); /*match ok*/
    }
    else
    {
      return (0); /*sorry*/
    }
  }

  if (endogamy == -1)
  {
    /*exogamy only*/
    if (p->group != suitor->group)
    {
      return (1); /*match ok*/
    }
    else
    {
      return (0); /*sorry*/
    }
  }

  if (endogamy > 0)
  {
    /* endogamy ~ proportion of marriages that are endogamous
       say yes to "endogamy" pct of endogamous matches
       and 1-"endogamy" pct of eXogamous matches
    */
    if ((p->group == suitor->group) &&
        (rrandom() <= endogamy))
    {
      return (1); /*match ok*/
    }
    else if ((p->group != suitor->group) &&
             (rrandom() <= (1 - endogamy)))
    {
      return (1); /*also ok*/
    }
    else
    {
      return (0); /*sorry*/
    }
  }

  if (endogamy < 0)
  {
    /* endogamy ~ proportion of marriages that are EXogamous
       say yes to "-endogamy" pct of EXogamous matches
       and 1-"-endogamy" pct of ENDogamous matches
    */
    float exogamy = -1 * endogamy;
    if ((p->group != suitor->group) &&
        (rrandom() <= (exogamy)))
    {
      return (1); /*match ok*/
    }
    else if ((p->group == suitor->group) &&
             (rrandom() <= (1 - exogamy)))
    {
      return (1); /*also ok*/
    }
    else
    {
      return (0); /*sorry*/
    }
  }

  logmsg("Reached the end of check_endogamy without return! This shouldn't happen\n", "", 1);
  stop("Reached the end of check_endogamy without return! This shouldn't happen");
  return(-1);
    
  /*
   */
}

double score1_OBSOLETE(struct person *p, struct person *suitor)
{
  struct person *w, *m;

  if (endogamy > 0)
  {
    if (p->group != suitor->group)
    {
      return -1;
    }
  }

  if (endogamy < 0)
  { /*exogamy preferred*/
    if (p->group == suitor->group)
    {
      return -1;
    }
  }

  /*
    return 0;
  */

  if (p->sex == FEMALE)
  {
    w = p;
    m = suitor;
  }
  else
  {
    m = p;
    w = suitor;
  }

  if (w->birthdate - m->birthdate > 60)
    return -1;
  if (m->birthdate - w->birthdate > 180)
    return -1;
  /*
    if (abs(w->egos_extra->marity - m->egos_extra->marity) > 0)
	return -1;
    if (w->egos_extra->marity - m->egos_extra->marity > 1)
	return -1;
    */

  if (m->birthdate - w->birthdate == 24)
    return 156;
  if (m->birthdate - w->birthdate > 24)
    return 180 - (m->birthdate - w->birthdate);
  if (m->birthdate - w->birthdate > 0)
    return 156 - (int)(m->birthdate - w->birthdate) * (13 / 7);

  return 156 - (int)(24 + w->birthdate - m->birthdate) * (13 / 7);
}

/*** execute transtion event **/
void transit(struct person *q)
{
  int destgroup, m;
  struct person *p;
  struct transition *txrec;
  /* create tl = list of household members who might migrate with ego
       ego is first person in tl does nothing unless hhtransit is set*/

  assemble_household(q);

  /*hook to possible enhancements BEFORE ego and or hh migrate*/
#ifdef ENHANCED
  enhance_transit_before(q, tl);
#endif

  /* printf("migration of %d \n", q->person_id);*/

  fflush(stdout);
  destgroup = q->next_event - NONTRANSIT;
  tl0 = tl;
  while (tl0 != NULL)
  {
    p = tl0->migrant;
    if (p->group != destgroup)
    {
      size_of_pop[p->group]--;
      size_of_pop[destgroup]++;

      /* Wed Jun 23 06:22:56 PDT 2010
	       recording transition event */

      txrec = NEW(struct transition);
      txrec->date = current_month;
      txrec->fromg = p->group;
      txrec->tog = destgroup;

      if (p->ltrans != NULL)
      {
        txrec->prior = p->ltrans;
      }
      else
      {
        txrec->prior = NULL;
      }
      p->ltrans = txrec;

      /** do the migration **/
      p->group = destgroup;

      /*
	      p->egos_extra->migration_date = current_month;
	    */
      /*
	    printf("%d migrates with %d\n", p->person_id, q->person_id);
	    */

      /* if on mqueue, take off; get new event */
      if (p->NEXT_ON_MQUEUE != NULL)
      {
        /*
		printf("transit:deleting %d marriage queue\n", p->person_id);
		fflush(stdout);
		*/
        queue_delete(p, MARRIAGE_QUEUE);
      }
      /* if not first migrant, then still on event queue,
			     * take off; get new event */
      /*
	    printf("transit:deleting %d from event queue\n", p->person_id);
	    */
      if (p != q)
        queue_delete(p, EVENT_QUEUE);
      m = date_and_event(p);
      m %= MAXUMONTHS;
      install_in_order(p, event_queue + m, EVENT_QUEUE);
    }

    tl = tl->next;
    /* */ free(tl0);
    tl0 = tl;
  }
  /*hook to possible enhancements AFTER ego and or hh migrate 
     AND after ego's HH (if hhtrans) have new events generated --
    ego's next event is not yet generated(?) BUT her group has been updated*/

  crnt_month_events[NONTRANSIT + destgroup]++;
#ifdef ENHANCED
  enhance_transit_after(q);
#endif
}

void assemble_household(struct person *p)
{
  struct marriage *p_prior;
  //struct person *current_child;

  /* start with self */
  tl0 = tl;

  /*
    while (tl0 != NULL) {
	tl = tl->next;
	cfree(tl0);
	tl0 = tl;
    }
    printf("assembling household of %d\n", p->person_id);
    */

  tl = NEW(struct transit_list);
  tl0 = tl;
  tl0->migrant = p;
  tl0->next = NULL;
  if (!hhmigration)
    return;
  /*
    printf("adding self %d %d\n", p->person_id, p->sex);
    */

  /* if migrant is female
		   * add current husband(s)
		   * and all minor children */
  if (p->sex == FEMALE)
  {
    p_prior = p->last_marriage;
    while (p_prior != NULL)
    {
      if (p_prior->reason_end == M_NULL)
      {
        tl0->next = NEW(struct transit_list);
        tl0 = tl0->next;
        tl0->migrant = p_prior->husband;
        tl0->next = NULL;
        /*
		printf("adding a husband %d\n",
		    p_prior->husband->person_id);
		*/
      }
      else
      {
        ;
        /*
		printf("marriage ended in death/divorce\n");
		*/
      }
      p_prior = p_prior->wifes_prior;
    }
    /*
	printf("adding egos minor children\n");
	*/
    add_minor_children(p);
  }
  else
  {
    /* if migrant is male
		       * add current wives
		       * and all minor, unmarried  children of current wives
		       * and all minor, unmarried  children of deceased wives
		       */
    p_prior = p->last_marriage;
    while (p_prior != NULL)
    {
      if (p_prior->reason_end == M_NULL)
      {
        tl0->next = NEW(struct transit_list);
        tl0 = tl0->next;
        tl0->migrant = p_prior->wife;
        tl0->next = NULL;
        /*
		printf("adding a current wife %d\n",
		    p_prior->wife->person_id);
		printf("and children of a current wife\n");
		*/
        add_minor_children(p_prior->wife);
      }
      /*
	    printf("and children of deceased partners\n");
	    */
      if (p_prior->reason_end == COCOHABDIES)
      {
        add_minor_children(p_prior->wife);
      }
      if (p_prior->reason_end == DEATH)
      {
        add_minor_children(p_prior->wife);
      }
      p_prior = p_prior->husbands_prior;
    }
  }
  /*
    print_migrants();
    */
}

void add_minor_children(struct person *p)
{
  int overage = FALSE;
  struct person *current_child;

  /*
    printf ("last child\n");
    */
  current_child = p->lborn;
  while ((current_child != NULL) && (overage == FALSE))
  {
    if (current_child->deathdate == 0)
    {
      if ((int)((current_child->birthdate - current_month) / 12) > 21)
      {
        overage = TRUE;
      }
      else
      {
        /*
		printf ("adding child\n");
		*/
        if (current_child->last_marriage == NULL)
        {
          tl0->next = NEW(struct transit_list);
          tl0 = tl0->next;
          tl0->migrant = current_child;
          tl0->next = NULL;
          /*
		    printf("adding minor unmarried child, via mother %d\n",
			current_child->person_id);
		    */
        }
      }
    }
    current_child = current_child->e_sib_mom;
  }
}

void print_migrants()
{
  tl0 = tl;
  while (tl0 != NULL)
  {
    printf("id of next migrant %d\n", tl0->migrant->person_id);
    tl0 = tl0->next;
  }
}
/****************************************************************/
/************************************************************************/
struct person * find_random_father(struct person *child)
{
  /** 
      called only from birth. If child has no father AND random_father
      is TRUE-- then we need to find one in order to give the kids
      some dna. Since all living people are on the event queue, we'll
      choose from there and take the first living male with age >=
      random_father_min_age.

      we will select a month at random and then check each person
      scheduled for an even in that month in random order. It turns
      out that the event_queue is stored in person_id order so we have
      to select at random from within the month.

      We'll call marriage_allowable to toss out incest and whatever else
      makes marriage inaddmissable EXCEPT present marital status
  **/

  int month, tries, rejects;
  struct queue_element *e;
  struct person *p, *random_father;

  struct possibleq
  {
    struct person *possible_father;
    struct possibleq *next;
  };

  struct possibleq *posq_ptr = NULL, *posq = NULL;
  int posq_count = 0;

  random_father = NULL;
  /*i = current_month % MAXUMONTHS;*/
  /* i is a random month */
  int nth;
  rejects = 0;
  month = (int)(rrandom() * (MAXUMONTHS - 1));

  for (tries = month; tries < (MAXUMONTHS + month); tries++)
  {

    /* e = event_queue + month;*/
    e = event_queue + (tries % MAXUMONTHS);
    nth = e->num;
    p = e->first;
    //char logstring[256];
    int marallowed = 0;
    /*It turns out that the event queue is stored in person_id order so it is
      necessary to randomize within months.  So this means that we are going
      to need to collect all of the qualifying fathers and then choose one at 
      random*/

    while (p != NULL && nth > 0)
    {

      /*      logmsg("person %d in quest for random dad\n",p->person_id,1);
       p is the person selected, should be alive */

      if ((p->sex == MALE) &&
          ((double)((current_month - p->birthdate) / 12) >=
           random_father_min_age) &&
          (marallowed = marriage_allowable(p, child->mother)) &&
          p != child)
      {

        /*
	  sprintf (logstring,"startmo: %d rejects:%d rand_dad: %d\n",month,
		 rejects,
		 p->person_id);
	logmsg("%s\n",logstring,1);
	*/
        /*return(p);  NOPE not so fast*/

        if (posq == NULL)
        {
          /* there is at least one person on the event q in the selected month
	     so we must build a list of possibles*/
          posq = NEW(struct possibleq);
          posq_ptr = posq;
          posq_ptr->possible_father = p;
        }
        else
        {
          /* the posq is already started so add a body */

          posq_ptr->next = NEW(struct possibleq);
          posq_ptr = posq_ptr->next;
          posq_ptr->possible_father = p;
        }
        posq_count++;
      }
      /* inspect rejects?  
      if(p->sex == MALE){
	sprintf(logstring,
	"chld:%d mom:%d p::id:%d sex:%d age:%f3.2 allowed:%d e_check:%d incest:%d endg:%d CHKadif:%d adiff:%d momage:%f3.2 dod:%d",
	child->person_id,
	child->mother->person_id,
	p->person_id, 
	p->sex,((float)(current_month - p->birthdate))/12,
	marriage_allowable(p,child->mother),
	enhance_check_spouse(p,child->mother),
	enhance_check_incest(p,child->mother),
	check_endogamy(p,child->mother),
	enhance_check_agedif(p,child->mother),
	(p->birthdate - child->mother->birthdate),
	((float)(current_month - child->mother->birthdate))/12,
	p->deathdate);

	logmsg("%s\n",logstring,1);
	}
      */

      (p->sex == MALE) && (rejects++);
      p = p->NEXT_PERSON;
      nth--;

    } /*loop over persons within month*/

    /* if we have at least one acceptable dad from this months eventq 
       pick one to return  */
    int select;
    select = posq_count - (int)(rrandom() * (posq_count - 1));
    posq_ptr = posq;
    /*
    if (posq_count >0){
      sprintf(logstring,"\n about to grab number %d from among %d qaulified",
	      select,posq_count);
      logmsg("%s\n",logstring,1);
      }*/
    while (posq_count > 0)
    {
      if (posq_count == select)
      {
        /* take a random random dad (#select from the end)*/
        random_father = posq_ptr->possible_father;
      }
      /* retun posq to the heap*/
      posq_ptr = posq->next;
      free(posq);
      posq = posq_ptr;
      posq_count--;
    }

    if (random_father != NULL)
    {
      return (random_father);
    }

  } /*month loop*/

  /* if we are here, then fathers are very scarce as in no living males
     over random_father_min_age */
  char logstring[256];
  sprintf(logstring, "\n month:%d child:%d No qualified father found in %d attempts \n",
          current_month, child->person_id, rejects);
  logmsg("%s\n", logstring, 1);
  return (NULL);
}
