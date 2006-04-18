#include "calcdb.h"
#include "bot.h"
#include "users.h"
#include "strcasestr.h"

void calcnotfound(char *response, int max, char *calcstring)
{
	/* Create the randomized "not found" response. */

	struct random_response {
		double probability;  	/* This is relative to the sum of */
				     	/* all probabilities in the table. */
					/* E.g. 0.1 will be 10% is sum is 1.0*/
					/* but 1% if sum is 10.0 */
		char *response;   /* %s will be substituted to calcstring */
	};

	static struct random_response responses[] = {
	    { 0.1,    "what is %s ?" },
	    { 0.1,    "wtf is %s ?" },
	    { 0.15,   "google it yourself" },
	    { 0.15,   "google it" },
	    { 0.1,    "how would I know ?" },
	    { 0.1,    "I don't know" },
	    { 0.1,    "nope, don't know" },
	    { 0.01,   "SOMBRERO!" },
	    { 0.1,    "why don't you google it ?" },
	    { 0.05,   "it's you again?" },
	    { 0.1,    "*You* tell me what is %s" },
	    { 0.1,    "forget it" },
	    { 0.1,    "naa, you don't need that" },
	    { 0.1,    "ask me not" },

	    /* total of all probabilities need not be exactly 1.0 */
	    /* (it's auto-scaled). But for clarity, we'll keep it */
	    /* not far from 1.0 */
	};
#define NUMRESP (sizeof(responses) / sizeof(responses[0]))
	static int sum_initialized = 0;
	static double total = 0.0;
	int k;
	double rrand;

	if( ! sum_initialized ) {
							/* auto-scale */
		total = 0.0;
		for( k = 0; k < NUMRESP; k++ ) {
			total += responses[k].probability;
		}
		sum_initialized = 1; /* hope we are not multithreaded */
	}


	strncpy( response, "calc not found.", max); /* safe net responde */

	rrand = total * ((double)rand() / (double)RAND_MAX);

	for( k = 0; k < NUMRESP; k++ ) {
		rrand -= responses[k].probability;
		if( rrand <= 0.0 || k == NUMRESP - 1) {
			snprintf( response, max, 
				  responses[k].response, calcstring );
            break;
		}
	}
#undef NUMRESP
}

#if 0
void main() { /* probabilities tester for calcnotfound() */
    int k;
    char resp[1000];
    FILE *fp;

    srand(time(NULL));

    fp = popen("sort | uniq -c", "w");
    if( fp == NULL )
        fp = stdout;

    for(k=0; k < 10000; k++) {
        calcnotfound( resp, sizeof(resp), "foo" );
        fprintf(fp, "%s\n", resp );
    }

    if( fp != stdout)
        pclose(fp);
}
#endif
