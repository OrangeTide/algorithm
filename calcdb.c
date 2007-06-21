/* calcdb.c version 0.7 */
/* all versions previous to 0.333 are completely public domain */
/* all contributed code falls under the same BSD style license as */
/* noted below unless the contributing author places a copyright */
/* notice in their file/s. */


/*
 *	 * Copyright (c) 2001 David T. Stiles
 *	 * All rights reserved.
 *	 *
 *	 * Redistribution and use in source and binary forms, with or without
 *	 * modification, are permitted provided that the following conditions
 *	 * are met:
 *	 * 1. Redistributions of source code must retain the above copyright
 *	 *		notice, this list of conditions and the following disclaimer.
 *	 * 2. Redistributions in binary form must reproduce the above copyright
 *	 *		notice, this list of conditions and the following disclaimer in the
 *	 *		documentation and/or other materials provided with the distribution.
 *	 * 3. All advertising materials mentioning features or use of this software
 *	 *		must display the following acknowledgement:
 *	 *		  This product includes software developed by David T. Stiles
 *	 * 4. The name David T. Stiles may not be used to endorse or promote
 *	 *		products derived from this software without specific prior written
 *	 *		permission.
 *	 *
 *	 * THIS SOFTWARE IS PROVIDED BY DAVID T. STILES `AS IS'' AND ANY
 *	 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *	 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *	 * ARE DISCLAIMED.  IN NO EVENT SHALL DAVID T. STILES BE LIABLE
 *	 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *	 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *	 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *	 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *	 * SUCH DAMAGE.
 *	 */

/* this code would not be possible without the patience and intelligence */
/* provided by many of the people from #c/efnet. I thank all of you sincerely. */

/* the calc database code, being rewritten on 12 January 2001 */

#include "calcdb.h"
#include "bot.h"
#include "users.h"
#include "strcasestr.h"


/*
 * this holds the name of the person/channel we will talk to. set in bot.c
 */
extern char MSGTO[MAXDATASIZE];


/*
 * variables needed by many functions in the module.
 */

static char **calc = NULL;			/* will become an array of pointers, each pointer pointing to an array. */
static long total_calcs = 0;		/* ummmm, duh */
static int MAXCALCS;					/* duh again. passed into loaddb */
static char CALCDB[MAXDATASIZE]; /* passed into loaddb, path/filename of the calc database */



/* before 2007-06-20, calc owner was single nick */
/* after  2007-06-20, calc owner is comma-seapaated list of nicks (chcalc adds to the list) */

/* fix_owner(). '|' in nickname is valid, but would break calcdb format.
 */

void fix_owner(char *owners)
{
	int k;
	for(k = 0; owners[k]; k++)
		if( owners[k] == '|')
			owners[k] = '-';
}

int getowners( int dbindex, char *owners, int max )
{
	char calcname[MAXDATASIZE];
	char calcowners[MAXDATASIZE];
	int k;

	strcpy(owners, "");
	if( dbindex < 0 || dbindex >= total_calcs || *(calc + dbindex) == NULL)
		return -1;
	k = chop( *(calc + dbindex), calcname, 0, ' ' );
	k = chop( *(calc + dbindex), calcowners, k, '|' );
	strncpy(owners, calcowners, max);
	owners[max-1] = 0;
	fix_owner(owners);

	return 0;
}



int is_owner( char *calc_owners, char *qowner)
{
	char enclosed_owners[MAXDATASIZE];
	char enclosed_query[MAXDATASIZE];

	snprintf(enclosed_owners, MAXDATASIZE, ",%s,", calc_owners);
	snprintf(enclosed_query, MAXDATASIZE, ",%s,", qowner);
	return 0 == strcasestr( enclosed_owners, enclosed_query);
}



void owncalc( char *name, char *dbindex, char *nick )
{
	char string[MAXDATASIZE];
	register int x, index;
	char tmpray[MAXDATASIZE];
	char calcname[MAXDATASIZE];
	char calcowners[MAXDATASIZE];

	if( name[0] ) strncpy( string, name, MAXDATASIZE );
	else strncpy( string, nick, MAXDATASIZE );

	tmpray[0] = '\0';
	for( x = atol( dbindex ); (x < total_calcs) && (x >= 0); x++ ) {
		if( !(*(calc + x)) ) return;
		index = chop( *(calc + x), calcname, 0, ' ' );
		index = chop( *(calc + x), calcowners, index, '|' );
		if( !strncasecmp( string, calcname, MAXDATASIZE ) ) {
			strncat( tmpray, calcowners, (MAXDATASIZE - 50) );
			strncat( tmpray, " ", (MAXDATASIZE - 50) );
		 }
	  }

	snprintf( string, MAXDATASIZE, "PRIVMSG %s :index: %i. results: %s", MSGTO, x - 1, tmpray );
	send_irc_message( string );

	return;
}



