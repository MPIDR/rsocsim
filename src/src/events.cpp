/* %W% %G% */
#include "defs.h"
#include <strings.h>
#include <stdio.h>
#include <sys/file.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include <Rcpp.h>

#include <Rcpp.h>
using namespace Rcpp;

#include "census.c"
#include "io.c"
#include "load.cpp"
#include "random.c"
#include "utils.c"
#include "xevents.c"

//using namespace Rcpp;

int load(char*);
void adjust_birth_for_bint();
void fix_pop_pointers();
void prepare_output_files(int);
void read_marlist(FILE*);
void read_otx(FILE*);
int read_initial_pop(FILE*);
int read_xtra(FILE*, int);
void delete_hash_table();
void death(struct person*);
void divorce(struct person*);
void birth(struct person*);
int fill_rate_gaps();
void print_segment_info(FILE*);
void census();
void child_census();
int process_month();
void write_popfiles(int);
//int date_and_event(struct person*);
void evaluate_marmkt(FILE*);
//int get_parity( struct person*);

#include <time.h>

#define PTR_NULL 0
#define PTR_N 1
#define PTR_Q 2

int global_verbose = 0;

char *a_tally_to_string[] = {
	"SELF",
	"SPOU",
	"BKID",
	"SKID",
	"BSONBKID",
	"BDAUBKID",
	"BSONSKID",
	"BDAUSKID",
	"SSONBKID",
	"SDAUBKID",
};

char *c_tally_to_string[] = {
	"SELF",
	"SPOU",
	"BDAD",
	"BMOM",
	"SDAD",
	"SMOM",
	"BPARBKID",
	"BDADBKID",
	"BMOMBKID",
	"BDADBDAD",
	"BDADBMOM",
	"BMOMBDAD",
	"BMOMBMOM",
	"BDADSDAD",
	"BDADSMOM",
	"BMOMSDAD",
	"BMOMSMOM",
	"SDADBDAD",
	"SDADBMOM",
	"SMOMBDAD",
	"SMOMBMOM",
	"BDADBDADBDAD",
	"BDADBDADBMOM",
	"BDADBMOMBDAD",
	"BDADBMOMBMOM",
	"BMOMBDADBDAD",
	"BMOMBDADBMOM",
	"BMOMBMOMBDAD",
	"BMOMBMOMBMOM",
	"BDADBPARBKID",
	"BDADBDADBKID",
	"BDADBMOMBKID",
	"BMOMBPARBKID",
	"BMOMBDADBKID",
	"BMOMBMOMBKID",
	"BDADBDADBPARBKID",
	"BDADBMOMBPARBKID",
	"BMOMBDADBPARBKID",
	"BMOMBMOMBPARBKID",
	"NUMCAT",
};
 
 
 
// [[Rcpp::export]]
int main1(int argc, char *argv[])

