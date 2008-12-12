/** pQueue.c : Priority Queue for functions.
 *
 * Copyright (c) 2002 Steve Mertz <steve@dragon-ware.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/
/*
 * Whom: Steve Mertz <steve@dragon-ware.com>
 * Date: 2002.12.16
 * Why:  To have a queue of functions that need to be executed in the future.
 *
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include "pQueue.h"

/* returns the current realtime */
pQueueTime_t pQueueRealtime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (pQueueTime_t)tv.tv_sec * PQUE_REALTIME_RESOLUTION +
		((pQueueTime_t)tv.tv_usec * PQUE_REALTIME_RESOLUTION)/10000000;
}

/** pQueueAdd()
 * Add an entry to the Priority Queue for execution later.
 *
 * Returns: success
*/
int pQueueAdd(struct pQueue **theQueue, pQueueTime_t executeTime, void (*func)(void*), void *args)
{
  struct pQueue *cur, *prv, *tmp;
	
  tmp = calloc(1, sizeof(struct pQueue));

  tmp->timeExecute = executeTime;
  tmp->func = func;
  tmp->args = args;
  tmp->next = NULL;

  prv = NULL;
  cur = *theQueue;
  /* Make sure we put it in the correct spot. */
  while (cur && cur->timeExecute <= tmp->timeExecute)
    {
      prv = cur;
      cur = cur->next;
    }
  if (!prv)
    {
      tmp->next = *theQueue;
      *theQueue = tmp;
    }
  else
    {
      prv->next = tmp;
      tmp->next = cur;
    }
  return 1;
}

/** pQueueRun()
 * Run a check against our Queue and execute any outstanding items.
 *
 * Returns: success
*/
pQueueTime_t pQueueRun(struct pQueue **theQueue, pQueueTime_t executeTime)
{
	struct pQueue *cur,*last;
	cur=*theQueue;
	while( cur && executeTime >= cur->timeExecute ) {
		last=cur;
		cur=cur->next;
		*theQueue=cur;
		last->func(last->args);
		free(last);
	}
	if (!*theQueue)
		return (pQueueTime_t)-1;
	else
		return (*theQueue)->timeExecute;
}

/** pQueueDump()
 * Dump some of the contents of the Queue to see what's there.
 *
 * Returns: void
*/
void pQueueDump(struct pQueue **list)
{
  struct pQueue *tmp = *list;
  if (!tmp)
    {
      return;
    }
  while (tmp)
    {
      fprintf(stdout, "Number: %lu\n", tmp->number);
      fprintf(stdout, "\tEntered: %llu\n", (unsigned long long)tmp->timeEntered);
      fprintf(stdout, "\tExecute: %llu\n", (unsigned long long)tmp->timeExecute);
      /* fprintf(stdout, "\tArgs: %s\n", tmp->args); */
      tmp = tmp -> next;
    }
}

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
