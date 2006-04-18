#include "calcdb.h"
#include "bot.h"
#include "users.h"
#include "strcasestr.h"


struct random_response {
	double probability;     /* This is relative to the sum of */
				/* all probabilities in the table. */
				/* E.g. 0.1 will be 10% is sum is 1.0*/
				/* but 1% if sum is 10.0 */
	char *response;   /* %s will be substituted to calcstring */
};


static struct random_response builtin_responses[] = {
	{ 0.1,    "what is %s ?" },
	{ 0.1,    "wtf is %s ?" },
	{ 0.15,   "google it yourself" },
	{ 0.15,   "google it" },
	{ 0.01,   "SOMBRERO!" },
	{ 0.1,    "how would I know ?" },
	{ 0.1,    "I don't know" },
	{ 0.1,    "nope, don't know" },
	{ 0.1,    "why don't you google it ?" },
	{ 0.1,    "*You* tell me what is %s" },
	{ 0.1,    "naa, you don't need that" },
	{ 0.1,    "Answer to '%s' is not in the database, Sir" },
	{ 0.1,    "record not found, sir." },
	{ 0.06,   "404" },
	{ 0.05,   "errno=ENOENT" },
	{ 0.05,   "calc nicht gefunden" },
	{ 0.02,   "Le CALC non trouve dans la base de donnees." },

	/* total of all probabilities need not be exactly 1.0 */
	/* (it's auto-scaled). But for clarity, we'll keep it */
	/* not far from 1.0 */
};

#define BUILTIN_NUMRESP (sizeof(builtin_responses) / sizeof(builtin_responses[0]))

static int testmode = 0;


static int loadresponses(struct random_response ** pp_resp)
{
	FILE *fp = NULL;
	char line[1000];
	struct random_response *table = NULL;
	int count = 0, len, numresp;
	double prob;
	char *endptr, *start;

	*pp_resp = NULL;

	fp = fopen("responses.txt", "r");
	if( fp == NULL )
		goto error;

	while(fgets(line, sizeof(line), fp)) {
		if(line[0] != '#' && line[0] != '\n')
			count++;
	}
	fclose(fp);
	fp = NULL;

	if( count == 0 )
		goto error;

	table = malloc(sizeof(struct random_response) * count );
	if( table == NULL )
		goto error;
	numresp = count;

	fp = fopen("responses.txt", "r");
	if( fp == NULL )
		goto error;

	count = 0;
	while(fgets(line, sizeof(line), fp)) {
		if( count >= numresp )
			break;              // in case file changed underfoot

		len = strlen(line);
		if( len > 0 && line[len-1] == '\n') // chomp
			line[len-1] = 0;                // chomp
		if( line[0] == '#' || line[0] == 0)
			continue;           // ignore comments and empty lines

		prob = strtod( line, &endptr );
		start = endptr + strspn(endptr, " \t");
		if( endptr == line || *start == 0)
			continue;                       // parse error

		table[count].probability = prob;
		table[count].response = strdup(start);
		if( table[count].response == NULL )
			continue;

		count++;
	}

	fclose(fp);
	fp = NULL;

	*pp_resp = table;
	return numresp;

error:
	if( fp )
		fclose(fp);
	if( table )
		free(table);

	return -1;
}



void calcnotfound(char *response, int max, char *calcstring)
{
	/* Create the randomized "not found" response. */

	static int sum_initialized = 0;
	static double total = 0.0;
	int k;
	double rrand;
	static struct random_response *responses = builtin_responses;
	int numresp = BUILTIN_NUMRESP;

	if( ! sum_initialized ) {
		numresp = loadresponses( &responses );
		if( numresp <= 0 || responses == NULL ) {
			responses = builtin_responses;   // fall back
			numresp = BUILTIN_NUMRESP;       // fall back

			if( testmode )
				fprintf(stderr, 
				"File responses.txt not present, using builtin table\n" );
		} else if( testmode ) {
			fprintf(stderr, "Using responses.txt file, %d entries\n",
				numresp );
		}

							/* auto-scale */
		total = 0.0;
		for( k = 0; k < numresp; k++ ) {
			total += responses[k].probability;
		}
		sum_initialized = 1; /* hope we are not multithreaded */
	}


	strncpy( response, "calc not found.", max); /* safe net responde */

	rrand = total * ((double)rand() / (double)RAND_MAX);

	for( k = 0; k < numresp; k++ ) {
		rrand -= responses[k].probability;
		if( rrand <= 0.0 || k == numresp - 1) {
			snprintf( response, max, 
				  responses[k].response, calcstring );
			break;
		}
	}
}



void calcnotfound_test() { /* probabilities tester for calcnotfound() */
	int k;
	char resp[1000];
	FILE *fp;

	srand(time(NULL));
	testmode = 1;

	fp = popen("sort | uniq -c", "w");
	if( fp == NULL )
		fp = stdout;

	printf("(run of %d)\n", 10000 );
	for(k=0; k < 10000; k++) {
		calcnotfound( resp, sizeof(resp), "foo" );
		fprintf(fp, "%s\n", resp );
	}

	if( fp != stdout)
		pclose(fp);
}