{
  Rcpp::Rcout << "start socsim main. MAXUYEARS: " << MAXUYEARS << "; MAXUMONTHS: " << MAXUMONTHS << std::endl;
  //Rprintf("-0at-----------%s  \n", argv[0]);
  //Rprintf("-1at-----------%s  \n", argv[1]);
  //Rprintf("-2at-----------%s  \n", argv[2]);
  //Rprintf("-3a-----------%s  \n", argv[3]);
  //Rprintf("-0a-----------%d  \n", argv[0]);
  //Rprintf("-1a-----------%d  \n", argv[1]);
  //Rprintf("-2a-----------%d  \n", argv[2]);
  //Rprintf("-3a-----------%d  \n", argv[3]);
  //Rcpp::Rcout << "useage: ratefile random_number" << argv[0] << "!!" << std::endl;
  //Rcpp::Rcout << "useage: ratefile random_number" << argv[1] << "!!" << std::endl;
  //Rcpp::Rcout << "start socsim main \n" << std::endl;
  
  //Rcpp::Rcout << "start socsim main2 \n" << std::endl;
  strcpy(rate_file_name, argv[1]);
  //Rcpp::Rcout << "start socsim main 3\n" << std::endl;
  //strcpy(rate_file_name, *++argv);
  //Rcpp::Rcout << "start socsim main 4\n" << std::endl;
  
  Rcpp::Rcout << "ratefile: "<< rate_file_name << "\n" << std::endl;
  //Rcpp::Rcout << "useage: ratefile random_number" << "!!"<< argv[2] << std::endl;
  
	clock_t timestart1,timestart2,timeend;
	double timedif1,timedif2;

	//perror("jooooo");

	//stop("after perror jooo.");
  timestart1 = clock();
  	
	char command_string[1024];
	struct queue_element *e, *q;
	int i, j, fy;
	char useage[1024];
	char logstring[1024];
	
	/* some initialization */
	/*****************************************************************
     *It all starts here .sup file has not been read yet and the sim 
     *has not begun. This is where many things should be initialized to 
     * their default values 
     ************************************************************/

  //printf("\nrand_max: %ld",RAND_MAX);

	current_segment = 1;
	current_offset = 0;
	current_fstatus = CLOSED;

	//return 12;
	
	for (e = event_queue, i = 0; i < MAXUMONTHS; e++, i++)
	{
		e->first = NULL;
		e->num = 0;
	}
	
	for (q = marriage_queue, i = MALE; i <= FEMALE; q++, i++)
	{
		q->first = NULL;
		q->num = 0;
	}

	for (i = 0; i < MAXGROUPS; i++)
	{
		size_of_pop[i] = 0;
	}

	if (marriage_eval == DISTRIBUTION)
	{
		/* initialize target distribution */
		int g;
		for (g = 0; g < MAXGROUPS; g++)
		{
			marriage_tots[g] = 0;
			for (i = 0; i < (2 * MAXUYEARS); i++)
			{
				marriage_agetarget[g][i] = 0;
				marriage_ageobs[g][i] = 0;
			}
		}
	}

	hetfert = 1;
	alpha = 0;
	betaT = 1;
	bint = 9;

	hhmigration = 0;
	read_xtra_file = 0;

	random_father = FALSE;
	random_father_min_age = 15;
	prop_males = 0.5112;
	endogamy = 0;				/* default to random-ogamy */
	marriage_eval = PREFERENCE; /* or PROB*/
	marriage_queues = 2;
	
	/** marriage_eval ==  PREFERENCE**/
	marriage_peak_age = 36;
	marriage_slope_ratio = 2;
	marriage_agedif_max = 120;
	marriage_agedif_min = -120;
	/**marriage_eval == DISTRIBUTION**/
	/* mean and sd are specified by group in order to default
       higher groups to lower group values we cannot initialize 
       until we know how many groups there are in the sim 
    */
	
	//Rcpp::Rcout << "start socsim main 5\n" << std::endl;
	//Rprintf(log_file_name, "%s%d.log", rate_file_name, ceed);
	
	//Rcpp::Rcout << "\nstart socsim main 6\n" << std::endl;
	
	
	int g = 0;
	for (g = 0; g < MAXGROUPS; g++)
	{
		if (g == 0)
		{
			/* default values go in g=0*/
			agedif_marriage_mean[g] = 2;
			agedif_marriage_sd[g] = 3;
		}
		else
		{
			agedif_marriage_mean[g] = -(MAXUYEARS-1);
			agedif_marriage_sd[g] = -(MAXUYEARS-1);
		}
	}
	child_inherits_group = FROM_MOTHER;
	parameter0 = parameter1 = parameter2 = parameter3 = parameter4 = parameter5 = 0;
	
	size_of_extra = 0; /*no extra variables/.opox by default*/
	if (argc < 2)
	{
	  warning("Warning: Unexpected condition occurred - too few arguments");
	  //return 12;
	  
	  Rcpp::Rcout << "useage: ratefile random_number" << useage << argv[0] << std::endl;
	 	warning("useage: %s ratefile random_number", useage);
		//error(useage);
		stop("stop!");
		//exit(1);
	}
	//Rcpp::Rcout << "useage: ratefile random_number" << useage << argv[1] << argv[2] << std::endl;
	
	//strcpy(rate_file_name, *++argv);
	
	//Rcpp::Rcout << "useage: ratefile random_number" << useage << argv[1] << argv[2] << std::endl;
	Rcpp::Rcout << "v18a!-command-line argv[0]: " << argv[0] << "| argv[1]: " << argv[1] << "| argv[2]: " << argv[2] << std::endl;
	
	//ceed = atoi(*++argv);
	ceed = atoi(argv[2]);
	Rcpp::Rcout << "random_number seed: " << ceed << "| command-line argv[1]: " << argv[1] << "| argv[2]: " << argv[2] << std::endl;
	
	
	// initstate(ceed, randstate, 256); //suggested by jim: replace with:
	srand(ceed);
	
	//Rcpp::Rcout << "useage: ratefile random_number" << useage << argv[1] << argv[2] << std::endl;
	
	/**srandom(ceed);  setting seed for random() NOT rrandom() **/ // todo:check if random is initialized correctly!
	sprintf(log_file_name, "sim_results_%s_s%d.log", rate_file_name, ceed);
	//Rprintf(log_file_name, "%s%d.log", rate_file_name, ceed);
	
	
	//return 12;
	/**debugging random numbercrap 
    fd_random = fopen("randomsex", "w");
    fd_allrandom = fopen("allrandom", "w");
    *****************************/
	if ((fd_log = fopen(log_file_name, "w")) == NULL)
	{
	  
	    Rcpp:Rprintf("can't open file:  %s ", log_file_name);
		perror("can't open log file\n");
	}
	
	//Rprintf("|%s|-\n",pop_out_name);
	//Rprintf("-----\n");
	logmsg("--t start with logmessages \n"," ",1);
	
	/**sprintf(logstring,"\n compiled from SVN revision %s on %s \n\n",
	   SVN_REV, COMP_DATE);*
	Rprintf("%s\n", logstring, 1);*/
	Rprintf("Socsim Version: %s\n",
		   ENHANCEMENT_NAME, 1);
	if (marriage_queues == 1)
	{
		logmsg("\n marriage_queues==1 , consequently all availalbe males are always on the marriage queue\n\n"," ",   1);
	}
	Rprintf("initialize_segment_vars\n");
	logmsg("initialize_segment_vars\n"," ",1);
	initialize_segment_vars();
	Rprintf("initialize_segment_vars done\n");
	Rcpp::Rcout << "18b - loading -.sup-file: " << rate_file_name << std::endl;
	if (load(rate_file_name) < 0)
		stop("can't load rate file! Stopping Simulation.");//exit(-1);
	Rprintf("------------4");
	if (marriage_eval == DISTRIBUTION)
	{
		/* sup file has been read; so if value is not -99 (or (MAXUYEARS-1)) then it was
	 user specified  agedif_marriage_{mean,sd}[0] holds the default
      value*/
	  
		Rcpp::Rcout << "marriage_eval == DISTRIBUTION . " << rate_file_name << std::endl;
		int g = 1;
		for (g = 1; g < MAXGROUPS; g++)
		{
			
			Rcpp::Rcout << "| " << g << "  .  " << MAXUYEARS << "  .  " << MAXGROUPS << std::endl;
			agedif_marriage_mean[g] = (agedif_marriage_mean[g] == -(MAXUYEARS-1)) ? agedif_marriage_mean[g - 1] : agedif_marriage_mean[g];
			agedif_marriage_sd[g] = (agedif_marriage_sd[g] == -(MAXUYEARS-1)) ? agedif_marriage_sd[g - 1] : agedif_marriage_sd[g];
		}
		initialize_marriage_targets();
	} else {
		
		Rcpp::Rcout << "marriage_eval != DISTRIBUTION . " << rate_file_name << std::endl;
	}
	//Rprintf("------------5");

	/*modify birth rates to account for bint MUST be called 
     BEFORE fill_rate_gaps*/
	/*    if(bint != 0){*/
	adjust_birth_for_bint();
	Rprintf("------------6");
	/*    }*/
	//stop("stop15");
#ifdef ENHANCED
	enhance_defaults();
#endif

	/*******/

	/**

    printf("current value of birth interval %f\n", bint);
    printf("current value of hetfert flag %d\n", hetfert);
    **/
  
	fprintf(fd_log,"I am here at events.cpp-pop_file_name. |%s\n",pop_file_name);
	//pop_file_name[0] = 0;
	strcat(pop_file_name, ".opop");
	fprintf(fd_log,"I am here at events.cpp-pop_file_name. |%s\n",pop_file_name);
	fflush(fd_log);
	strcat(mar_file_name, ".omar");
	strcat(xtra_file_name, ".opox");
	strcat(pyr_file_name, ".pyr");
	strcat(stat_file_name, ".stat");
	strcat(otx_file_name, ".otx");
	
	Rprintf("------------7");
	/*** Tue Oct 22 12:50:50 PDT 2002

    strcat(pop_out_name, ".opop");
    strcat(mar_out_name, ".omar");
    strcat(xtra_out_name, ".opox");

    strcat(stat_file_name, ".stat");
    **/

	if ((fd_pop = fopen(pop_file_name, "r")) == NULL)
	{
	  //   Rprintf("------------pop_file error");
	  Rprintf("can't open file:  %s ", pop_file_name);
	  
	  //logmsg("can't open initial  pop file; can't simuluate..%s \n",pop_file_name,1);//exit
	  fprintf(fd_log,"can't open initial  pop file; can't simuluate..%s \n",pop_file_name);
	  stop("can't open initial  pop file; can't simuluate.. %s \n",pop_file_name);//exit
		perror("can't open initial  pop file; can't simuluate..\n");
	}
	if ((fd_mar = fopen(mar_file_name, "r")) == NULL)
	{
	  logmsg("can't openmarriage file Hope that's OK"," ",1);
	  warning("can't openmarriage file Hope that's OK");//exit
		//perror("can't open  marriage file Hope that's OK\n");
	}
	
	//Rprintf("------------aa1");

	if ((fd_otx = fopen(otx_file_name, "r")) == NULL)
	{
	  warning("can't open  transition history file. Hope that's OK\n");
	  //perror("can't open  transition history file. Hope that's OK\n");
	}
	//Rprintf("------------aa2");
  	//stop("here we stop!");
	if (read_xtra_file)
	{
		if ((fd_xtra = fopen(xtra_file_name, "r")) == NULL)
		{
			printf("xtra file is named %s\n", xtra_file_name);
		    //perror("can't open  xtra file despite being asked to\n");
		    stop("can't open  xtra file despite being asked to\n");
			//exit;
		}
	}
	/** open output files for writing**/
	
	Rprintf("\n output file names:\n %s|%s|%s|%s|%s|%s|\n",pop_file_name,mar_file_name,xtra_file_name,pyr_file_name,stat_file_name,otx_file_name);
	fprintf(fd_log,"\n output file names:\n %s|%s|%s|%s|%s|%s|\n",pop_file_name,mar_file_name,xtra_file_name,pyr_file_name,stat_file_name,otx_file_name);
		
	logmsg("----------------------- prepare_output_files_1 \n"," ",1);
	
	//Rprintf("------------aa3");
	prepare_output_files(0);
	
	//Rprintf("------------aa3a");
	// fd_rn = open_write("random_number");
	// fprintf(fd_rn, "%d\n", ceed);
	// fflush(fd_rn);
	// fprintf(fd_rn, "%d\n", irandom());
	//Rprintf("------------aa3b");
	//logmsg("opening pop pyramid file %s\n", pyr_file_name,1);
	
	//Rprintf("------------aa3c");
	fd_pyr = open_write(pyr_file_name);
	
	//Rprintf("------------aa4d");
	//logmsg("Reading initial population file %s\n", pop_file_name,1);
	/*    printf("Reading initial population file %s\n",pop_file_name);*/
	int pop_rows = 0;
	pop_rows = read_initial_pop(fd_pop);
	fclose(fd_pop);
	
	//Rprintf("------------aa5");
	if (fd_mar != NULL)
	{
	  	Rprintf("Reading initial marriage file %s\n", mar_file_name);
		fprintf(fd_log,"Reading initial marriage file %s\n", mar_file_name);
		read_marlist(fd_mar);
		fclose(fd_mar);
	}
	else
	{
		logmsg("No initial marriage file to read\n"," ", 1);
		/* Make sure that no one in init pop is married or else
	 problems will ensue. eg attempt to generate future divorce date
	 EVEN if divorce rates are zero.  Not sure what else, but its a 
	 bad idea to let this happen.*/
		struct person *p;
		char logstring[256];

		for (p = person0; p != NULL; p = p->down)
		{
			if (p->mstatus != SINGLE)
			{
				sprintf(logstring, "\n\nEncountered personid=%d with mstatus=%d (NOT SINGLE)\nSince no initial .omar file was found this is madness and can only lead to tears", p->person_id, p->mstatus);
				logmsg("%s\n", logstring, 1);
				//perror("No one in initial .opop can be married if you do not supply an initial .omar file. Duhhhhh.\n...Exitting");
				stop("No one in initial .opop can be married if you do not supply an initial .omar file. Duhhhhh.\n...Exitting");
				//exit(-1);
			}
		}
	}
	
	//Rprintf("------------aa7");
	if (fd_otx != NULL)
	{
		//logmsg("Reading initial transition history file %s\n", otx_file_name,1);
		fprintf(fd_log,"Reading initial transition history file %s\n", otx_file_name,1);
		read_otx(fd_otx);
		fclose(fd_otx);
	}
	else
	{
		logmsg("No initial transition history file to read\n"," ", 1);
	}
	Rprintf("fix pop pointers..\n");
	
	fix_pop_pointers();
	
	//Rprintf("------------aa8");
	if (read_xtra_file)
	{
		int rows_read = 0;
		char logstring[128];
		//logmsg("\n\nReading initial xtra file %s\n", xtra_file_name, 1);
		fprintf(fd_log,"\n\nReading initial xtra file %s\n", xtra_file_name);
		rows_read = read_xtra(fd_xtra, pop_rows);
		// sprintf(logstring, "%d observations read from %s",
		// 		rows_read, xtra_file_name);
		//logmsg("%s\n", logstring, 0);
		fprintf(fd_log,"%d observations read from %s\n",
				rows_read, xtra_file_name);

		fclose(fd_xtra);
	}
	else
	{
		//logmsg("xtra file NOT read: read_extra_file=%d\n", " ",read_xtra_file);
		fprintf(fd_log,"xtra file NOT read: read_extra_file=%d\n", read_xtra_file);
	}

	/* numgroups willbe reset to reflect groups in initial pop if nec 
       in fill_rate_gaps
    */
	
	//Rprintf("------------aa9");
	delete_hash_table();
	
	//Rprintf("------------aa3");
	current_month = last_event_date + 1;
	Rprintf("Starting month is %d\n", current_month);
	Rprintf("Initial size of pop %d  ", last_person->person_id);
	Rprintf("(living: %d)\n", size_of_pop[0]);

	if (fill_rate_gaps() < 0){
		perror("bad rate set\n");
	  warning("bad rate set\n");
	}
	
	//Rprintf("------------aa3s");
	/* dump_rates(); */

	if (random_epsilon)
	{
		epsilon = irandom() % (max_e_index + 1);
		printf("random epsilon ");
	}
	else if (fixed_epsilon)
	{
		printf("fixed epsilon ");
		epsilon = epsilon % (max_e_index + 1);
	}
	//Rprintf("------------aa32");
	//logmsg("-----aa32.\n", "", 1);


	//logmsg(" grogro001 \n", "", 1);
	if (random_epsilon || fixed_epsilon)
	{
		printf("value of epsilon: %d table index %d \n",
			   epsilon, (int)epsilon / 12);
	}

	//logmsg(" grogro1111 \n", "", 1);
	while (current_segment <= num_segments)
	{
		
		//logmsg(" grogroaa \n", "", 1);
		/** open output files for pos seg writing **/

		if (write_output)
			prepare_output_files(current_segment);

		if (take_census)
		{
			/* stat_file needs to be open if take_census specified
	   other output files are independent of this */
			char stat_file_seg[1024];
			char segnum[6];
			strcpy(stat_file_name_seg, stat_file_name);
			sprintf(segnum, "%d", current_segment);

			strcat(stat_file_name_seg, segnum);
			fd_stat = open_write(stat_file_name_seg);
		}
		//Rprintf("------------aa35");

		//logmsg("--aa35\n", "", 1);
		print_segment_info(fd_log);
		//logmsg("--aa35\n", "", 1);
		print_segment_info(stdout);
		//logmsg("--aa35\n", "", 1);
		population_pyramid(fd_pyr); /*blows guts here*/

		logmsg("Simulating...\n", "", 1);
		//Rcpp::Rcout << "Simulating...." << std::endl;
		
		//Rprintf("------------aa3h");
		//logmsg("--ab36\n", "", 1);
		/***
	  dump_rates();
      ***/

		new_events_for_all();
		
		//Rprintf("------------b");
		Rprintf("New events generated for all living persons\n");
		/*
	  dump_queue();
	*/

		if (duration_of_segment == 0)
			perror("length of segment not specified\n");
		stop_month = current_month + duration_of_segment - 1;
		printf("current month %d stop month %d duration %d\n",
			   current_month, stop_month, duration_of_segment);

		/*
	  printf("processing event queue\n");
	*/
		
		Rprintf("------------b1");
		logmsg("|m0|", "", 1);
		timestart2 = clock();
		int done = 0;
		//stop("just before month-loop");
		for (; current_month <= stop_month; current_month++)
		{
			
	    	//printf ("current month %d\n", current_month);
	    	
	    	//Rprintf("-m-");
		  
		  //logmsg("|m|", "", 1);
			if (take_census)
			{
				if (((current_month - 1 - last_event_date) % 12) == 0)
				{
					fy = firstyear + (int)(current_month - 1 - last_event_date) / 12;
					census();
					fprintf(fd_stat, "AGED:MALE %d %s %d",
							fy, a_tally_to_string[0], a_tally[0][0]);
					for (j = 1; j < A_NUMCAT; j++)
					{
						fprintf(fd_stat, " %s %d",
								a_tally_to_string[j], a_tally[0][j]);
					}
					fprintf(fd_stat, "\n");
					fprintf(fd_stat, "AGED:FEMALE %d %s %d",
							fy, a_tally_to_string[0], a_tally[1][0]);
					for (j = 1; j < A_NUMCAT; j++)
					{
						fprintf(fd_stat, " %s %d",
								a_tally_to_string[j], a_tally[1][j]);
					}
					fprintf(fd_stat, "\n");

					/*
		sprintf(command_string,
		"nawk '{print $2, $4}' < output.%d | sort | uniq -c > awk.out.%d\n",
		fy, fy);
		printf("executing %s", command_string);
		system(command_string);
	      */

					child_census();
					fprintf(fd_stat, "CHILD:MALE %d %s %d",
							fy, c_tally_to_string[0], c_tally[0][0]);
					for (j = 1; j < C_NUMCAT; j++)
					{
						fprintf(fd_stat, " %s %d",
								c_tally_to_string[j], c_tally[0][j]);
					}
					fprintf(fd_stat, "\n");
					fprintf(fd_stat, "CHILD:FEMALE %d %s %d",
							fy, c_tally_to_string[0], c_tally[1][0]);
					for (j = 1; j < C_NUMCAT; j++)
					{
						fprintf(fd_stat, " %s %d",
								c_tally_to_string[j], c_tally[1][j]);
					}
					fprintf(fd_stat, "\n");

					/*
		sprintf(command_string,
		"nawk '{print $2, $4}' < childoutput.%d | sort | uniq -c > awk.childout.%d\n",
		fy, fy);
		printf("executing %s", command_string);
		system(command_string);
	      */
				}
			}
			/* clear current month event counter*/
			
	    	int ecount;
			for (ecount = E_BIRTH; ecount < NUMEVENTS; ecount++)
			{
				crnt_month_events[ecount] = 0;
			}
			
	    	//Rprintf("-mp1");
		  	//logmsg("|m2|", "", 1);
	    	process_month();
	    	//logmsg("|m2s|", "", 1);
	    	
	    	//logmsg("|m3|", "", 1);
	    //	Rprintf("-mpend");
	    	if (1)
			{
				struct queue_element *mqmales, *mqfems;
				mqmales = marriage_queue + (1 - FEMALE);
				mqfems = marriage_queue + (1 - MALE);
				/*increment marriage queue time counters*/
				time_waiting[MALE] += mqmales->num;
				time_waiting[FEMALE] += mqfems->num;
				
				//logmsg("|m4|", "", 1);
				Rcpp::checkUserInterrupt();
				if (current_month % 5==0){			
					timeend = clock();
					timedif1 = (double)(timeend - timestart1) / CLOCKS_PER_SEC;
					timedif2 =(double)(timeend - timestart2) / CLOCKS_PER_SEC;
					timestart2 = clock();
					printf("month:%5d PopLive:%6d Brths:%4d Dths:%4d Mrgs:%3d Dvs:%3d Mq:%5d Fq:%d ti1: %.1f ti2: %.6f %.4f\r",
						current_month, size_of_pop[0],
						crnt_month_events[E_BIRTH],
						crnt_month_events[E_DEATH],
						crnt_month_events[E_MARRIAGE],
						crnt_month_events[E_DIVORCE],
						mqmales->num, mqfems->num,
						timedif1,timedif2,1000000000*timedif2/(mqmales->num * mqmales->num +1));
					if (current_month % 200==0){
						printf("\n");
						fprintf(fd_log,"month:%5d PopLive:%6d Brths:%4d Dths:%4d Mrgs:%3d Dvs:%3d Mq:%5d Fq:%d ti1: %.1f ti2: %.6f %.4f\n",
						current_month, size_of_pop[0],
						crnt_month_events[E_BIRTH],
						crnt_month_events[E_DEATH],
						crnt_month_events[E_MARRIAGE],
						crnt_month_events[E_DIVORCE],
						mqmales->num, mqfems->num,
						timedif1,timedif2,1000000000*timedif2/(mqmales->num * mqmales->num +1));
					}
				}
				if (size_of_pop[0] == 0 && !done)
				{
					printf("\n");
					done = 1;
				}
				fflush(stdout);
			}
		}
		
		//logmsg("|m5|", "", 1);
	    //Rprintf("-m|pa");
		fprintf(fd_log,"\nsegment %d complete current month: %d\n",	current_segment, current_month);
		printf("\nsegment %d complete current month: %d\n",	current_segment, current_month);
		//logmsg("%s\n", logstring, 1);

		if (take_census)
		{
			/** need to close fd_stat if census was taken */
			fclose(fd_stat);
		}

		/*population_pyramid();  */
		current_segment++;

		/*
	if (current_segment >  num_segments) {
	  marriage_tally();
	}
	*/

		/*marriage_tally(fd_log);  kill for now to prevent segfault 
	  Thu Jan 28 17:29:41 PST 2010 */

		/**
	 ** write out intermediate population files
	 ** 
	 **/
	
		logmsg(" grogro \n", "", 1);
		
	fprintf(fd_log, "agga lolo num_segments %i \n",num_segments);
	fprintf(fd_log, "agga lolo curr_segment %i \n",current_segment);
		if (current_segment < num_segments)
		{
			if (write_output)
			{
				write_popfiles(current_segment);
			}
		}

		if (current_segment <= num_segments)
		{
			/** 
	   ** current_segment has been incremented so this stuff
	   ** is going to happen before segment number current_segment
	   ** runs. won't this happen every segment ? do we need this
	   ** if() here?
	   **/

			sprintf(logstring, "Population Total / living : %d / %d",
					last_person->person_id, size_of_pop[0]);
			logmsg("%s\n", logstring, 1);
			logmsg("about to empty event queue\n", "", 0);

			printf("==================================================\n");
			printf("\n\nSetting up rates and queues for segment %d..\n\n",
				   current_segment);
			printf("--------------------------------------------------\n");

			logmsg("about to (re)initialize segment vars -----\n", "", 0);
			fprintf(fd_log,"about to (re)initialize segment vars fprintf");
			fflush(fd_log);
			/* dump_rates(2); */
			initialize_segment_vars();

			fprintf(fd_log,"after (re)initializing segment vars");
			fflush(fd_log);
			//logmsg("about read rate file %s\n", rate_file_name, 0);
			fprintf(fd_log,"about to read rate file %s\n", rate_file_name);
			fflush(fd_log);
			if (load(rate_file_name) < 0){
			  stop("could not read rate_file_name %s",rate_file_name);
			}
				
			adjust_birth_for_bint();

			if (fill_rate_gaps() < 0)
				perror("bad rate set\n");
		}
		else
		{
			/* This never executes.  */
			logmsg("exiting the segment loop\n", "", 0);
		}
	}
	logmsg("\n Simulation Complete \n", "", 1);

	fprintf(fd_log, "agga number of groups  %i \n",NUMBER_OF_GROUPS);
	fprintf(fd_log, "agga num_segments %i \n",num_segments);
	fprintf(fd_log, "agga curr_segment %i \n",current_segment);
	
	fprintf(fd_log, "agga read_xtra_file! %i \n",read_xtra_file);
	fprintf(fd_log, "agga read_xtra_file! %d \n",read_xtra_file);

	fprintf(fd_log, "males on queue\n");
	/* this looks like it's just for debugging*/
	if (marriage_queues == 1)
	{
		/*      inspect_entry(marriage_queue + MALE, MARRIAGE_QUEUE,fd_log);*/
	}

	fprintf(fd_log, "person-years on queue: %d\n", (int)time_waiting[MALE] / 12);
	fprintf(fd_log, "females on queue\n");
	/*inspect_entry(marriage_queue + FEMALE, MARRIAGE_QUEUE,fd_log);*/

	fprintf(fd_log, "person-years on queue: %d\n",
			(int)time_waiting[FEMALE] / 12);

	if (marriage_eval == DISTRIBUTION)
	{
		/**End of segment warning if marriage_eval==DISTRIBUTION and
	 the female marriage queue is NOT empty**/
		if (marriage_queues == 1)
		{
			struct queue_element *femq;
			femq = marriage_queue + 0;

			if (time_waiting[FEMALE] > 0)
			{
				char logstring[256];
				sprintf(logstring, "WARNING marriage_queues = 1; and marriage_eval == 'distribution'\n and yet females have failed to find matches immediately\n Months spent on queue:%d\n", time_waiting[FEMALE]);

				logmsg("%s\n", logstring, 1);
			}
		}

		evaluate_marmkt(fd_log);
	}

	fprintf(fd_log, "now printing popfiles..\n");
	write_popfiles(0);

	/*
    */
	sprintf(logstring, "total size of pop %d\n", last_person->person_id);
	logmsg("%s\n", logstring, 1);
	sprintf(logstring, "living size of pop %d\n", size_of_pop[0]);
	logmsg("%s\n", logstring, 1);
	for (i = 1; i <= numgroups; i++)
	{
		fprintf(fd_log, "living pop group %d  %d\n", i, size_of_pop[i]);
	}

	fprintf(fd_log, "now printing populationpyramid");
	population_pyramid(stdout);
	population_pyramid(fd_pyr);
	
	fclose(fd_pyr);
	fclose(fd_log);
	// fclose(fd_rn);
	Rcpp::Rcout << "\n\nSocsim Main Done" << std::endl;
	return 1;
	/*
    printf("done with population pyramid\n");
    */
	/*
    dump_queue();
    */
}

