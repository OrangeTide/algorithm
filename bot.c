/*
 * bot.c version 0.333
 * all versions previous to 0.333 are completely public domain
 * all contributed code falls under the same BSD style license as
 * noted below unless the contributing author places a copyright
 * notice in their file/s.
 */


/*
 *	 Copyright (c) 2001 David T. Stiles
 *	 All rights reserved.
 * 
 *	 Redistribution and use in source and binary forms, with or without
 *	 modification, are permitted provided that the following conditions
 *	 are met:
 *	 1. Redistributions of source code must retain the above copyright
 *		 notice, this list of conditions and the following disclaimer.
 *	 2. Redistributions in binary form must reproduce the above copyright
 *		 notice, this list of conditions and the following disclaimer in the
 *		 documentation and/or other materials provided with the distribution.
 *	 3. All advertising materials mentioning features or use of this software
 *		 must display the following acknowledgement:
 *			This product includes software developed by David T. Stiles
 *	 4. The name David T. Stiles may not be used to endorse or promote
 *		 products derived from this software without specific prior written
 *		 permission.
 * 
 *	 THIS SOFTWARE IS PROVIDED BY DAVID T. STILES `AS IS'' AND ANY
 *	 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *	 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *	 ARE DISCLAIMED.	IN NO EVENT SHALL THE AUTHOR BE LIABLE
 *	 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *	 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *	 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *	 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *	 SUCH DAMAGE.
 */

/* this code would not be possible without the patience and intelligence
 * provided by many of the people from #c/efnet. I thank all of you sincerely.
 */

/*
 * a client/bot irc thingy. 1 dec 98
 * This #c project is officially named "Sahib" on 3 February 1999
 * serious modification/cleaning/extending proceeding on 1 September 2000
 * merging Aegis' code starting sometime in October of 2000.
 * ripped out the database and turned it into a separate server 13 Oct. 2000
 * checking comments on 31 jan 2001
 * radical restructuring of all source code starting on 4 june 2001
 */

#include "autovoice.h"
#include "bot.h"
#include "calcdb.h"
#include "command.h"
#include "dcalc.h"
#include "notify.h"
#include "proto.h"
#include "rc.h"
#include "rpn.h"
#include "users.h"
#include "wcalc.h"


/* yeah, i know that globals are considered evil. byte me. */

/* config file variables. declared in the order that they are read, to facilitate
 * config file re/creation. PORT and MAXCALCS have defaults that are
 * overwritten by the contents of the config file
 */

	static	char SERVER[MAXDATASIZE];	/* name of the irc server to connect with */
	static	int  PORT = 6667;				/* default to the standard irc server port */
	static	char NICK1[MAXDATASIZE];	/* preferred nickname to use */
	static	char NICK2[MAXDATASIZE];	/* alternate nickname */
	static	char USER[MAXDATASIZE];		/* the USER irc protocol message */
	static	int  MAXCALCS = 5000;		/* make the default max database size conservative */
	static	char CALCDB[MAXDATASIZE];	/* name of, and possibly path to, the calc database file */
	static	char DEF_CHAN[MAXDATASIZE]; /* name of default channel to talk on */
	static	char ON_CONNECT_SCRIPT[MAXDATASIZE]; /* execute this on connect */


/* other misc local globals that are needed. i fail to see any non-hacked way
 * way to get rid of these.
 */

	static	int  sockfd;					/* used for the socket file descriptor */
	static	struct message cur_msg;		/* defined in bot.h, holds a parsed irc message */


/* this variable is extern'ed to ALL modules needing to send their own irc messages */

	char MSGTO[MAXDATASIZE];	/* set to nick/channel on each incoming message */
	char BOTNAME[MAXDATASIZE]; /* holds the actual nickname in use by the bot */


	sig_atomic_t keep_going=1;	/* flag to break out of the inner loop */
	sig_atomic_t verbose=1;		 /* flag to enable verbose debugging */

/******************************----BEGIN CODE----*********************************/



