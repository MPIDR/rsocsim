#include <string.h>
//#include <sys/wait.h> //outcommenting suggested by jim
#include <math.h>
#include <stdlib.h>

//#include "defs.h"

#include <Rcpp.h>
using namespace Rcpp;

/* the context needed to handle nested files */
struct l_context
{
	struct rateset *rateset; /* rateset being built */
	char *file;
	int lineno; /* current line number */
};

/* maximum line length */
#define LINELENGTH 1024

/* keywords */
enum l_keyword
{
	k_birth,
	k_death,
	k_marriage,
	k_divorce,
	k_cohab_prob,
	k_birthtarget,
	k_transittarget,
	k_take_census,
	k_single,
	k_married,
	k_divorced,
	k_widowed,
	k_cohabiting,
	k_input_file,
	k_output_file,
	k_execute,
	k_include,
	k_done,
	k_female,
	k_male,
	k_run,
	k_end,
	k_segments,
	k_sex_ratio,
	k_proportion_male,
	k_float,
	k_integer,
	k_double,
	k_string,
	k_read_xtra,
	k_size_of_xtra,
	k_lc_init,
	k_lc_ax,
	k_lc_bx,
	k_lc_k_val,
	k_lc_k_mean,
	k_lc_k_std_dev,
	k_lc_k_start_list,
	k_lc_epsilon,
	k_fix_epsilon,
	k_random_epsilon,
	k_random_father,
	k_random_father_min_age,
	k_alpha,
	k_betaT,
	k_bint,
	k_duration_specific,
	k_factor,
	k_hhmigration,
	k_hetfert,
	k_endogamy,
	k_duration,
	k_transit,
	k_marriage_peak_age,
	k_marriage_slope_ratio,
	k_marriage_agedif_max,
	k_marriage_agedif_min,
	k_agedif_marriage_mean,
	k_agedif_marriage_sd,
	k_marriage_eval,
	k_marriage_queues,
	k_firstyear,
	k_hettrans,
	k_write_output,
	k_child_inherits_group,
	k_parameter0,
	k_parameter1,
	k_parameter2,
	k_parameter3,
	k_parameter4,
	k_parameter5,
	k_parameter6,
	k_parameter7,
	k_parameter8,
	k_parameter9,
	k_parameter10,
	k_unknown
};

//enum l_keyword l_lookup_keyword();
enum l_keyword l_lookup_keyword(register char*);

void l_error(struct l_context*, char*);
void error(char*);//,char*,int, char*,char*,char*,char*);
void add_rate_block(int, int, double ) ;
void add_rate_table(int, int, double ) ;
int	read_integer(struct l_context*,char*);
char l_scan_line(char*,char**);
int l_process_line(char*,struct l_context*,FILE*);
void add_lc_rate_block(int, int, double);
int	keyword_to_integer(struct l_context*, enum l_keyword);
void warn_about_rates( struct age_block*, struct age_block*);
// in utils.c:
int get_expected_number_of_births(int);
int get_expected_number_of_transits(int,int);

/* for rate sets and  access */
/* rates and state */
int event, sex, parity, mstatus, group, dest, value;

struct age_block *current_block;
struct age_block *recall = NULL;

struct age_table *current_table;
struct age_table *recall_table = NULL;
/**
 * forward declarations *
char *index_to_sex[] = {
	"male",
	"female",
};
char *index_to_event[] =
	{
		"birth",
		"marriage",
		"divorce",
		"death",
		"transit1",
		"transit2",
		"transit3",
		"transit4",
		"transit5",
		"transit6",
		"transit7",
		"transit8",
		"transit9",
		"transit10",
		"transit11",
		"transit12",
		"transit13",
		"transit14",
		"transit15",
		"transit16",
		"transit17",
		"transit18",
		"transit19",
		"transit20",
		"transit21",
		"transit22",
		"transit23",
		"transit24",
		"transit25",
		"transit26",
		"transit27",
		"transit28",
		"transit29",
		"transit30",
		"transit31",
		"transit32",
		"transit33",
		"transit34",
		"transit35",
		"transit36",
		"transit37",
		"transit38",
		"transit39",
		"transit40",
		"transit41",
		"transit42",
		"transit43",
		"transit44",
		"transit45",
		"transit46",
		"transit47",
		"transit48",
		"transit49",
		"transit50",
		"transit51",
		"transit52",
		"transit53",
		"transit54",
		"transit55",
		"transit56",
		"transit57",
		"transit58",
		"transit59",
		"transit60",
		"null",
};
char *index_to_mstatus[] =
	{
		"null",
		"single",
		"divorced",
		"widowed",
		"married",
		"cohabiting",
};
 **/
char command[80];

struct kt_holding_array
{
	int year;
	double kt;
};
struct kt_holding_array kt_temp[MAXUYEARS];
int kt_start_year;
int reading_kt_values = FALSE;
int reading_epsilons = FALSE;
int reading_rate_table = FALSE;

//int numgroups = 0; /*initialize*/
/*int numgroups = NUMBER_OF_GROUPS ;  figure it out from reading rates*/

char logstring[1024];

int load( char *file)
{
	
	//Rcpp::Rcout << "18b-load rate file . " << file << std::endl;
	fprintf(fd_log,"open: %i, load rate file: |%s\n",current_fstatus,file);
	fflush(fd_log);
	struct l_context cx;
	char line[LINELENGTH];
	FILE *fp;

	if (current_fstatus == OPEN)
	{
		fp = current_fp;
		cx.lineno = current_lineno;
		current_fstatus = CLOSED;
	}
	else
	{
		if ((fp = fopen(file, "r")) == 0)
		{
		  
			char path[255];
			
			getcwd(path, 255);
			warning("Current working directory: %s\n", path);
			/*    error("Can't open \"%s\"", "rate_file");*/
			warning("Can not open ratefile: %s", file);

			/*return -1;*/
			stop("cant open ratefile");
			//exit(-1);
		}
		cx.lineno = 0;
		current_offset = 0;
	}

	//logmsg("openning %s \n", file, 1);
	fprintf(fd_log,"\nopening %s \n", file);
	fprintf(fd_log,"starting with line %d ", current_lineno);
	fprintf(fd_log,"at %ld, seekset= %i \n", current_offset, SEEK_SET);
	Rcpp::Rcout << "18b-load.cpp->load . " << file << std::endl;
	fflush(fd_log);
	fseek(fp, current_offset, SEEK_SET); // before, SEEK_SET was 0
	//somehow this jumps to the wrong position, if we don't start at the beginning.

	fseek(fp,0,SEEK_SET);
	int found = 0;
	int count = 0;
	while (!found && fgets(line, sizeof line, fp) != NULL) {
		if (count == cx.lineno){
        	found = true;
    	} else {
        	count++;
		}
	}
	cx.file = file;

	Rcpp::Rcout << "#load.cpp->load 4. " << file << std::endl;
	while (fgets(line, sizeof line, fp) != 0)
	{
		cx.lineno++;
		
		Rcpp::Rcout << "18b-load.cpp->load . " << line << std::endl;
		//fprintf(fd_log,"@FILE: %s LINE: %s \n", cx.file, line);
		//fflush(fd_log);
		if (current_fstatus == OPEN) /*triggered by run command */
		{
			Rcpp::Rcout << "18b-load.cpp->load . current_fstatus==OPEN " << std::endl;
			return 1;
		}
		if (l_process_line(line, &cx, fp) < 0)
		{
			/* just quit on any error for now */
			(void)fclose(fp);
			printf("FILE: %s LINE: %s\n", cx.file, line);
			fprintf(fd_log,"Error in load..l_process_line;FILE: %s LINE: %s \n", cx.file, line);
			fflush(fd_log);
			return -1;
		}
	}
	/* XXX, debugging
     {
     printf("%s\n", line);
     }
  */
	(void)fclose(fp);

	return 0;
}

/*
 * Process a line
 */