/********************************************************************/

void initialize_marriage_targets()
{
	/**New marriage pref infrastructure-- setup targets**/

	double sig2;			 /*variance of agedif distribution*/
	double female_older = 1; /*this screws things up 1 is a good value*/
	double fem_factor;

	int g, i, agediff;
	double pisig2, prob;
	double norm_tots[MAXGROUPS];

	/*  sig2=pow(agedif_marriage_sd,2);
      pisig2=sqrt((2*PI*sig2));*/

	for (g = 0; g < MAXGROUPS; g++)
	{
		norm_tots[g] = 0;

		sig2 = pow(agedif_marriage_sd[g], 2);
		pisig2 = sqrt((2 * PI * sig2));

		for (i = 0; i < 2 * MAXUYEARS; i++)
		{
			agediff = i - MAXUYEARS;

			/*headsup: female_older does not work well */
			fem_factor = (agediff < 0) ? female_older : 1;

			prob = fem_factor *
				   1 / pisig2 *
				   exp(-1 * pow((double)(agediff - agedif_marriage_mean[g]), 2) / (2 * sig2));

			/* playing
      prob= (agediff > -8 && agediff < 8)? 0.2:0;
      prob= (agediff >  -5 && agediff < -3 )? 0 : prob;
      prob= (agediff >  3  && agediff < 5 )? 0 : prob;
      */
			prob = (ABS(prob) < .0001) ? -1 : prob; /*thin the tails*/
			marriage_agetarget[g][i] = prob;
			norm_tots[g] += (prob > 0) ? prob : 0; /*prob can be negative fem_factor */
		}
	}

	for (g = 0; g < MAXGROUPS; g++)
	{
		for (i = 0; i < (2 * MAXUYEARS); i++)
		{
			marriage_agetarget[g][i] = (marriage_agetarget[g][i] > 0) ? marriage_agetarget[g][i] / norm_tots[g] : marriage_agetarget[g][i];
		}
	}

	/*
    for(i=0;i<(2*MAXUYEARS);i++){
  printf("%d  %f\n",i,marriage_agetarget[1][i]);
      }
  */

} /*end initialize_marriagetargets*/

