/*
 * Copyright (c) 2007-2013 Antti Kantee.  All Rights Reserved.
 * Copyright (c) 2014 Justin Cormack.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Based partly on code from Xen Minios with the following license */

/* 
 ****************************************************************************
 * (C) 2005 - Grzegorz Milos - Intel Research Cambridge
 ****************************************************************************
 *
 *        File: sched.c
 *      Author: Grzegorz Milos
 *     Changes: Robert Kaiser
 *              
 *        Date: Aug 2005
 * 
 * Environment: Xen Minimal OS
 * Description: simple scheduler for Mini-Os
 *
 * The scheduler is non-preemptive (cooperative), and schedules according 
 * to Round Robin algorithm.
 *
 ****************************************************************************
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmk/types.h>
#include <bmk/queue.h>

#include <bmk/kernel.h>
#include <bmk/sched.h>
#include <bmk/string.h>
#include <bmk/memalloc.h>

#define TLS_COUNT 2
#define NAME_MAXLEN 16

#define THREAD_RUNNABLE 0x01
#define THREAD_MUSTJOIN	0x02
#define THREAD_JOINED	0x04
#define THREAD_EXTSTACK	0x08
#define THREAD_TIMEDOUT 0x10

#define THREAD_STACKSIZE (1<<16)

struct bmk_thread {
	char bt_name[NAME_MAXLEN];

	void *bt_tls[TLS_COUNT];

	int64_t bt_wakeup_time;

	int bt_flags;
	int bt_errno;

	void *bt_stackbase;

	/* context is saved here during context switch */
	struct bmk_tcb {
		void *btcb_sp;
		void *btcb_ip;
	} bt_tcb;

	TAILQ_ENTRY(bmk_thread) bt_entries;
};

static TAILQ_HEAD(, bmk_thread) zombies = TAILQ_HEAD_INITIALIZER(zombies);
static TAILQ_HEAD(, bmk_thread) threads = TAILQ_HEAD_INITIALIZER(threads);

static void (*scheduler_hook)(void *, void *);

static struct bmk_thread *current_thread = NULL;
struct bmk_thread *
bmk_sched_current(void)
{

	return current_thread;
}

static int
is_runnable(struct bmk_thread *thread)
{

	return thread->bt_flags & THREAD_RUNNABLE;
}

static void
set_runnable(struct bmk_thread *thread)
{

	thread->bt_flags |= THREAD_RUNNABLE;
}

static void
clear_runnable(struct bmk_thread *thread)
{

	thread->bt_flags &= ~THREAD_RUNNABLE;
}

static void *
stackalloc(void)
{

	return bmk_xmalloc(THREAD_STACKSIZE);
}

static void
stackfree(void *stack)
{

	bmk_memfree(stack);
}

bmk_time_t
bmk_clock_now(void)
{
	uint64_t val;
	unsigned long eax, edx;

	/* um um um */
	__asm__ __volatile__("rdtsc" : "=a"(eax), "=d"(edx));
	val = ((uint64_t)edx<<32)|(eax);

	/* just approximate that 1 cycle = 1ns.  "good enuf" for now */
	return val;
}

static void
sched_switch(struct bmk_thread *prev, struct bmk_thread *next)
{

	current_thread = next;
	bmk_cpu_sched_switch(&prev->bt_tcb, &next->bt_tcb);
}

#define ONE_SEC_IN_NS (1000*1000*1000ULL)

void
bmk_sched(void)
{
	struct bmk_thread *prev, *next, *thread, *tmp;
	bmk_time_t tm, wakeup;

	prev = bmk_sched_current();

	/* could do time management a bit better here */
	do {
		tm = bmk_clock_now();
		next = NULL;
		TAILQ_FOREACH_SAFE(thread, &threads, bt_entries, tmp) {
			if (!is_runnable(thread)
			    && thread->bt_wakeup_time >= 0) {
				if (thread->bt_wakeup_time <= tm) {
					thread->bt_flags |= THREAD_TIMEDOUT;
					bmk_sched_wake(thread);
				} else if (thread->bt_wakeup_time < wakeup)
					wakeup = thread->bt_wakeup_time;
			}
			if (is_runnable(thread)) {
				next = thread;
				/* Put this thread on the end of the list */
				TAILQ_REMOVE(&threads, thread, bt_entries);
				TAILQ_INSERT_TAIL(&threads, thread, bt_entries);
				break;
			}
		}
		if (next)
			break;
		/*
		 * no runnables.  hlt for a while.  TODO: make accurate
		 */
		bmk_cpu_nanohlt();
	} while (1);

	if (prev != next) {
		sched_switch(prev, next);
	}

	/* reaper */
	TAILQ_FOREACH_SAFE(thread, &zombies, bt_entries, tmp) {
		if (thread != prev) {
			TAILQ_REMOVE(&zombies, thread, bt_entries);
			if ((thread->bt_flags & THREAD_EXTSTACK) == 0)
				stackfree(thread->bt_stackbase);
			bmk_memfree(thread);
		}
	}
}