int l_process_line(char *line,struct l_context *cx,FILE *fp)
{
	//Rcpp::Rcout << "18b-l_process_line. beginning. line:|" << line << "|" ;//<< std::endl;
					
	char *words[LINELENGTH]; /* always big enough */
	int i;
	int nwords;
	enum l_keyword k;
	char logstring[1024];
	enum l_keyword kword;
	int destgroup = 0;

	/*
	printf("%s\n", line);
	*/
	nwords = l_scan_line(line, words);
	//Rcpp::Rcout << "18b-l_process_line. beginning2 line:|" << line << "| ---" << nwords << std::endl;
	if (nwords == 0) /* blank line */
		goto out;
	if (**words == '*') /* comment */
		goto out;
	switch (**words)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '.':
	case '-':
		/*
		if (cx->file == 0)
			l_create_rateset(cx, rateset);
		*/
		{
			//Rcpp::Rcout << "18b-l_process_line. case-block 1 - numbers " << std::endl;
					
			int years, months;
			double prob;
			int kt_year;
			double kt_year_val;

			/* line of rates */
			if (nwords == 3)
			{
				if (sscanf(words[0], "%d", &years) != 1 ||
					sscanf(words[1], "%d", &months) != 1 ||
					sscanf(words[2], "%lf", &prob) != 1)
				{
					l_error(cx, "Not a valid rate line");
					return -1;
				}
				/* or line of kt values */
			}
			else if (nwords == 2)
			{
				if (!reading_kt_values)
				{
					l_error(cx, "Not a valid rate line");
				}
				if (sscanf(words[0], "%d", &kt_year) != 1 ||
					sscanf(words[1], "%lf", &kt_year_val) != 1)
				{
					l_error(cx, "Not a valid kt value line");
					return -1;
				}
				if (kt_year >= kt_start_year)
				{
					last_kt_index[group][sex]++;
					read_kt_vals[group]
								[sex][last_kt_index[group][sex]] = kt_year_val;
				}
			}
			else
			{
				printf("\n line: %s\n", line);
				l_error(cx, "\nNot a valid rate/kt value line\n");
				return -1;
			}

			if (nwords == 3)
			{
				if ((years > MAXUYEARS) || (years < 0))
				{
					l_error(cx, "Not a valid rate line");
					return -1;
				}
				else if ((years + (int)months / 12 > MAXUYEARS) || (months < 0))
				{
					l_error(cx, "Not a valid rate line");
					return -1;
				}
				if (read_ax_or_bx != -1)
					add_lc_rate_block(years, months, prob);
				else if (reading_rate_table == TRUE)
					add_rate_table(years, months, prob);
				else
				
					//Rcpp::Rcout << "18b-l_process_line. just before add_rate_block " << line << " | " << years << "y " << months << "m. prob: " << prob << std::endl;
					add_rate_block(years, months, prob);
			}
		}
		goto out;
	}
	//Rcpp::Rcout << "18b-l_process_line...after.." << std::endl;
	/* not a line of rates, process command */

	/* is the last group of rates complete? */
	if (recall != NULL)
	{
		 /*
		 
		 old behavior: throw error. abort. If rates are not defined up to MAXUYEARS, then the program will crash.
		 new behavior: extend the last age block to MAXUYEARS. or create a new one with the same rates up until Maxuyears
		 
		 */
		 
		Rcpp::Rcout << "18b-l_process_line... recall!=NULL "<< std::endl;
		Rcpp::Rcout << "Incomplete rate set: " << recall->upper_age << std::endl;
		current_block->upper_age = MAXUMONTHS;
		current_block->next = NULL;
		recall = NULL;
		//l_error(cx, "Incomplete rate set");
		//stop("Incomplete rate set");
		//exit(-1);
		/*return -1; */
	}

	/*
	if (recall != NULL)
	    print_rate_table(recall);
	*/

	//Rcpp::Rcout << "18b-l_process_line...befor switch l_lookup_keyword " << line << std::endl;
	switch (k = l_lookup_keyword(*words))
	{
	case k_birth:
		event = E_BIRTH;
		if (nwords != 5)
		{
			stop(logstring, "\n\nmalformed birth specifier in %s at line %d\n\n",
					cx->file, cx->lineno);
			logmsg("%s\n", logstring, 1);
			//exit(-1);
		}
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		mstatus = keyword_to_integer(cx, l_lookup_keyword(words[3]));
		parity = read_integer(cx, words[4]);
		if (parity >= MAXPARITY)
		{
			l_error(cx, "maximum parity exceeded");
			return -1;
		}
		/* printf("birth %d %d %d\n", group, mstatus, parity); */
		current_block = NEW(struct age_block);
		current_block->previous = NULL;
		birth_rate_set[group][mstatus][parity] = current_block;
		recall = current_block;
		numgroups = (group > numgroups) ? group : numgroups;
		/*l_read_birthrates(cx)*/;
		return 1;
		break;
	case k_death:
	case k_divorce:
	case k_marriage:
		/*printf("death/marriage/divorce\n");*/
		event = keyword_to_integer(cx, k);
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		mstatus = keyword_to_integer(cx, l_lookup_keyword(words[3]));
		current_block = NEW(struct age_block);
		current_block->previous = NULL;
		rate_set[group][event][sex][mstatus] = current_block;
		recall = current_block;
		numgroups = (group > numgroups) ? group : numgroups;
		return 1;
		break;
	case k_cohab_prob:
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		current_table = NEW(struct age_table);
		cohab_probs[group][sex] = current_table;
		reading_rate_table = TRUE;
		recall_table = current_table;
		return 1;
		break;
	case k_birthtarget:
		logmsg("birth target group %s =", words[1], 0);
		logmsg("%s\n", words[2], 0);
		group = read_integer(cx, words[1]);
		birth_target[group] = read_integer(cx, words[2]);
		return 1;
		break;
	case k_transittarget:
		logmsg("transit target group %s ->", words[1], 0);
		logmsg("%s :", words[2],1);
		logmsg("%s\n", words[3],1);
		group = read_integer(cx, words[1]);
		dest = read_integer(cx, words[2]);
		transit_target[group][dest] = read_integer(cx, words[3]);
		/* destination of transit rate implies group existance*/
		numgroups = (group > numgroups) ? group : numgroups;
		numgroups = (dest > numgroups) ? dest : numgroups;
		return 1;
		break;
	case k_take_census:
		sscanf(words[1], "%d", &take_census);
		sprintf(logstring,
				"take census %d\n", take_census);
		logmsg("%s", logstring, 0);
		break;
	case k_sex_ratio:
	case k_proportion_male:
		sscanf(words[1], "%lf", &prop_males);
		sprintf(logstring, "prop males %lf\n", prop_males);
		logmsg("%s", logstring, 0);
		return 1;
		break;

	case k_random_father:
		sscanf(words[1], "%d", &random_father);
		sprintf(logstring,
				"find random fathers %d\n", random_father);
		logmsg("%s", logstring, 0);
		break;

	case k_random_father_min_age:
		sscanf(words[1], "%d", &random_father_min_age);
		sprintf(logstring,
				"min age of random fathers(yrs) %d\n", random_father);
		logmsg("%s", logstring, 0);
		break;

	case k_input_file:
		fprintf(fd_log,"I am here at load.cpp-k_input_file. |%s\n",pop_file_name);
		//pop_file_name;
		//memset(pop_file_name, 0, 1024);
		memset(pop_file_name, 0, 1024 * (sizeof pop_file_name[0]) );
		fprintf(fd_log,"I am here at load.cpp-k_input_file. |%s\n",pop_file_name);
		//pop_file_name[0] = '\0';
		strcpy(pop_file_name, words[1]);
		fprintf(fd_log,"I am here at load.cpp-k_input_file. |%s\n",pop_file_name);
		fflush(fd_log);
		strcpy(mar_file_name, words[1]);
		strcpy(xtra_file_name, words[1]);
		strcpy(otx_file_name, words[1]);
		return 1;
		break;
	case k_output_file:
		// todo: instead of words[1], create a string from
		// * "sim_results_"
		// * the name of the supplement-file
		// * and the seed
		// then create a subdirectory with that name
		// and put the output files there

		// concatenate "sim_results_" with the name of the supplement file:
		//sprintf(output_file_name, "sim_results_%s", words[1]);
		char buffer_output_dir [250];
		char buffer_output_fn [250];
		sprintf (buffer_output_dir, "sim_results_s%s_%d/", rate_file_name, ceed);
		sprintf (buffer_output_fn, "%sresult", buffer_output_dir);

		// create the directory:
		mkdir(buffer_output_dir);
		strcpy(pop_out_name, buffer_output_fn);
		strcpy(mar_out_name, buffer_output_fn);
		strcpy(xtra_out_name, buffer_output_fn);
		strcpy(pyr_file_name, buffer_output_fn);
		strcpy(stat_file_name, buffer_output_fn);
		strcpy(prefix_out_name, buffer_output_fn);
		strcpy(otx_out_name, buffer_output_fn);
		return 1;
		break;
	case k_read_xtra:
		logmsg("read_xtra: %s\n", words[1], 0);
		read_xtra_file = read_integer(cx, words[1]);

		return 1;
		break;

	case k_size_of_xtra:
		logmsg("size_of_xtra: %s\n", words[1], 0);
		size_of_extra = read_integer(cx, words[1]);
		return 1;
		break;

		/* it is questionable whether this should be read or
		   not it is perhaps only useful in enhanced
		   versions */

	case k_execute:
		strcpy(command, "");
		for (i = 1; i < nwords; i++)
		{
			strcat(command, words[i]);
			strcat(command, " ");
		}
		
		fprintf(fd_log, "process line, command: %s | - fprintf \n",command);
		system(command);
		return 1;
		break;

	case k_marriage_queues:
		marriage_queues = read_integer(cx, words[1]);
		if (marriage_queues != 1 && marriage_queues != 2)
		{
			stop( "marriage_queues out of range\n");
			//exit(-1);
		}
		sprintf(logstring, "marriage_queues : %d\n",
				marriage_queues);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

		/** determine the marriage evaluation scheme 1 or 2**/
		/** aka preference or distribution **/
	case k_marriage_eval:
		if (strcmp(words[1], "preference") == 0)
		{
			marriage_eval = PREFERENCE;
		}
		else if (strcmp(words[1], "distribution") == 0)
		{
			marriage_eval = DISTRIBUTION;
		}
		else
		{

			stop("ERROR:marriage_eval parameter makes no sense\n");
			//exit(-1);
		}
		sprintf(logstring, "marriage_eval : %d\n",
				marriage_eval);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

		/** marriage_eval == 1 aka AGEDIF**/
	case k_marriage_peak_age:

		sscanf(words[1], "%lf", &marriage_peak_age);
		/* sprintf(logstring,"marriage_peak_age : %f\n", marriage_peak_age);
	     logmsg("%s\n",logstring,0);*/

		return 1;
		break;

	case k_marriage_slope_ratio:

		sscanf(words[1], "%lf", &marriage_slope_ratio);
		/*sprintf(logstring,"marriage_peak_age : %f\n", marriage_slope_ratio);
	    logmsg("%s\n",logstring,0);*/
		return 1;
		break;

	case k_marriage_agedif_max:
		marriage_agedif_max = read_integer(cx, words[1]);
		return 1;
		break;

	case k_marriage_agedif_min:
		marriage_agedif_min = read_integer(cx, words[1]);
		return 1;
		break;

		/**marriage_eval==PROB scheme */
	case k_agedif_marriage_mean:
		if (nwords != 3)
		{
			stop("agedif_marriage_mean : malformed wrong number of parameters\n");
			//exit(-1);
		}
		group = read_integer(cx, words[1]);
		sscanf(words[2], "%f", &agedif_marriage_mean[group]);
		sprintf(logstring, "agedif_mariage_mean (group %d)  : %f\n", group,
				agedif_marriage_mean[group]);
		logmsg("%s\n", logstring, 0);

		if (agedif_marriage_mean[group] > MAXUYEARS ||
			agedif_marriage_mean[group] < -MAXUYEARS)
		{
			stop("agedif_marriage_mean gr out of bounds\n");
			//exit(-1);
		}
		return 1;
		break;

	case k_agedif_marriage_sd:
		if (nwords != 3)
		{
			stop( "agedif_marriage_sd : malformed wrong number of parameters\n");
			//exit(-1);
		}

		group = read_integer(cx, words[1]);

		sscanf(words[2], "%f", &agedif_marriage_sd[group]);
		sprintf(logstring, "agedif_mariage_sd (group %d) : %f\n",
				group,
				agedif_marriage_sd[group]);
		logmsg("%s\n", logstring, 0);

		if (agedif_marriage_sd[group] < 0)
		{

			stop( "agedif_marriage_sd less than 0");
			//exit(-1);
		}
		return 1;
		break;

	case k_parameter0:

		sscanf(words[1], "%lf", &parameter0);
		sprintf(logstring, "parameter0 : %f\n", parameter0);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter1:

		sscanf(words[1], "%lf", &parameter1);
		sprintf(logstring, "parameter1 : %f\n", parameter1);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter2:

		sscanf(words[1], "%lf", &parameter2);
		sprintf(logstring, "parameter2 : %f\n", parameter2);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter3:

		sscanf(words[1], "%lf", &parameter3);
		sprintf(logstring, "parameter3 : %f\n", parameter3);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter4:

		sscanf(words[1], "%lf", &parameter4);
		sprintf(logstring, "parameter4 : %f\n", parameter4);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter6:

		sscanf(words[1], "%lf", &parameter6);
		sprintf(logstring, "parameter6 : %f\n", parameter6);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter7:

		sscanf(words[1], "%lf", &parameter7);
		sprintf(logstring, "parameter7 : %f\n", parameter7);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter8:

		sscanf(words[1], "%lf", &parameter8);
		sprintf(logstring, "parameter8 : %f\n", parameter8);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter9:

		sscanf(words[1], "%lf", &parameter9);
		sprintf(logstring, "parameter9 : %f\n", parameter9);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_parameter10:

		sscanf(words[1], "%lf", &parameter10);
		sprintf(logstring, "parameter10 : %f\n", parameter10);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;

	case k_alpha:
		sscanf(words[1], "%lf", &alpha);
		sprintf(logstring, "alpha %lf\n", alpha);
		if (alpha > 1 || alpha < 0)
		{
			stop("ERROR:alpha out of bounds 0<=alpha<=1\n");
			//exit(-1);
		}
		logmsg("%s\n", logstring, 0);
		return 1;
		break;
	case k_betaT:
		sscanf(words[1], "%lf", &betaT);
		sprintf(logstring, "betaT %lf\n", betaT);
		logmsg("%s\n", logstring, 0);
		return 1;
		break;
	case k_bint:
		sprintf(logstring, "birth interval %s\n", words[1]);
		logmsg("%s\n", logstring, 0);
		bint = (double)read_integer(cx, words[1]);
		return 1;
		break;
	case k_duration_specific:
		/* mark as duration specific event/group/mstatus &etc rateset
		 valid for transition events and cohabitiation? */
		kword = l_lookup_keyword(words[1]);
		event = keyword_to_integer(cx, l_lookup_keyword(words[1]));
		if (kword == k_transit)
		{
			destgroup = read_integer(cx, words[5]);
			event += destgroup;
		}
		group = read_integer(cx, words[2]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[3]));
		mstatus = keyword_to_integer(cx, l_lookup_keyword(words[4]));
		duration_specific[group][event][sex][mstatus] = 1;
		sprintf(logstring,
				"group %d %s %s %s  is  durartion specific\n",
				group, index_to_sex[sex],
				index_to_mstatus[mstatus],
				index_to_event[event]);
		logmsg("%s\n", logstring, 1);
		return 1;
		break;
	case k_factor:
		event = keyword_to_integer(cx, l_lookup_keyword(words[1]));
		group = read_integer(cx, words[2]);
		if (event == E_BIRTH)
		{
			mstatus = keyword_to_integer(cx, l_lookup_keyword(words[3]));
			sscanf(words[4], "%lf",
				   &(birth_rate_factors[group][mstatus]));
			sprintf(logstring,
					"birth rate factor: gp %d mstatus %s : %lf\n",
					group, words[3],
					birth_rate_factors[group][mstatus]);
			logmsg("%s\n", logstring, 0);
		}
		else
		{
			sex = keyword_to_integer(cx, l_lookup_keyword(words[3]));
			mstatus = keyword_to_integer(cx, l_lookup_keyword(words[4]));
			sscanf(words[5], "%lf",
				   &(rate_factors[group][event][sex][mstatus]));
			sprintf(logstring,
					"factor for %s group %d sex %s mstatus %s : %lf\n",
					words[1], group, words[3], words[4],
					rate_factors[group][event][sex][mstatus]);
			logmsg("%s\n", logstring, 0);
		}
		return 1;
		break;
	case k_hhmigration:
		hhmigration = read_integer(cx, words[1]);
		if (hhmigration)
			logmsg("household migration supported\n", "", 0);
		else
			logmsg("no household migration\n", "", 0);
		return 1;
		break;
	case k_hetfert:
		hetfert = read_integer(cx, words[1]);
		if (hetfert)
			logmsg("heterogeneous fertility supported\n", "", 0);
		else
			logmsg("no heterogeneous fertility\n", "", 0);
		return 1;
		break;
	case k_hettrans:
		hettrans = read_integer(cx, words[1]);
		if (hettrans)
			logmsg("heterogeneous transitions supported\n", "", 0);
		else
			logmsg("no heterogeneous transition rates\n", "", 0);
		return 1;
		break;
	case k_endogamy:
		/*endogamy  =  read_integer(cx, words[1]);*/
		sscanf(words[1], "%lf", &endogamy);
		sprintf(logstring, "endogamy %lf\n", endogamy);
		logmsg("%s\n", logstring, 0);

		return 1;
		break;
	case k_child_inherits_group:

		if (strcmp(words[1], "from_mother") == 0)
		{
			child_inherits_group = FROM_MOTHER;
		}
		else if (strcmp(words[1], "from_father") == 0)
		{
			child_inherits_group = FROM_FATHER;
		}
		else if (strcmp(words[1], "from_same_sex_parent") == 0)
		{
			child_inherits_group = FROM_SAME_SEX_PARENT;
		}
		else if (strcmp(words[1], "from_opposite_sex_parent") == 0)
		{
			child_inherits_group = FROM_OPPOSITE_SEX_PARENT;
		}
		else if ((read_integer(cx, words[1]) >= 1) &&
				 read_integer(cx, words[1]) <= MAXGROUPS)
		{
			child_inherits_group = read_integer(cx, words[1]);
			numgroups = (child_inherits_group > numgroups) ? child_inherits_group : numgroups;
		}
		else
		{
			stop("\nERROR: argument not implemented for child_inherits_group try\n           from_mother \n           from_father			\
\n        from_same_sex_parent\n           from_opposite_sex_parent or group\n");
			//exit(-1);
		}

		printf("on birth children get group from %s\n", words[1]);
		return 1;
		break;
	case k_duration:
		logmsg("duration: %s\n", words[1], 0);
		duration_of_segment = read_integer(cx, words[1]);
		return 1;
		break;
	case k_write_output:
		logmsg("write pop/mar/xtra after current segment %s\n",
			   words[1], 0);
		write_output = read_integer(cx, words[1]);
		return 1;
		break;
	case k_segments:
		logmsg("number of segments: %s\n", words[1], 0);
		num_segments = read_integer(cx, words[1]);
		return 1;
		break;

	case k_firstyear:
		logmsg("starting month is January of %s\n", words[1], 0);
		firstyear = read_integer(cx, words[1]);
		return 1;
		break;
	case k_transit:
		/*printf("transit from %s to %s\n", words[1], words[4]);*/
		logmsg("reading rates for: transit from %s to ", words[1],0);
		logmsg("%s\n", words[4],0);
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		mstatus = keyword_to_integer(cx, l_lookup_keyword(words[3]));
		dest = read_integer(cx, words[4]);
		event = keyword_to_integer(cx, k) + dest;
		current_block = NEW(struct age_block);
		current_block->previous = NULL;
		rate_set[group][event][sex][mstatus] = current_block;
		recall = current_block;
		/* destination of transit rate implies group existance*/
		numgroups = (group > numgroups) ? group : numgroups;
		numgroups = (dest > numgroups) ? dest : numgroups;
		return 1;
		break;
	case k_end:
		logmsg("Last segment...\n"," ",0);
		return 1;
	case k_run:
		/*logmsg("simulating ... \n");*/
		current_offset = ftell(fp);
		current_fstatus = OPEN;
		current_lineno = cx->lineno;
		/*printf("offset %ld line %d\n", current_offset, cx->lineno);*/
		fprintf(fd_log,"\ncurrent_offset: %ld; line: %d\n", current_offset, cx->lineno);
		current_fp = fp;
		return 1;
		break;
	case k_include:
	    fprintf(fd_log,"loading %s ... ", words[1]);
		load(words[1]);
		fprintf(fd_log,"done loading %s\n", words[1]);
		return 1;
		break;
	case k_done:
		if (reading_kt_values)
		{
			reading_kt_values = FALSE;
			printf("done reading kt values\n");
		}
		return 1;
		break;
	case k_lc_init:
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		if (lc_rate_set[group][sex] == NULL)
		{
			lc_rate_set[group][sex] = NEW(struct lc_rates);
			numgroups = (group > numgroups) ? group : numgroups;
			for (i = 0; i < NUMLC; i++)
			{
				lc_rate_set[group][sex]->ck[i] = 0;
			}
			if (nwords == 4)
			{
				sscanf(words[3], "%d", &(kt_start_year));
				lc_rate_set[group][sex]->start_year = kt_start_year;
				lc_rate_set[group][sex]->ck[K_START_YEAR] = 1;
				printf("start year for %d %s %d\n", group, words[2],
					   lc_rate_set[group][sex]->start_year);
			}
		}
		else
		{
			printf("l c rates  for %d %s previously initialized\n",
				   group, words[2]);
		}
		return 1;
		break;
	case k_lc_ax:
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		if (lc_rate_set[group][sex] == NULL)
		{
			l_error(cx, "uninitialized lc rate set");
			return -1;
		}
		lc_rate_set[group][sex]->ck[AX] = 1;
		current_block = NEW(struct age_block);
		current_block->previous = NULL;
		lc_rate_set[group][sex]->ax_bx = current_block;
		recall = current_block;
		read_ax_or_bx = AX;
		numgroups = (group > numgroups) ? group : numgroups;
		return 1;
		break;
	case k_lc_bx:
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		if (lc_rate_set[group][sex] == NULL)
		{
			l_error(cx, "uninitialized lc rate set");
			return -1;
		}
		lc_rate_set[group][sex]->ck[BX] = 1;
		if (!lc_rate_set[group][sex]->ck[AX])
		{
			l_error(cx, "bx without preceding ax");
		}
		else
		{
			current_block = lc_rate_set[group][sex]->ax_bx;
			recall = current_block;
		}
		read_ax_or_bx = BX;
		return 1;
		break;
	case k_lc_k_val:
	case k_lc_k_mean:
	case k_lc_k_std_dev:
	case k_lc_k_start_list:
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		last_kt_index[group][sex] = -1;
		if (lc_rate_set[group][sex] == NULL)
		{
			l_error(cx, "uninitialized lc rate set");
			return -1;
		}
		switch (k)
		{
		case k_lc_k_val:
			sscanf(words[3], "%lf",
				   &(lc_rate_set[group][sex]->val));
			lc_rate_set[group][sex]->ck[K_VAL] = 1;
			printf("val %lf\n", lc_rate_set[group][sex]->val);
			lc_rate_set[group][sex]->start_year = -1;
			numgroups = (group > numgroups) ? group : numgroups;
			break;
		case k_lc_k_mean:
			sscanf(words[3], "%lf",
				   &(lc_rate_set[group][sex]->mean));
			lc_rate_set[group][sex]->ck[K_MEAN] = 1;
			current_kt_index[group][sex] = 01;
			last_kt_index[group][sex] = -1;
			printf("mean %lf\n", lc_rate_set[group][sex]->mean);
			numgroups = (group > numgroups) ? group : numgroups;
			break;
		case k_lc_k_std_dev:
			sscanf(words[3], "%lf",
				   &(lc_rate_set[group][sex]->std_dev));
			lc_rate_set[group][sex]->ck[K_STD_DEV] = 1;
			printf("sd %lf\n", lc_rate_set[group][sex]->std_dev);
			numgroups = (group > numgroups) ? group : numgroups;
			break;
		case k_lc_k_start_list:
			reading_kt_values = TRUE;
			if (lc_rate_set[group][sex]->ck[K_START_YEAR] != 1)
			{
				l_error(cx, "need lc start year");
				return -1;
			}
			break;
		}
		return 1;
		break;
	case k_lc_epsilon:
		group = read_integer(cx, words[1]);
		sex = keyword_to_integer(cx, l_lookup_keyword(words[2]));
		e_index = read_integer(cx, words[3]);
		max_e_index = MAX(e_index * 12 + 11, max_e_index);
		current_table = NEW(struct age_table);
		for (i = 0; i < 12; i++)
		{
			lc_epsilon[group][sex][e_index * 12 + i] = current_table;
		}
		reading_rate_table = TRUE;
		reading_epsilons = TRUE;
		recall_table = current_table;
		return 1;
		break;
	case k_random_epsilon:
		random_epsilon = read_integer(cx, words[1]);
		if (random_epsilon)
			printf("randomized epsilons used\n");
		if (fixed_epsilon)
		{
			l_error(cx, "fixed and random epsilon cannot be set");
			return -1;
		}
		break;
	case k_fix_epsilon:
		group = read_integer(cx, words[1]);
		epsilon = read_integer(cx, words[2]);
		fixed_epsilon = 1;
		if (random_epsilon)
		{
			l_error(cx, "fixed and random epsilon cannot be set");
			return -1;
		}
		return 1;
		break;
	case k_unknown:
	default:
		l_error(cx, "Syntax error");
		return -1;
	}
