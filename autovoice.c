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
#include "notify.h"
#include "bot.h"

static void av_onjoin(void *p, struct message *msg) {
	assert(msg!=NULL);
	if(!msg) return;
	printf("Saw a join - msgto:'%s' nick:'%s'\n", msg->msgto, msg->nick);
}

int autovoice_init()
{
	notify_register("JOIN", av_onjoin, 0);
	return 1; /* succcess */
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