/* return if there are network difficulties, otherwise, this is where the main purpose
 * of this program really begins. an endless loop around select().
 * select on stdin and the socket.
 */

void main_loop( void )
{ 
	int whatever;
	fd_set fdgroup;
	struct timeval tv;

	for( ;; )
	 {
		FD_ZERO(&fdgroup);
		FD_SET(STDIN_FILENO, &fdgroup);
		FD_SET(sockfd, &fdgroup);
		tv.tv_sec = 360;				/* if no data for 6 minutes, something is wrong. */
		tv.tv_usec = 0;

		whatever = select( (sockfd + 1) , &fdgroup, NULL, NULL, &tv);
 
		if( !whatever ) break;		/* we must not be connected anymore, return. */
		if( FD_ISSET( sockfd, &fdgroup ) ) if( process_in( ) ) break;
		if( FD_ISSET( STDIN_FILENO, &fdgroup ) ) if( process_out( ) ) break;

	 }

	return;
}



/* you can pretend you have a console through this code. '/' will allow raw irc protocol out.
 * btw, there are no real sanity checks on this function. the person at the console is supposed
 * to be not malicious. fgets should prevent accidental buffer overruns though. :)
 */

int process_out( void )
{
	char ray[MAXDATASIZE], tmp[MAXDATASIZE];

	fgets( tmp, MAXDATASIZE - 1, stdin );
	if( tmp[0] == '/' ) {
		size_t cmdlen, argstart;

		/* handle some commands */
		cmdlen = strcspn( tmp+1, WHITESPACE );
		argstart = 1 + cmdlen + strspn( tmp + 1 + cmdlen, WHITESPACE );

		if(cmdlen == 4 && !memcmp( "quit", tmp+1, 4 )) {
			/* /quit was entered */
			if(tmp[argstart]) {
				snprintf( ray, sizeof ray, "quit :%s", tmp + argstart );
			} else {
				snprintf( ray, sizeof ray, "quit" );
			}
			send_irc_message( ray );
			keep_going=0;
		} else if(cmdlen == 3 && !memcmp( "msg", tmp+1, 3 )) {
			/* /msg was entered . put the : in for people */
			size_t nickstart, nicklen;
			nickstart = argstart;
			nicklen = strcspn( tmp+nickstart, WHITESPACE );
			argstart = nickstart + nicklen + strspn( tmp + nickstart + nicklen, WHITESPACE );
			snprintf( ray, sizeof ray, "privmsg %.*s :%s", nicklen, tmp + nickstart,	tmp + argstart );
			send_irc_message( ray );
		} else {
			strncpy( ray, (tmp + 1), MAXDATASIZE );
			send_irc_message( ray );
		}
	} else {
		if(DEF_CHAN[0]) {
			snprintf( ray, MAXDATASIZE, "PRIVMSG %s :%s", DEF_CHAN, tmp );
			send_irc_message( ray );
		} else {
			printf("Warning. no default channel selected. Ignoring message\n");
		}
	}
	return 0;
}



/* ugh. this one is impossible to comment on. what it does is make sure that
 * only complete lines are processed. It uses a static buffer to hold the
 * incomplete information until the next packet comes in. X is the placeholder
 * index. marked High Priority for recoding. this code sucks badly... even
 * though it works
 * this hurts my eyes every time i see it.
 */

int process_in( void )
{
	char buf[MAXDATASIZE], *ptr;
	int numbytes = 0;	 
	int z = 0;
	int y = 0;
	static int x = 0;
	static char tmpbuf[MAXDATASIZE];

	if( (numbytes = recv( sockfd, buf, MAXDATASIZE, 0)) == -1)
	 {
		perror("recv");
		return 1;
	 }

	if( !numbytes ) return 1;
	if( numbytes < MAXDATASIZE ) buf[numbytes] = '\0';
	buf[MAXDATASIZE - 1] = '\0';

	ptr = buf;

	for( y = 0; y < numbytes; y++)
	 {
		while( *(ptr + y) != '\n' )
		 {
			tmpbuf[x] = *( ptr + y );
			x+=1; y+=1;
			if( x == MAXDATASIZE ) x--;
			if( *(ptr + y) == '\0' ) break;
		 }
		tmpbuf[x] = '\0';
		if( *(ptr + y) == '\n' )
		 {
			 x = 0;
			 parse_incoming( tmpbuf );
			 for( z = 0; z < MAXDATASIZE; z++ ) tmpbuf[z] = '\0';
		 }
	 }


	return 0;
}



