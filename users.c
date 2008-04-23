/* users.c version 0.333 */
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

/* user stuff. ugh!
 * i load in the data file on users into a linked list that is defined in
 * users.h. *usr is the anchor, *trv is is the pointer i use to travel
 * through the list. *lag makes it so i do not need a doubly linked list by
 * providing a pointer to the node previous to the one *trv is pointing at.
 * i use crypt to generate password hashes. you may need to provide the
 * flag -lcrypto in the makefile. i have to in slackware but not in openbsd.
 */

#include <stdlib.h>

#include "users.h"
#include "bot.h"

static struct user *usr;
static struct user *trv;
static struct user *lag;
static FILE *fp;
static long int total_users = 0;


/* call only once during the entire course of the program! or else... */
int loadusers( char *filename )
{
	fp = fopen( filename, "r" );
	if( !fp ) return 1;

	usr = malloc( sizeof( struct user ) );
	if( !usr ) return( 1 );
	lag = trv = usr;

	while( !feof( fp ) ) {
		fgets( trv->data, USERINFO, fp );
		clean_message( trv->data );
		if( !trv->data[0] ) break;
		++total_users;
		lag = trv;
		trv->next = malloc( sizeof( struct user ) );
		trv = trv->next;
		if(!trv) return( 1 );
	  }

	free( trv );
	lag->next = NULL;

	fclose( fp );

	return 0;
}




void saveusers( char *filename )
{
	fp = fopen( filename, "w" );
	if( !fp ) return;

	trv = usr;

	while( trv->next ) { fprintf( fp, "%s\n", trv->data ); trv = trv->next; }
	fprintf( fp, "%s\n", trv->data );
	fclose( fp );

	return;
}



void rmuser( char *passwd, char *name, char *rmname )
{
	int x = 0;
	char sndmsg[MAXDATASIZE];

	/* validate user before we do what they want */

	if( valid_login( name, passwd ) ) snprintf( sndmsg, MAXDATASIZE, "privmsg %s :credentials verified.", name );
	else {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :access denied.", name );
		send_irc_message( sndmsg );
		return;
	  }

	if( rmname[0] == '\0' ) return;

	x = valid_user( rmname );
	if( !x ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user: %s not found.", name, rmname );
		send_irc_message( sndmsg );
		return;
	  }

	lag->next = trv->next;
	free( trv );
	--total_users;
	

	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user: %s removed.", name, rmname );
	send_irc_message( sndmsg );
	saveusers( "user.list" );
	return;
}