void bmk_cpu_sched_bouncer(void);
struct bmk_thread *
bmk_sched_create(const char *name, void *cookie, int thrflags,
	void (*f)(void *), void *data,
	void *stack_base, unsigned long stack_size)
{
	struct bmk_thread *thread;
	void *stack;

	thread = bmk_xmalloc(sizeof(*thread));
	bmk_memset(thread, 0, sizeof(*thread));

	if (!stack_base) {
		assert(stack_size == 0);
		stack_size = THREAD_STACKSIZE;
		stack_base = stackalloc();
	} else {
		thread->bt_flags = THREAD_EXTSTACK;
	}

	thread->bt_stackbase = stack_base;
	stack = (uint8_t *)stack_base + stack_size;
	bmk_cpu_sched_create(thread, f, data, &stack);

	thread->bt_tcb.btcb_sp = stack;
	thread->bt_tcb.btcb_ip = bmk_cpu_sched_bouncer;
	
	/* enotyet */
	//thread->bt_cookie = cookie;

	bmk_strncpy(thread->bt_name, name, sizeof(thread->bt_name)-1);

	thread->bt_wakeup_time = -1;

	set_runnable(thread);
	TAILQ_INSERT_HEAD(&threads, thread, bt_entries);

	return thread;
}

struct join_waiter {
    struct bmk_thread *jw_thread;
    struct bmk_thread *jw_wanted;
    TAILQ_ENTRY(join_waiter) jw_entries;
};
static TAILQ_HEAD(, join_waiter) joinwq = TAILQ_HEAD_INITIALIZER(joinwq);

void
bmk_sched_exit(void)
{
	struct bmk_thread *thread = bmk_sched_current();
	struct join_waiter *jw_iter;

	/* if joinable, gate until we are allowed to exit */
	while (thread->bt_flags & THREAD_MUSTJOIN) {
		thread->bt_flags |= THREAD_JOINED;

		/* see if the joiner is already there */
		TAILQ_FOREACH(jw_iter, &joinwq, jw_entries) {
			if (jw_iter->jw_wanted == thread) {
				bmk_sched_wake(jw_iter->jw_thread);
				break;
			}
		}
		bmk_sched_block(thread);
		bmk_sched();
	}

	/* Remove from the thread list */
	TAILQ_REMOVE(&threads, thread, bt_entries);
	clear_runnable(thread);
	/* Put onto exited list */
	TAILQ_INSERT_HEAD(&zombies, thread, bt_entries);

	/* Schedule will free the resources */
	for (;;) {
		bmk_sched();
	}
}

void
bmk_sched_join(struct bmk_thread *joinable)
{
	struct join_waiter jw;
	struct bmk_thread *thread = bmk_sched_current();

	assert(joinable->bt_flags & THREAD_MUSTJOIN);

	/* wait for exiting thread to hit thread_exit() */
	while (! (joinable->bt_flags & THREAD_JOINED)) {

		jw.jw_thread = thread;
		jw.jw_wanted = joinable;
		TAILQ_INSERT_TAIL(&joinwq, &jw, jw_entries);
		bmk_sched_block(thread);
		bmk_sched();
		TAILQ_REMOVE(&joinwq, &jw, jw_entries);
	}

	/* signal exiting thread that we have seen it and it may now exit */
	assert(joinable->bt_flags & THREAD_JOINED);
	joinable->bt_flags &= ~THREAD_MUSTJOIN;

	bmk_sched_wake(joinable);
}

void
bmk_sched_nanosleep(bmk_time_t nsec)
{
	struct bmk_thread *thread = bmk_sched_current();
	uint64_t now;

	now = bmk_clock_now();
	thread->bt_wakeup_time = now + nsec;
	clear_runnable(thread);
	bmk_sched();
}

void
bmk_sched_setwakeup(struct bmk_thread *thread, bmk_time_t abstime)
{

	thread->bt_wakeup_time = abstime;
}

void
bmk_sched_wake(struct bmk_thread *thread)
{

	thread->bt_wakeup_time = -1;
	set_runnable(thread);
}

void
bmk_sched_block(struct bmk_thread *thread)
{

	thread->bt_wakeup_time = -1;
	clear_runnable(thread);
}

static struct bmk_thread init_thread;

void
bmk_sched_init(void)
{
	struct bmk_thread *thread = &init_thread;

	//getcontext(&thread->ctx);

	bmk_strncpy(thread->bt_name, "init", sizeof(thread->bt_name)-1);
	thread->bt_flags = 0;
	thread->bt_wakeup_time = -1;
	set_runnable(thread);
	TAILQ_INSERT_TAIL(&threads, thread, bt_entries);
	current_thread = thread;
}

void
bmk_sched_set_hook(void (*f)(void *, void *))
{

	scheduler_hook = f;
}

struct bmk_thread *
bmk_sched_init_mainthread(void *cookie)
{

	//current_thread->bt_cookie = cookie;
	return current_thread;
}

int *
bmk_sched_geterrno(void)
{
	struct bmk_thread *thread = bmk_sched_current();

	return &thread->bt_errno;
}

void
bmk_sched_settls(struct bmk_thread *thread, unsigned int which, void *value)
{

	if (which >= TLS_COUNT)
		panic("out of bmk sched tls space");
	thread->bt_tls[which] = value;
}

void *
bmk_sched_gettls(struct bmk_thread *thread, unsigned int which)
{

	if (which >= TLS_COUNT)
		panic("out of bmk sched tls space");
	return thread->bt_tls[which];
}