/* full ircd messages come through here for parsing.
 * this form of parsing is based around the "typical" irc message where someone
 * actually says something. if it is some other type of message, the parser tries
 * to copy into msgarg1 etc but will fall through when the arguments don't fall
 * neatly into place.
 */

void parse_incoming( char *ptr )
{
	int position = 0;
	int boot;

	boot = clean_message( ptr );

	if( ptr[0] == 'P' ) {  /* this is will catch server PINGs */
		reply_ping( ptr );
		return;					/* no need to continue parsing */
	}

	memset( &cur_msg, 0, sizeof(cur_msg) );  /* clear the buffer... i want a safe default */

	/* do an initial breakup of the message based on whitespace */

	if( *ptr != '\0' ) position = chop( ptr, cur_msg.userline, 1, ' ' );  /* 1 to skip the leading colon */
	if( ptr[position] != '\0' ) position = chop( ptr, cur_msg.msgtype, position, ' ' );
	if (cur_msg.msgtype[0] == 'N' && cur_msg.msgtype[1] == 'I') ++position; /*skip a colon since there is no "to", msgto holds new /nick */
	if( ptr[position] != '\0' ) position = chop( ptr, cur_msg.msgto, position, ' ' ); /* holds new NICK if previous statements succeeds */

	if( ptr[position] != '\0' ) position = chop( ptr, cur_msg.fulltext, ptr[position]==':' ? position + 1 : position, '\n' );  /* + 1 to skip the colon */

	/*break down the host/nick type stuff */

	position = chop( ptr, cur_msg.nick, 1, '!' );
	if( ptr[position] != '\0' ) position = chop( ptr, cur_msg.logname, position, '@' );
	if( ptr[position] != '\0' ) position = chop( ptr, cur_msg.hostname, position, ' ' );

	/* now that the protocol crap is gone, let's parse what the person said. */
	if( (cur_msg.msgtype[0] == 'P') || (cur_msg.msgtype[0] == 'N') ) { /*PRIVMSG or NOTICE, NICK triggers too though */
		position = chop( cur_msg.fulltext, cur_msg.msgarg1, 0, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg2, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg3, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg4, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg5, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg6, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg7, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg8, position, ' ' );
		if( ptr[position] != '\0' ) position = chop( cur_msg.fulltext, cur_msg.msgarg9, position, ' ' );
	}

	/* console output with simple formatting */
	if( !strncasecmp( cur_msg.msgtype, "PRIVMSG", MAXDATASIZE ) )
		printf( "|%s|  %s: %s\n", cur_msg.msgto, cur_msg.nick, cur_msg.fulltext );
	else
		puts( ptr );

	/* color and attribute boot */
	if (boot && strncmp(cur_msg.msgto, BOTNAME, MAXDATASIZE) && strncmp(cur_msg.nick, BOTNAME, MAXDATASIZE)) {
		char buffer[MAXDATASIZE];
		snprintf(buffer, MAXDATASIZE, "kick %s %s :%s",
			cur_msg.msgto, cur_msg.nick,
			"No color or text attributes allowed.");
		send_irc_message(buffer);
	}

	make_a_decision();
	return;
}

/* after the message from the ircd has been parsed, i need to decide what course of
 * action to take based upon that input. this is where events are triggered.
 */

