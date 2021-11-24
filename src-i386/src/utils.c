/* %W% %G% */
//#include "defs.h"
#include <strings.h>
#include <stdio.h>
#include <sys/file.h>
#include <math.h>
#include <stdlib.h>

/*
Don't use:
#define AGE(x) current_month - (x)->birthdate + 1
This is because census is set before the current month.
Age for the purpose of the census is before the month begins.
This way, children born on Jan 1 are still under 18. Also,
children born in December are only one month old.
*/
#define AGE(x) current_month - (x)->birthdate
#define INRANGE(x) ((AGE((x)) >= 840) && (AGE((x)) < 1020)) ? 1 : 0
#define CHILD(x) (AGE((x)) < 216)
#define ALIVE(x) ((x)->deathdate == 0) ? 1 : 0

#define CHECK(x) (INRANGE((x)) && ALIVE((x))) ? 1 : 0
#define ISCHILD(x) (CHILD((x)) && ALIVE((x))) ? 1 : 0


void fill_in_rates(double*,struct age_block*);

int get_expected_number_of_births(int group)
{

	int count[MAXUMONTHS][MAXPARITY][NUMMARSTATUS];
	double rate_table[MAXUMONTHS];
	double num_births = 0;
	struct person *pp;
	int age;
	int a, p, m;

	pp = person0;

	for (a = 1; a < MAXUMONTHS; a++)
		for (p = 1; p < MAXPARITY; p++)
			for (m = SINGLE; m < NUMMARSTATUS; m++)
				count[a][p][m] = 0;

	while (pp != NULL)
	{
		if (pp->deathdate == 0)
		{
			if (pp->sex == FEMALE)
			{
				if (pp->group == group)
				{
					age = current_month - pp->birthdate;
					count[age][get_parity(pp)][pp->mstatus]++;
				}
			}
		}
		pp = pp->down;
	}

	printf("expected number of births for group %d\n", group);
	for (p = 1; p < MAXPARITY; p++)
	{
		for (m = SINGLE; m < NUMMARSTATUS; m++)
		{
			fill_in_rates(rate_table, birth_rate_set[group][m][p]);
			for (a = 1; a < MAXUMONTHS; a++)
			{
				num_births += rate_table[a] * count[a][p][m];
				/*
		printf("rate tale %lf count %d\n",
			rate_table[a], count[a][p][m]);
		*/
			}
		}
	}

	num_births = num_births * duration_of_segment;
	printf("expected number of births %d \n", (int)num_births);
	return (int)num_births;
}

int	get_expected_number_of_transits(int from, int dest)
{

	int count[MAXUMONTHS][NUMSEXES][NUMMARSTATUS];
	double rate_table[MAXUMONTHS];
	double num_transits = 0;
	struct person *pp;
	int age;
	int a, s, m, i;

	pp = person0;

	for (a = 1; a < MAXUMONTHS; a++)
		for (s = 1; s < NUMSEXES; s++)
			for (m = SINGLE; m < NUMMARSTATUS; m++)
				count[a][s][m] = 0;

	while (pp != NULL)
	{
		if (pp->deathdate == 0)
		{
			if (pp->group == from)
			{
				age = current_month - pp->birthdate;
				for (i = 0;
					 (i < duration_of_segment) && (age + i < MAXUMONTHS ); i++)//todo: i<duration_of_segment, is never used. replaced , with &&
				{
					count[age + i][pp->sex][pp->mstatus]++;
				}
			}
		}
		pp = pp->down;
	}

	printf("computing expected number of migrations from group %d dest %d\n",
		   from, dest);
	for (s = MALE; s < NUMSEXES; s++)
	{
		for (m = SINGLE; m < NUMMARSTATUS; m++)
		{
			fill_in_rates(rate_table, rate_set[from][NONTRANSIT + dest][s][m]);
			for (a = 1; a < MAXUMONTHS; a++)
			{
				num_transits += rate_table[a] * count[a][s][m];
			}
		}
	}

	printf("expected number of transits %d \n", (int)num_transits);
	return (int)num_transits;
}

void fill_in_rates( double *rate_table,struct age_block *rate_set)
{
	struct age_block *current_block;
	int i;
	current_block = rate_set;

	for (i = 0; i < MAXUMONTHS; i++)
		*(rate_table + i) = 0;

	i = 0;
	while (current_block != NULL)
	{
		for (; i <= current_block->upper_age; i++)
		{
			*(rate_table + i) = -current_block->lambda;
		}
		current_block = current_block->next;
	}
}

double	ident(struct person *p, struct age_block *crnt_block)
{
	return crnt_block->lambda;
}

double twofold( struct person *p, struct age_block *crnt_block)
{
	return crnt_block->lambda * 2;
}

double usefertmult(struct person *p,struct age_block *crnt_block)
{
	if (!hetfert)
		return crnt_block->lambda;
	else
		return p->fmult * crnt_block->lambda;
}

/***
 ** this is used in fill_rate_gaps to produce null rate
 ** sets for cases where it makes sense to have no rates
 ** eg  simulations with no male mort rates are ok if there are
 ** no males
 ***/

double usenullmult(struct person *p, struct age_block *crnt_block)
{
	return crnt_block->lambda;
}

/******************************************************************/