out:
	//Rcpp::Rcout << "18b-l_process_line. just before out" << std::endl;
	return 0;
}

/*
	   * Break a line up into words, and return the number of words scanned.
	   * The original line is destroyed.
	   */
char l_scan_line(char *line,char **words)
{
	register char *p, *q = NULL;
	register char **wp = words;
	register int state;

	for (p = line, state = 0; state >= 0; p++)
		switch (state)
		{
		case 0:
			/* initial, blank skipping */
			switch (*p)
			{
			case ' ':
			case '\t':
				break;
			case '\0':
			case '\n':
			case '#':
				state = -1;
				break;
			case '"':
				q = *wp++ = p;
				state = 2;
				break;
			default:
				q = (*wp++ = p) + 1;
				state = 1;
			}
			break;
		case 1:
			/* word */
			switch (*p)
			{
			case '#':
				*q = '\0';
				state = -1;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\0':
				*q = '\0';
				state = 0;
				break;
			case '"':
				state = 2;
				break;
			default:
				*q++ = *p;
			}
			break;
		case 2:
			/* quoted word */
			switch (*p)
			{
			case '\\':
				state = 3;
				break;
			case '"':
				state = 1;
				break;
			case '\0':
				*q = '\0';
				state = -1;
				break;
			default:
				*q++ = *p;
				state = 2;
			}
			break;
		case 3:
			/* escaped character */
			switch (*p)
			{
			case '\0':
			case '\n':
				*q = '\0';
				state = -1;
				break;
			default:
				*q++ = *p;
			}
			break;
		}
	*wp = 0;
	return wp - words;
}