void listcalc( char *name, char *dbindex, char *nick )
{
	register int x, index;
	char tmpray[MAXDATASIZE];
	char calcname[MAXDATASIZE];
	char calcowners[MAXDATASIZE];
	char string[MAXDATASIZE];


	if( name[0] ) strncpy( string, name, MAXDATASIZE );
	else strncpy( string, nick, MAXDATASIZE );

	tmpray[0] = '\0';
	for( x = atol( dbindex ); (x < total_calcs) && (x >= 0); x++ ) {
		if( !(*(calc + x)) ) return;
		index = chop( *(calc + x), calcname, 0, ' ' );
		index = chop( *(calc + x), calcowners, index, '|' );
			/* after 2007-06-20, calcowners is comma-separated list */
		/* if( strncasecmp( string, calcowner, MAXDATASIZE ) ) continue; */
		if( !is_owner( calcowners, string )) continue;
		if( (strlen(tmpray) + strlen(calcname)) > (MAXDATASIZE - 50) ) break;
		strncat( tmpray, calcname, (MAXDATASIZE - 50) );
		strncat( tmpray, " ", (MAXDATASIZE - 50) );
	  }

	snprintf( string, MAXDATASIZE, "PRIVMSG %s :last index: %i. results: %s", MSGTO, x - 1, tmpray );
	send_irc_message( string );

	return;
}



void searchcalc( char *searchkey, char *dbindex )
{
	register int x, index;
	char tmpray[MAXDATASIZE];
	char calcname[MAXDATASIZE];
	char calcowners[MAXDATASIZE];
	char calcdata[MAXDATASIZE];
	char string[MAXDATASIZE];
	const char *ptr = NULL;


	if( !searchkey[0] ) strncpy( string, MSGTO, MAXDATASIZE );
	else strncpy( string, searchkey, MAXDATASIZE );

	tmpray[0] = '\0';
	for( x = atol( dbindex ); (x < total_calcs) && (x >= 0); x++ ) {
		if( !(*(calc + x)) ) return;
		index = chop( *(calc + x), calcname, 0, ' ' );
		index = chop( *(calc + x), calcowners, index, '|' );
		index = chop( *(calc + x), calcdata, index, '\0' );
		// strstr() for case-sensitive search, strcasestr() for case-insensitive.
		ptr = strcasestr( calcdata, string );
		if( ptr == NULL ) continue;
		if( (strlen(tmpray) + strlen(calcname)) > (MAXDATASIZE - 50) ) break;
		if( (strlen(tmpray) + strlen(calcname)) > (MAXDATASIZE - 50) ) break;
		strncat( tmpray, calcname, (MAXDATASIZE - 50) );
		strncat( tmpray, " ", (MAXDATASIZE - 50) );
	  }

	snprintf( string, MAXDATASIZE, "PRIVMSG %s :index: %i. results: %s", MSGTO, x - 1, tmpray );
	send_irc_message( string );

	return;
}



void rmcalc( char *passwd, char *name, char *rmstring )
{
	int x;
	char *ptr;
	char sndmsg[MAXDATASIZE];

	if( !valid_login( name, passwd ) ){
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :failed login", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	x = findcalc( rmstring );

	if( x == -1) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :%s not found.", MSGTO, rmstring );
		send_irc_message( sndmsg );
		return;
	  }

	total_calcs--;

#if 0 /* "mish"lerner. 
       *   the code till end of the function can rewritten mush shorter as: */
       * and ptr is not needed */
	free( *(calc + x) );
	if( x != total_calcs )
		*(calc + x) = *(calc + total_calcs);
	snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :%s removed.", MSGTO, rmstring );
	send_irc_message( sndmsg );
	savedb( CALCDB );
	return;
#endif
	if( x == total_calcs ) {
		free( *(calc + x) );
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :%s removed.", MSGTO, rmstring );
		send_irc_message( sndmsg );
		savedb( CALCDB );
		return;
	  }

	ptr = *(calc + x);
	*(calc + x) = *(calc + total_calcs);
	free( ptr );
	ptr = NULL;
	snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :%s removed.", MSGTO, rmstring );
	send_irc_message( sndmsg );

	savedb( CALCDB );

	return;
}