double usedeathmult(struct person *p,struct age_block *crnt_block)
{
	/** 
      Just like usefertmult this will be called from modify_rates and
      will affect mort rates if the person has a dmult. There is no
      standard death multiplier as there is an hfert this is just here
      to make it easier to add it in. By def p->dmult should =1.

      probably want to exponentiat dmult when it is generated
      dmult is like XBeta in a cox prophaz model
  **/

	if (p->dmult == 1)
	{
		return (crnt_block->lambda);
	}
	else
	{

		return (p->dmult * crnt_block->lambda);
	}
}

/******************************************************************/

double usetransmult(struct person *p,struct age_block *crnt_block)
{
	/**    if ( p->egos_extra->tmult == NULL) 
	 should equate to (but compile better) carlm 7/25/00
	 
	 Fri Jul 19 16:03:20 PDT 2002 
	 this function seems goofy -- if ! hettrans, we ought to
	 return ** before we look for tmult. In running as set
	 this will croak unless there is a tmult read in.

    if (! p->egos_extra->tmult )
       perror("attempt to modify transit rate no tmult found");
    if (!hettrans)
	return crnt_block->lambda;
    else
	return p->egos_extra->tmult * crnt_block->lambda;
}

Mon Sep 28 06:33:50 PDT 2009 hettrans is read from .sup in load.c but
the standard version of socism just puts a 1.0 in tmult. tmult exists
as part of the person record but it is there so that enhancement.c can
fiddle it.   SO -- to make transition multipliers work you have to
put hettrans=x where x!=0 in the .sup and you have to make sure that
p->tmult is set before it is used.  the value of hettrans is read from
.sup and is global.


  ***/
	if (!hettrans)
		return crnt_block->lambda;

	/*  if (! p->egos_extra->tmult )*/
	if (!p->tmult)
		perror("attempt to modify transit rate no tmult found");
	else
		return p->tmult * crnt_block->lambda;
}

/*
double
lookup_cohab_prob(p)
struct person *p;
{
    struct age_table *crnt_table;

    crnt_table = cohab_probs[p->group][p->sex];
    if (crnt_table== NULL) {
	printf("current month %d\n", current_month);
	perror("null table with %d %d\n", p->group, p->sex);
    }
    while (crnt_table->upper_age < (current_month - p->birthdate))
	crnt_table = crnt_table->next;
    return crnt_table->prob;
}
*/

double table_lookup( int age, struct age_table *crnt_table)
{
	if (crnt_table == NULL)
	{
		printf("current month %d\n", current_month);
		perror("null table");
	}
	while (crnt_table->upper_age < age)
		crnt_table = crnt_table->next;
	return crnt_table->prob;
}

