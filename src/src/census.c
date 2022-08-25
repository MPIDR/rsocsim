/* %W% %G% */
//#include "defs.h"
#include <strings.h>
#include <stdio.h>
#include <sys/file.h>
#include <stdlib.h>
#include <math.h>

double census1960_sm[] = { 9020, 8350, 7600, 5658, 2448, 946, 593, 455, 360,
    342, 326, 302, 239, 205, 159, 99, 44, 21,};

double census1960_mm[] = { 0, 0, 0, 231, 2208, 3786, 4625, 5011, 4752, 4507,
    4006, 3477, 2861, 2431, 1812, 1125, 545, 283,};

double census1960_sf[] = { 8600, 7960, 7250, 4843, 1326, 470, 349, 337, 320,
    327, 347, 334, 272, 253, 205, 140, 83, 50,};

double census1960_mf[] = { 0, 0, 0, 928, 3490, 4364, 5023, 5396, 4959, 4667,
    4107, 3646, 3140, 2772, 2140, 1404, 742, 435,};

double census1960_tm[] = { 9020, 8350, 7600, 5889, 4656, 4732, 5218, 5466,
				5112, 4849, 4332, 3779, 3100, 2636, 1971, 1224, 589, 304, };

double census1960_tf[] = { 8600, 7960, 7250, 5771, 4816, 4834, 5372, 5733,
				5279, 4994, 4454, 3980, 3412, 3025, 2345, 1544, 825, 485, };

int single1960[] = {37167, 33466, };
int notsingle1960[] = {41660, 47213, };
int total1960[] = {78827, 80679, };

double single_scale[2], notsingle_scale[2], total_scale[2];


