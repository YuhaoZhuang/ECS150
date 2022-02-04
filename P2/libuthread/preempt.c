#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include<string.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
/* HZ converted to milliseconds */
#define INTERVAL (1000/HZ)

/* interrupt global variable */
struct sigaction sa; 
/* timer global variable */
struct itimerval timer;
struct itimerval timer_save;
/* singal set global variable */
sigset_t block_alarm;

void preempt_disable(void)
{
	/* block the signal related to SIGVTALRM */
	sigprocmask(SIG_BLOCK, &block_alarm, NULL);
}

void preempt_enable(void)
{
  	/* unblock the signal related to SIGVTALRM */
	sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
	
}

void preempt_start(void)
{
	/* set singal set and add SIGVTALRM in order to block/unblock the singal */
	sigemptyset(&block_alarm);
	sigaddset(&block_alarm, SIGVTALRM);
	/* set singal handler to block the thread whenever SIGVTALRM is called */
	sa.sa_handler = (void (*)(int))uthread_yield; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, NULL);
	/* save a copy of current timer */
	getitimer(ITIMER_VIRTUAL, &timer_save);
	/* set timer to interrupt 100 times per second */
	timer.it_value.tv_sec = 0;
  	timer.it_value.tv_usec = INTERVAL * 1000;
  	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = INTERVAL * 1000;
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void)
{
	/* restore timer to previous state */
	setitimer(ITIMER_VIRTUAL, &timer_save, NULL);
}