void census()
{
	struct person *p, *spouse, *mother, *father;
	struct person *stepmother, *stepfather;
	struct person *mbubbe, *mzayde, *pbubbe, *pzayde;
	struct person *stepmbubbe, *stepmzayde, *steppbubbe, *steppzayde;
	struct marriage *m;
	FILE *fd_tmp;
	int fy, i;

	char tmp_name[100];

	fy = firstyear + (int)(current_month - 1 - last_event_date) / 12;
	sprintf(tmp_name, "output.%d", fy);

	/*
    if ((fd_tmp =  fopen(tmp_name, "w")) == NULL) {
	    perror("can't open file: tmp_name file\n") ;
    }
    */

	for (i = 0; i < A_NUMCAT; i++)
	{
		a_tally[0][i] = 0;
		a_tally[1][i] = 0;
	}

	/*printf("in census %d\n", current_month);*/

	p = person0;
	for (p = person0; p != NULL; p = p->down)
	{
		if (p->deathdate != 0)
		{
			continue;
		}
		if (CHECK(p))
		{
			/*
	    fprintf(fd_tmp,"%d %d %d self\n", p->person_id, p->sex, AGE(p));
	    */
			a_tally[p->sex][A_SELF]++;
		}

		if ((m = p->last_marriage) != NULL)
		{
			if (m->reason_end == M_NULL)
			{
				if (p->sex == FEMALE)
					spouse = m->husband;
				else
					spouse = m->wife;
				if (CHECK(spouse))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d spou %d\n",
			spouse->person_id, spouse->sex,
			AGE(spouse), p->person_id);
		    */
					a_tally[spouse->sex][A_SPOU]++;
				}
			}
		}

		mother = p->mother;
		father = p->father;

		if (p->mother == NULL)
		{
			mbubbe = NULL;
			mzayde = NULL;
		}
		else
		{
			mbubbe = p->mother->mother;
			mzayde = p->mother->father;
		}
		if (p->father == NULL)
		{
			pbubbe = NULL;
			pzayde = NULL;
		}
		else
		{
			pbubbe = p->father->mother;
			pzayde = p->father->father;
		}

		if (mother != NULL)
		{
			/*
	    printf("mother\n");
	    fflush(stdout);
	    */
			if (CHECK(mother))
			{
				/*
		fprintf(fd_tmp,"%d 1 %d bkid %d\n",
		    mother->person_id, AGE(mother), p->person_id);
		*/
				a_tally[1][A_BKID]++;
			}
		}
		if (father != NULL)
		{
			/*
	    printf("father\n");
	    fflush(stdout);
	    */
			if (CHECK(father))
			{
				/*
		fprintf(fd_tmp,"%d 0 %d bkid %d\n",
		    father->person_id, AGE(father), p->person_id);
		*/
				a_tally[0][A_BKID]++;
			}
		}

		if (mbubbe != NULL)
		{
			/*
	    printf("mbubbe\n");
	    fflush(stdout);
	    */
			if (CHECK(mbubbe))
			{
				/*
		fprintf(fd_tmp,"%d 1 %d bdaubkid %d\n",
		    mbubbe->person_id, AGE(mbubbe), p->person_id);
		*/
				a_tally[1][A_BDAUBKID]++;
			}
		}
		if (mzayde != NULL)
		{
			/*
	    printf("mzayde\n");
	    fflush(stdout);
	    */
			if (CHECK(mzayde))
			{
				/*
		fprintf(fd_tmp,"%d 0 %d bdaubkid %d\n",
		    mzayde->person_id, AGE(mzayde), p->person_id);
		*/
				a_tally[0][A_BDAUBKID]++;
			}
		}

		if (pbubbe != NULL)
		{
			/*
	    printf("pbubbe\n");
	    fflush(stdout);
	    */
			if (CHECK(pbubbe))
			{
				/*
		fprintf(fd_tmp,"%d 1 %d bsonbkid %d\n",
		    pbubbe->person_id, AGE(pbubbe), p->person_id);
		*/
				a_tally[1][A_BSONBKID]++;
			}
		}
		if (pzayde != NULL)
		{
			/*
	    printf("pzayde\n");
	    fflush(stdout);
	    */
			if (CHECK(pzayde))
			{
				/*
		fprintf(fd_tmp,"%d 0 %d bsonbkid %d\n",
		    pzayde->person_id, AGE(pzayde), p->person_id);
		*/
				a_tally[0][A_BSONBKID]++;
			}
		}

		if (mother != NULL)
		{
			if (mother->last_marriage != NULL)
			{
				if ((stepfather = mother->last_marriage->husband) != father)
				{
					if (CHECK(stepfather))
					{
						/*
			printf("stepfather\n");
			fflush(stdout);
			*/
						/*
			fprintf(fd_tmp,"%d 0 %d skid %d\n",
			    stepfather->person_id,
			    AGE(stepfather), p->person_id);
			*/
						a_tally[0][A_SKID]++;
					}

					if ((stepfather->mother != NULL) && (CHECK(stepfather->mother)))
					{
						/*
			printf("stepfather mother\n");
			fflush(stdout);
			*/
						/*
			fprintf(fd_tmp,"%d 1 %d bsonskid %d\n",
			    stepfather->mother->person_id,
			    AGE(stepfather->mother),
			    p->person_id);
			*/
						a_tally[1][A_BSONSKID]++;
					}
					if ((stepfather->father != NULL) &&
						(CHECK(stepfather->father)))
					{
						/*
			printf("stepfather father\n");
			fflush(stdout);
			*/
						/*
			fprintf(fd_tmp,"%d 0 %d bsonskid %d\n",
			    stepfather->father->person_id,
			    AGE(stepfather->father),
			    p->person_id);
			*/
						a_tally[0][A_BSONSKID]++;
					}
				}
			}
		}

		if (father != NULL)
		{
			if ((stepmother = father->last_marriage->wife) != mother)
			{
				if (CHECK(stepmother))
				{
					/*
		    printf("stepmother\n");
		    fflush(stdout);
		    */
					/*
		    fprintf(fd_tmp,"%d 1 %d skid %d\n",
			stepmother->person_id,
			AGE(stepmother), p->person_id);
		    */
					a_tally[1][A_SKID]++;
				}

				if ((stepmother->mother != NULL) && (CHECK(stepmother->mother)))
				{
					/*
		    printf("stepmother mother\n");
		    fflush(stdout);
		    */
					/*
		    fprintf(fd_tmp,"%d 1 %d bdauskid %d\n",
			stepmother->mother->person_id,
			AGE(stepmother->mother), p->person_id);
		    */
					a_tally[1][A_BDAUSKID]++;
				}

				if ((stepmother->father != NULL) &&
					(CHECK(stepmother->father)))
				{
					/*
		    printf("stepmother father\n");
		    fflush(stdout);
		    */
					/*
		    fprintf(fd_tmp,"%d 0 %d bdauskid %d\n",
			stepmother->father->person_id,
			AGE(stepmother->father), p->person_id);
		    */
					a_tally[0][A_BDAUSKID]++;
				}
			}
		}

		if (mbubbe != NULL)
		{
			if (mbubbe->last_marriage != NULL)
			{
				if ((stepmzayde = mbubbe->last_marriage->husband) != mzayde)
				{
					if (CHECK(stepmzayde))
					{
						/*
			printf("stepmzayde\n");
			fflush(stdout);
			*/
						/*
			fprintf(fd_tmp,"%d 0 %d sdaubkid %d\n",
			    stepmzayde->person_id,
			    AGE(stepmzayde), p->person_id);
			*/
						a_tally[0][A_SDAUBKID]++;
					}
				}
			}
		}

		if (mzayde != NULL)
		{
			if ((stepmbubbe = mzayde->last_marriage->wife) != mbubbe)
			{
				if (CHECK(stepmbubbe))
				{
					/*
		    printf("stepmbubbe\n");
		    fflush(stdout);
		    */
					/*
		    fprintf(fd_tmp,"%d 1 %d sdaubkid %d\n",
			stepmbubbe->person_id,
			AGE(stepmbubbe), p->person_id);
		    */
					a_tally[1][A_SDAUBKID]++;
				}
			}
		}

		if (pbubbe != NULL)
		{
			if (pbubbe->last_marriage != NULL)
			{
				if ((steppzayde = pbubbe->last_marriage->husband) != pzayde)
				{
					if (CHECK(steppzayde))
					{
						/*
			printf("steppzayde\n");
			fflush(stdout);
			*/
						/*
			fprintf(fd_tmp,"%d 0 %d ssonbkid %d\n",
			    steppzayde->person_id,
			    AGE(steppzayde), p->person_id);
			*/
						a_tally[0][A_SSONBKID]++;
					}
				}
			}
		}

		if (pzayde != NULL)
		{
			if ((steppbubbe = pzayde->last_marriage->wife) != pbubbe)
			{
				if (CHECK(steppbubbe))
				{
					/*
		    printf("steppbubbe\n");
		    fflush(stdout);
		    */
					/*
		    fprintf(fd_tmp,"%d 1 %d ssonbkid %d\n",
			steppbubbe->person_id,
			AGE(steppbubbe), p->person_id);
		    */
					a_tally[1][A_SSONBKID]++;
				}
			}
		}
	}
	/*
    fclose(fd_tmp);
    */
}