/********************************************************************/
int process_month()
{
  
  //logmsg("|pm|", "", 1);
	int i, g, s;
	struct queue_element *e;
	struct person *p;

	i = current_month % MAXUMONTHS;
	e = event_queue + i;

	/*
    printf("processing month %d on queue %d\n", current_month, e->num);
    */

	/*
    printf("value of i %d and number on queue %d\n", i, e->num);
    */
   
	//printf ("3.1 process month %d\n", "");
	while (e->num > 0)
	{
		int nth;

		/*
	inspect_entry(e, EVENT_QUEUE);
	*/
		p = e->first;
		nth = 1;
		if (e->num > 1)
		{
			/* nth can be viewed as the number of times
			       to advance the pointer, with a max of 
			       nth - 1 
			    */

			/*
	    printf("queue\n");
	    printf ("got this nth from the draw %d\n", nth);
	    */
	   		//here was the error (possibly related to the different random number generation on windows and linux)
	   		double rr = rrandom(); // without this: segfault on windows,
			   				// because sometimes nth ==e->num and the loop will go one person too far
			nth = (int)(rr * e->num);
			if(nth==e->num){
				nth--;
				printf("\nError!? nth==e->num, %d; person p will be undefined.\n",nth);
				fflush(stdout);
			}
			while (--nth >= 0)
			{
				p = p->NEXT_PERSON;
			}
		}
		//logmsg("|pm1|", "", 1);

	//printf ("3.2process month %d\n", "");
		/*
	printf("event queue trying to delete person id %d \n",
	    p->person_id);
	*/

		queue_delete(p, EVENT_QUEUE);

	//printf ("3.3 process month %d\n", "");
		/*
	printf("deleted person id %d mstatus %s next event %s\n",
	    p->person_id, index_to_mstatus[p->mstatus],
	    index_to_event[p->next_event]);
	fflush(stdout);
	*/

		// logmsg("next event all -", "", 1);
		if (p->next_event == E_DEATH)
		{
			//logmsg("next event... death 1  \n", "", 1);
			death(p);
		}
		else if (p->next_event == E_BIRTH)
		{

#ifdef ENHANCED
			enhance_birth_pre(p);
#else
			//logmsg("next event... birth 1  \n", "", 1);
			birth(p);
#endif
		}
		else if (p->next_event == E_DIVORCE)
		{
			// logmsg("next event... divorce 1  \n", "", 1);
			divorce(p);
		}
		else if (p->next_event == E_MARRIAGE)
		{
			//logmsg("next event... marriage\n", "", 1);
			marriage(p);
		}
		else if ((p->next_event >= TRANSIT1) &&
				 (p->next_event < TRANSIT1 + numgroups))
		{
			//logmsg("next event... transit   \n", "", 1);
			transit(p);
		}

		/*
	printf("done with person id %d\n", p->person_id);
	*/

		/*
	inspect_entry(e, EVENT_QUEUE);
	*/
	}

	/*
    printf("done with queue \n");
    */

	/* prepare current month slot for MAXUMONTHSth kt_val*/
	
	//Rprintf("|");
	//logmsg("|pm8|", "", 1);
	for (g = 1; g <= numgroups; g++)
	{
		//Rprintf("g#");
	  
	  //logmsg("|g#|", "", 1);
		for (s = 0; s < NUMSEXES; s++)
		{
			if (lc_rate_set[g][s] != NULL)
			{
				struct lc_rates *lc = lc_rate_set[g][s];
				double prev;

				/* did this get updated earlier in the loop? */
				if (lc == lc_rate_set[g - 1][s])
					continue;

				/*
		printf("current month %d group %d sex %d 1801 %lf 1812 %lf \n",
		    current_month, g, s, *(kt_vals[g][s] + 601),
		    *(kt_vals[g][s] + 612));
		*/
				prev = *(kt_vals[g][s] + ((current_month - 1) % MAXUMONTHS));
				if ((int)current_month / 12 == (int)(current_month - 1) / 12)
				{
					*(kt_vals[g][s] + (current_month % MAXUMONTHS)) = prev;
					continue;
				}
				if (current_kt_index[g][s] > last_kt_index[g][s])
				{
					*(kt_vals[g][s] + (current_month % MAXUMONTHS)) =
						prev + lc->mean + lc->std_dev * normal();
					/*
			prev + lc->mean ;
			*/
					/*

		    */
				}
				else
				{
					*(kt_vals[g][s] + (current_month % MAXUMONTHS)) =
						read_kt_vals[g][s][current_kt_index[g][s]];
					current_kt_index[g][s]++;
					/*
		    printf("read--current intdex %d\n", current_kt_index[g][s]);
		    */
				}

				/*
		printf("prev %lf lc->mean %lf\n", prev,  lc->mean);
		printf("next value %lf\n",
		    *(kt_vals[g][s] + (current_month % MAXUMONTHS)));
		*/
			}
		}
	}
	//Rprintf(":");
	
	//logmsg(":", "", 1);
	if (random_epsilon || fixed_epsilon)
	{
	  //logmsg(":3\n", "", 1);
		epsilon = (epsilon + 1) % (max_e_index + 1);
	}
	//logmsg("(2)\n", " ", 1);
  
  return 1;
	/*
    printf("done with new kt processing\n");
    */
}

void new_events_for_all()
{
	struct person *p;
	int m;

	p = person0;

	while (p != NULL)
	{
		// detect segfault by testing some things (but it shouldnt happen here):
		if (!(p->sex==0 || p->sex==1)){
			printf("\n\nError, invalid person, sex = %d\n",p->sex);
		}
		if (p->deathdate != 0)
		{
			/*
	  printf("person %d is dead\n", p->person_id);
	*/
			p->next_event = E_NULL;
			p = p->down;
			continue;
		}

		/**
	 printf("id / marital status %d / %d\n",p->person_id, p->mstatus);
      **/

		if ((marriage_queues == 1) && (p->mstatus <= WIDOWED) && (p->sex == MALE))
		{

			/* this will put all men on the marriage queue and will generate
	   a new event for them */
			install_in_order(p, marriage_queue + p->sex, MARRIAGE_QUEUE);
			p->mqueue_month = current_month;
		}
		/** Normal situation **/
		m = date_and_event(p);
		/** print event before scheduling**
	  if(p->next_event == E_BIRTH){
	  char logstring[256];
	  sprintf(logstring,
	  "pid: %d age: %d event: %d age at e date: %d",
	  p->person_id, (current_month - p->birthdate),
	  p->next_event,(m - p->birthdate));
	  logmsg("%s\n",logstring,1);
	  }
	  /** end print shtick**/
		m %= MAXUMONTHS;
		install_in_order(p, event_queue + m, EVENT_QUEUE);
		/**
	 inspect_entry(event_queue + m, EVENT_QUEUE);
	 printf("person_id %d\n", p->person_id);
	 person_id 42895
      **/

		p = p->down;
	}
}

/*
mqueue_install(p, q)
    struct person *p;
    struct queue_element *q;
{

    if (q->first == NULL) {
	q->first = p;
	q->num++;
	p->pointer_type[MARRIAGE_QUEUE] = PTR_Q;
	p->MQUEUE = q;
    } else {
	q->num++;
	p->pointer_type[MARRIAGE_QUEUE] = PTR_N;
	p->NEXT_ON_MQUEUE = q->first;
	q->first = p;
    }

}
*/

void install_in_orderNOTWRONGBUTSLOW(struct person *p, struct queue_element *e, int q_type)
	/** 1/13/14 This is Marcia's routine that puts individiduals on queues
in personid/birth order.  There does not appear to be any reason other
than esthetics for people to be installed in order since all events
are executed in random order and marriage requires a working queue to
be constructed. Perhaps I am wrong and this will blow up, but the sample
simulation works without sorting and generally things run much faster so
this is going into purgatory  **/

	/** installs people onto the queue in person_id order **/

//	struct person *p;
//struct queue_element *e;
//int q_type;
{
	struct person *after, *before;
	int j;

	/* printf("trying to install %d\n", p->person_id); */

	if (e->first == NULL)
	{
		e->num++;
		e->first = p;
		p->pointer_type[q_type] = PTR_Q;
		SET_NEXT_NODE(p, e);
		/* printf("at head\n"); */
		return;
	}
	else if (e->first->person_id > p->person_id)
	{
		e->num++;
		p->pointer_type[q_type] = PTR_N;
		SET_NEXT_ELEMENT(p, e->first);
		e->first = p;
		/* printf("at head, non-null\n"); */
		return;
	}
	else
	{
		after = e->first;
		before = e->first;
		j = 1;
		while ((after->person_id < p->person_id) && (j < e->num))
		{
			before = after;
			after = NEXT_ELEMENT(after);
			j++;
		}
		if (j < e->num || after->person_id > p->person_id)
		{
			p->pointer_type[q_type] = PTR_N;
			SET_NEXT_ELEMENT(p, after);
			SET_NEXT_ELEMENT(before, p);
			e->num++;
			/* printf("internal\n"); */
			return;
		}
		else if (j == e->num)
		{
			p->pointer_type[q_type] = PTR_Q;
			SET_NEXT_NODE(p, e);
			after->pointer_type[q_type] = PTR_N;
			SET_NEXT_ELEMENT(after, p);
			e->num++;
			/* printf("at end\n"); */
			return;
		}
		else
		{
		  printf("unknown queue circumstances\n");
		  stop("unknown queue circumstances\n");//exit(1);
		}
	}
}
/******************************************/
void install_in_order(struct person *p, struct queue_element *e, int q_type)
	/** installs people onto the queue in person_id order **/
	/** 1/13/14 EXCEPT THAT IT DOESN't this replaces Marcia's old
routine. It simply puts everyone at the front of the queue. It works
and is much faster  **/

{
	struct person *after, *before;

	/* printf("trying to install %d\n", p->person_id); */

	if (e->first == NULL)
	{
		/*No one in queue yet*/
		e->num++;
		e->first = p;
		e->last = p;
		p->pointer_type[q_type] = PTR_Q;
		SET_NEXT_NODE(p, e);
		/* printf("at head\n"); */
		return;
	}
	/**  let's not bother with pid ordering; the queue exists and p
	 joins as first member**/
	e->num++;
	p->pointer_type[q_type] = PTR_N;
	SET_NEXT_ELEMENT(p, e->first);
	e->first = p;
	/* printf("at head, non-null\n"); */
	return;
}
/****************************/
void dump_queue()
{
	int i, j;
	struct person *p;
	struct queue_element *e;

	for (e = event_queue, i = 0; i < MAXUMONTHS; e++, i++)
	{
		if (e->num)
		{
			printf("month entry %d\n", i);
			for (p = e->first, j = 1; j < e->num; p = p->NEXT_PERSON, j++)
			{
				printf("      id %d age %d mult %f\n",
					   p->person_id, (current_month - p->birthdate), p->fmult);
			}
			printf("      id %d age %d mult %f\n",
				   p->person_id, (current_month - p->birthdate), p->fmult);
		}
	}
}

