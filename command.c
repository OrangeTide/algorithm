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

#include <assert.h>
#include "bot.h"
#include "command.h"
#include "notify.h"

static void got_message(void *p, struct message *msg)
{
	assert(msg!=NULL);
	if(!msg) return; /* ignore msg==NULL */

	/* this sets msgto to the msg sender's nick if it was a privmsg to the bot itself.*/
	/* otherwise it makes it possible to talk to the channel that sent the message. */
	if( !strcmp( msg->msgto, BOTNAME ) )
		strncpy( MSGTO, msg->nick, MAXDATASIZE );
	else
		strncpy( MSGTO, msg->msgto, MAXDATASIZE );

	/* switch on the first character of the first "word" in the message */
	switch( tolower(msg->msgarg1[0]) ) {
		case 1:
			if( !strncasecmp( BOTNAME, msg->msgto, MAXDATASIZE ) ) { do_ctcp(); return; }
			break;
		case 'c':
			if( !strncasecmp( "chpass", msg->msgarg1, MAXDATASIZE ) ) { chpass_stub(); return; }
			if( !strncasecmp( "calc", msg->msgarg1, MAXDATASIZE ) ) { docalc_stub(); return; }
			if( !strncasecmp( "clac", msg->msgarg1, MAXDATASIZE ) ) { docalc_stub(); return; }
			if( !strncasecmp( "chcalc", msg->msgarg1, MAXDATASIZE ) ) { chcalc_stub(); return; }
			break;
		case 'o':
			if( !strncasecmp( "op", msg->msgarg1, MAXDATASIZE ) ) { oppeople_stub(); return; }
			if( !strncasecmp( "owncalc", msg->msgarg1, MAXDATASIZE ) ) { owncalc_stub(); return; }
			break;
		case 'p':
			if( !strncasecmp( "proto", msg->msgarg1, MAXDATASIZE ) ) { proto_stub(); return; }
			break;
		case 'w':
			if( !strncasecmp( "whois", msg->msgarg1, MAXDATASIZE ) ) { whois_stub(); return; }
			if( !strncasecmp( "wcalc", msg->msgarg1, MAXDATASIZE ) ) { wcalc_stub(); return; }
			break;
		case 'a':
			if( !strncasecmp( "adduser", msg->msgarg1, MAXDATASIZE ) ) { adduser_stub(); return; }
			break;
		case 'h':
			if( !strncasecmp( "help", msg->msgarg1, MAXDATASIZE ) ) { help(); return; }
			break;
		case 'r':
			if( !strncasecmp( "rmuser", msg->msgarg1, MAXDATASIZE ) ) { rmuser_stub(); return; }
			if( !strncasecmp( "rmcalc", msg->msgarg1, MAXDATASIZE ) ) { rmcalc_stub(); return; }
			if( !strncasecmp( "rawirc", msg->msgarg1, MAXDATASIZE ) ) { rawirc(); return; }
			if( !strncasecmp( "rcalc", msg->msgarg1, MAXDATASIZE ) ) { rpn_stub(); return; }
			if( !strncasecmp( "recalc", msg->msgarg1, MAXDATASIZE ) ) { chcalc_stub(); return; }
			if( !strncasecmp( "rot13", msg->msgarg1, MAXDATASIZE ) ) { rot13_stub(); return; }
			break;
		case 'm':
			if( !strncasecmp( "mkcalc", msg->msgarg1, MAXDATASIZE ) ) { mkcalc_stub(); return; }
			break;
		case 'l':
			if( !strncasecmp( "listcalc", msg->msgarg1, MAXDATASIZE ) ) { listcalc_stub(); return; }
			if( !strncasecmp( "lsusers", msg->msgarg1, MAXDATASIZE ) ) { lsusers_stub(); return; }
			if( !strncasecmp( "login", msg->msgarg1, MAXDATASIZE ) ) { help(); return; }
			break;
		case 'x':
			if( !strncasecmp( "xpln", msg->msgarg1, MAXDATASIZE ) ) { docalc_stub(); return; }
			break;
		case 'd':
			if( !strncasecmp( "dcalc", msg->msgarg1, MAXDATASIZE ) ) { dcalc_stub(); return; }
			break;
		case 's':
			if( !strncasecmp( "searchcalc", msg->msgarg1, MAXDATASIZE ) ) { searchcalc_stub(); return; }
			break;
		case '8':
			if( !strncasecmp( "8ball", msg->msgarg1, MAXDATASIZE) ) { mball_stub(); return; }
			break;
	}
}

int command_init(void)
{
	notify_register("PRIVMSG", got_message, 0);
	notify_register("NOTICE", got_message, 0);
	return 1; /* succcess */
}