void child_census()
{
	struct person *p, *spouse, *mother, *father, *sibling;
	struct person *stepmother, *stepfather;
	struct person *mbubbe, *mzayde, *pbubbe, *pzayde;
	struct person *stepmbubbe, *stepmzayde, *steppbubbe, *steppzayde;
	struct person *auntuncle, *greatauntuncle;
	struct marriage *m;
	FILE *fd_tmp;
	int i, fy, sex;
	char tmp_name[100];

	fy = firstyear + (int)(current_month - 1 - last_event_date) / 12;
	sprintf(tmp_name, "childoutput.%d", fy);
	/*
    if ((fd_tmp =  fopen(tmp_name, "w")) == NULL) {
	    perror("can't open file: tmp_name file\n") ;
    }
    */

	for (i = 0; i < C_NUMCAT; i++)
	{
		c_tally[0][i] = 0;
		c_tally[1][i] = 0;
	}

	/* printf("in child census %d\n", current_month); */

	p = person0;
	for (p = person0; p != NULL; p = p->down)
	{
		if (!ISCHILD(p))
			continue;
		sex = p->sex;
		/*
	fprintf(fd_tmp,"%d %d %d self\n", p->person_id, sex, AGE(p));
	*/
		c_tally[sex][C_SELF]++;

		if ((m = p->last_marriage) != NULL)
		{
			if (m->reason_end == M_NULL)
			{
				if (sex == FEMALE)
					spouse = m->husband;
				else
					spouse = m->wife;
				if (ALIVE(spouse))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d spou %d\n",
			spouse->person_id, sex, AGE(spouse), p->person_id);
		    */
					c_tally[sex][C_SPOU]++;
				}
			}
		}
		mother = p->mother;
		father = p->father;

		if (p->mother == NULL)
		{
			mbubbe = NULL;
			mzayde = NULL;
		}
		else
		{
			mbubbe = p->mother->mother;
			mzayde = p->mother->father;
		}
		if (p->father == NULL)
		{
			pbubbe = NULL;
			pzayde = NULL;
		}
		else
		{
			pbubbe = p->father->mother;
			pzayde = p->father->father;
		}

		if (mother != NULL)
		{
			if (ALIVE(mother))
			{
				/*
		fprintf(fd_tmp,"%d %d %d bmom %d\n",
		    mother->person_id, sex, AGE(mother), p->person_id);
		*/
				c_tally[sex][C_BMOM]++;
			}
		}
		if (father != NULL)
		{
			if (ALIVE(father))
			{
				/*
		fprintf(fd_tmp,"%d %d %d bdad %d\n",
		    father->person_id, sex, AGE(father), p->person_id);
		*/
				c_tally[sex][C_BDAD]++;
			}
		}

		if (mbubbe != NULL)
		{
			if (ALIVE(mbubbe))
			{
				/*
		fprintf(fd_tmp,"%d %d %d bmombmom %d\n",
		    mbubbe->person_id, sex, AGE(mbubbe), p->person_id);
		*/
				c_tally[sex][C_BMOMBMOM]++;
			}

			if (mbubbe->mother != NULL)
			{
				if (ALIVE(mbubbe->mother))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmombmombmom %d\n",
			mbubbe->mother->person_id, sex,
			AGE(mbubbe->mother), p->person_id);
		    */
					c_tally[sex][C_BMOMBMOMBMOM]++;
				}
			}
			if (mbubbe->father != NULL)
			{
				if (ALIVE(mbubbe->father))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmombmombdad %d\n",
			mbubbe->father->person_id, sex,
			AGE(mbubbe->father), p->person_id);
		    */
					c_tally[sex][C_BMOMBMOMBDAD]++;
				}

				if (mbubbe->mother != NULL)
				{
					greatauntuncle = mbubbe->father->lborn;
					while (greatauntuncle != NULL)
					{
						if ((ALIVE(greatauntuncle)) &&
							(greatauntuncle != mbubbe))
						{
							if (greatauntuncle->mother == mbubbe->mother)
							{
								/*
				fprintf(fd_tmp,"%d %d %d bmombmombparbkid %d\n",
				    greatauntuncle->person_id, sex,
				    AGE(greatauntuncle), p->person_id);
				*/
								c_tally[sex][C_BMOMBMOMBPARBKID]++;
							}
						}
						greatauntuncle = greatauntuncle->e_sib_dad;
					}
				}
			}
		}
		if (mzayde != NULL)
		{
			if (ALIVE(mzayde))
			{
				/*
		fprintf(fd_tmp,"%d %d %d bmombdad %d\n",
		    mzayde->person_id, sex, AGE(mzayde), p->person_id);
		*/
				c_tally[sex][C_BMOMBDAD]++;
			}

			auntuncle = mzayde->lborn;
			while (auntuncle != NULL)
			{
				if ((ALIVE(auntuncle)) && (auntuncle != mother))
				{
					if (auntuncle->mother == mbubbe)
					{
						/*
			fprintf(fd_tmp,"%d %d %d bmombparbkid %d\n",
			    auntuncle->person_id, sex,
			    AGE(auntuncle), p->person_id);
			*/
						c_tally[sex][C_BMOMBPARBKID]++;
					}
					else
					{
						/*
			fprintf(fd_tmp,"%d %d %d bmombdadbkid %d\n",
			    auntuncle->person_id, sex,
			    AGE(auntuncle), p->person_id);
			*/
						c_tally[sex][C_BMOMBDADBKID]++;
					}
				}
				auntuncle = auntuncle->e_sib_dad;
			}

			if (mzayde->mother != NULL)
			{
				if (ALIVE(mzayde->mother))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmombdadbmom %d\n",
			mzayde->mother->person_id, sex,
			AGE(mzayde->mother), p->person_id);
		    */
					c_tally[sex][C_BMOMBDADBMOM]++;
				}
			}
			if (mzayde->father != NULL)
			{
				if (ALIVE(mzayde->father))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmombdadbdad %d\n",
			mzayde->father->person_id, sex,
			AGE(mzayde->father), p->person_id);
		    */
					c_tally[sex][C_BMOMBDADBDAD]++;
				}

				if (mzayde->mother != NULL)
				{
					greatauntuncle = mzayde->father->lborn;
					while (greatauntuncle != NULL)
					{
						if ((ALIVE(greatauntuncle)) &&
							(greatauntuncle != mzayde))
						{
							if (greatauntuncle->mother == mzayde->mother)
							{
								/*
				fprintf(fd_tmp,"%d %d %d bmombdadbparbkid %d\n",
				    greatauntuncle->person_id, sex,
				    AGE(greatauntuncle), p->person_id);
				*/
								c_tally[sex][C_BMOMBDADBPARBKID]++;
							}
						}
						greatauntuncle = greatauntuncle->e_sib_dad;
					}
				}
			}
		}

		if ((mzayde == NULL) && (mbubbe != NULL))
		{
			auntuncle = mbubbe->lborn;
			while (auntuncle != NULL)
			{
				if ((ALIVE(auntuncle)) && (auntuncle != mother))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmombmombkid %d\n",
			auntuncle->person_id, sex,
			AGE(auntuncle), p->person_id);
		    */
					c_tally[sex][C_BMOMBMOMBKID]++;
				}
				auntuncle = auntuncle->e_sib_mom;
			}
		}

		if ((mzayde != NULL) && (mbubbe != NULL))
		{
			auntuncle = mbubbe->lborn;
			while (auntuncle != NULL)
			{
				if ((ALIVE(auntuncle)) && (auntuncle->father != mzayde))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmombmombkid %d\n",
			auntuncle->person_id, sex,
			AGE(auntuncle), p->person_id);
		    */
					c_tally[sex][C_BMOMBMOMBKID]++;
				}
				auntuncle = auntuncle->e_sib_mom;
			}
		}

		if (pbubbe != NULL)
		{
			if (ALIVE(pbubbe))
			{
				/*
		fprintf(fd_tmp,"%d %d %d bdadbmom %d\n",
		    pbubbe->person_id, sex, AGE(pbubbe), p->person_id);
		*/
				c_tally[sex][C_BDADBMOM]++;
			}

			if (pbubbe->mother != NULL)
			{
				if (ALIVE(pbubbe->mother))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadbmombmom %d\n",
			pbubbe->mother->person_id, sex,
			AGE(pbubbe->mother), p->person_id);
		    */
					c_tally[sex][C_BDADBMOMBMOM]++;
				}
			}
			if (pbubbe->father != NULL)
			{
				if (ALIVE(pbubbe->father))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadbmombdad %d\n",
			pbubbe->father->person_id, sex,
			AGE(pbubbe->father), p->person_id);
		    */
					c_tally[sex][C_BDADBMOMBDAD]++;
				}

				if (pbubbe->mother != NULL)
				{
					greatauntuncle = pbubbe->father->lborn;
					while (greatauntuncle != NULL)
					{
						if ((ALIVE(greatauntuncle)) &&
							(greatauntuncle != pbubbe))
						{
							if (greatauntuncle->mother == pbubbe->mother)
							{
								/*
				fprintf(fd_tmp,"%d %d %d bdadbmombparbkid %d\n",
				    greatauntuncle->person_id,
				    sex, AGE(greatauntuncle), p->person_id);
				*/
								c_tally[sex][C_BDADBMOMBPARBKID]++;
							}
						}
						greatauntuncle = greatauntuncle->e_sib_dad;
					}
				}
			}
		}
		if (pzayde != NULL)
		{
			if (ALIVE(pzayde))
			{
				/*
		fprintf(fd_tmp,"%d %d %d bdadbdad %d\n",
		    pzayde->person_id, sex, AGE(pzayde), p->person_id);
		*/
				c_tally[sex][C_BDADBDAD]++;
			}

			auntuncle = pzayde->lborn;
			while (auntuncle != NULL)
			{
				if ((ALIVE(auntuncle)) && (auntuncle != father))
				{
					if (auntuncle->mother == pbubbe)
					{
						/*
			fprintf(fd_tmp,"%d %d %d bdadbparbkid %d\n",
			    auntuncle->person_id, sex,
			    AGE(auntuncle), p->person_id);
			  */
						c_tally[sex][C_BDADBPARBKID]++;
					}
					else
					{
						/*
			fprintf(fd_tmp,"%d %d %d bdadbdadbkid %d\n",
			    auntuncle->person_id, sex,
			    AGE(auntuncle), p->person_id);
			  */
						c_tally[sex][C_BDADBDADBKID]++;
					}
				}
				auntuncle = auntuncle->e_sib_dad;
			}

			if (pzayde->mother != NULL)
			{
				if (ALIVE(pzayde->mother))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadbdadbmom %d\n",
			pzayde->mother->person_id, sex,
			AGE(pzayde->mother), p->person_id);
		    */
					c_tally[sex][C_BDADBDADBMOM]++;
				}
			}
			if (pzayde->father != NULL)
			{
				if (ALIVE(pzayde->father))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadbdadbdad %d\n",
			pzayde->father->person_id, sex,
			AGE(pzayde->father), p->person_id);
		    */
					c_tally[sex][C_BDADBDADBDAD]++;
				}

				if (pzayde->mother != NULL)
				{
					greatauntuncle = pzayde->father->lborn;
					while (greatauntuncle != NULL)
					{
						if ((ALIVE(greatauntuncle)) &&
							(greatauntuncle != pzayde))
						{
							if (greatauntuncle->mother == pzayde->mother)
							{
								/*
				fprintf(fd_tmp,"%d %d %d bdadbdadbparbkid %d\n",
				    greatauntuncle->person_id, sex,
				    AGE(greatauntuncle), p->person_id);
				*/
								c_tally[sex][C_BDADBDADBPARBKID]++;
							}
						}
						greatauntuncle = greatauntuncle->e_sib_dad;
					}
				}
			}
		}

		if ((pzayde == NULL) && (pbubbe != NULL))
		{
			auntuncle = pbubbe->lborn;
			while (auntuncle != NULL)
			{
				if ((ALIVE(auntuncle)) && (auntuncle != father))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadbmombkid %d\n",
			auntuncle->person_id, sex,
			AGE(auntuncle), p->person_id);
		    */
					c_tally[sex][C_BDADBMOMBKID]++;
				}
				auntuncle = auntuncle->e_sib_mom;
			}
		}

		if ((pzayde != NULL) && (pbubbe != NULL))
		{
			auntuncle = pbubbe->lborn;
			while (auntuncle != NULL)
			{
				if ((ALIVE(auntuncle)) && (auntuncle->mother != pbubbe))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadbmombkid %d\n",
			auntuncle->person_id, sex,
			AGE(auntuncle), p->person_id);
		    */
					c_tally[sex][C_BDADBMOMBKID]++;
				}
				auntuncle = auntuncle->e_sib_mom;
			}
		}

		if (mother != NULL)
		{
			if (mother->last_marriage != NULL)
			{
				if ((stepfather = mother->last_marriage->husband) != father)
				{
					if (ALIVE(stepfather))
					{
						/*
			fprintf(fd_tmp,"%d %d %d sdad %d\n",
			stepfather->person_id, sex,
			AGE(stepfather), p->person_id);
		    */
						c_tally[sex][C_SDAD]++;
					}

					if ((stepfather->mother != NULL) && (ALIVE(stepfather->mother)))
					{
						/*
			fprintf(fd_tmp,"%d %d %d sdadbmom %d\n",
			    stepfather->mother->person_id, sex,
			    AGE(stepfather->mother), p->person_id);
			*/
						c_tally[sex][C_SDADBMOM]++;
					}
					if ((stepfather->father != NULL) &&
						(ALIVE(stepfather->father)))
					{
						/*
			fprintf(fd_tmp,"%d %d %d sdadbdad %d\n",
			    stepfather->father->person_id, sex,
			    AGE(stepfather->father), p->person_id);
			*/
						c_tally[sex][C_SDADBDAD]++;
					}
				}
			}

			/* half siblings via mother include those via null father */
			sibling = mother->lborn;
			while (sibling != NULL)
			{
				if ((ALIVE(sibling)) && (sibling != p))
				{
					if (sibling->father != NULL)
					{
						if (sibling->father == father)
						{
							/*
			    fprintf(fd_tmp,"%d %d %d bparbkid %d\n",
				sibling->person_id, sex,
				AGE(sibling), p->person_id);
			    */
							c_tally[sex][C_BPARBKID]++;
						}
						else
						{
							/*
			    fprintf(fd_tmp,"%d %d %d bmombkid %d\n",
				sibling->person_id, sex,
				AGE(sibling), p->person_id);
			    */
							c_tally[sex][C_BMOMBKID]++;
						}
					}
					else
					{
						/*
			fprintf(fd_tmp,"%d %d %d bmombkid %d\n",
			    sibling->person_id, sex,
			    AGE(sibling), p->person_id);
			*/
						c_tally[sex][C_BMOMBKID]++;
					}
				}
				sibling = sibling->e_sib_mom;
			}
		}

		if (father != NULL)
		{
			if ((stepmother = father->last_marriage->wife) != mother)
			{
				if (ALIVE(stepmother))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d smom %d\n",
			stepmother->person_id, sex,
			AGE(stepmother), p->person_id);
		    */
					c_tally[sex][C_SMOM]++;
				}
				if ((stepmother->mother != NULL) && (ALIVE(stepmother->mother)))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d smombmom %d\n",
			stepmother->mother->person_id, sex,
			AGE(stepmother->mother), p->person_id);
		    */
					c_tally[sex][C_SMOMBMOM]++;
				}
				if ((stepmother->father != NULL) &&
					(ALIVE(stepmother->father)))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d smombdad %d\n",
			stepmother->father->person_id, sex,
			AGE(stepmother->father), p->person_id);
		    */
					c_tally[sex][C_SMOMBDAD]++;
				}
			}

			/* half siblings via father are those with other mother--
		 * as mothers cannot be null */
			sibling = father->lborn;
			while (sibling != NULL)
			{
				if ((ALIVE(sibling)) && (sibling->mother != mother))
				{
					/*
			fprintf(fd_tmp,"%d %d %d bdadbkid %d\n",
			    sibling->person_id, sex,
			    AGE(sibling), p->person_id);
		    */
					c_tally[sex][C_BDADBKID]++;
				}
				sibling = sibling->e_sib_dad;
			}
		}

		if (mbubbe != NULL)
		{
			if (mbubbe->last_marriage != NULL)
			{
				if ((stepmzayde = mbubbe->last_marriage->husband) != mzayde)
				{
					if (ALIVE(stepmzayde))
					{
						/*
			fprintf(fd_tmp,"%d %d %d bmomsdad %d\n",
			    stepmzayde->person_id, sex,
			    AGE(stepmzayde), p->person_id);
			*/
						c_tally[sex][C_BMOMSDAD]++;
					}
				}
			}
		}

		if (mzayde != NULL)
		{
			if ((stepmbubbe = mzayde->last_marriage->wife) != mbubbe)
			{
				if (ALIVE(stepmbubbe))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bmomsmom %d\n",
			stepmbubbe->person_id, sex,
			AGE(stepmbubbe), p->person_id);
		    */
					c_tally[sex][C_BMOMSMOM]++;
				}
			}
		}

		if (pbubbe != NULL)
		{
			if (pbubbe->last_marriage != NULL)
			{
				if ((steppzayde = pbubbe->last_marriage->husband) != pzayde)
				{
					if (ALIVE(steppzayde))
					{
						/*
			fprintf(fd_tmp,"%d %d %d bdadsdad %d\n",
			    steppzayde->person_id, sex,
			    AGE(steppzayde), p->person_id);
			*/
						c_tally[sex][C_BDADSDAD]++;
					}
				}
			}
		}

		if (pzayde != NULL)
		{
			if ((steppbubbe = pzayde->last_marriage->wife) != pbubbe)
			{
				if (ALIVE(steppbubbe))
				{
					/*
		    fprintf(fd_tmp,"%d %d %d bdadsmom %d\n",
			steppbubbe->person_id, sex,
			AGE(steppbubbe), p->person_id);
		    */
					c_tally[sex][C_BDADSMOM]++;
				}
			}
		}
	}
	/*
    fclose(fd_tmp);
    */
}