void make_a_decision( void )
{
	if(verbose>4) {
		fprintf(stderr, "%s(): message type='%s'\n", __func__, cur_msg.msgtype);
	}
	/* don't let it talk to itself, so make sure this is first in the list. Changes botname if bot is changing its nick*/
	if( !strncasecmp( BOTNAME, cur_msg.nick, MAXDATASIZE ) ) {
		if (cur_msg.msgtype[0] == 'N' && cur_msg.msgtype[1] == 'I') {
			strncpy(BOTNAME, cur_msg.msgto, MAXDATASIZE);
		}
		return;
	}

	notify_report_message(&cur_msg);
}


/* how to comment? this function is almost pointless as is. its purpose will become
 * clear later... after it is fully implemented. all it does now is ensure that a
 * newline char is sent after each message and provide a wrapper for the network I/O.
 * eventually, the bot will monitor its own output here.
 */

void send_irc_message( char *sndmsg )
{
	char newline = '\n';
	if(verbose>0) {
		fprintf(stderr, "OUT> %s\n", sndmsg);
	}

	if( send (sockfd, sndmsg, strlen( sndmsg ), 0) == -1) perror("send");
	if( send (sockfd, &newline, 1, 0) == -1) perror("send");
	sleep( 1 );		/* pause for a second to help keep things from getting too crazy. */
	return;
}


/*******************************-----begin stubs-----************************************/

/* convert what is said to rot13 */

void rot13_stub()
{
	char tmpray[MAXDATASIZE];
	char data[MAXDATASIZE];
	char *ptr = NULL;
	int x = 0;

	ptr = cur_msg.fulltext;
	x = chop( cur_msg.fulltext, tmpray, 0, ' ' ); /* i use chop to find out where ptr should point */
	ptr += x; /* skip the first argument but get the rest of the sentence */
	x = 0; /* reset to 0 so it can be used in the loop below. */

	memset( tmpray, '\0', MAXDATASIZE );
	memset( data, '\0', MAXDATASIZE );

	while( *ptr != '\0' ) {
		data[x] = *ptr;
		if( (data[x] >= 'A') && (data[x] <= 'Z') ) {
			if( (data[x] >= 'A') && (data[x] <= 'M') ) data[x] += 13;
			else data[x] = 'A' + (12 - ('Z' - data[x]) );
		}
		if( (data[x] >= 'a') && (data[x] <= 'z') ) {
			if( (data[x] >= 'a') && (data[x] <= 'm') ) data[x] += 13;
			else data[x] = 'a' + (12 - ('z' - data[x]) );
		}
		x++, ptr++;
	}

	snprintf( tmpray, MAXDATASIZE, "privmsg %s :rotated: %s", MSGTO, data );
	send_irc_message( tmpray );

	return;
}


/* lsusers */
void lsusers_stub()
{
	list_users( cur_msg.nick );
	return;
}


/* chpass stub finished */

void chpass_stub()
{
	chpass( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4 );
	return;
}


/* docalc stub finished */

void docalc_stub()
{
	docalc( cur_msg.msgarg2 );
	return;
}


/* op_people stub finished */

void oppeople_stub()
{
	if( MSGTO[0] == '#' ) return; /* do not respond to op requests made in the channel */

	/* making getting ops easier for other than the default channel */
	
	if( (cur_msg.msgarg2[0] == '#') || (cur_msg.msgarg2[0] == '&') ) {
		if( cur_msg.msgarg4[0] )
			oppeople( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4, cur_msg.nick );
		else
			oppeople( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.nick, cur_msg.nick );
		return;
	}
	/* op only in the default channel */
	
	if( DEF_CHAN[0] ) {
		if( cur_msg.msgarg3[0] )
			oppeople( DEF_CHAN, cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.nick );
		else
			oppeople( DEF_CHAN, cur_msg.msgarg2, cur_msg.nick, cur_msg.nick );
	}
	return;

}


/* owncalc stub finished */

void owncalc_stub()
{
	owncalc( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.nick );
	return;
}


/* whois stub finished */

void whois_stub()
{
	whois( cur_msg.msgarg2 );
	return;
}