enum l_keyword l_lookup_keyword(register char *word)
{
	/**************************************************************
     ** this is used to decode .sup files the first element is the
     ** string that may or may not be in the .sup file the second
     ** element is the enumarated type that defines a case in the switch
     **************************************************************/

	struct tab
	{
		char *name;
		enum l_keyword token;
	};

	static struct tab tab[] = {
		"birth",
		k_birth,
		"death",
		k_death,
		"divorce",
		k_divorce,
		"marriage",
		k_marriage,
		"cohab_prob",
		k_cohab_prob,
		"birthtarget",
		k_birthtarget,
		"transittarget",
		k_transittarget,
		"take_census",
		k_take_census,
		"sex_ratio",
		k_sex_ratio,
		"proportion_male",
		k_sex_ratio,
		"input_file",
		k_input_file,
		"output_file",
		k_output_file,
		"execute",
		k_execute,
		"include",
		k_include,
		"end",
		k_end,
		"single",
		k_single,
		"married",
		k_married,
		"divorced",
		k_divorced,
		"widowed",
		k_widowed,
		"cohabiting",
		k_cohabiting,
		"F",
		k_female,
		"M",
		k_male,
		"segments",
		k_segments,

		"done",
		k_done,
		"run",
		k_run,
		"read_xtra",
		k_read_xtra,
		"size_of_xtra",
		k_size_of_xtra,
		"float",
		k_float,
		"lc_init",
		k_lc_init,
		"lc_ax",
		k_lc_ax,
		"lc_bx",
		k_lc_bx,
		"lc_k_val",
		k_lc_k_val,
		"lc_k_mean",
		k_lc_k_mean,
		"lc_k_std_dev",
		k_lc_k_std_dev,
		"lc_k_start_list",
		k_lc_k_start_list,
		"lc_epsilon",
		k_lc_epsilon,
		"fix_epsilon",
		k_fix_epsilon,
		"random_epsilon",
		k_random_epsilon,
		"random_father",
		k_random_father,
		"random_father_min_age",
		k_random_father_min_age,
		"integer",
		k_integer,
		"double",
		k_double,
		"string",
		k_string,
		"alpha",
		k_alpha,
		"betaT",
		k_betaT,
		"bint",
		k_bint,
		"duration_specific",
		k_duration_specific,
		"factor",
		k_factor,
		"hhmigration",
		k_hhmigration,
		"hetfert",
		k_hetfert,
		"endogamy",
		k_endogamy,
		"write_output",
		k_write_output,
		"child_inherits_group",
		k_child_inherits_group,
		"duration",
		k_duration,
		"firstyear",
		k_firstyear,
		"transit",
		k_transit,
		"hettrans",
		k_hettrans,
		"agedif_marriage_mean",
		k_agedif_marriage_mean,
		"agedif_marriage_sd",
		k_agedif_marriage_sd,
		"marriage_eval",
		k_marriage_eval,
		"marriage_queues",
		k_marriage_queues,