/*************************************************************************
 * get_extra: expects a pointer to a person and an integer 
 * index value. returns the index'th value in the linked list
 * used to access extra socio-demog like variables 
 ************************************************************************/
float get_extra( struct person *p, int i)
{
	struct extra_data *first;
	struct extra_data *current;
	int indx;
	if ((i < 0) || (i > size_of_extra))
	{
		stop("Attempt to read stupid position in extra vars: %d", " ",i);
		//exit(-1);
	}
	indx = 0;
	if (p == NULL)
	{
		stop("Attempt to reference extra variable of nonexistant person\n", "", 1);
		//exit(-1);
	}
	if (p->extra == NULL)
	{
		stop("Attempt to reference uninitialized extra variable personid:%d\n",
			   " ",(int) p->person_id);
		//exit(-1);
	}

	current = p->extra;
	while (indx < i)
	{
		if (current->next != NULL)
		{
			current = current->next;
			indx++;
		}
		else
		{
			stop("%s\n", "Attempt to read uninitialized value in get_extra",1);
			//exit(-1);
		}
	}
	return (current->value);
}

/*****************************************************************
   *put_extra expects a pointer to a person an integer and a value
   *the value will be stuck into the index'th place on the
   *egos_extra linked list. all mallocs will be done as implied
   *****************************************************************/