void inspect_entry(struct queue_element *e, int q_type, FILE *fd)

{
	printf( "inspect_entry 0 number on queue %d--in id: group format\n", e->num);
			
	struct person *p;
	int i;

/*
	if(q_type==0){
	    printf("qtype0 - person");
	} else {
		printf("q_type123 - marriage");
	}
  fflush(stdout);
*/
	if (e->num < 1)
	{
		fprintf(fd, "empty queue\n");
		return;
	}
	else
	{
		fprintf(fd, "number on queue %d--in id: group format\n", e->num);
		
		for (p = e->first, i = 1; i < e->num; i++)
		{
			// p = ((q_type) == 0)? (p)->u_event_queue.next_person: (p)->u_marriage_queue.next_on_mqueue;
			if(q_type==0){
				p = p->u_event_queue.next_person;
			} else {
				p = p->u_marriage_queue.next_on_mqueue;
			}

			fprintf(fd, " z%d-%d : %d\t", i,p->person_id, p->group);
		}

		fprintf(fd, "%d : %d\n", p->person_id, p->group);

		fprintf(fd, "current i %d\n", i);
		
    	//fflush(stdout);
	}
	fprintf(fd, "\n");
}

void queue_delete( struct person *p, int q_type)
{
	struct person *before, *p1;
	struct queue_element *e;
	int q_d_verbose = 0;
	/*    if(current_segment >=2) q_d_verbose=1; */

	if (q_d_verbose)
	{
		printf("deleting person %d from q type %d on %d\n",
			   p->person_id, q_type, current_month);
	}

	// detect undefined person by testing wether the sex is valid:
	if (!(p->sex==0 || p->sex==1)){
		printf("\n\nError, invalid person, sex = %d\n",p->sex);
		fflush(stdout);
	}
	if (p->pointer_type[q_type] == PTR_NULL)
	{
		printf("person %d current month %d\n", p->person_id, current_month);
		fflush(stdout);
		perror("trying to delete something with a null queue ptr\n");
	}

	/*
    if (NEXT_ELEMENT(p) == NULL) 
	perror("trying to delete something with a null next ptr\n");
    */

	p1 = p;
	while (p1->pointer_type[q_type] != PTR_Q)
	{
		if (q_d_verbose >4)
			printf("in queue_delete: finding node %d\n", p1->person_id);
		p1 = NEXT_ELEMENT(p1);
	}

	e = NEXT_NODE(p1);

	if (q_d_verbose)
	{
		printf("in queue_delete, before deletion: num on queue %d\n", e->num);
		fprintf(fd_log, "before inspect-entry, %d-\n",p1->person_id);
		inspect_entry(e, q_type, fd_log);
		printf("gagain queue_delete, before deletion: after inspecting entry\n");
	}

	if (e->first == p)
	{
		if (q_d_verbose)
			printf("in queue_delete: deleting first element\n");
		if (e->num == 1)
		{
			e->first = NULL;
		}
		else
		{
			e->first = NEXT_ELEMENT(p);
		}
	}
	else
	{
		if (q_d_verbose)
			printf("in queue_delete: deleting non-first element\n");
		before = e->first;
		while (p1 = NEXT_ELEMENT(before),
			   p1->person_id != p->person_id)
		{
			//printf("in queue_delete: looooping!t\n");
			// fflush(stdout);
			before = NEXT_ELEMENT(before);
			if (q_d_verbose>4)
				printf("in queue_delete: looping is at %d\n", p1->person_id);
		}
		if (q_d_verbose)
			printf("in queue_delete: found before element\n");
		if (p1->pointer_type[q_type] == PTR_Q)
		{
			if (q_d_verbose)
				printf("in queue_delete: before element is penult\n");
			before->pointer_type[q_type] = PTR_Q;
			SET_NEXT_NODE(before, e);
		}
		else
		{
			if (q_d_verbose)
				printf("in queue_delete: before element is ordinary\n");
			SET_NEXT_ELEMENT(before, NEXT_ELEMENT(p));
		}
	}

	if (p == NULL){
		perror("bad pointer returned from queue delete");
		printf("peroor - p==Null! ");
	}
	p->pointer_type[q_type] = PTR_NULL;
	SET_NEXT_ELEMENT(p, NULL);
	e->num--;

	if (q_d_verbose)
	{
		printf("in queue_delete, after deletion: num on queue %d\n", e->num);
		printf("in queue_delete, after deletion: inspecting entry\n");
		inspect_entry(e, q_type, fd_log);
	}
}

/** modify_rates loops through the entire rate block and 
    modifies rates per dmult/fmult/tmult
**/
void modify_rates(int event,struct age_block *first_block,struct person *p)
  /*int event;
    struct age_block *first_block;
    struct person *p;*/
{

	struct age_block *crnt_block;

	crnt_block = first_block;

	while (crnt_block != NULL)
	{
		int event_sw = NUMEVENTS + 1;
		/*
	printf("modify_rates: current event %s\n", index_to_event[event]);
	printf("original lambda %f\n", crnt_block->lambda);
	*/
		if (event == E_BIRTH)
		{
			event_sw = 1;
		}
		else if (event < NUMEVENTS)
		{
			event_sw = 0;
		}
		switch (event_sw)
		{

		case 0:
			crnt_block->modified_lambda =
				crnt_block->mult(p, crnt_block) *
				rate_factors[p->group][event][p->sex][p->mstatus];
			break;

			/*      case E_BIRTH: */
		case 1:
			crnt_block->modified_lambda =
				crnt_block->mult(p, crnt_block) * birth_adjust[p->group] *
				birth_rate_factors[p->group][p->mstatus];
			/* Obsolete Renewal-Reward Therom based adjustment? *
	if (1.0 /-crnt_block->modified_lambda > bint) {
	  crnt_block->modified_lambda =
	    -1.0/((1.0/-crnt_block->modified_lambda) - bint);
	    } */
			if (crnt_block->modified_lambda > -0.000000000000001)
				crnt_block->modified_lambda = -0.000000000000001;
			break;
		default:
			perror("unknown rate in rate block");
		}
		if (global_verbose>4)
		{
			printf("modified lambda %f\n", crnt_block->modified_lambda);
		}
		crnt_block = crnt_block->next;
	}
}

int	datev(struct age_block *first_block,int age, int time_shift)
{
	int waiting_time, duration;
	long long int itemp;
	struct age_block *crnt_block;
	double logu, u;
	int first_possible_month;
	double temp;

	/*
    printf("entering datev\n");
    fflush(stdout);
    */

	if (first_block == NULL)
		perror("datev: got here with a null rate set\n");

	/*    age =  MIN(age + time_shift, 1199);*/
	age = MIN(age + time_shift, (MAXUMONTHS - 1));
	first_possible_month = current_month + time_shift;
	crnt_block = first_block;

	/*
    printf("current age %d\n", age);
    */
	while (age >= crnt_block->upper_age)
	{
		crnt_block = crnt_block->next;
	}

	/*
    printf("event\n");
    */
	u = rrandom();

	while (u <= 0)
	{
		/*
	printf("event\n");
	*/
		u = rrandom();
	}

	logu = log(u);
	if (global_verbose>2)
	{
		printf("log u: %f\n", logu);
	}
	waiting_time = 0;

	while (logu < 0)
	{
		duration = crnt_block->upper_age - age;
		if (crnt_block->previous != NULL)
			if (age < crnt_block->previous->upper_age)
				duration = crnt_block->width;

		temp = crnt_block->modified_lambda;

		/*
	printf("temp %g\n", temp);
	*/
		temp = logu / temp;
		itemp = (long long int)temp;
		if (itemp > duration)
			waiting_time += duration;
		else
			waiting_time += itemp;

		if (global_verbose>3)
		{
			printf("current mod lambda: %g\n", crnt_block->modified_lambda);
			printf("waiting time: %d\n", waiting_time);
		}
		/*
	*/

		logu = logu - (duration * crnt_block->modified_lambda);
		if (global_verbose)
		{
			printf("log u: %G\n", logu);
		}
		if ((crnt_block->next == NULL) && (logu < 0))
		{
			waiting_time = (MAXUMONTHS - 1) - age;
			logu = 0;
		}
		else
			crnt_block = crnt_block->next;
	}

	/*
      printf("first_possible_month %d waiting_time %d\n",
          first_possible_month, waiting_time);
    */

	/*
    printf("exiting datev\n");
    */

	return first_possible_month + waiting_time;
};

int lc_datev(int g, int s, int age,struct person *p)
{
	int waiting_time;
	double logu, u;
	int first_possible_month;
	double temp_rate;
	double ax, bx, kt, ext;
	int ax_bx_index, kt_index, ep = epsilon;

	if (lc_rate_set[g][s] == NULL)
		perror("lc_datev: got here with a null rate set\n");

	ax_bx_index = age;
	kt_index = current_month % MAXUMONTHS;

	/*
    epsilon = irandom() % (max_e_index + 1);
    */

	if (random_epsilon || fixed_epsilon)
	{
		ep = epsilon;
		ext = table_lookup(ax_bx_index, lc_epsilon[1][p->sex][ep]);
	}
	else
	{
		ext = 0;
	}

	/*
    printf("index %d, age %d, ext %lf\n", ep, ax_bx_index, ext);
    */

	first_possible_month = current_month;

	/*
    printf("current age %d\n", age);
    */

	u = rrandom();

	while (u <= 0)
	{
		u = rrandom();
	}

	logu = log(u);
	if (global_verbose)
	{
		printf("log u: %f\n", logu);
	}
	waiting_time = -1;

	/* note that calculation stops automatically
		 * when age at death reaches MAXUMONTHS, via the index */

	while ((logu < 0) && (ax_bx_index < MAXUMONTHS))
	{
		ax = *(ax_vals[g][s] + ax_bx_index);
		bx = *(bx_vals[g][s] + ax_bx_index);
		kt = *(kt_vals[g][s] + kt_index);

		/*
	kt = *(kt_vals[g][s] + kt_index) +
	    lc_rate_set[g][s]->mean * (double) p->egos_extra->factor;
	*/

		/*
	temp_rate = exp(ax + bx * kt);
	ext = 0;
	*/

		temp_rate = exp(ax + bx * kt + ext);

		if (log(1 - temp_rate) / 12 > -0.0000001)
			temp_rate = -0.0000001;
		else
			temp_rate = log(1 - temp_rate) / 12;

		/*
	if (global_verbose)
	    printf("temp_rate %g logu %g ax %g bx %g kt %g\n", temp_rate, logu,
		    ax, bx, kt);
	*/

		logu = logu - temp_rate;

		waiting_time++;
		ax_bx_index++;
		kt_index = (kt_index + 1) % MAXUMONTHS;

		if (random_epsilon || fixed_epsilon)
		{
			ep = (ep + 1) % (max_e_index + 1);
			ext = table_lookup(ax_bx_index, lc_epsilon[1][p->sex][ep]);
		}
		else
		{
			ext = 0;
		}

		/*
	printf("index %d, age %d, ext %lf\n", ep, ax_bx_index, ext);
	*/
	}

	/*
      printf("first_possible_month %d waiting_time %d\n",
          first_possible_month, waiting_time);
      */

	return first_possible_month + waiting_time;
};

