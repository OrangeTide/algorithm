/* calcdb.h version 0.333 */
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

/* 12 January 2001 */
#ifndef _CALCDB_H
#define _CALCDB_H 1


#include "bot.h"


int loaddb( char *filename, int maxdbsize );
int savedb( char *filename );
void docalc( char *calcstring );
int findcalc( char *string );
void rmcalc( char *passwd, char *name, char *rmstring );
void mkcalc( char *pass, char *name, char *newcalc, char *newcalctext );
void chcalc( char *pass, char *name, char *calcname, char *newcalctext );
void owncalc( char *name, char *index, char *nick );
void listcalc( char *name, char *dbindex, char *nick );
void searchcalc( char *searchkey, char *dbindex );
void calcnotfound(char *response, int max, char *calcstring);
void calcnotfound_test();



#endif

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