		"marriage_peak_age",
		k_marriage_peak_age,
		"marriage_slope_ratio",
		k_marriage_slope_ratio,
		"marriage_agedif_max",
		k_marriage_agedif_max,
		"marriage_agedif_min",
		k_marriage_agedif_min,
		"parameter0",
		k_parameter0,
		"parameter1",
		k_parameter1,
		"parameter2",
		k_parameter2,
		"parameter3",
		k_parameter3,
		"parameter4",
		k_parameter4,
		"parameter5",
		k_parameter5,
		"parameter6",
		k_parameter6,
		"parameter7",
		k_parameter7,
		"parameter8",
		k_parameter8,
		"parameter9",
		k_parameter9,
		"parameter10",
		k_parameter10,
		0,
		k_unknown,
	};
	register struct tab *p;
	/* heads up p is not a person */
	for (p = tab;
		 p->name && (*p->name != *word || strcmp(p->name, word) != 0);
		 p++)

		;
	return p->token; /* k_unknown if no match*/
}

l_create_rateset(register struct l_context *cx, char *name)
{
	cx->file = name;
	printf("creating rate set\n");
}

void l_error(struct l_context *cx, char *message)
	/* error message for load()  tells where error occurred in .sup file */
{
	/*  error("\"%s\" line %d: %s",  cx->file,   cx->lineno, message);*/

    fprintf(fd_log, "l_error - file: %s line:%d : %s",
			cx->file,
			cx->lineno,
			message);
}

void add_rate_table(int years, int months, double prob)
{
	if ((prob < 0) && (!reading_epsilons))
		error("\"Rate table contains a negative probability\"");

	current_table->upper_age = 12 * years + months;
	current_table->prob = prob;
	if (current_table->upper_age < MAXUMONTHS)
	{
		current_table->next = NEW(struct age_table);
		current_table = current_table->next;
	}
	else
	{
		/*
	print_prob_table(recall_table);
	*/
		reading_rate_table = FALSE;
		reading_epsilons = FALSE;
		current_table->next = NULL;
		recall_table = NULL;
	}
}

void add_rate_block(int years, int months, double prob) 
{

	if (prob < 0)
		error("\"Rate file contains a negative rate\"");
	if (prob == 0)
		prob = 0.000000000001;

	current_block->upper_age = 12 * years + months;

	if (current_block->previous == NULL)
	{
		current_block->width = current_block->upper_age;
		/*
	printf("taking NULL part\n");
	*/
	}
	else
		current_block->width = current_block->upper_age -
							   current_block->previous->upper_age;

	if (event == E_BIRTH)
	{
		current_block->lambda = -prob;
		if (current_block->lambda > -0.000000000000001)
			current_block->lambda = -0.000000000000001;
			/*
	printf("current birth rate %g\n", current_block->lambda);
      */
#ifdef ENHANCED
		current_block->mult = enhance_usefertmult;
#else
		current_block->mult = usefertmult;
#endif
	}
	else if ((event > NONTRANSIT) && (event < NUMEVENTS))
	{
		current_block->lambda = log(1 - prob);
		current_block->mult = usetransmult;
	}
	else
	{
		/* death, marriage, divorce */
		current_block->lambda = log(1 - prob);
		current_block->mult = ident;
		if (event == E_DEATH)
		{
#ifdef ENHANCED
			current_block->mult = enhance_usedeathmult;
#else
			current_block->mult = usedeathmult;
#endif
		}
	}

	current_block->mu = 0;

	if (current_block->upper_age < MAXUMONTHS)
	{
		Rcpp::Rcout << "18b-add_rate_block| current_block->upper_age < MAXUMONTHS | " << current_block->upper_age  << " upper age | "<< std::endl;
		current_block->next = NEW(struct age_block);
		current_block->next->previous = current_block;
		current_block = current_block->next;
	}
	else
	{
		Rcpp::Rcout << "18b-add_rate_block| current_block->upper_age >= MAXUMONTHS | " << current_block->upper_age  << " upper age |---------last block "<< std::endl;
		current_block->next = NULL;
		recall = NULL;
	}
}

void add_lc_rate_block(int years, int months, double prob)
{
	Rcpp::Rcout << "18b-add_lc_rate_block| | " << current_block->upper_age  << " upper age | " << read_ax_or_bx << std::endl;
	if (read_ax_or_bx == BX)
	{
		if (current_block->upper_age != 12 * years + months)
		{
			/*
	    printf(" block %d compute %d\n",
		current_block->upper_age, 12 * years + months);
	    */
	   	    Rcpp::Rcout << "18b-add_lc_rate_block BX.3| | " << current_block->upper_age  << " upper age | " << read_ax_or_bx << std::endl;
			exit(-1);
			error("\"Rate file ax and bx out of alignment\"");
		}
	}
	else
	{
		current_block->upper_age = 12 * years + months;
	}

	if (current_block->previous == NULL)
	{
		current_block->width = current_block->upper_age;
		/*
	printf("taking NULL part\n");
	*/
	}
	else
		current_block->width = current_block->upper_age -
							   current_block->previous->upper_age;

	if (read_ax_or_bx == AX)
	{
		current_block->lambda = prob;
		current_block->mult = ident;
	}
	else
	{
		current_block->mu = prob;
	}

	if ((current_block->upper_age < MAXUMONTHS) && (read_ax_or_bx == AX))
	{
		current_block->next = NEW(struct age_block);
		current_block->next->previous = current_block;
	}

	if (current_block->upper_age < MAXUMONTHS)
	{
		current_block = current_block->next;
	}
	else
	{
		current_block->next = NULL;

		/*
	if (read_ax_or_bx == BX) {
	    print_rate_table(recall);
	}
	*/

		read_ax_or_bx = -1;
		recall = NULL;
	}
}

int	read_integer(struct l_context *cx,char *w)
{
	/*
	printf("atoi output %d\n", atoi(w));
	*/
	return atoi(w);
}

int	keyword_to_integer(struct l_context *cx, enum l_keyword k)
{
	switch (k)
	{
	case k_birth:
		return E_BIRTH;
		break;
	case k_death:
		return E_DEATH;
		break;
	case k_divorce:
		return E_DIVORCE;
		break;
	case k_marriage:
		return E_MARRIAGE;
		break;
	case k_transit:
		return NONTRANSIT;
		break;
		/*	case k_transit1:
	    return TRANSIT1;
	    break;
	case k_transit2:
	    return TRANSIT2;
	    break;
	case k_transit3:
	    return TRANSIT3;
	    break;
	    */
	case k_female:
		/*
	    printf("FEMALE\n");
	    */
		return FEMALE;
		break;
	case k_male:
		/*
	    printf("MALE\n");
	    */
		return MALE;
		break;
	case k_single:
		/*
	    printf("SINGLE\n");
	    */
		return SINGLE;
		break;
	case k_married:
		/*
	    printf("MARRIED\n");
	    */
		return MARRIED;
		break;
	case k_divorced:
		/*
	    printf("DIVORCED\n");
	    */
		return DIVORCED;
		break;
	case k_widowed:
		/*
	    printf("WIDOWED\n");
	    */
		return WIDOWED;
		break;
	case k_cohabiting:
		/*
	    printf("COHABITING\n");
	    */
		return COHABITING;
		break;
	default:
		l_error(cx, "Syntax error");
		return -1;
	}
}

/* this version is mainly cribbed from the Pascal */
/* Lee-Carter checking is a grafted on */