/*************************************************************************/

int date_and_event(struct person *p)
{
	int m, m1, i;
	int delta;
	struct age_block *first_block;

	/*
    printf("entering d_event\n");
    printf("relevant rate set: sex %s mstatus  %s \n",
	index_to_sex[p->sex], index_to_mstatus[p->mstatus]);
    */

	/* generate a death date */
	/*
    printf("death rate set: group %d sex %s mstatus  %s \n",
	p->group, index_to_sex[p->sex], index_to_mstatus[p->mstatus]);
    */

	/*
    write_person(p, stdout);
    printf("death rate set\n");
    */

	/* anyone whose current age = MAXUMONTHS-1 must die in the current
       month We need to police this b/c geriatric married people can
       have new events generated during their 1199th month if their
       spouse dies before their own scheduled death is executed. */

	if ((current_month - p->birthdate) == (MAXUMONTHS - 1))
	{
		p->next_event = E_DEATH;
		return (current_month);
	}

	if (lc_rate_set[p->group][p->sex] == NULL)
	{
		first_block = rate_set[p->group][E_DEATH][p->sex][p->mstatus];
		modify_rates(E_DEATH, first_block, p);
		m = datev(first_block, current_month - p->birthdate, 0);
	}
	else
	{
		m = lc_datev(p->group, p->sex, current_month - p->birthdate, p);
	}

	/*
	printf("death rate set: sex %s mstatus  %s \n",
	index_to_sex[p->sex], index_to_mstatus[p->mstatus]);
	printf("death of %d scheduled for %d\n", p->person_id, m);
    */

	p->next_event = E_DEATH;

	/*
    printf("birth rate set\n");
    */
	/* if female, generate a birth date */
	if (p->sex == FEMALE)
	{
		int par;
		int next_pos_birth;
		/*
	par = p->egos_extra->parity;
	*/
		par = get_parity(p);
		/*
	printf("rate set: %d %s %d\n",
		p->group, index_to_mstatus[p->mstatus], par);
	*/
		/* use bint for next poss birth */
		if (p->lborn == NULL)
		{
			next_pos_birth = current_month;
		}
		else
		{
			next_pos_birth =
				MAX(p->lborn->birthdate + (int)bint, current_month);

			/* if death is scheduled at current month, don't
			     * even calculate next birth */
			/*
		printf("%d birth for %d aged %d on %d death on %d\n",
		    current_month, p->person_id,
		    current_month - p->birthdate, next_pos_birth);
	    */
		}

		first_block =
			birth_rate_set[p->group][p->mstatus][MIN(par, MAXPARITY - 1)];
		modify_rates(E_BIRTH, first_block, p);

		/**printing fert rate blocks to verify that all are the smae
	   if hetfert==0**/
		if (0)
		{
			struct age_block *block_ptr;
			char logstring[256];
			block_ptr = first_block;
			while (block_ptr != NULL)
			{
				sprintf(logstring, "up: %d  width: %d lam: %lf  modlam: %lf\n",
						block_ptr->upper_age, block_ptr->width, block_ptr->lambda,
						block_ptr->modified_lambda);
				logmsg("%s", logstring, 1);
				block_ptr = block_ptr->next;
			}
		} /**end of stupid printing temp**/

		m1 = datev(first_block, current_month - p->birthdate,
				   next_pos_birth - current_month);
		/*
	    printf("remaining bint %d \n", next_pos_birth - current_month);
	   printf("birth for %d scheduled for %d\n", p->person_id, m1);
	    printf("%d birth for %d aged %d on %d death on %d\n",
		current_month, p->person_id,
		current_month - p->birthdate, m1, m);
	*/
		if (m1 < m)
		{
			p->next_event = E_BIRTH;
			m = m1;
		}
	}
	/* generate a marriage date */
	/*
	printf("marriage of %d scheduled for %d\n", p->person_id, m1);
	printf("marriage rate set\n");
    */

	/*
    if (p->mstatus == COHABITING) {
	first_block  = rate_set[p->group][E_MARRIAGE][p->sex][p->mstatus];
	modify_rates(E_MARRIAGE, first_block,  p);
	m1 = datev(first_block,
		current_month - p->last_marriage->date_start, 0);
	if (m1 < m) {
	    p->next_event = E_MARRIAGE;
	    m = m1;
	}
    } else {
    }
    */
	/* Wed Apr 17 17:00:43 PDT 2013
       is there a reason that a person on the marrque should NOT
       have a marriage event? -- s/he could get put on the queue again 
       and that could be bad -- but s/couldn't s/he have another search
       Going to change this for the one queue marriage plan
    */
	/*  if ((p->NEXT_ON_MQUEUE == NULL) &&*/
	if ((p->NEXT_ON_MQUEUE == NULL || marriage_queues == 1) &&
		(rate_set[p->group][E_MARRIAGE][p->sex][p->mstatus] != NULL))
	{
		/*
	printf("marriage rate set: group %d sex %s mstatus  %s \n",
	    p->group, index_to_sex[p->sex], index_to_mstatus[p->mstatus]);
	*/
		first_block = rate_set[p->group][E_MARRIAGE][p->sex][p->mstatus];
		if (duration_specific[p->group][E_MARRIAGE][p->sex][p->mstatus])
		{
			if (p->mstatus == COHABITING)
				delta = current_month - p->last_marriage->date_start;
			else
				delta = current_month - p->last_marriage->date_end;
		}
		else
		{
			delta = current_month - p->birthdate;
		}
		modify_rates(E_MARRIAGE, first_block, p);
		m1 = datev(first_block, delta, 0);
		if (m1 < m)
		{
			p->next_event = E_MARRIAGE;
			m = m1;
			if (marriage_queues == 1 && p->sex == MALE)
			{
			  //logmsg("\nmarriage_queues==1 yet marriage event gen'ed for %d\n"," ", p->person_id);
			  fprintf(fd_log,"\nmarriage_queues==1 yet marriage event gen'ed for %d\n",p->person_id);
			  stop("\nmarriage_queues==1 yet marriage event gen'ed for %d\n",p->person_id);//exit(-1);
			}
		}
	}

	if ((p->mstatus == MARRIED) || (p->mstatus == COHABITING))
	{
		/*
	printf("divorce rate set: group %d sex %s mstatus  %s \n",
	    p->group, index_to_sex[p->sex], index_to_mstatus[p->mstatus]);
	*/
		first_block = rate_set[p->group][E_DIVORCE][p->sex][p->mstatus];
		modify_rates(E_DIVORCE, first_block, p);
		m1 = datev(first_block,
				   current_month - p->last_marriage->date_start, 0);
		if (m1 < m)
		{
			p->next_event = E_DIVORCE;
			m = m1;
		}
	}

	/*
    printf("migration rate set(s), if any\n");
    */
	/* all relevant transition events are possible */
	for (i = 1; i <= numgroups; i++)
	{
		if ((first_block =
				 rate_set[p->group][NONTRANSIT + i][p->sex][p->mstatus]) != NULL)
		{

			/**Wed Jun 23 06:19:16 PDT 2010 adding option for duration spec. 
	     transit rates **/

			if (
				(duration_specific[p->group][NONTRANSIT + i][p->sex][p->mstatus]) && (p->ltrans != NULL))
			{
				/* if the rate is durations specific AND a transition has 
		 occurred since birth...*/
				delta = current_month - p->ltrans->date;
			}
			else
			{
				/* otherwise use birthdate as starting point of duration */
				delta = current_month - p->birthdate;
			}
			modify_rates(NONTRANSIT + i, first_block, p);

			/*	
		printf("using %d %s %s %s\n", p->group, index_to_sex[p->sex],
		index_to_mstatus[p->mstatus], index_to_event[NONTRANSIT + i]);
		*/
			/* m1 = datev(first_block, current_month - p->birthdate, 0);*/
			m1 = datev(first_block, delta, 0);
			if (m1 < m)
			{
				p->next_event = NONTRANSIT + i;
				m = m1;
			}
		}
	}

	/*
    if (p->person_id == 11047)
    printf("next event for %d is %s\n",
    p->person_id, index_to_event[p->next_event]);
    
    printf("exiting date_and_event\n");
    */

	/*
      if (m == current_month) 
      printf("event scheduled for current month %s\n",
      index_to_event[p->next_event]);
    */
	return m;
}