void marriage_tally(FILE *fd)
{
    struct person *p;
    int single_matrix[20][2][MAXGROUPS];
    int notsingle_matrix[20][2][MAXGROUPS];
    int total_matrix[20][2][MAXGROUPS];
    int i, g;

    fprintf (fd,"MALES on marriage queue %d \n", marriage_queue->num); 
    fprintf (fd,"FEMALES on marriage queue %d \n", (marriage_queue + 1)->num); 

    for (g = 1; g <= numgroups; g++) {
	      for (i = 0; i < 20; i++) {
	          single_matrix[i][MALE][g] = 0;
	          single_matrix[i][FEMALE][g] = 0;
	          notsingle_matrix[i][MALE][g] = 0;
	          notsingle_matrix[i][FEMALE][g] = 0;
	          total_matrix[i][MALE][g] = 0;
	          total_matrix[i][FEMALE][g] = 0;
	}
    }

    p = person0;

    while (p != NULL) {
	      if (p->deathdate == 0) {
	          i = (int) ((current_month - p->birthdate)/60);

		  printf("agegrp %d mstat %d group %d smat %d \n",i,
			 p->mstatus,p->group,
			 single_matrix[i][p->sex][p->group]); 
	          if (p->mstatus == SINGLE) {
	              single_matrix[i][p->sex][p->group]++;
	          } else {
	              notsingle_matrix[i][p->sex][p->group]++;
            }
	         total_matrix[i][p->sex][p->group]++;
	      }
	      p = p->down;
    }

    for (g = 1; g <= 1; g++) {

      /* odd seg fault here */
	      fprintf(fd,"GROUP %d MALE SINGLE ", g);

	      for (i = 0; i < 20; i++)
                  fprintf(fd,"%d ", single_matrix[i][MALE][g]);
	      fprintf(fd,"\n");
	      fprintf(fd,"GROUP %d MALE NOTSINGLE ", g);
	      for (i = 0; i < 20; i++)
                  fprintf(fd,"%d ", notsingle_matrix[i][MALE][g]);
	      fprintf(fd,"\n");
	      fprintf(fd,"GROUP %d FEMALE SINGLE ", g);
	      for (i = 0; i < 20; i++)
                  fprintf(fd,"%d ", single_matrix[i][FEMALE][g]);
	      fprintf(fd,"\n");
	      fprintf(fd,"GROUP %d FEMALE NOTSINGLE ", g);
	      for (i = 0; i < 20; i++)
                  fprintf(fd,"%d ", notsingle_matrix[i][FEMALE][g]);
	      fprintf(fd,"\n");
    }
          /*
          */
    for (g = 1; g <= 1; g++) {
	      int sum_sm = 0;
	      int sum_sf = 0;
	      int sum_mm = 0;
	      int sum_mf = 0;
	      int sum_tm = 0;
	      int sum_tf = 0;
	      int x;

	      fprintf(fd,"GROUP %d MALE SINGLE ", g);
	      for (i = 0; i < 17; i++) {
	          fprintf(fd,"%d ", single_matrix[i][MALE][g]);
	          sum_sm += single_matrix[i][MALE][g];
	      }
	      x = single_matrix[17][MALE][g] + single_matrix[18][MALE][g]+
	              single_matrix[19][MALE][g];
	      fprintf(fd,"%d\n", x);
	      sum_sm += x;
        single_scale[MALE] = (double) single1960[MALE]/sum_sm;
	      fprintf(fd,"sum_sm %d scale factor %lf\n", sum_sm, single_scale[MALE]);
	      fprintf(fd,"DIFF ");
	      for (i = 0; i < 17; i++) {
		fprintf(fd,"%lf ", census1960_sm[i] -
			single_scale[MALE] * single_matrix[i][MALE][g]);
	      }
	      fprintf(fd,"%lf\n", census1960_sm[17] - single_scale[MALE] * x);
      
	      fprintf(fd,"GROUP %d MALE NOTSINGLE ", g);
	      for (i = 0; i < 17; i++) {
		fprintf(fd,"%d ", notsingle_matrix[i][MALE][g]);
	          sum_mm += notsingle_matrix[i][MALE][g];
	      }
	      x = notsingle_matrix[17][MALE][g] + notsingle_matrix[18][MALE][g]
   	          + notsingle_matrix[19][MALE][g];
	      fprintf(fd,"%d\n", x);
	      sum_mm += x;
			  notsingle_scale[MALE] = (double) notsingle1960[MALE]/sum_mm;
	      fprintf(fd,"sum_mm %d scale factor %lf\n", sum_mm, notsingle_scale[MALE]);
	      fprintf(fd,"DIFF ");
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%lf ", census1960_mm[i] -
                      notsingle_scale[MALE] * notsingle_matrix[i][MALE][g]);
	      }
	      fprintf(fd,"%lf\n", census1960_mm[17] - notsingle_scale[MALE] * x);
      
	      fprintf(fd,"GROUP %d MALE TOTAL ", g);
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%d ", total_matrix[i][MALE][g]);
	          sum_tm += total_matrix[i][MALE][g];
	      }
	      x = total_matrix[17][MALE][g] + total_matrix[18][MALE][g]
   	          + total_matrix[19][MALE][g];
	      fprintf(fd,"%d\n", x);
	      sum_tm += x;
              total_scale[MALE] = (double) total1960[MALE]/sum_tm;
	      fprintf(fd,"sum_tm %d scale factor %lf\n", sum_tm, total_scale[MALE]);
	      fprintf(fd,"TOTAL DIFF ");
	      for (i = 0; i < 17; i++) {
                   fprintf(fd,"%lf ", census1960_tm[i] -
                        total_scale[MALE] * total_matrix[i][MALE][g]);
	      }
	      fprintf(fd,"%lf\n", census1960_tm[17] - total_scale[MALE] * x);
            
	      fprintf(fd,"GROUP %d FEMALE SINGLE ", g);
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%d ", single_matrix[i][FEMALE][g]);
	          sum_sf += single_matrix[i][FEMALE][g];
	      }
	      x = single_matrix[17][FEMALE][g] + single_matrix[18][FEMALE][g]
   	          + single_matrix[19][FEMALE][g];
	      fprintf(fd,"%d\n", x);
	      sum_sf += x;
              single_scale[FEMALE] = (double) single1960[FEMALE]/sum_sf;
	      fprintf(fd,"sum_sf %d scale factor %lf\n", sum_sf,
	         single_scale[FEMALE]);
	      fprintf(fd,"DIFF ");
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%lf ", census1960_sf[i] -
                      single_scale[FEMALE] * single_matrix[i][FEMALE][g]);
	      }
	      fprintf(fd,"%lf\n", census1960_sf[17] - single_scale[FEMALE] * x);
      
	      fprintf(fd,"GROUP %d FEMALE NOTSINGLE ", g);
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%d ", notsingle_matrix[i][FEMALE][g]);
	          sum_mf += notsingle_matrix[i][FEMALE][g];
	      }
	      x = notsingle_matrix[17][FEMALE][g] +
                  notsingle_matrix[18][FEMALE][g] +
                  notsingle_matrix[19][FEMALE][g];
	      fprintf(fd,"%d\n", x);
	      sum_mf += x;
              notsingle_scale[FEMALE] = (double) notsingle1960[FEMALE]/sum_mf;
	      fprintf(fd,"sum_mf %d scale factor %lf\n",
	          sum_mf, notsingle_scale[FEMALE]);
	      fprintf(fd,"DIFF ");
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%lf ", census1960_mf[i] -
                     notsingle_scale[FEMALE] * notsingle_matrix[i][FEMALE][g]);
	      }
	      fprintf(fd,"%lf\n", census1960_mf[17] - notsingle_scale[FEMALE] * x);
      
	      fprintf(fd,"GROUP %d FEMALE TOTAL ", g);
	      for (i = 0; i < 17; i++) {
                  fprintf(fd,"%d ", total_matrix[i][FEMALE][g]);
	          sum_tf += total_matrix[i][FEMALE][g];
	      }
	      x = total_matrix[17][FEMALE][g] + total_matrix[18][FEMALE][g]
   	          + total_matrix[19][FEMALE][g];
	      fprintf(fd,"%d\n", x);
	      sum_tf += x;
               total_scale[FEMALE] = (double) total1960[FEMALE]/sum_tf;
	      fprintf(fd,"sum_tf %d scale factor %lf\n",
                   sum_tf, total_scale[FEMALE]);
	      fprintf(fd,"TOTAL DIFF ");
	      for (i = 0; i < 17; i++) {
                   fprintf(fd,"%lf ", census1960_tf[i] -
                   total_scale[FEMALE] * total_matrix[i][FEMALE][g]);
	      }
	      fprintf(fd,"%lf\n", census1960_tf[17] - total_scale[FEMALE] * x);
	  }
  }