void put_extra(struct person *p,int i,float x)
{
	struct extra_data *current;
	struct extra_data *first;

	int indx;
	if ((i < 0) || (i > size_of_extra))
	{
		stop("Attempt to write stupid number of extra vars %s"," ", i);
		//exit(-1);
	}
	indx = 0;

	if (p->extra == NULL)
	{
		p->extra = NEW(struct extra_data);
		p->extra->next = NULL;
		p->extra->value = 0;
	}
	current = p->extra;
	while (indx < i)
	{
		if (current->next == NULL)
		{
			current->next = NEW(struct extra_data);
			current->next->value = 0;
			current->next->next = NULL;
		}

		indx++;
		current = current->next;
	}
	/* indx == i */
	current->value = x;
}

/*****************************************************************/

int get_parity(	struct person *p)
	/************************************************************
   returns number of births to person p if female
   else returns zero
  ************************************************************/
{
	int indx;
	struct person *child;

	if (p->sex == FEMALE)
	{
		if (p->lborn == NULL)
		{
			return (0);
		}
		indx = 0;
		child = p->lborn;
		while (child != NULL)
		{
			child = child->e_sib_mom;
			indx++;
		}
		return (indx);
	}
	else
	{
		/* zero for males for now */
		return (0);
	}
}
/************************************************************************/
int get_previous_mstatus(struct person *p)