/* adduser stub finished */

void adduser_stub()
{
	adduser( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4, cur_msg.msgarg5 );
	return;
}


/* rmuser stub finished */

void rmuser_stub()
{
	rmuser( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4 );
	return;
}


/* rmcalc stub finished */

void rmcalc_stub()
{
	rmcalc( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4 );
	return;
}


/*
 * i parse cur_msg.fulltext to step over command arguments and login info.
 * the actual calc data can obviously have spaces, so the original
 * parsing of the irc message is insufficient. note that 'newcalctext' gets
 * overwritten by each call to chop(). after the last call, it holds the calc data.
 */

void mkcalc_stub()
{
	int y;
	char newcalctext[MAXDATASIZE];

	y = chop( cur_msg.fulltext, newcalctext, 0, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, '\0' );
	
	mkcalc( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4, newcalctext );
	return;
}


void chcalc_stub()
{
	int y;
	char newcalctext[MAXDATASIZE];

	y = chop( cur_msg.fulltext, newcalctext, 0, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, ' ' );
	y = chop( cur_msg.fulltext, newcalctext, y, '\0' );
	
	chcalc( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.msgarg4, newcalctext );
	return;
}



/* listcalc stub finished */

void listcalc_stub()
{
	listcalc( cur_msg.msgarg2, cur_msg.msgarg3, cur_msg.nick );
	return;
}


/* searchcalc stub */

void searchcalc_stub()
{
	searchcalc( cur_msg.msgarg2, cur_msg.msgarg3 );
	return;
}


/*stub for couts RPN calculator */

int rpn_stub( void )
{
	int x = 0;
	char tmpray[MAXDATASIZE], answer[MAXDATASIZE];

	rpn_calc( cur_msg.fulltext + (strlen( cur_msg.msgarg1 ) + 1), answer, (MAXDATASIZE - 2) );

	if( x != RPN_OK )
		snprintf( tmpray, MAXDATASIZE, "privmsg %s :error: %s", MSGTO, answer );
	else
		snprintf( tmpray, MAXDATASIZE, "privmsg %s :cout says: %s", MSGTO, answer );

	send_irc_message( tmpray );

	return 0;
}



/* stub for demoncrat's "normal" calculator code */

int dcalc_stub( void )
{
	char tmpray[MAXDATASIZE];
	Value v;
	const char *plaint;


	plaint = dcalc(&v, cur_msg.fulltext + (strlen( cur_msg.msgarg1 ) + 1) );

	if( plaint )
		snprintf( tmpray, MAXDATASIZE, "privmsg %s :answer: %s", MSGTO, plaint);
	else
		snprintf( tmpray, MAXDATASIZE, "privmsg %s :answer: %.16g", MSGTO, v);

	send_irc_message( tmpray );

	return 0;
}


int wcalc_stub( void )
{
	char tmpray[MAXDATASIZE];
	size_t len;

	snprintf(tmpray, MAXDATASIZE, "privmsg %s : ", MSGTO);
	len = strlen(tmpray);
	wcalc(tmpray + len, MAXDATASIZE - len - 1, cur_msg.fulltext + (strlen( cur_msg.msgarg1 ) + 1) );

	send_irc_message( tmpray );

	return 0;
}

int proto_stub( void )
{
	char tmpray[MAXDATASIZE];
	size_t len;
	snprintf(tmpray, MAXDATASIZE, "privmsg %s : ", MSGTO);
	len = strlen(tmpray);
	proto_result(tmpray+len, sizeof tmpray - len - 1, cur_msg.fulltext + (strlen( cur_msg.msgarg1 ) + 1) );
	send_irc_message( tmpray );
	return 0;
}

/********************************-----end stubs-----*************************************/



/* repsond only to private requests for help. the actual messages are #defined in bot.h
 * i should make these loadable from a file rather than compiled in.
 * erm... why don't i make this a switch() like i did in make_a_decision()?
 */

