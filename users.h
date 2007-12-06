/* users.h version 0.333 */
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

#ifndef _USERS_H
#define _USERS_H 1


#include <sys/types.h>
#include <stdio.h>
#include <strings.h>
#include <stdarg.h>

#include <unistd.h>
#include "bot.h"

#define MAXUSERS 1024
#define USERINFO 512


struct user {
   char data[MAXDATASIZE];
   struct user *next;
  };



int valid_user( char *nickname );
int loadusers( char *filename );
void oppeople( char *chan, char *name, char *passwd, char *nick );
void op( char *nick, char *channel );
void adduser(  char *pass, char *name, char *newupass, char *newuname );
int valid_password( char *passwd );
int valid_login( char *name, char *passwd );
void whois( char *name );
void saveusers( char *filename );
void rmuser( char *passwd, char *name, char *rmname );
void chpass( char *passwd, char *name, char *newpass );
void get_salt( char *ray );
int list_users( void );

#endif /* !_USERS_H */
