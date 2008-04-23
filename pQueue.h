/** pQueue.h : Priority Queue for functions. 
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


#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

typedef unsigned long long pQueueTime_t;

#define PQUE_REALTIME	pQueueRealtime()
#define PQUE_REALTIME_RESOLUTION	1
#define PQUE_SECONDS  * PQUE_REALTIME_RESOLUTION
#define PQUE_MINUTES  *(60 PQUE_SECONDS)
#define PQUE_HOURS    *(3600 PQUE_SECONDS)
#define PQUE_DAYS     *(86400 PQUE_SECONDS)
#define PQUE_WEEKS    *(604800 PQUE_SECONDS)
#define PQUE_MONTHS   *(2592000 PQUE_SECONDS) 
#define PQUE_YEARS    *(31557600 PQUE_SECONDS)	/* 365.25 days a year */ 

struct pQueue
{
  unsigned long number;   /* Number in the Queue */
  pQueueTime_t timeEntered;     /* Time entered in the Queue */
  pQueueTime_t timeExecute;     /* Time we should execute */
  
  void (*func)(void *);   /* Function we are going to hold */
  void *args;             /* The arguments for the function */
  
  struct pQueue *next;
};

pQueueTime_t pQueueRealtime(void);
int pQueueAdd(struct pQueue **theQueue, pQueueTime_t executeTime, void (*func)(void*), void *args);
int pQueueRun(struct pQueue **theQueue, pQueueTime_t aTime);
void pQueueDump(struct pQueue **theQueue);

#endif  /* __PRIORITY_QUEUE_H__ */

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