void help( void )
{
	char tmpray[MAXDATASIZE];

	if( strncasecmp( cur_msg.msgto, BOTNAME, MAXDATASIZE ) ) return;

	if( !strncasecmp( cur_msg.msgarg2, "commands", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, COMMANDS );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "syntax", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, SYNTAX );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "adduser", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, ADDUSER );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "rmuser", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, RMUSER );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "op", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, OP );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "chpass", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, CHPASS );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "rmcalc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, RMCALC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "owncalc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, OWNCALC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "mkcalc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, MKCALC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "chcalc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, CHCALC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "rawirc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, RAWIRC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "whois", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, WHOIS );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "listcalc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, LISTCALC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "searchcalc", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, SEARCHCALC );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "chattr", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, CHATTR );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "lsusers", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, LSUSERS );
		send_irc_message( tmpray );
		return;
	}
	if( !strncasecmp( cur_msg.msgarg2, "rot13", MAXDATASIZE ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, ROT13 );
		send_irc_message( tmpray );
		return;
	}


	snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :%s", cur_msg.nick, HELPHELP );
	send_irc_message( tmpray );

	return;
}



/* CTCPs are accomplished through 'NOTICE' messages. the first letter of each
 * important CTCP is unique so i merely switch on that value. the default is
 * to merely echo back the same type of NOTICE but without arguments. (this covers ping)
 * VERSION is the only CTCP that i care about currently.
 */

void do_ctcp( void )
{
	char ray[MAXDATASIZE];

	switch( cur_msg.msgarg1[1] ) {
		case 'V':
			snprintf( ray, MAXDATASIZE, "NOTICE %s :\001VERSION ircII.5 not quite all there yet.\001", cur_msg.nick );
			break;
		default:
			snprintf( ray, MAXDATASIZE, "NOTICE %s :%s\001", cur_msg.nick, cur_msg.msgarg1 );
			break;
	}

	send_irc_message( ray );
	return;
}



/* this function validates user/pass and then sends the rest of the text to the server.
 * the code is trivial enough to remain inside of bot.c
 */

void rawirc( void )
{
	int y;
	char tmpray[MAXDATASIZE];

	if( !valid_login( cur_msg.msgarg3, cur_msg.msgarg2 ) ) {
		snprintf( tmpray, MAXDATASIZE, "PRIVMSG %s :failed login", cur_msg.nick );
		send_irc_message( tmpray );
		return;
	}

	/* this is the same parsing technique described in mkcalc_stub() */
	y = chop( cur_msg.fulltext, tmpray, 0, ' ' );
	y = chop( cur_msg.fulltext, tmpray, y, ' ' );
	y = chop( cur_msg.fulltext, tmpray, y, ' ' );
	y = chop( cur_msg.fulltext, tmpray, y, '\0' );

	send_irc_message( tmpray );

	return;
}



/* let the ^A chars stay, but no other nonprintable chars... walking through the array.
 * i really like pointer arithmetic, i apologise to those who find this unreadable.
 */

int clean_message( char *msg )
{
	register size_t x;
	register int boot = 0;
	for( x = 0; x < strlen(msg); x++ ) {
		if( *(msg+x) == 1 ) continue;
		if( !isprint( *(msg+x) ) ) {
			if( *(msg+x) == '\r' ) { *(msg+x) = '\0'; continue; }
			if( *(msg+x) == '\n' ) { *(msg+x) = '\0'; continue; }
			if( !isspace( *(msg+x) ) ) {
				if (isascii(*(msg+x)) && iscntrl(*(msg+x)))
					boot = 1;
				*(msg+x) = ' ';
			}
		}
	}
	return boot;
}



/* the first array is the array you wish to break up nondestructively
 * the second array will hold the section that is "chopped off".
 * 'position' is the location in the array you want it to start looking for 'separator'.
 * the separator character is the token you wish to chop on. e.g. a space character.
 * this function is intentionally dangerous. it assumes that only proper NULL terminated
 * arrays will be passed to it.
 */

