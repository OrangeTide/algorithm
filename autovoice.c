/* autovoice.c */
/*
 * Copyright (c) 2008 Jon Mayo
 * This work may be modified and/or redistributed, as long as this license and
 * copyright text is retained. There is no warranty, express or implied.
 *
 * Last Updated: April 22, 2008
 */

#include <assert.h>
#include "autovoice.h"
#include "bot.h"
#include "rc.h"
#include "notify.h"

#define MAX_CHANNEL_LIST	1024	/* string used to hold all channel names */
#define MAX_CHANNEL_NAME	60		/* length of an individual channel name */

struct channel_list {
	char name[MAX_CHANNEL_NAME];
	struct channel_list *next;
} *enabled_channels;

static int is_autovoice_channel(const char *channel)
{
	struct channel_list *curr;
	for(curr=enabled_channels;curr;curr=curr->next) {
		if(!strcasecmp(curr->name, channel)) return 1; /* found */
	}
	return 0; /* not found */
}

static void autovoice_enable_channel(const char *channel)
{
	struct channel_list *newent;
	assert(channel!=NULL);
	assert(channel[0]!=0);
	if(!channel || channel[0]==0) return; /* ignore - empty */
	if(is_autovoice_channel(channel)) return; /* ignore - already found */

	newent=malloc(sizeof *newent);
	if(!newent) return; /* out of memory */

	strncpy(newent->name, channel, sizeof newent->name);
	newent->name[sizeof newent->name-1]=0;
	newent->next=enabled_channels;
	enabled_channels=newent;

	if(verbose>0) {
		fprintf(stderr, "autovoice: enabled voicing for: %s\n", newent->name);
	}
}

static void av_onjoin(void *p, struct message *msg)
{
	char ray[MAXDATASIZE];

	assert(msg!=NULL);
	if(!msg) return;
	if(!msg->msgto[0]) return; /* ignore msgto */
	if(!msg->nick[0]) return; /* ignore nick - seems weird */
	/* TODO: check nick!user@host for bans with fnmatch() */

	if(verbose>2) {
		fprintf(stderr, "autovoice: Saw a join - msgto:'%s' nick:'%s'\n", msg->msgto, msg->nick);
	}

	if(is_autovoice_channel(msg->msgto+1)) {
		/* TODO: delay and forget voicing if someone voices first */
		snprintf( ray, MAXDATASIZE, "mode %s +v %s", msg->msgto+1, msg->nick );
		send_irc_message( ray );
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
		 fprintf(stderr, "failed loading %s\n", "autovoice.channels" );
		 channels[0]=0; /* empty string */
	}

	/* get a list of channel names seperated by whitespace */
	for(i=0;channels[i];) {

		/* null terminate the token */
		len=strcspn(channels+i, " \t\r\n\v");
		if(!len) break;
		if(channels[i+len]) {
			next=i+len;
		} else {
			next=i+len+1; /* move past the null terminator */
			channels[i+len]=0;
		}
		autovoice_enable_channel(channels+i);

		i=next; /* go to point after the token */
		i+=strspn(channels+i, " \t\r\n\v");
	}
		
	
	notify_register("JOIN", av_onjoin, 0);
	return 1; /* succcess */
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