void adduser( char *pass, char *name, char *newupass, char *newuname )
{
	char sndmsg[MAXDATASIZE];
	char salt[MAXDATASIZE];
	char *hash;

	/* validate user before we do what they want */
	if( valid_login( name, pass ) ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :credentials verified.", name );
		send_irc_message( sndmsg );
	  }
	else {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :access denied.", name );
		send_irc_message( sndmsg );
		return;
	  }

	if( newupass[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s : new password is invalid", name );
		send_irc_message( sndmsg );
		return;
	  }
	if( newuname[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s : new username is invalid", name );
		send_irc_message( sndmsg );
		return;
	  }

	if( valid_user( newuname ) ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user exists.", name );
		send_irc_message( sndmsg );
		return;
	  }

	lag->next = malloc( sizeof( struct user ) );
	trv = lag->next;
	trv->next = NULL;

	get_salt( salt );

	hash = crypt( newupass, salt );

	snprintf( trv->data, MAXDATASIZE, "%s %s 0", newuname, hash );

	++total_users;
	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user: %s added.", name, newuname );
	send_irc_message( sndmsg );

	saveusers( "user.list" );
	return;
}



void chpass( char *passwd, char *name, char *newpass )
{
	int x;
	char sndmsg[MAXDATASIZE];
	char tmpray[MAXDATASIZE];
	char salt[MAXDATASIZE];
	char *hash;

	/* validate user before we do what they want */
	if( !valid_login( name, passwd ) ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :access denied.", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	if( newpass[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :null passwords are not allowed.", MSGTO );
		send_irc_message( sndmsg );
		return;
	  }

	x = chop( trv->data, tmpray, 0, ' ' );
	x = chop( trv->data, tmpray, x, ' ' ); /* these two calls to chop() are to init x. the data is discarded */
	strncpy( tmpray, &trv->data[x], MAXDATASIZE );

	get_salt( salt );

	hash = crypt( newpass, salt );
	snprintf( trv->data, MAXDATASIZE, "%s %s %s", name, hash, tmpray );

	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :password changed.", MSGTO );
	send_irc_message( sndmsg );

	saveusers( "user.list" );
	return;
}



void whois( char *name )
{
	int position = 0;
	char tmpray[MAXDATASIZE];

	position = valid_user( name );
	if( !position ) {
		snprintf( tmpray, MAXDATASIZE, "privmsg %s :%s is not known to me.", MSGTO, name );
		send_irc_message( tmpray );
		return;
	  }

	snprintf( tmpray, MAXDATASIZE, "privmsg %s :I know who %s is.", MSGTO, name );
	send_irc_message( tmpray );

	return;
}



/* returns a positive value if the user is found. */

int valid_user( char *nickname )
{
	char checklistname[USERINFO];

	lag = usr;
	trv = usr->next;

	while( trv ){
		chop( trv->data, checklistname, 0, ' ' );
		if( strcmp( nickname, checklistname ) ) {
			lag = trv;
			trv = trv->next;
			continue;
		}
		return 1;
	}

	return 0;
}


/* returns true/false */

int valid_password( char *passwd )
{
	int x;
	char checklistpword[MAXDATASIZE];
	char salt[MAXDATASIZE];
	char *hash;

	x = chop( trv->data, checklistpword, 0, ' ' );
	x = chop( trv->data, checklistpword, x, ' ' ); /* this now holds the password in hashed form. */

	salt[0] = checklistpword[0]; /* the salt is held in the first two characters */
	salt[1] = checklistpword[1];
	salt[2] = '\0'; /* since this is address space on the stack, i will null terminate it explicitly */

	hash = crypt( passwd, salt );

	if( strcmp( hash, checklistpword ) ) return 0; /* passwd hashes don't match */
	return 1;
}


/*this is basically a wrapper for the two functions that verify users. */
int valid_login( char *name, char *passwd )
{
	int position = 0;

	if( !valid_user( name ) ) return 0;
	position = valid_password( passwd );
	return position;
}



/* no comment right now. sorry. */

void oppeople( char *chan, char *passwd, char *name, char *nick )
{
	char sndmsg[MAXDATASIZE];


	if( valid_login( name, passwd ) ){
		snprintf( sndmsg, MAXDATASIZE, "mode %s +o %s", chan, nick );
		send_irc_message( sndmsg );
		return;
	  }

	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :either your username or password is incorrect. channels require a #.", MSGTO );
	send_irc_message( sndmsg );

	return;
}



void get_salt( char *ray )
{

	long x = 0;

	x = 1 + (rand() % 26);
	x += 64;
	*ray = (char)x;

	x = 1 + (rand() % 26);
	x+= 64;
	*(ray + 1) = (char)x;
	*(ray + 2) = '\0';

	return;
}


/* this function just lists all of the users in the database and messages it to the person who made the query.
 * the magic number of twelve is used because no nick can go beyond nine and I want to append some custom
 * characters to the end of the list. Code is more complex due to minimizing the number of messages.
 */

int list_users( char *nick )
{
	long x = 0;
	long position = 0;
	char checklistname[USERINFO];
	char nickstore[MAXDATASIZE];
	char sndmsg[MAXDATASIZE];

	nickstore[0] = '\0';
	lag = usr;
	trv = usr->next;

	while( trv ){
		if( position >= MAXDATASIZE - 40 ) {
			snprintf( sndmsg, MAXDATASIZE, "privmsg %s :%s==MORE==", nick, nickstore );
			send_irc_message( sndmsg );
			nickstore[0] = '\0';
			position = 0;
		}

		x = chop( trv->data, checklistname, 0, ' ' );
		strncat( nickstore, checklistname, 12 );
		strncat( nickstore, " ", 12 );

		position = position + x + 1; /* +1 to account for the space being added.*/
		lag = trv;
		trv = trv->next;
	}
	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :%sare known to me.", nick, nickstore );
	send_irc_message( sndmsg );

	return 0;
}