int chop( char *in, char *out, int position, char separator )
{
	int x = 0;

	while( *(in + position) != separator )
	 {
		*(out + x) = *(in + position);
		x+=1; position+=1;
		if( *(in + position) == '\0' ) break;
		if( *(in + position) == '\r' ) break;
		if( *(in + position) == '\n' ) break;
	 }

	*(out + x) = '\0';
	return (*(in + position)) ? position+1 : position; /* thanks for the simpler 'if' construct Xgc */
}



/* the name of the function explains it fairly well. it merely replies to any
 * PING requests from IRC servers.
 * ptr holds "PING irc.home.com" so i start copying from the end of "PING "
 * onwards, which is 5 chars, to get the irc server name for the PONG response.
 */

void reply_ping( char *ptr )
{
	char str[MAXDATASIZE];

	snprintf( str, MAXDATASIZE - 5, "PONG %s", (ptr + 5) );
	send_irc_message( str );

	return;
}



/* hide most of the network stuff here.
 * pass in a host name, port, and a file descriptor and this function
 * will take care of the network details. the value it returns should
 * be assigned to a file descriptor or evaluated for errors.
 */

int host_connect( char *exthost, int extport, int extsockfd )
{
	struct hostent *he;
	struct sockaddr_in their_addr;

	if ((he=gethostbyname( exthost )) == NULL)
	 {
		 perror("gethostbyname");
		 return -1;
	 }

	if ((extsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	 {
		 perror("socket");
		 close( extsockfd );
		 return -1;
	 }

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(extport);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
	memset( &their_addr.sin_zero, '\0', 8 );

	if (connect(extsockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
	 {
		 perror("connect");
		 close( extsockfd );
		 return -1;
	 }


	return extsockfd;

}

int run_script( const char *filename ) 
{ 
	char ray[MAXDATASIZE];
	FILE *f;

	f=fopen( filename, "r" );
	if( !f ) {
		perror( filename );
		return 1;
	}

	while( fgets( ray, sizeof ray, f ) ) {
		send_irc_message( ray );
	}

	fclose( f );
	return 0;
}

/* connect to the irc server and register.
 * ugly ugly ugly
 */

int irc_connect( void )
{
	char ray[MAXDATASIZE];
	int numbytes = 0;

	sockfd = 0;

	if( (sockfd = host_connect(SERVER, PORT, sockfd)) < 1) return 1;

	strncpy( BOTNAME, NICK1, MAXDATASIZE );
	send_irc_message( USER );

	memset( ray, '\0', sizeof(ray) );
	snprintf( ray, MAXDATASIZE, "nick %s", BOTNAME );
	send_irc_message( ray );

	sleep( 3 );

	numbytes = recv( sockfd, ray, MAXDATASIZE, 0);
	if( numbytes == -1 )
		{
		perror("recv");
		close( sockfd );
		return 1;
	 }

	if( strstr( ray, "PING" ) ) { reply_ping( ray ); }
	if( strstr( ray, "Nickname is already in use" ) )
		{
		strncpy( BOTNAME, NICK2, MAXDATASIZE );
		memset( ray, '\0', sizeof(ray) );
		snprintf( ray, MAXDATASIZE, "nick %s", BOTNAME );
		send_irc_message( ray );

		if( (numbytes = recv( sockfd, ray, MAXDATASIZE, 0)) == -1)
		 {
			perror("recv");
			close( sockfd );
			return 1;
		 }
		if( strstr( ray, "Nickname is already in use" ) ) { close( sockfd ); return 1; }
	}

	memset( ray, '\0', sizeof(ray) );
	snprintf( ray, MAXDATASIZE, "mode %s +i", BOTNAME );
	send_irc_message( ray );

	run_script(ON_CONNECT_SCRIPT);

	return 0;
}


static int load_item_str(struct config_node *root, const char *name, size_t buf_max, char *buf, const char *desc) {
	struct config_node *item;
	item=config_find(root,name);
	if(!item || !config_get_str(item,buf,buf_max)) { 
		 fprintf(stderr, "failed loading %s\n", desc );
		 return 0;
	}
	clean_message( buf );
	printf( "%s:			%s\n", desc, buf);
	return 1;
}

static int load_item_int(struct config_node *root, const char *name, int *i, const char *desc) {
	struct config_node *item;
	item=config_find(root,name);
	if(!item || !config_get_int(item,i)) { 
		 fprintf(stderr, "failed loading %s\n", desc );
		 return 0;
	}
	printf( "%s:			%u\n", desc, *i);
	return 1;
}

/* this function loads variables from bot.cfg
 * into global variables.
 */

static struct config_node *load_cfg( void )
{
	struct config_node *root;
	struct config_node *curr;

	root=config_parser("bot.cfg");
	if(!root) return 0;

	/* look for a "bot" node */
	curr=config_find(root,"bot");
	if(!curr) return 0;
	if(curr->child) { 
		int res;
		curr=curr->child;

		puts( "\n--------------- bot.cfg data ---------------\n" );

		load_item_int(curr,"verbose", &verbose, "verbose debug level");
		res= load_item_str(curr,"server",sizeof SERVER, SERVER, "irc server")
		&& load_item_int(curr,"port", &PORT, "server port")
		&& load_item_str(curr,"nick", sizeof NICK1, NICK1, "nick")
		&& load_item_str(curr,"alt_nick", sizeof NICK2, NICK2, "alternate nick")
		&& load_item_str(curr,"userline", sizeof USER, USER, "user line")
		&& load_item_int(curr,"max_db", &MAXCALCS, "max db entries")
		&& load_item_str(curr,"database", sizeof CALCDB, CALCDB, "calc database filename")
		&& load_item_str(curr,"default_channel", sizeof DEF_CHAN, DEF_CHAN, "default channel")
		&& load_item_str(curr,"on_connect", sizeof ON_CONNECT_SCRIPT, ON_CONNECT_SCRIPT, "script for connect");
		puts( "\n--------------- data loaded ---------------\n" );

		if( !DEF_CHAN[0] )
		printf("Warning. No default channel selected. Some commands may not work.\n");
		if(!res) {
			config_free(root);
			return 0; /* failure */
		}
	}
	return root;
}



/* this is only performed once during the entire execution of the program
 * any failures here are critical and will stop program execution.
 */

int prep( void )
{
	struct config_node *config_root;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGFPE, SIG_IGN);
	srand( time( NULL ) );
	config_root=load_cfg();

	if( !config_root ) { puts( "failed at end of load_cfg()"); return 10; }
	if( loadusers( "user.list" ) ) { puts( "failed loading the user.list " ); return 15; }
	if( loaddb( CALCDB, MAXCALCS ) ) { puts( "failed loading the calc database." ); return 20; }
	if( !command_init() ) { puts( "failed to load the command module." ); return 30; } 
	if( !proto_init() ) { puts( "failed to load the proto database." ); return 40; }
	if( !autovoice_init(config_root) ) { puts( "failed to load the autovoice module." ); return 50; } 
	config_free(config_root);
	return 0;
}



/* Only exit if the config files are not found, else endlessly cycle through reconnects */

int main(int argc, char *argv[])
{
	int x = 0;

	if( argc > 1 && 0 == strcmp("-T", argv[1])) {
		calcnotfound_test();
		exit(0);
	}

	if( argc ) printf( "%s is loading, please wait...\n\n", argv[0] );

	if( prep() ) {
		puts( "preparations failed. do you have a bot.cfg, calcdb.data, and user.list file?" );
		return 10;
	}

	while(keep_going) {
		printf( "\n\nattempting to connect to %s, please wait...\n\n", SERVER );
		x = irc_connect();
		sleep( 3 );		/* 3 seconds to keep from hitting the server too much with reconnects */
		if( x ) {
			puts("hm. i could not connect dude.");
			continue;
		}
		main_loop();
		close( sockfd );
		puts( "disconnected..." );
	}
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