{
	/** Mon Apr 19 09:59:26 PDT 2010 Called from divorce and death in
      when the deceased is cohabitting.  p in this case is
      deceased/divorcer's spouse, whose post event mstatus needs to
      revert to his/her previous value. But of course it's complicated
      especially in case of polygamy
  **/
	struct marriage *cmar;
	struct marriage *pmar;
	int prev_mstat;
	int mstat_found = 0;

	cmar = p->last_marriage;
	prev_mstat = SINGLE; /* most likely but... */

	/** checking of other previous mstats **/
	/** walk back through ps previous marraiges **/
	pmar = (p->sex == FEMALE ? cmar->wifes_prior : cmar->husbands_prior);

	while (pmar != NULL)
	{
		switch (pmar->reason_end)
		{
		case (M_NULL):
			/*still married to someone else */
			prev_mstat = MARRIED; //todo: this was a single = before
			mstat_found = 1;
			break;
		case (DIVORCE):
			/* previous divorce */
			prev_mstat = DIVORCED; //todo: what? assignment or really only a comparison? this was == before
			mstat_found = 1;
			break;
		case (DEATH):
			/* previous widowhood */
			prev_mstat = WIDOWED;
			mstat_found = 1;
			break;
		case (BREAKCOHAB):
			/*break up*/
			prev_mstat = SINGLE;
			/* keep looking backward */
			break;
		case (MARAFTERCOHAB):
			/** should never get here **/
			perror("found maraftercohab but no marriage thereafter");
			break;
		case (COCOHABDIES):
			/** death of squeeze **/
			prev_mstat = SINGLE;
			break;
		} /*switch*/
		if (mstat_found == 1)
		{
			/** no need to look further **/
			break;
		}
		else
		{
			pmar = (p->sex == FEMALE ? pmar->wifes_prior : pmar->husbands_prior);
		}
	} /*while*/
	return (prev_mstat);
}
/******************************************************************/
int get_previous_mstatusOBSOLETE( struct person *p)
{
	struct marriage *prior;
	/*
    This is called on disolution of cohab to determine what
    the new mstatus should be *after* cohab breaks up 
    Marcia used egos_extra to keep track of previous mstat
  */

	/* start with the current (about to end marriage) */
	prior = p->last_marriage;

	if (prior == NULL)
	{
		return (SINGLE); /* no prior marriage */
	}
	/* look at previous marriages to find what we need */

	while (prior != NULL)
	{
		if (p->sex == MALE)
		{
			prior = prior->husbands_prior;
		}
		else
		{
			prior = prior->wifes_prior;
		}

		if (prior == NULL)
		{
			return (SINGLE);
		}

		if (prior->reason_end == DEATH)
		{
			return (WIDOWED);
		}

		if (prior->reason_end == DIVORCE)
		{
			return (DIVORCED);
		}
		if (prior->reason_end == M_NULL)
		{
			return (MARRIED); /* Ouch -- this could theoretically be an
			   uneded cohabitation -- if this is called
			   only on disolution of cohab and we further
			   assume noone can have more than one
			   co-cohabitant at a time that we are ok... I
			   think. But heads up.
			*/
		}
	}
	stop("%s\n", "ran out of prior marriages in get_prior_mstatus", 1);
	//exit(-1);
}

/******************************************************************/

int get_marity(struct person *p)
{
	/*
    Counts current and previous marriages returns integer
  */

	int indx;
	struct marriage *prior;

	indx = 0;
	prior = p->last_marriage;
	while (prior != NULL)
	{
		indx++;
		prior = (p->sex == FEMALE) ? prior->wifes_prior : prior->husbands_prior;
	}
	return (indx);
}