void chcalc( char *pass, char *name, char *calcname, char *newcalcdata )
{
	char sndmsg[MAXDATASIZE];
	char owners[MAXDATASIZE];
	int x;

	if( calcname[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :No calc name provided.", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( (x = findcalc( calcname )) == -1) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :no such calc", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( !valid_login( name, pass ) ){
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :failed login", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( newcalcdata[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :You can not make an empty calc.", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

		/* add nick to the calc_owners list if it's not already there */
	getowners(x, owners, sizeof(owners));
	if( ! is_owner( owners, name)) {
		strncat( owners, ",", (MAXDATASIZE - 50) );
		strncat( owners, name, (MAXDATASIZE - 50) );
	}

	    /* we rely on the fact that *(calc + x) is already allocated to the max size */
	    /* Indeed, that's how loaddb() and mkcalc() allocate the lines. */
	    /* If loaddb() and mkcalc() allocated at actual size, we'd need to realoc here */

	fix_owner(owners);
	snprintf( *(calc + x), MAXDATASIZE, "%s %s|%s", calcname, owners, newcalcdata );
	snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :calc %s changed.", MSGTO, calcname );
	send_irc_message( sndmsg );

	savedb( CALCDB );

	return;
}



void mkcalc( char *pass, char *name, char *newcalc, char *newcalcdata )
{
	char sndmsg[MAXDATASIZE];

	if( !valid_login( name, pass ) ){
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :failed login", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( newcalc[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :No calc name provided.", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( newcalcdata[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :You can not make an empty calc.", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( findcalc( newcalc ) != -1) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :calc exists", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	*(calc + total_calcs) = calloc( sizeof( char * ), MAXDATASIZE );
	if( !(*(calc + total_calcs)) ) return;

	fix_owner(name);
	snprintf( *(calc + total_calcs), MAXDATASIZE, "%s %s|%s", newcalc, name, newcalcdata );
	snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s :calc %s added.", MSGTO, newcalc );
	send_irc_message( sndmsg );

	total_calcs++;
	savedb( CALCDB );

	return;
}



void docalc( char *calcstring )
{
	int x, y;
	char tmpray[MAXDATASIZE], calcray[MAXDATASIZE];

	x = findcalc( calcstring );

	if( x >= 0 ) {
		y = chop( (*(calc + x)), calcray, 0, ' ' );
		y = chop( (*(calc + x)), calcray, y, '|' );
		y = chop( (*(calc + x)), calcray, y, '\n' );
	  }
	else calcnotfound( calcray, MAXDATASIZE, calcstring );

	snprintf( tmpray, MAXDATASIZE,"privmsg %s :%s", MSGTO, calcray );

	send_irc_message( tmpray );
	return;
}



int findcalc( char *string )
{
	register int x, index;
	char tmpray[MAXDATASIZE];

	for( x = 0; x < total_calcs; x++ ) {
		if( !(*(calc + x)) ) return -1;
		index = chop( *(calc + x), tmpray, 0, ' ' );
		if( index ) if( !strncasecmp( string, tmpray, MAXDATASIZE ) ) return x;
	  }

	return -1;
}



int savedb( char *filename )
{
	FILE *fp;
	register int x;

	fp = fopen( filename, "w" );
	if( !fp ) { perror(filename); return 1; }

	for( x = 0; x < total_calcs; x++ ) {
		if( !(*(calc + x)) ) break;
		fputs( *(calc + x), fp );
		fputc( '\n', fp );
	  }

	fclose( fp );
	return 0;
}



int loaddb( char *filename, int maxdbsize )
{
	FILE *fp;
	register int x;

	strncpy( CALCDB, filename, MAXDATASIZE );
	MAXCALCS = maxdbsize;

	fp = fopen( CALCDB, "r" );
	if( !fp ) { perror(CALCDB); return 1; }

	calc = calloc( sizeof( char * ), MAXCALCS );
	if( !calc ) { puts( "inital allocation error in loaddb()." ); return 1; }

	for( x = 0; x < MAXCALCS; x++ ) {
		*(calc + x) = calloc( sizeof( char * ), MAXDATASIZE );
		if( !(*(calc + x)) ) { puts( "memory allocation failed" ); break; }
		fgets( *(calc + x), MAXDATASIZE, fp );
		if( feof( fp ) )	{
			fclose( fp );
			free(*(calc + x));
			*(calc + x) = NULL;
			return 0;
		  }
		clean_message( *(calc + x) );
		total_calcs++;
	  }

	fclose( fp );

	return 0;
}