int fill_rate_gaps()
{
	int s, p, g, m, e, i;
	int expected;
	struct lc_rates *lc;
	struct age_block *cur, *zero_block, *zero_fert_block;
	double prev = 0;

	/* which rates are LC rates? Are they complete? This is as
 * good a place as any to fill the ax, bx, and kt_val arrays.
 * Also set the death rate set pointers to the ax_bx structure
 * to allow the right kind of default.
 * Marital status always defaults, so just go through the group
 * defaults.
 */

	/* Tue Aug  1 17:02:46 PDT 2006
 * Currently the routine does a return -1 if certain rate sets are
 * null at a minimum it should exit because the program will crash
 * if the birth_rate_set is not filled in -- and it won't be if
 * single male mort rates are missing.
 
 *** a better strategy is to point to a zero rate block and
 *** warn that certain events are logically impossible given the
 *** rates.  That's ok e.g a simulation with only females.

*/

	/*** create a block of zero rates for fert mort &etc **/
	/*** it is NOT ok to use the same struct for both birth_rates 
	 and the other rates b/c when initialize_segment_vars unwinds
    the rate set structures, it will free() the zero block MORE than
    once causing extremely suboptimal behavior **/

	//logmsg("Rates imply simulation will have %d groups\n", " ", numgroups);
	fprintf(fd_log, "Simulation will have %i groups - fprintf \n",numgroups);
	fprintf(fd_log, "Simulation will have %i groups in ipop - fprintf \n",numgroups_in_ipop);
	//logmsg("Initial population has max group id %d \n", " ",numgroups_in_ipop);
	if (numgroups != numgroups_in_ipop)
	{
		numgroups = MAX(numgroups, numgroups_in_ipop);
	}
	
	fprintf(fd_log, "Simulation will have %i groups - fprintf \n",numgroups);
	
	zero_block = NEW(struct age_block);
	zero_block->previous = NULL;
	zero_block->upper_age = MAXUMONTHS;
	zero_block->width = MAXUMONTHS;

	zero_block->lambda = -0.00000000000001;
	zero_block->mult = usenullmult;
	zero_block->mu = 0;
	zero_block->next = NULL;

	zero_fert_block = NEW(struct age_block);
	zero_fert_block->previous = NULL;
	zero_fert_block->upper_age = MAXUMONTHS;
	zero_fert_block->width = MAXUMONTHS;

	zero_fert_block->lambda = -0.0000000000001;
	zero_fert_block->mult = usenullmult;
	zero_fert_block->mu = 0;
	zero_fert_block->next = NULL;

	for (s = MALE; s < NUMSEXES; s++)
	{
		for (g = 1; g <= numgroups; g++)
		{
			if (lc_rate_set[g][s] != NULL)
			{
				int prod1, prod2;
				int i, j;

				lc = lc_rate_set[g][s];
				prod1 = lc->ck[AX] * lc->ck[BX] * lc->ck[K_VAL] * lc->ck[K_MEAN] * lc->ck[K_STD_DEV];
				prod2 = lc->ck[AX] * lc->ck[BX] * lc->ck[K_START_YEAR];
				if (prod1 + prod2 < 1)
				{
					error("\"incomplete LC rate set\"");
					return -1;
				}
				else if (prod1 + prod2 > 1)
				{
					error("\"inconsistent specifiation for LC rate set\"");
					return -1;
				}
				else
				{
					int offset = (current_month % MAXUMONTHS);
					printf("checking current month %d\n", current_month);

					/* cover the completeness check */
					rate_set[g][E_DEATH][s][SINGLE] = lc->ax_bx;

					/* kt's vary over time */
					kt_vals[g][s] =
						(double *)malloc(MAXUMONTHS * sizeof(double));

					/* do the calculations for linear
				 * continutation in the assigned kt_case */
					if (prod2 == 1)
					{
						/*
			printf("using last index %d\n", last_kt_index[g][s]);
			*/
						lc->mean = (read_kt_vals[g][s][last_kt_index[g][s]] - read_kt_vals[g][s][0]) / last_kt_index[g][s];
						printf("computed mean %lf\n", lc->mean);
						lc->std_dev = 0;
						for (current_kt_index[g][s] = 0;
							 current_kt_index[g][s] < MIN(MAXUYEARS, last_kt_index[g][s]);
							 current_kt_index[g][s]++)
						{
							prev = read_kt_vals[g][s][current_kt_index[g][s]];
							/*
			    printf("using prev %lf at index %d\n",
				prev, current_kt_index[g][s]);
			    */
							for (j = 0; j < 12; j++)
							{
								*(kt_vals[g][s] + offset) = prev;
								offset = (offset + 1) % MAXUMONTHS;
							}
						}

						/* do the linear continuation if necessary */
						for (i = current_kt_index[g][s]; i < MAXUYEARS; i++)
						{
							prev = prev + lc->mean + lc->std_dev * normal();
							/*
			    prev = prev + lc->mean ;
			    */
							for (j = 0; j < 12; j++)
							{
								*(kt_vals[g][s] + offset) = prev;
								offset = (offset + 1) % MAXUMONTHS;
							}
						}
					}
					/*
			    printf("using prev %lf at index %d\n", prev, i);
			    */

					if (prod1 == 1)
					{
						/*
			current_kt_index[g][s] = 0;
			*/
						prev = lc->val;
						for (j = 0; j < 12; j++)
						{
							*(kt_vals[g][s] + offset) = prev;
							offset = (offset + 1) % MAXUMONTHS;
						}
						for (i = 1; i < MAXUYEARS; i++)
						{
							prev = prev + lc->mean + lc->std_dev * normal();
							/*
			    prev = prev + lc->mean ;
			    */
							for (j = 0; j < 12; j++)
							{
								*(kt_vals[g][s] + offset) = prev;
								offset = (offset + 1) % MAXUMONTHS;
							}
						}
					}

					/*
		    printf("group %d sex %d mean %lf \n", g, s, lc->mean);
		    for (i = 0; i < MAXUMONTHS; i++) {
			printf("i %d kt %lf\n", i,  *(kt_vals[g][s] + i));
		    }
		    */

					/* ax and bx do not */
					ax_vals[g][s] =
						(double *)malloc(MAXUMONTHS * sizeof(double));
					bx_vals[g][s] =
						(double *)malloc(MAXUMONTHS * sizeof(double));
					cur = lc->ax_bx;
					i = 0;
					while (cur != NULL)
					{
						while (i < cur->upper_age)
						{
							*(ax_vals[g][s] + i) = cur->lambda;
							*(bx_vals[g][s] + i) = cur->mu;
							i++;
						}
						cur = cur->next;
					}
				}
			}
		}
	}

	/* do the default by group */
	for (s = MALE; s < NUMSEXES; s++)
	{
		/* Thu Mar 11 15:59:01 PST 2010 changing to numgroups 
	 for (g = 2; g < MAXGROUPS; g++) { */
		for (g = 2; g <= numgroups; g++)
		{
			if ((lc_rate_set[g][s] == NULL) && (lc_rate_set[g - 1][s] != NULL))
			{
				lc_rate_set[g][s] = lc_rate_set[g - 1][s];
				ax_vals[g][s] = ax_vals[g - 1][s];
				bx_vals[g][s] = bx_vals[g - 1][s];
				kt_vals[g][s] = kt_vals[g - 1][s];
			}
		}
	}

	/* need birth rates for group 1 parity 0 married and single */
	/** default for group1/married or single/parity0 == ZERO **/
	if (birth_rate_set[1][MARRIED][0] == NULL)
	{
		birth_rate_set[1][MARRIED][0] = zero_fert_block;
	}
	if (birth_rate_set[1][SINGLE][0] == NULL)
	{
		birth_rate_set[1][SINGLE][0] = zero_fert_block;
	}

	/* need rates for group 1 single male and female for death, marriage*/
	/* need rates for group 1 married male and female for divorce */
	for (s = MALE; s < NUMSEXES; s++)
	{
		if (rate_set[1][E_DEATH][s][SINGLE] == NULL)
		{

			rate_set[1][E_DEATH][s][SINGLE] = zero_block;
		}

		if (rate_set[1][E_MARRIAGE][s][SINGLE] == NULL)
		{
			rate_set[1][E_MARRIAGE][s][SINGLE] = zero_block;
		}

		if (rate_set[1][E_DIVORCE][s][MARRIED] == NULL)
		{
			rate_set[1][E_DIVORCE][s][MARRIED] = zero_block;
		}
	}

	/* now do the defaults */
	/* birth rates, group 1
 * set parity 0 rates to the rates for the previous group
 * set higher-parity rates to the rates for the next-lower parity
 *
 * Note that rates for cohabiting women are equal to those for married
 * women (unless otherwise specified).
 */

	/*
    for (p = 0; p < MAXPARITY; p++) {
	if (birth_rate_set[1][MARRIED][p] == NULL) {
	    printf("null rate set for married parity %d\n", p);
	} else {
	    printf("non-null rates for married parity %d\n", p);
	}
    }
    */

	/* heads up -- birth_rate_set[1][married,single][0] is set to zero above
       So married birthrates default to ZERO NOT to the single rate */

	for (m = SINGLE; m <= COHABITING; m++)
	{
		if (m > SINGLE)
			if (birth_rate_set[1][m][0] == NULL)
				birth_rate_set[1][m][0] = birth_rate_set[1][m - 1][0];
		for (p = 1; p < MAXPARITY; p++)
			if (birth_rate_set[1][m][p] == NULL)
				birth_rate_set[1][m][p] = birth_rate_set[1][m][p - 1];
	}

	for (m = SINGLE; m <= COHABITING; m++)
	{
		/*	for (g = 2; g < MAXGROUPS; g++) {*/
		for (g = 2; g <= numgroups; g++)
		{
			for (p = 0; p < MAXPARITY; p++)
			{
				if (birth_rate_set[g][m][p] == NULL)
					birth_rate_set[g][m][p] = birth_rate_set[g - 1][m][p];
			}
		}
	}

	/* similarly for the non-birth rates */
	/* start with group 1 and default back by marital status, avoiding
 * polygamy for the moment (though it can be set)
 */

	for (e = E_MARRIAGE; e <= E_DEATH; e++)
		for (s = MALE; s < NUMSEXES; s++)
			for (m = SINGLE; m <= COHABITING; m++)
			{
				if ((e == E_MARRIAGE) && (m == MARRIED))
				{
					continue;
				}
				else
				{
					if (rate_set[1][e][s][m] == NULL)
					{
						rate_set[1][e][s][m] = rate_set[1][e][s][m - 1];
						if (duration_specific[1][e][s][m - 1])
						{
							duration_specific[1][e][s][m] = 1;
							printf("group 1 duration-specific event %s sex %s ms %s\n",
								   index_to_event[e],
								   index_to_sex[s],
								   index_to_mstatus[m]);
						}
					}
				}
			}

	/*
printf("filling in rates for event %s sex %s mstatus %s\n",
index_to_event[e], index_to_sex[s], index_to_mstatus[m]);
*/
	for (e = E_MARRIAGE; e <= E_DEATH; e++)
	{
		for (s = MALE; s < NUMSEXES; s++)
		{
			/* Thu Mar 11 16:00:24 PST 2010
	     for (g = 2; g < MAXGROUPS; g++) { */
			for (g = 2; g <= numgroups; g++)
			{
				for (m = SINGLE; m <= COHABITING; m++)
				{
					if (rate_set[g][e][s][m] == NULL)
					{
						rate_set[g][e][s][m] = rate_set[g - 1][e][s][m];

						if ((e == E_DEATH) && (rate_set[g][e][s][m] == NULL))
						{
							/** still null **/
							sprintf(logstring,
									"WARNING: rates for %s %s %s group %d are missing and default to  NULL\n",
									index_to_mstatus[m],
									index_to_sex[s],
									index_to_event[e],
									g);
							logmsg("%s", logstring, 1);
						}
						if (duration_specific[g - 1][e][s][m])
						{
							duration_specific[g][e][s][m] = 1;

							printf("group %d duration-specific event %s sex %s ms %s\n",
								   g,
								   index_to_event[e],
								   index_to_sex[s],
								   index_to_mstatus[m]);
						}
					}
				}
			}
		}
	}

	/* Tue Mar 16 12:32:18 PDT 2010  make sure transit events rates are null
       for transferring into undefined groups.
       Fri Apr  9 14:54:04 PDT 2010
       This causes num_segments to be reset to zero in 64bit version. 
       It *should* be redundant code since rate_set is set to null in
       initialize_segment_variables. 
    */
	/*
    for (e = NONTRANSIT+1+numgroups; e <  NUMEVENTS; e++){
      for(g=1; g<= MAXGROUPS; g++){
	for(m= SINGLE; m<= COHABITING;m++){
	  for(s=MALE; s< NUMSEXES; s++){
	    rate_set[g][e][s][m]=NULL;
	}
      }
    }
    }
    */

	/* adjust to reflect birth targets */
	for (g = 1; g <= numgroups; g++)
	{
		if (birth_target[g] > 0)
		{
			printf("group %d target %d\n", group, birth_target[g]);
			int Bexpected = get_expected_number_of_births(g);
			if (Bexpected <= 0)
			{
				char logsting[256];
				sprintf(logstring, "ERROR birthtarget is set BUT  group %d has 0 or fewer expected births. This could result from the init pop having no females of childbearing age. Disable birthtarget for this group if you want results.", g);
				logmsg("%s\n", logstring, 1);
				stop("-1 fill-rate-gaps - Birthtarget is set But grou .......");
				//exit(-1);
			}
			else
			{
				birth_adjust[g] = (double)(birth_target[g] / Bexpected);

				printf("group %d expected number %d target %d adjust %lf  \n",
					   g, Bexpected, birth_target[g], birth_adjust[g]);
			}
		}
		for (dest = 1; dest <= numgroups; dest++)
		{
			if (transit_target[g][dest] > 0)
			{
				printf("group %d dest %d target %d\n", group, dest,
					   transit_target[g][dest]);
				expected = get_expected_number_of_transits(g, dest);
				if (expected == 0)
				{
					printf("rate and/or pop at risk of transit from %d to %d is 0\n",
						   group, dest);
					printf("no increase possible\n");
				}
				else
				{
					if (expected > size_of_pop[g])
					{
						printf("number of migrants cannot exceed pop. base\n");
						expected = MIN(size_of_pop[g], expected);
					}
					transit_adjust[g][dest] =
						(double)transit_target[g][dest] / expected;
					printf("group %d dest %d expected number %d ",
						   g, dest, expected);
					printf("target %d adjust %lf \n",
						   transit_target[g][dest], transit_adjust[g][dest]);
				}
			}
		}
	}

	/*
    printf("maximum e index %d\n", max_e_index + 1);
    for (s = MALE; s < NUMSEXES; s++) {
	for (i = 0; i <= MAXKT; i++) {
	    printf("group %d index %d index mod max %d\n",
		s, i, i % (max_e_index + 1));
	    print_prob_table(lc_epsilon[1][s][i % (max_e_index + 1)]);
	}
    }
    */

	warn_about_rates(zero_block, zero_fert_block);

	return 1;
}