void initialize_segment_vars()
{
	int g, gd, e, s, m, p, i;
	struct age_block *b, *bb, **r_ptr, **s_ptr;

	struct age_table *t, *tt;

	struct lc_rates **lc_ptr, **alc_ptr;

	double **ax_ptr, **bx_ptr, **kt_ptr, **aax_ptr, **abx_ptr, **akt_ptr;

	/**
   ** This code replaces something more  complex fromthe Feitel
   ** era. Instead of unpacking the rate pointers according to
   ** inheritance rules, this just starts at zero and looks for things
   ** that point to each cell. such things are set to NULl then the
   ** original cell is cfreed. This prevents cfree of already
   ** dereferenced stuff. bald spot dedicated to this problem
   **/
	//warning("beginning");fprintf(fd_log,"begi1");
	for (g = 0; g < MAXGROUPS * NUMEVENTS * NUMMARSTATUS * NUMSEXES; g++)
	{
		r_ptr = (&(rate_set[0][0][0][0]) + g);
		if (*r_ptr != NULL)
		{

			for (e = g + 1; e < MAXGROUPS * NUMEVENTS * NUMMARSTATUS * NUMSEXES; e++)
			{
				s_ptr = (&(rate_set[0][0][0][0]) + e);
				if (*s_ptr == *r_ptr)
				{
					*s_ptr = NULL;
				}
			}
			b = *r_ptr;
			while ((bb = b->next) != NULL)
			{
				/* */ free(b);
				b = bb;
			}
			/* */ free(b);
			*r_ptr = NULL;
		}
	}
	//warning("beginning2");fprintf(fd_log,"begi2");

	/******************************
   ** Mon Jul 29 14:56:02 PDT 2002 this replaces code formerly above
   **and below this  point which (1) unwound the birth_rate_set
   **according to inheritance rules and then cfreed. That proved
   **problematic as elements of birth_rate_set[] would get freed more
   **than once -- solaris gcc is suprisingly robust to this, linux gcc
   **compiles fine of course but seg faults 
   *********************************/

	for (g = 0; g < MAXGROUPS * NUMMARSTATUS * MAXPARITY; g++)
	{
		r_ptr = (&(birth_rate_set[0][0][0]) + g);
		if (*r_ptr != NULL)
		{

			for (e = g + 1; e < MAXGROUPS * NUMMARSTATUS * MAXPARITY; e++)
			{
				s_ptr = (&(birth_rate_set[0][0][0]) + e);
				if (*s_ptr == *r_ptr)
				{
					*s_ptr = NULL;
				}
			}
			b = *r_ptr;
			while ((bb = b->next) != NULL)
			{
				/* */ free(b);
				b = bb;
			}
			/* */ free(b);
			*r_ptr = NULL;
		}
	}
	//warning("begi3"); fprintf(fd_log,"begi3");
	for (g = 1; g < MAXGROUPS; g++)
	{
		for (m = 0; m < NUMMARSTATUS; m++)
		{
			birth_rate_factors[g][m] = 1;
		}
	}
	
	//warning("begi4");
	/* zero the duration_specific rates and rate factors */
	for (g = 0; g < MAXGROUPS; g++)
		for (e = 0; e < NUMEVENTS; e++)
			for (s = 0; s < NUMSEXES; s++)
				for (m = 0; m < NUMMARSTATUS; m++)
				{
					duration_specific[g][e][s][m] = 0;
					rate_factors[g][e][s][m] = 1;
				}

	/**************************
   ** Tue Sep 17 13:11:02 PDT 2002
   ** Just as for the simpler rate sets, It is necessary to
   ** rewrite the reinititialization routines in order to avoid
   ** releasing memory twice.  Same algorithm as described above: 
   ** march down the array and looking for higher indexed pointers to the 
   ** same memory location -- lc_rate_set needs to be cfree'ed but not
   ** unwound as carefuly as other rate sets
   **  ax_vals,bx_vals,kt_vals are 
   ** just arrays of ptrs to doubles. 
   ****************************/
	
	//warning("begi4.2");
	for (g = 0; g < MAXGROUPS * NUMSEXES; g++)
	{
		lc_ptr = (&(lc_rate_set[0][0]) + g);

		ax_ptr = (&(ax_vals[0][0]) + g);
		bx_ptr = (&(bx_vals[0][0]) + g);
		kt_ptr = (&(kt_vals[0][0]) + g);

		for (e = g + 1; e < MAXGROUPS * NUMSEXES; e++)
		{
			alc_ptr = (&(lc_rate_set[0][0]) + e);

			aax_ptr = (&(ax_vals[0][0]) + e);
			abx_ptr = (&(bx_vals[0][0]) + e);
			akt_ptr = (&(kt_vals[0][0]) + e);

			if (*lc_ptr == *alc_ptr) // march2020 "=" --> "==" it is a comparison, not an assignment, right?
			{
				*alc_ptr = NULL;
			}
			if (*ax_ptr == *aax_ptr)
			{
				*aax_ptr = NULL;
			}
			if (*bx_ptr == *abx_ptr)
			{
				*abx_ptr = NULL;
			}
			if (*kt_ptr == *akt_ptr)
			{
				*akt_ptr = NULL;
			}
		}

		if (*lc_ptr != NULL)
		{
			/* */ free(*lc_ptr);
			*lc_ptr = NULL;
		}

		if (*ax_ptr != NULL)
		{
			free(*ax_ptr);
			*ax_ptr = NULL;
		}
		if (*bx_ptr != NULL)
		{
			free(*bx_ptr);
			*bx_ptr = NULL;
		}
		if (*kt_ptr != NULL)
		{
			free(*kt_ptr);
			*kt_ptr = NULL;
		}
	}
	
	//warning("begi5");
	for (g = 1; g < MAXGROUPS; g++)
	{
		for (s = 0; s < NUMSEXES; s++)
		{
			if (cohab_probs[g][s] != NULL)
			{
				t = cohab_probs[g][s];
				while ((tt = t->next) != NULL)
				{
					/* */ free(t);
					t = tt;
				}
				/* */ free(t);
				cohab_probs[g][s] = NULL;
			}
		}
	}
	
	//warning("begi6");

	for (g = 1; g < MAXGROUPS; g++)
	{
		for (s = 0; s < NUMSEXES; s++)
		{
			for (i = 0; i < MAXKT; i++)
			{
				if (lc_epsilon[g][s][i] != NULL)
				{
					t = lc_epsilon[g][s][i];
					while ((tt = t->next) != NULL)
					{
						/* */ free(t);
						t = tt;
					}
					/* */ free(t);
					lc_epsilon[g][s][i] = NULL;
				}
			}
		}
	}
	
	//warning("begi7");fprintf(fd_log,"begi7");
	for (g = 1; g < MAXGROUPS; g++)
	{
		birth_target[g] = -1;
		birth_adjust[g] = 1.0;
		for (gd = 1; gd < MAXGROUPS; gd++)
		{
			transit_target[g][gd] = -1;
			transit_adjust[g][gd] = 1.0;
		}
	}

	//warning("gebi 8");fprintf(fd_log,"begi8");
	/* empty all the queues, if segment > 1 */

	if (current_segment >= 1)
	{
		struct queue_element *e, *q;
		int i;

		for (e = event_queue, i = 0; i < MAXUMONTHS; e++, i++)
		{
			/*
	printf("i %d num %d\n", i, e->num);
      */
			while (e->num)
			{
				/*
	  printf("deleting %d\n", e->first->person_id);
	*/
				queue_delete(e->first, EVENT_QUEUE);
			}
			e->first = NULL;
			e->num = 0;
		}

		/*
      printf("working on marriage queue\n");
    */

		for (q = marriage_queue, i = MALE; i <= FEMALE; q++, i++)
		{
			while (q->first != NULL)
			{
				queue_delete(q->first, MARRIAGE_QUEUE);
			}
			q->first = NULL;
			q->num = 0;
		}
	}
	//warning("bogie bodi 9");
	//fprintf(fd_log,"begi9");
	/* initialize variables to defaults or error-trigger values */
	write_output = 0; /* only write intermediate pop files 
		     if instructed to in each segment*/

	take_census = 0; /*only take census is instructed for current seg */
	duration_of_segment = 0;

	use_lc_rates = 0;
	read_ax_or_bx = -1;
	hhmigration = 0;
	random_epsilon = 0;
	fixed_epsilon = 0;
	numgroups = 0; /* this will be learned from reading rate sets and init pop*/
	
	//stop("begi 10!");
}