/**********************************************************************/
void evaluate_marmkt(FILE *fd)
{
/* this makes sense only when MSCOREPROB is in effect. It will print the
   values in marriage_agetarget against those realized.  */

  
  int g,i,rxs,expected_marriages;// observed_marriages;
  double xs_mars=0,observedP=0,expectedP,oeratio=0,pctwrong=0,score=0;
  struct person *groom;
  struct person *bride;
    
  bride = NEW(struct person);
  groom = NEW(struct person);
  groom->birthdate=2000; /*months*/
  groom->sex=MALE; bride->sex=FEMALE;
  
  

  fprintf(fd,"ageDiff\ttargetProp \tobservedProp\tE(mars)\tO(mars)\texcess\tO/E\tscore\n");
  for(g=1;g<=numgroups;g++){
    for(i=0;i<(2*MAXUYEARS); i++){
      bride->group=g;

      bride->birthdate= groom->birthdate +(i-MAXUYEARS)*12 ;
      score= score3(bride,groom);
      observedP=((double)(marriage_ageobs[g][i]))/((double)(marriage_tots[g]));
      expectedP=(marriage_agetarget[g][i]>0)?marriage_agetarget[g][i]:0;
      expected_marriages=(marriage_agetarget[g][i]*marriage_tots[g]);
      expected_marriages=(expected_marriages <0)?0:expected_marriages;
      //observed_marriages=marriage_ageobs[g][i];
      xs_mars= marriage_ageobs[g][i]-expected_marriages;

      rxs=round(xs_mars);
      
      if(marriage_ageobs[g][i]<=0){
	oeratio= 0;
      }else{
	oeratio=(expected_marriages<=0)?
	  1:
	  (double)marriage_ageobs[g][i]/expected_marriages;
      }

            
      fprintf(fd,"%d\t%e\t%e\t%d\t%d\t%d\t%f\t%f\n",
	      (i-MAXUYEARS),
	      expectedP,
	      observedP,
	      expected_marriages,
	      marriage_ageobs[g][i],
	      rxs,
	      oeratio,
	      score
	      );
      pctwrong+=ABS(expectedP-observedP);

      /*
      fprintf(fd,"%d == %g == %g == %g == %g == %d\n",
	     (i-MAXUYEARS),
	     marriage_agetarget[g][i],
	     ((double)marriage_ageobs[g][i]/ (double)marriage_tots[g]),
	      marriage_ageobs[g][i],
	      xs_mars,
	      marriage_tots[g]);
      */
    }
    /*pctwrong=(nwrong/marriage_tots[g])/2; */
    fprintf(fd,"Absolute sum of fract expected-observed in group %d:%f\n",
	    g,pctwrong);
  }
  

	  
}
