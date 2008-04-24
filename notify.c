/* notify.c */
/*
 * Copyright (c) 2008 Jon Mayo
 * This work may be modified and/or redistributed, as long as this license and
 * copyright text is retained. There is no warranty, express or implied.
 *
 * Last Updated: April 22, 2008
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "bot.h"
#include "notify.h"
#include "strhash.h"

#define HASH_SZ 256 /* hash table size - does not have to be power of 2 */

struct notify_handler {
    struct notify_handler **prev, *next;
    void (*func)(void *p, struct message *msg);
    void *p;
};

struct notify_head {
	char *type; /* key for the entry */
	struct notify_handler *head;
	struct notify_head *next; /* collision */
};

/* hash table for notify based on prototype message type */
static struct notify_head *notify_hash[HASH_SZ];
/* TODO: add a table for event based notification. like myname events */

/* if should_create is non-zero, it can only return error for out of memory */ 
static struct notify_head *find_head(const char *type, int should_create, struct notify_head ***prev)
{
	unsigned h;
	struct notify_head *curr, **_prev;
	h=strcasehash(type)%HASH_SZ;
	for(_prev=&notify_hash[h],curr=*_prev;curr;_prev=&curr->next,curr=*_prev) {
		if(!strcasecmp(curr->type, type)) {
			if(prev) *prev=_prev;
			return curr; /* found entry */
		}
	}
	/** not found **/
	if(should_create) {
		curr=malloc(sizeof *curr);
		if(!curr) {
			perror("malloc()");
			return 0; /* out of memory */
		}
		curr->type=strdup(type);
		curr->head=0;
		/* insert onto the top */
		curr->next=notify_hash[h];
		notify_hash[h]=curr;
		return curr; /* created new entry */
	}
	return 0; /* not found */
}

static struct notify_handler *add_entry(struct notify_handler **head, void (*func)(void *p, struct message *msg), void *p)
{
    struct notify_handler *ret;
	assert(head!=NULL);
	assert(func!=NULL);

	if(!head || !func) return NULL; /* failure - bad arguments */

    ret=malloc(sizeof *ret);

	assert(ret!=NULL);
    if(!ret) return NULL;

    ret->next=*head;
    ret->prev=head;
    ret->p=p;
    ret->func=func;
	*head=ret;

    return ret;
}

static void remove_and_free_handler(struct notify_handler *h) 
{
    if(h) {
        /* remove from list */
        if(h->next) {
            h->next->prev=h->prev;
        }
        (*h->prev)=h->next;

        /* TODO: how do we free p ?? */
        free(h);
    }
}

/* detach a notify_head and free it */
static void remove_and_free_head(struct notify_head *head, struct notify_head **prev) {
	assert(head!=NULL);
	assert(prev!=NULL);
	assert(head->head==NULL);
	if(!head || !prev || head->head) return; /* ignore invalid state */
	*prev=head->next;
	free(head->type);
	free(head);
}

/* push a notify event out for a message */
void notify_report_message(struct message *msg)
{
	struct notify_head *head;
	struct notify_handler *curr;
	if(!msg) return;
	if(msg->msgtype[0]==0) return; /* ignore untyped messages*/
	if(verbose>2) {
		fprintf(stderr, "Reporting msgtype:%s\n", msg->msgtype);
	}
	head=find_head(msg->msgtype, 0, 0);
	if(!head) return; /* no handler - ignore */

	if(verbose>2) {
		fprintf(stderr, "Found head for msgtype:%s\n", msg->msgtype);
	}
	for(curr=head->head;curr;curr=curr->next) {
		if(verbose>2) {
			fprintf(stderr, "calling func:%p for msgtype:%s\n", curr->func, msg->msgtype);
		}
		if(curr->func) {
			curr->func(curr->p, msg);
		}
	}
}

/* register a notify handler 
 * returns 0 on failure */
int notify_register(const char *type, void (*func)(void *p, struct message *msg), void *p)
{
	struct notify_head *head;

	if(!type) return 0; /* failure */
	head=find_head(type, 1, 0);
	if(!head) return 0; /* failure - probably out of memory */
    return add_entry(&head->head, func, p)!=NULL;
}

/* unregister a handler with matching function pointer
 * keep calling to unregister all. returns 0 when done. */
int notify_unregister(const char *type, void (*func)(void *p, struct message *msg))
{
    struct notify_handler *curr, **prev;
	struct notify_head *head, **headprev;

	if(!type) return 0; /* failure */
	head=find_head(type, 0, &headprev);
	if(!head) return 0; /* failure - not registered */

    prev=&head->head;
    for(curr=*prev;curr;prev=&curr->next,curr=curr->next) {
        if(curr->func==func) {
            remove_and_free_handler(curr);
            curr=*prev;
			assert(headprev!=NULL);
			if(!head->head && headprev) { /* it's empty - free it */
			 	remove_and_free_head(head, headprev);
			}
            return 1; /* removed */
        }
    }
    return 0; /* not found */
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
