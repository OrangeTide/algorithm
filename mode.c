/* mode.c */
/*
 * Copyright (c) 2008 Jon Mayo
 * This work may be modified and/or redistributed, as long as this license and
 * copyright text is retained. There is no warranty, express or implied.
 *
 * Original: April 25, 2008
 * Last Updated: April 25, 2008
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "bot.h"
#include "mode.h"

/* there is no parse_mode_end */
void parse_mode_begin(struct mode_parser *st, const char *fulltext) {
    assert(fulltext != NULL);
    assert(st != NULL);
    if(!st) return;
    st->mode_curr=fulltext;
    st->plus_or_minus=1;
    st->arg_curr=fulltext;
    if(st->arg_curr) {
        /* skip first 'word' */
        st->arg_curr+=strcspn(st->arg_curr, WHITESPACE);
        st->arg_curr+=strspn(st->arg_curr, WHITESPACE);
    }
}

/* gets the new flag and argument */
int parse_mode_next(struct mode_parser *st, char mode_flag[2], char *arg, size_t arglen) {
    unsigned sublen;

    if(!st->arg_curr || !st->arg_curr[0]) return 0; /* done */
    if(!st->mode_curr) return 0; /* done */

    /* resolve any + or - characters */
    while(1) {
        if(!st->mode_curr[0] || isspace(st->mode_curr[0]))
            return 0; /* done */

        if(*st->mode_curr=='+') {
            st->plus_or_minus=1;
        } else if(*st->mode_curr=='-') {
            st->plus_or_minus=0;
        } else {
            /* found a flag */
            mode_flag[0]=st->plus_or_minus?'+':'-';
            mode_flag[1]=*st->mode_curr++;
            break;
        }
        st->mode_curr++;
    }

    /* these modes don't take parameters - add more for fancier irc servers */
    if(mode_flag[0]=='i' || mode_flag[0]=='m' || mode_flag[0]=='n' || mode_flag[0]=='p' || mode_flag[0]=='s' || mode_flag[0]=='t') {
        arg[0]=0; /* no arg */
        return 1;
    }

    /* find an arg */
    st->arg_curr+=strspn(st->arg_curr, WHITESPACE); /* skip whitespaces */
    sublen=strcspn(st->arg_curr, WHITESPACE); /* find end of string */
    if(sublen>=arglen) { /* argument too long for arglen */
        memcpy(arg, st->arg_curr, arglen-1);
        arg[arglen-1]=0;
    } else {
        memcpy(arg, st->arg_curr, sublen);
        arg[sublen]=0;
    }
    st->arg_curr+=sublen;
    return 1; /* success */
}

#if 0 /* example code */
int main() {
    struct mode_parser st;
    char str[]="+o-ovv algorithm x suravad zid";
    char flag[2];
    char buf[50];

    parse_mode_begin(&st, str);
    while(parse_mode_next(&st, flag, buf, sizeof buf)){
        printf("%c%c %s\n", flag[0], flag[1], buf);
    }
}
#endif
