/* autovoice.c */
/*
 * Copyright (c) 2008 Jon Mayo
 * This work may be modified and/or redistributed, as long as this license and
 * copyright text is retained. There is no warranty, express or implied.
 *
 * Original: April 22, 2008
 * Last Updated: April 25, 2008
 */

#include <assert.h>
#include "autovoice.h"
#include "bot.h"
#include "debug.h"
#include "rc.h"
#include "mode.h"
#include "notify.h"

#define MAX_CHANNEL_LIST	1024	/* string used to hold all channel names */

struct naughty_list {
	char nick[MAXNICKSIZE];
	struct naughty_list *next;
};

struct channel_list {
	char name[MAX_CHANNEL_NAME];
	struct channel_list *next;
	struct naughty_list *naughty_head;
} *enabled_channels;

static struct channel_list *autovoice_channel_lookup(const char *channel)
{
	struct channel_list *curr;
	for(curr=enabled_channels;curr;curr=curr->next) {
		if(!strcasecmp(curr->name, channel)) return curr; /* found */
	}
	return 0; /* not found */
}

static void autovoice_enable_channel(const char *channel)
{
	struct channel_list *newent;
	assert(channel!=NULL);
	assert(channel[0]!=0);
	if(!channel || channel[0]==0) return; /* ignore - empty */
	if(autovoice_channel_lookup(channel)) return; /* ignore - already found */

	newent=malloc(sizeof *newent);
	if(!newent) return; /* out of memory */

	strncpy(newent->name, channel, sizeof newent->name);
	newent->name[sizeof newent->name-1]=0;
	newent->next=enabled_channels;
	enabled_channels=newent;

	if(verbose>0) {
		INFO("autovoice: enabled voicing for: %s\n", newent->name);
	}
}

static struct naughty_list *find_naughty(struct channel_list *ch, const char *nick, struct naughty_list ***prev)
{
	struct naughty_list *curr, **_prev;

	assert(ch != NULL);
	assert(nick != NULL);
	if(!ch || !nick) return 0;

	for(_prev=&ch->naughty_head,curr=*_prev;curr;_prev=&curr->next,curr=*_prev) {
		/* TODO: support the funny ASCII folding that some ircds use */
		if(!strcasecmp(nick, curr->nick)) {
			/* found the entry */
			if(prev) *prev=_prev;
			return curr;
		}
	}
	return 0; /* not found */
}

static int is_naughty(struct channel_list *ch, const char *nick)
{
	return find_naughty(ch, nick, 0)!=NULL;
}

static void add_naughty(struct channel_list *ch, const char *nick)
{
	struct naughty_list *newent;

	assert(ch != NULL);
	assert(nick != NULL);

	if(verbose>0) {
		INFO("autovoice: %s will no longer be voiced in %s\n", nick, ch->name);
	}

	/* add a new entry */
	newent=malloc(sizeof *newent);
	strncpy(newent->nick, nick, sizeof newent->nick);
	newent->nick[sizeof newent->nick-1]=0;
	newent->next=ch->naughty_head;
	ch->naughty_head=newent;
}

static void remove_naughty(struct channel_list *ch, const char *nick)
{
	struct naughty_list *curr, **prev;

	assert(ch != NULL);
	assert(nick != NULL);

	curr=find_naughty(ch, nick, &prev);
	if(!curr) {
		if(verbose>3) {
			INFO("autovoice: do not see %s in %s naughty list\n", nick, ch->name);
		}
		return; /* nothing to do */
	}

	if(verbose>0) {
		INFO("autovoice: %s will once again be voiced in %s\n", nick, ch->name);
	}

	assert(prev != NULL);

	BUGON(!prev, return); /* weirdness - give up */

	/* remove the entry from the list */
	*prev=curr->next;
	free(curr);

	/* TODO: remove entry from voice queue */
}

static void av_onjoin(void *p, struct message *msg)
{
	char ray[MAXDATASIZE];
	struct channel_list *ch;

	assert(msg!=NULL);
	if(!msg) return;
	if(!msg->msgto[0]) return; /* ignore msgto */
	if(!msg->nick[0]) return; /* ignore nick - seems weird */
	/* TODO: check nick!user@host for bans with fnmatch() */

	if(verbose>2) {
		INFO("autovoice: Saw a join - msgto:'%s' nick:'%s'\n", msg->msgto, msg->nick);
	}

	ch=autovoice_channel_lookup(msg->msgto);
	if(ch) {
		if(is_naughty(ch, msg->nick)) {
			if(verbose>0) {
				INFO("autovoice: nick %s won't be voiced in %s.\n", msg->nick, msg->msgto);
			}
			
			return; /* do no voice people who were -v'd */
		}
		/* TODO: delay and forget voicing if someone voices first */
		snprintf( ray, MAXDATASIZE, "mode %s +v %s", msg->msgto, msg->nick );
		send_irc_message( ray );
	}
}

static void av_onmode(void *p, struct message *msg)
{
	char buf[MAXNICKSIZE];
	char flag[2];
	struct mode_parser mp;
	struct channel_list *ch;

	/* TODO: find autovoice channel entry */
	assert(msg!=NULL);
	if(!msg) return;
	if(!msg->msgto[0]) return; /* ignore msgto */
	if(!msg->nick[0]) return; /* ignore nick - seems weird */

	ch=autovoice_channel_lookup(msg->msgto);
	if(!ch) {
		if(verbose>0) {
			INFO("autovoice: ignored %s - not a channel we are monitoring\n", msg->msgto);
		}
		return; /* ignore - not a channel we are monitoring */
	}

	parse_mode_begin(&mp, msg->fulltext);
	while(parse_mode_next(&mp, flag, buf, sizeof buf)){
		if(verbose>1) {
			INFO("autovoice: MODE %s %c%c %s\n",
				msg->msgto,
				flag[0],
				flag[1],
				buf
			);
		}
		if(flag[1]=='v') {
			if(flag[0]=='+') {
				INFO("remove..\n");
				remove_naughty(ch, buf);
			} else {
				INFO("add..\n");
				add_naughty(ch, buf);
			}
		}
	}
}

int autovoice_init(struct config_node *config_root)
{
	struct config_node *config_curr, *item;
	char channels[MAX_CHANNEL_LIST];
	int i, next, len;

	config_curr=config_find(config_root, "autovoice");
	if(!config_curr) return 0;
	config_curr=config_curr->child;
	if(!config_curr) return 0;

	/* parse config */
	item=config_find(config_curr, "channels");

	if(!item || !config_get_str(item, channels, sizeof channels)) {
		 ERROR("failed loading %s\n", "autovoice.channels" );
		 channels[0]=0; /* empty string */
	}

	/* get a list of channel names seperated by whitespace */
	for(i=0;channels[i];) {
		/* null terminate the token */
		len=strcspn(channels+i, WHITESPACE);
		if(!len) break;
		next=i+len;
		if(channels[next]) {
			channels[next]=0; /* null terminate */
			next++;
			next+=strspn(channels+next, WHITESPACE);
		}
		autovoice_enable_channel(channels+i);
		i=next;
	}
		
	
	notify_register("JOIN", av_onjoin, 0);
	notify_register("MODE", av_onmode, 0);
	/* get the CHANMODES */
	/* notify_register("005", av_on005, 0); */
	return 1; /* succcess */
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