void population_pyramid(FILE *fd_pyr)
{
	struct person *p;
	int tally_matrix[27][2][MAXGROUPS];
	int sum[2][MAXGROUPS];
	int limit, row, g, i, j, num, num1, num2, num3;
	float percent, percent1, percent2, percent3;
	int pyr_age;
	/*    char leftline[80], rightline[80];*/
	char leftline[81], rightline[81];

	char logstring[256];

	/**disable for now until we can figure out why it crashs on small
    pop  --carlm10/99**/
	/* Fri Jul 19 15:27:52 PDT 2002 tun population_pyramid back on
       printf("Population Pyramid Compilation disabled\n");
       return;*/

	if (MAXUYEARS == 100)
		limit = 23;
	else
		limit = 27;

	/*** Fri Jul 19 15:18:52 PDT 2002
	 changed to g= 1 from g=0 hopefuly this will cure the pop pyramid
	 crash problem
    ****/

	/*    for (g = 0; g <= numgroups; g++) { */
	for (g = 0; g <= NUMBER_OF_GROUPS; g++)
	{
		for (i = 0; i < limit; i++)
		{
			tally_matrix[i][MALE][g] = 0;
			tally_matrix[i][FEMALE][g] = 0;
		}
		sum[MALE][g] = 0;
		sum[FEMALE][g] = 0;
	}

	p = person0;

	/* tally the population by age, sex, and group 
    * index 0 can ge used for summing over all the non-zero
    * values of the index. Here, the sum array takes the role.
    */

	logmsg("--write popu pyra\n", "", 1);
	fflush(stdout);
	while (p != NULL)
	{
		/*
	printf("id = %d\n",p->person_id);fflush(stdout);	
      */
		if (p->deathdate == 0)
		{
			/* write each living peson..
	   printf("pid=%d age=%d sex=%d group=%d\n",p->person_id,
	       (current_month - p->birthdate),
	       p->sex, p->group);
	*/
			pyr_age = current_month - p->birthdate;
			if (pyr_age <= 1)
			{
				row = 1;
			}
			else
			{
				if (pyr_age <= 11)
				{
					row = 2;
				}
				else
				{
					row = (int)(pyr_age / 60) + 3;
					if (row >= limit)
					{
						/* someone lived too long*/
						//sprintf(logstring,"person %d lived too long check group %d mortality rates ",p->person_id, p->group);
						fprintf(fd_log,"person %d lived too long check group %d mortality rates \n",p->person_id, p->group);
						//logmsg("%s\n", logstring, 1);
						row = limit - 1;
					}
				}
			}

			tally_matrix[row][p->sex][p->group]++;
			tally_matrix[row][p->sex][0]++;

			sum[p->sex][0]++;
			sum[p->sex][p->group]++;
		}

		/*	printf("                   tallying %d  of %d \r", p->person_id, last_person->person_id );
		fflush(stdout);
      */
		p = p->down;
	}
	printf("\n");

	/* write the header */
	/* Fri Jul 19 15:21:02 PDT 2002
       changed g=0 t g=1
    */

	/** barf out sum and tally **/

	for (g = 0; g <= NUMBER_OF_GROUPS; g++)
	{
		/*
      printf("group %d males: %d  females: %d\n",
	     g,sum[MALE][g], sum[FEMALE][g]);
      */
		/* fprintf(fd_pyr, "\f") */
		if (sum[MALE][g] + sum[FEMALE][g] == 0)
			continue;
		if (sum[MALE][g] == 0)
			continue;
		if (sum[FEMALE][g] == 0)
			continue;

		fprintf(fd_pyr, "\n");
		fprintf(fd_pyr, "\n");
		if (current_segment == 1)
		{
			fprintf(fd_pyr,
					"Population Pyramid at the start of simulation ");
		}
		else
		{
			fprintf(fd_pyr, "Population Pyramid at the end of Segment %-3d",
					current_segment - 1);
		}
		fprintf(fd_pyr, "Month: %-6d\n", current_month);
		if (g == 0)
			fprintf(fd_pyr, "             All groups :");
		else
			fprintf(fd_pyr, "             Group: %-3d", g);
		fprintf(fd_pyr, "Total Population: %-3d", size_of_pop[g]);
		fprintf(fd_pyr, "\n");
		fprintf(fd_pyr, "---------------MALE--------------------|");
		fprintf(fd_pyr, "--------------FEMALE-------------------\n");

		/* crashes here on group=1 no problem writing totals */

		fflush(fd_pyr);
		/* do the proportional writes, youngest last */
		for (i = limit - 1; i >= 4; i--)
		{
			strcpy(leftline, "");
			strcpy(rightline, "");

			if (sum[MALE][g] == 0)
				percent = 0;
			else
				percent = 100 * tally_matrix[i][MALE][g] / sum[MALE][g];
			num = (int)percent * 2;

			/** will over write memory boundary without this if more than
		40 pct of age pop in one age  group**/
			num = num > 80 ? 80 : num;

			for (j = 1; j <= 39 - num; j++)
				strcat(leftline, " ");
			for (j = 1; j <= num; j++)
				strcat(leftline, "M");

			if (sum[FEMALE][g] == 0)
				percent = 0;
			else
				percent = 100 * tally_matrix[i][FEMALE][g] / sum[FEMALE][g];
			num = (int)percent * 2;
			/** will over write memory boundary without this if more than
	     40 pct of age pop in one age  group**/
			num = num > 80 ? 80 : num;
			for (j = 1; j <= num; j++)
				strcat(rightline, "F");
			for (j = 1; j <= 39 - num; j++)
				strcat(rightline, " ");

			fprintf(fd_pyr, "%s|%s\n", leftline, rightline);
		}

		strcpy(leftline, "");
		strcpy(rightline, "");

		if (sum[MALE][g] == 0)
		{
			percent1 = 0;
			percent2 = 0;
			percent3 = 0;
		}
		else
		{
			percent1 = 100 * tally_matrix[1][MALE][g] / sum[MALE][g];
			percent2 = 100 * tally_matrix[2][MALE][g] / sum[MALE][g];
			percent3 = 100 * tally_matrix[3][MALE][g] / sum[MALE][g];
		}

		/*
	fprintf(fd_pyr, 
	"g %d tm 1 2 3  male %d %d %d %d %d %d %d %d sum male g %d\n",
	       g,
		tally_matrix[1][MALE][g], 
		tally_matrix[2][MALE][g], 
		tally_matrix[3][MALE][g], 
		tally_matrix[4][MALE][g], 
		tally_matrix[5][MALE][g], 
		tally_matrix[6][MALE][g], 
		tally_matrix[7][MALE][g], 
		tally_matrix[8][MALE][g], 
		sum[MALE][g]);
	fflush(fd_pyr);
	*/

		num1 = (int)percent1 * 2;
		num2 = (int)percent2 * 2;
		num3 = (int)percent3 * 2;

		/*fprintf(fd_pyr,"nums  %d %d %d\n",num1,num2,num3);
	fflush(fd_pyr);
	*/
		/** nums over 80 write beyond bounds of leftline screw everything */
		num1 = num1 > 80 ? 80 : num1;
		num2 = num2 > 80 ? 80 : num2;
		num3 = num3 > 80 ? 80 : num3;

		if (num1 + num2 + num3 < 39)
		{
			for (j = 1; j <= 39 - num1 - num2 - num3; j++)
				strcat(leftline, " ");
			for (j = 1; j <= num1; j++)
				strcat(leftline, "i");
			for (j = 1; j <= num2; j++)
				strcat(leftline, "m");
			for (j = 1; j <= num3; j++)
				strcat(leftline, "M");
		}
		else
		{
			/** only room for 40% in  any bar -- if the population (init for 
	    example) is all babies we need to avoid the fancy bars**/
			for (j = 1; j <= 39; j++)
				strcat(leftline, "M");
		}
		if ((percent1 > 40) || (percent2 > 40) || (percent3 > 40))
		{
			leftline[0] = '<';
		}
		if (sum[FEMALE][g] == 0)
		{
			percent1 = 0;
			percent2 = 0;
			percent3 = 0;
		}
		else
		{
			percent1 = 100 * tally_matrix[1][FEMALE][g] / sum[FEMALE][g];
			percent2 = 100 * tally_matrix[2][FEMALE][g] / sum[FEMALE][g];
			percent3 = 100 * tally_matrix[3][FEMALE][g] / sum[FEMALE][g];
		}

		/*	fprintf(fd_pyr, "g %d tm 1 female %d sum female g %d\n",
	       g,tally_matrix[1][FEMALE][g], sum[FEMALE][g]);
	fflush(fd_pyr);
	*/

		num1 = (int)percent1 * 2;
		num2 = (int)percent2 * 2;
		num3 = (int)percent3 * 2;

		/*
	  fprintf(fd_pyr,"nums  %d %d %d\n",num1,num2,num3);
	  fflush(fd_pyr);
	*/
		/** nums over 80 write beyond bounds of leftline screw everything */
		num1 = num1 > 80 ? 80 : num1;
		num2 = num2 > 80 ? 80 : num2;
		num3 = num3 > 80 ? 80 : num3;

		if (num1 + num2 + num3 < 39)
		{
			for (j = 1; j <= num3; j++)
				strcat(rightline, "F");
			for (j = 1; j <= num2; j++)
				strcat(rightline, "f");
			for (j = 1; j <= num1; j++)
				strcat(rightline, "i");
			for (j = 1; j <= 39 - num1 - num2 - num3; j++)
				strcat(rightline, " ");
		}
		else
		{
			/** only room for 40% in  any bar -- if the population (init for 
	      example) is all babies we need to avoid the fancy bars**/
			for (j = 1; j <= 39; j++)
				strcat(rightline, "F");
		}

		if ((percent1 > 40) || (percent2 > 40) || (percent3 > 40))
		{
			/* rightline[39]='>';*/
			strcat(rightline, ">");
		}

		fprintf(fd_pyr, "%s|%s\n", leftline, rightline);

		/* write the scale */
		fprintf(fd_pyr, "   ------+---------+---------+---------+");
		fprintf(fd_pyr, "---------+---------+---------+------\n ");
		fprintf(fd_pyr, "        15        10         5         0");
		fprintf(fd_pyr, "         5        10        15        \n");
	}
}

void print_segment_info(FILE *fd)
{
	int g, d, m, count;
	fprintf(fd, "\n\n - - - - - - - - - - - - - - - - - - - - - -  - - - - - \n\n");
	fprintf(fd, "Segment NO:\t%d of %d set to run with following macro options\n", current_segment, num_segments);
	fprintf(fd, "Duration: %d\n", duration_of_segment);

	fprintf(fd, "\n-- I/O options --\n");
	fprintf(fd, "Input pop file name\t%s\n", pop_file_name);
	if (fd_mar != NULL)
	{
		fprintf(fd, "Input mar file name\t%s\n", mar_file_name);
	}
	else
	{
		fprintf(fd, "No marriage file read\n");
	}

	if (fd_otx != NULL)
	{
		fprintf(fd, "Transition history file:%s", otx_file_name);
	}
	else
	{
		fprintf(fd, "Transition history file NOT read\n");
	}

	if (read_xtra_file)
	{
		fprintf(fd, "Xtra variables read from:%s\n ", xtra_file_name);
	}
	else
	{
		fprintf(fd, "Xtra variables NOT read\n");
	}

	/* fprintf(fd,"write census file:");
  if(take_census){ fprintf(fd,"yes\n"); } else {fprintf(fd,"no\n"); };
  */
	fprintf(fd, "\n-- Fertility options --\n");
	fprintf(fd, "Sex Ratio(prop male births):\t %lf\n", prop_males);
	fprintf(fd, "Minimum Birth Interval:\t%f months\n", bint);
	fprintf(fd, "Fertility Multipler inheritance specified:");
	if ((alpha != 0) || (betaT != 1))
	{

		fprintf(fd, "Fert Mult inheritance: alpha=%f betaT=%f\n", alpha, betaT);
	}
	else
	{
		fprintf(fd, "no\n");
	}

	fprintf(fd, "Birth rate factors specified:");
	count = 0;
	/*  for (g = 1; g < MAXGROUPS; g++) { */
	for (g = 1; g <= numgroups; g++)
	{
		for (m = 0; m < NUMMARSTATUS; m++)
		{
			if (birth_rate_factors[g][m] != 1)
				count++;
		}
	}
	if (count)
	{
		fprintf(fd, "yes\n");
	}
	else
	{
		fprintf(fd, "no\n");
	};

	count = 0;
	for (g = 1; g <= NUMBER_OF_GROUPS; g++)
	{
		if (birth_target[g] != -1)
			count++;
	}
	fprintf(fd, "Birth Targets Specified:");
	if (count)
	{
		fprintf(fd, "yes\n");
	}
	else
	{
		fprintf(fd, "no\n");
	};

	if (random_father)
	{
		fprintf(fd, "Random_father enabled\n");
		fprintf(fd, "min age random father:%d", random_father_min_age);
	}
	else
	{
		fprintf(fd, "Random_father disabled\n");
	}
	fprintf(fd, "\n-- Nuptiality Options --\n");
	fprintf(fd, "marriage_queues==%d\n", marriage_queues);
	fprintf(fd, "Endogamy/Exogamy:");

	if (endogamy == 1)
	{
		fprintf(fd, " ENdogamy\n");
	}
	else if (endogamy == -1)
	{
		fprintf(fd, " EXogamy\n");
	}
	else if (endogamy == 0)
	{
		fprintf(fd, " random\n");
	}
	else if (endogamy > 0)
	{
		fprintf(fd, " %5lf of endogamous suitors accepted\n", endogamy);
	}
	else
	{
		fprintf(fd, " endgamy paremter %5lf ..  %5lf of EXogamous suitors accepted\n",
				endogamy, -1 * endogamy);
	}

	if (marriage_eval == PREFERENCE)
	{
		fprintf(fd, "marriages evaluated by 'preference' over age diff\n");
		fprintf(fd, "marriage_peak_age %lf\n", marriage_peak_age);
		fprintf(fd, "marriage_slope_ratio %lf\n", marriage_slope_ratio);
		fprintf(fd, "marriage_agedif_max %d\n", marriage_agedif_max);
		fprintf(fd, "marriage_agedif_min %d\n", marriage_agedif_min);
	}
	if (marriage_eval == DISTRIBUTION)
	{
		fprintf(fd, "marriages evaluated by 'distribution' of age diff\n");
		for (g = 1; g <= numgroups; g++)
		{
			fprintf(fd, "mean/sd of agediff for females of group:%d : %f/%f\n",
					g, agedif_marriage_mean[g], agedif_marriage_sd[g]);
		}
	}
	fprintf(fd, "\n-- Migration Options --\n");
	fprintf(fd, "Transition rate targets set:");
	count = 0;
	for (g = 1; g <= NUMBER_OF_GROUPS; g++)
	{
		for (d = 1; d <= NUMBER_OF_GROUPS; d++)
		{
			if (transit_target[g][d] > -1)
			{
				count++;
			}
		}
	}
	if (count)
	{
		fprintf(fd, "yes\n");
	}
	else
	{
		fprintf(fd, "no\n");
	};

	fprintf(fd, "child group identity : ");
	switch (child_inherits_group)
	{
	case FROM_MOTHER:
		fprintf(fd, " inherited from MOTHER\n");
		break;
	case FROM_FATHER:
		fprintf(fd, " inherited from FATHER\n");
		break;
	case FROM_SAME_SEX_PARENT:
		fprintf(fd, " inherited from SAME SEX PARENT\n");
		break;
	case FROM_OPPOSITE_SEX_PARENT:
		fprintf(fd, " inherited from OPPOSITE SEX PARENT\n");
		break;
	default:
		fprintf(fd, " always %d\n", child_inherits_group);
	}

	fprintf(fd, "\n  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||  ||\n\n");
	fflush(fd);
}
