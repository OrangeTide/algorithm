/* bot.h version 0.333 */
/* all versions previous to 0.333 are completely public domain */
/* all contributed code falls under the same BSD style license as */
/* noted below unless the contributing author places a copyright */
/* notice in their file/s. */


/*
 *  * Copyright (c) 2001 David T. Stiles
 *  * All rights reserved.
 *  *
 *  * Redistribution and use in source and binary forms, with or without
 *  * modification, are permitted provided that the following conditions
 *  * are met:
 *  * 1. Redistributions of source code must retain the above copyright
 *  *    notice, this list of conditions and the following disclaimer.
 *  * 2. Redistributions in binary form must reproduce the above copyright
 *  *    notice, this list of conditions and the following disclaimer in the
 *  *    documentation and/or other materials provided with the distribution.
 *  * 3. All advertising materials mentioning features or use of this software
 *  *    must display the following acknowledgement:
 *  *      This product includes software developed by David T. Stiles
 *  * 4. The name David T. Stiles may not be used to endorse or promote
 *  *    products derived from this software without specific prior written
 *  *    permission.
 *  *
 *  * THIS SOFTWARE IS PROVIDED BY DAVID T. STILES `AS IS'' AND ANY
 *  * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  * ARE DISCLAIMED.  IN NO EVENT SHALL DAVID T. STILES BE LIABLE
 *  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  * SUCH DAMAGE.
 *  */

/* this code would not be possible without the patience and intelligence */
/* provided by many of the people from #c/efnet. I thank all of you sincerely. */

#ifndef _BOT_H
#define _BOT_H 1

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include "pQueue.h"



#define MAXDATASIZE 515
#define MAXNICKSIZE 60				/* max length of a nick */
#define MAX_CHANNEL_NAME 60			/* max length of an individual channel name */
#define NEWLINE '\n'
#define WHITESPACE " \t\r\n"

extern char MSGTO[MAXDATASIZE]; /* set to nick/channel on each incoming message */
extern char BOTNAME[MAXDATASIZE]; /* holds the actual nickname in use by the bot */
extern sig_atomic_t keep_going; /* flag to break out of the inner loop */
extern sig_atomic_t verbose;    /* flag to enable verbose debugging */

extern struct pQueue *action_queue;

void send_irc_message( char *sndmsg );
void parse_incoming( char *ptr );
void make_a_decision( void);
void main_loop( void );
void reply_ping( char *ptr );
int prep( void );
int process_out( void );
int host_connect( char *exthost, int extport, int extsockfd );
int chop( char *in, char *out, int position, char separator );
int process_in( void );
int irc_connect( void );
void do_ctcp( void );
int clean_message( char *msg );
int prep( void );
int dcalc_stub( void );
int wcalc_stub( void );
void rawirc( void );
int rpn_stub( void );
void help( void );
void chpass_stub( void );
void docalc_stub( void );
void oppeople_stub( void );
void owncalc_stub( void );
void whois_stub( void );
void adduser_stub( void );
void rmuser_stub( void );
void rmcalc_stub( void );
void mkcalc_stub( void );
void chcalc_stub( void );
void listcalc_stub( void );
void searchcalc_stub( void );
void lsusers_stub( void );
void rot13_stub( void );
int proto_stub( void );
void mball_stub( void );
void enable_stub( void );
void disable_stub( void );


/* this is what an irc message will be broken down to */

struct message {
   char nick[MAXDATASIZE];
   char userline[MAXDATASIZE];
   char msgtype[MAXDATASIZE];
   char msgto[MAXDATASIZE];
   char fulltext[MAXDATASIZE];
   char logname[MAXDATASIZE];
   char hostname[MAXDATASIZE];
   char msgarg1[MAXDATASIZE];
   char msgarg2[MAXDATASIZE];
   char msgarg3[MAXDATASIZE];
   char msgarg4[MAXDATASIZE];
   char msgarg5[MAXDATASIZE];
   char msgarg6[MAXDATASIZE];
   char msgarg7[MAXDATASIZE];
   char msgarg8[MAXDATASIZE];
   char msgarg9[MAXDATASIZE];
  };



#define HELPHELP "you should /msg me help commands or help <command-name>."
#define COMMANDS "calc, op, chpass, whois, rmcalc, mkcalc, chcalc, owncalc, searchcalc, listcalc, rmuser, adduser, rawirc, lsusers, rot13, enable, disable. Try, help syntax or help commandname."
#define SYNTAX "Most user commands take the form of COMMAND PASSWORD USERNAME ARGUMENT/S. The op command requires only a password if your nick is the same as your username."
#define ADDUSER "adduser yourpass yourlogin newpass newlogin"
#define CHPASS "chpass yourpass yourlogin newpass"
#define RMUSER "rmuser yourpass yourlogin username-to-delete"
#define RMCALC "rmcalc yourpass yourlogname calc-to-delete"
#define MKCALC "mkcalc yourpass yourlogname calckey calcdata"
#define CHCALC "chcalc yourpass yourlogname calckey calcdata"
#define OP "op #channel yourpass yourlogin, or merely, op yourpass, if your nick, username, and default channel all synchronize."
#define CHATTR "chattr yourpass yourlogin username attributes."
#define RAWIRC "rawirc yourpass yourlogin raw-irc-protocol   no leading / is needed."
#define OWNCALC "owncalc calcname index. will print who the owner of a calc is. will detect erroneus duplicates as well. index can be used to start the search at other than the beginning of the database."
#define LISTCALC "listcalc username index. will print a list of calcs owned by username. index can be used to start the search at other than the beginning of the database."
#define SEARCHCALC "searchcalc substring index. will search the calc data field for an occurrence of substring. index can be used to start the search at other than the beginning of the database."
#define WHOIS "whois username."
#define LSUSERS "lsusers will list all known users in as few messages as possible."
#define ROT13 "rot13 will repeat your message in rot13. usage: rot13 this sentence will be encrypted in rot13."
#define ENABLE "enable yourpass yourlogin feature"
#define DISABLE "disable yourpass yourlogin feature"

#define IS_CHPASS_ENABLED     "is_chpass_enabled"
#define IS_CALC_ENABLED       "is_calc_enabled"
#define IS_CHCALC_ENABLED     "is_chcalc_enabled"
#define IS_OP_ENABLED         "is_op_enabled"
#define IS_OWNCALC_ENABLED    "is_owncalc_enabled"
#define IS_PROTO_ENABLED      "is_proto_enabled"
#define IS_WHOIS_ENABLED      "is_whois_enabled"
#define IS_WCALC_ENABLED      "is_wcalc_enabled"
#define IS_ADDUSER_ENABLED    "is_adduser_enabled"
#define IS_HELP_ENABLED       "is_help_enabled"
#define IS_RMUSER_ENABLED     "is_rmuser_enabled"
#define IS_RMCALC_ENABLED     "is_rmcalc_enabled"
#define IS_RAWIRC_ENABLED     "is_rawirc_enabled"
#define IS_RCALC_ENABLED      "is_rcalc_enabled"
#define IS_ROT13_ENABLED      "is_rot13_enabled"
#define IS_MKCALC_ENABLED     "is_mkcalc_enabled"
#define IS_LISTCALC_ENABLED   "is_listcalc_enabled"
#define IS_LSUSERS_ENABLED    "is_lsusers_enabled"
#define IS_DCALC_ENABLED      "is_dcalc_enabled"
#define IS_SEARCHCALC_ENABLED "is_searchcalc_enabled"
#define IS_AUTOVOICE_ENABLED  "is_autovoice_enabled"

#endif /* !_BOT_H */

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
