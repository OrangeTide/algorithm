/* login.c version 0.333 */
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
/* user stuff. ugh! */

#include <stdlib.h>

#include "login.h"
#include "bot.h"


extern struct message current_message;

static struct user *usr;
static struct user *trv;
static struct user *lag;
static long int total_users = 0;




void rmuser( void )
{
	int x = 0;
	char sndmsg[MAXDATASIZE];

	/* validate user before we do what they want */

	if( valid_login( ) ) snprintf( sndmsg, MAXDATASIZE, "privmsg %s :credentials verified.", current_message.nick );
	else {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :access denied.", current_message.nick );
		send_irc_message( sndmsg );
		return;
	  }

	/* arg 4 is the user to delete */	
	if( current_message.msgarg4[0] == '\0' ) return;

	x = valid_user( current_message.msgarg4 );
	if( !x ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user: %s not found.", current_message.nick, current_message.msgarg4 );
		send_irc_message( sndmsg );
		return;
	  }

	lag->next = trv->next;
	free( trv );
	--total_users;
	

	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user: %s removed.", current_message.nick, current_message.msgarg4 );
	send_irc_message( sndmsg );
	saveusers( "user.list" );
	return;
}



void adduser( void )
{
	char sndmsg[MAXDATASIZE];
	char *hash;

	/* validate user before we do what they want */
	if( valid_login( ) ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :credentials verified.", current_message.nick );
		send_irc_message( sndmsg );
	  }
	else {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :access denied.", current_message.nick );
		send_irc_message( sndmsg );
		return;
	  }

	if( current_message.msgarg4[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s : password is invalid", current_message.nick );
		send_irc_message( sndmsg );
		return;
	  }
	if( current_message.msgarg5[0] == '\0' ) {
		snprintf( sndmsg, MAXDATASIZE, "PRIVMSG %s : username is invalid", current_message.nick );
		send_irc_message( sndmsg );
		return;
	  }

	if( valid_user( current_message.msgarg5 ) ) {
		snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user exists.", current_message.nick );
		send_irc_message( sndmsg );
		return;
	  }

	lag->next = malloc( sizeof( struct user ) );
	trv = lag->next;
	trv->next = NULL;

	hash = crypt_md5( current_message.msgarg4, saltgen_md5(rand()+getpid()));

	snprintf( trv->data, MAXDATASIZE, "%s %s 0", current_message.msgarg5, hash );

	++total_users;
	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :user: %s added.", current_message.nick, current_message.msgarg5 );
	send_irc_message( sndmsg );

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


/*this is basically a wrapper for the two functions that verify users. */
int valid_login( void )
{
	if( current_message.msgarg3[0] != '\0' ) {
		if( !valid_user( current_message.msgarg3 ) ) return 0;
	  }
	else { if( !valid_user( current_message.nick ) ) return 0; }

	return valid_password();
}



/* this is an evil function that is in place of a real user database */

void op_people( void )
{
	char sndmsg[MAXDATASIZE];


	if( valid_login( ) ){
		snprintf( sndmsg, MAXDATASIZE, "mode #c +o %s", current_message.nick );
		send_irc_message( sndmsg );
		return;
	  }

	snprintf( sndmsg, MAXDATASIZE, "privmsg %s :either your username or password is incorrect.", current_message.nick );
	send_irc_message( sndmsg );

	return;
}