/************************************************************************/

void warn_about_rates( struct age_block *zero_block, struct age_block *zero_fert_block)
{

	/* This will check for rate situations that might be 
     wrong */

	char logstring[256];
	int rates_test = 0, group, s, grp, event, sex, mstat;
	struct age_block *crnt;

	/* missing key fert rates */
	logmsg("\n checking rates for sanity\n", "", 1);
	/*  for(group=1;group < MAXGROUPS; group ++){ */
	for (group = 1; group <= numgroups; group++)
	{
		if (birth_rate_set[group][MARRIED][0] != zero_fert_block)
		{
			rates_test++;
		}
	}
	if (rates_test == 0)
	{
		logmsg("   ***NO BIRTH RATES specified for parity 0 married females"
			   "of ANY group \n",
			   "", 1);
	}

	/* missing key marriage rates */

	rates_test = 0;
	for (s = MALE; s < NUMSEXES; s++)
	{
		/* for(group=1;group < MAXGROUPS; group ++){ */
		for (group = 1; group <= numgroups; group++)
		{
			if (rate_set[group][E_MARRIAGE][s][SINGLE] != zero_block)
			{
				rates_test++;
			}
		}
		if (rates_test == 0)
		{
			char logstring[256];
			sprintf(logstring,
					"   ***NO MARRIAGE RATES for for  SINGLE %s "
					"of ANY group\n",
					index_to_sex[s]);
			logmsg("%s\n", logstring, 1);
		}
	}
	rates_test = 0;

	if (marriage_queues == 1)
	{
		rates_test = 0;
		for (group = 1; group <= numgroups; group++)
		{
			for (mstat = SINGLE; mstat <= WIDOWED; mstat++)
			{
				if (rate_set[group][E_MARRIAGE][MALE][mstat] != zero_block)
				{
					rates_test++;
				}
			}
		}
		if (rates_test > 0)
		{

			logmsg("%s\n", "WARNING marriage_queues==1  yet marriage rates are \
specified for unmarried males THESE RATES WILL BE IGNORED.",
				   1);
			/* exit(-1);*/

			/*Zero those male marriage rates in a more ambitious world
	I would recover the RAM but ..*/
			int g, mstat;
			for (g = 0; g < MAXGROUPS; g++)
			{
				for (mstat = SINGLE; mstat < NUMMARSTATUS; mstat++)
				{
					rate_set[g][E_MARRIAGE][MALE][mstat] = NULL;
				}
			}
		}
		rates_test = 0;
	}
	/* missing key death rates */

	for (s = MALE; s < NUMSEXES; s++)
	{
		rates_test = 0;
		/*    for (group =1; group < MAXGROUPS ; group++){ */
		for (group = 1; group <= numgroups; group++)
		{
			if (rate_set[group][E_DEATH][s][SINGLE] != zero_block)
			{
				rates_test++;
			}
		}
		if (rates_test == 0)
		{
			char logstring[512];
			sprintf(logstring,
					"   ***NO  DEATH RATES specified for  SINGLE %ss \n"
					"   ***of ANY group This is unusual -- are you SURE you know \n"
					"   ***what you\'re doing\n",
					index_to_sex[s]);
			logmsg("%s\n", logstring, 1);
		}
	}

	/* width zero rates */

	/*  for (grp =1; grp < MAXGROUPS ; grp++){*/
	for (grp = 1; grp <= numgroups; grp++)
	{
		for (event = E_MARRIAGE; event < NUMEVENTS; event++)
		{
			for (sex = MALE; sex < NUMSEXES; sex++)
			{
				for (mstat = SINGLE; mstat < NUMMARSTATUS; mstat++)
				{
					crnt = rate_set[grp][event][sex][mstat];
					if (crnt != NULL && crnt != zero_block)
					{
						/*
	    sprintf(logstring,
			" CHECKING: group: %d - %s  - %s - %s",
			grp,index_to_event[event],index_to_sex[sex],
			index_to_mstatus[mstat]);
			logmsg("%s\n",logstring); */
						while (crnt != NULL)
						{
							if (crnt->width <= 0)
							{
								sprintf(logstring,
										"* - - - - - - - - - - - - - - - - - - - - - - \n"
										"* group: %d event: %s  sex: %s mstatus: %s \n"
										"* upper age: %d  has rate with zero duration\n"
										"* - - - - - - - - - - - - - - - - - - - - - - \n",
										grp, index_to_event[event], index_to_sex[sex],
										index_to_mstatus[mstat], crnt->upper_age);
								logmsg("%s\n", logstring,0);
							}
							crnt = crnt->next;
						}
					}
				}
			}
		}
	}
}

dump_rates( int mgroup)
{

	if ((mgroup < 1) || (mgroup > numgroups))
	{
		mgroup = numgroups;
	}
	int g, e, s, m, p;
	/*    for (g = 1; g <= numgroups; g++) */
	for (g = 1; g <= mgroup; g++)
		for (e = 0; e < NUMEVENTS; e++)
			for (s = 0; s < NUMSEXES; s++)
				for (m = 0; m < NUMMARSTATUS; m++)
				{
					if (rate_set[g][e][s][m] != NULL)
					{
						printf(" group: %d event %s sex %s mstatus %s \n",
							   g, index_to_event[e], index_to_sex[s],
							   index_to_mstatus[m]);
						print_rate_table(rate_set[g][e][s][m]);
					}
				}

	/*    for (g = 1; g <= numgroups; g++) **/
	for (g = 1; g <= mgroup; g++)
		for (p = 0; p < MAXPARITY; p++)
			for (m = 0; m < NUMMARSTATUS; m++)
			{
				if (birth_rate_set[g][m][p] != NULL)
				{
					printf(" group: %d birth parity %d mstatus %s \n",
						   g, p, index_to_mstatus[m]);
					print_rate_table(birth_rate_set[g][m][p]);
				}
			}
}

void print_rate_table(struct age_block *r)
{
	struct age_block *cur;
	cur = r;
	while (cur != NULL)
	{
		printf("UPPER AGE: %d  WIDTH: %d PROB1: %f  PROB2: %f\n",
			   cur->upper_age, cur->width, cur->lambda, cur->mu);
		cur = cur->next;
	}
	/*
    */
}

print_prob_table(struct age_table *t)
{
	struct age_table *cur;
	cur = t;
	while (cur != NULL)
	{
		printf("UPPER AGE: %d   PROB1: %lf\n",
			   cur->upper_age, cur->prob);
		cur = cur->next;
	}
	/*
    */
}

/*
do_command(command)
char *command;
{
    int pid;

    switch (pid = fork()) {
    case -1:		
	error ("Fork");
	return;

    case 0:
	exec_command(command);
	break;

    default:
	printf("parent: waiting for child");
	wait_for_child(pid);
	printf("parent: child reaped\n");
	break;
    }
}

wait_for_child(pid)
int pid;
{
    int wpid;
    union wait status;

    while ((wpid = wait(&status)) >= 0) {
	if (wpid = pid)
	    break;
	fprintf(stderr, "parent: reaped %d not %d\n", wpid, pid);
    }

    if (wpid < 0) {
	perror("parent:wait");
    } else {
	fprintf("parent: pid %d exited:\n", pid);
	if (status.w_termsig)
	    printf("\tsignal %d%s",
		status.w_termsig,
		status.w_coredump ? " (core dumped) \n" : " \n");
	else
	    printf("\texit status %d\n", status.w_retcode);
    }
}

exec_command(command)
char *command;
{
    (void) execl(command, command, (char *) NULL);
    fprintf(stderr, "child: execl:");
    perror(command);
    _exit(1);
}
*/

void error(char *fmt)//,char *a,int b, char *c,char *d,char *e,char *f)
{
	char buf[1024];
	register char *p;

	(void)sprintf(buf, fmt); //a, b, c, d, e, f);
	for (p = buf; *p++;)
		;
	p[-1] = '.';
	*p = 0;
	*p++ = '\n';
	*p = 0;
	fputs(buf, stderr);
}
double get_lambda(int g, int m, int p, int age)
/* select appropriate lambda from ferts for group,mstat,parity, age a */
{
	int i;
	struct age_block *current;
	struct age_block *ferts;
	p = MAX(p, 0);
	ferts = birth_rate_set[g][m][p];
	current = ferts;
	while (age > current->upper_age)
	{
		current = current->next;
	}
	return (current->lambda);
}

/***************************************************************/

void adjust_birth_for_bint()
{
	/** implements correction for birth rates that adjusts for bint
      births are nonhomogeneous poisson process new rates are
      lambda/p(bint in force)

      p(bint in force) = 1 - sum(1 - exp(lamda_i))

      side effect: birth rates are going to varry by month

      ASSUMES that fill_rate_gaps has NOT BEEN CALLED !!!
**/
	struct age_block *top_new_fert, *new_fert, *old_fert, *prev_fert;
	int g, m, p, i, j;

	/* IF we have parity specific rates we need to add an n+1th parity
     b/c each parity rate set is bint modified for parity n-1.  0
     parity rates determine the bint mod for parity 1 and so on BUT
     the highest parity specified also covers higher unspec'ed
     parities. Thus it must be bint modified according to its own
     rates.  BUT also n-1's rates; thus and an extra rate set is
     required. sigh */

	char logstring[256];
	sprintf(logstring,
			"\n\n Adjusting fertility rates for %d month birth interval \n",
			(int)bint);
	logmsg("%s", logstring, 1);

	for (g = 1; g <= numgroups; g++)
	{
		for (m = SINGLE; m < NUMMARSTATUS; m++)
		{
			for (p = 2; p < MAXPARITY; p++)
			{
				if (birth_rate_set[g][m][p] == NULL &&
					birth_rate_set[g][m][p - 1] != NULL)
				{
					/*need to copy; pointing would be fragile see free() below*/
					new_fert = NEW(struct age_block);
					birth_rate_set[g][m][p] = new_fert;
					old_fert = birth_rate_set[g][m][(p - 1)];
					while (old_fert != NULL)
					{
						new_fert->upper_age = old_fert->upper_age;
						new_fert->width = old_fert->width;
						new_fert->lambda = old_fert->lambda;
						new_fert->mult = old_fert->mult;
						new_fert->mu = old_fert->mu;
						new_fert->modified_lambda = old_fert->modified_lambda;
						if (old_fert->next != NULL)
						{
							new_fert->next = NEW(struct age_block);
							new_fert->next->previous = new_fert;
							new_fert = new_fert->next;
							new_fert->next = NULL; /*just in case this is last*/
						}
						old_fert = old_fert->next;
					}
					break;
				}
			}
		}
	}

	for (g = 1; g <= numgroups; g++)
	{
		for (m = SINGLE; m < NUMMARSTATUS; m++)
		{
			for (p = (MAXPARITY - 1); p >= 0; p--)
			{
				/** have to max -> 0 with parity */

				if (p == 0 & birth_rate_set[g][m][1] != NULL)
				{
					break;
				}
				/* if we ONLY have parity 0 rates than they must be modified;
	   but it we have parity 1+ rates than there can be no bint affecting
	   parity zero rates so we are done with this g,m */
				if (birth_rate_set[g][m][p] != NULL)
				{
					old_fert = birth_rate_set[g][m][p];
					/* get pbint */
					double pbint[MAXUMONTHS];
					int lrange, urange;
					for (i = 0; i < MAXUMONTHS; i++)
					{
						lrange = MAX(0, i - bint);
						urange = MAX(0, i);
						pbint[i] = 0;
						for (j = lrange; j < urange; j++)
						{
							pbint[i] += 1 - exp(get_lambda(g, m, (p - 1), j)); /*lambda is -lambda*/
						}
						pbint[i] = 1 - pbint[i];
					}
					/* create new fert rate structure to replace oldfert */

					top_new_fert = new_fert = NEW(struct age_block);
					new_fert->upper_age = 0;
					new_fert->width = 1;
					new_fert->lambda = get_lambda(g, m, p, 0);
					new_fert->previous = NULL;
					new_fert->mult = old_fert->mult;
					
					Rcpp::Rcout << "18b-load.cpp->adjust_birth_for_bint. " << std::endl;
					for (i = 1; i < MAXUMONTHS; i++)
					{
						prev_fert = new_fert;
						new_fert->next = NEW(struct age_block);
						new_fert = new_fert->next;
						new_fert->next = NULL; /*last time through must point to ground*/
						new_fert->previous = prev_fert;
						new_fert->upper_age = i;
						new_fert->width = 1;
						new_fert->lambda = get_lambda(g, m, p, i) / pbint[i];
						new_fert->mult = top_new_fert->mult;
					}

					/** reassigning birth_rate pointer to modified age block array **/
					birth_rate_set[g][m][p] = top_new_fert;
					/** recovering memory from old birth_rate array **/
					while (old_fert->next != NULL)
					{
						old_fert = old_fert->next;
						free(old_fert->previous);
					}
					free(old_fert);

					struct age_block *block_ptr;
					char logstring[256];
					char endbit[10];

					sprintf(logstring, "reading fertility rates for: group=%d mstat=%d parity=%d", g, m, p);
					logmsg("%s\n", logstring, 1);
					if (birth_rate_set[g][m][p + 1] == NULL || p == MAXPARITY)
					{
						sprintf(endbit, "+\n");
					}
					else
					{
						sprintf(endbit, "\n");
					}
					logmsg("%s", endbit, 1);
					block_ptr = birth_rate_set[g][m][p];
					while (block_ptr != NULL)
					{ 
						/* lets print this stuf for now - or maybe not..
						if (-(block_ptr->lambda) > 0.00001)
						{
							sprintf(logstring, "topAge: %d  width: %d lambda: %lf \n",
									block_ptr->upper_age, block_ptr->width, -block_ptr->lambda);
							logmsg("%s", logstring, 0);
						} / */
						block_ptr = block_ptr->next;
					}
				} /* if not NULL */
			}	 /*parity*/
		}		  /*mstat*/
	}			  /*group*/
}
