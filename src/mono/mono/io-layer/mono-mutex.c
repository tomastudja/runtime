/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Jeffrey Stedfast <fejj@ximian.com>
 *
 *  Copyright 2002 Ximain, Inc. (www.ximian.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>

#include "mono-mutex.h"


#ifndef HAVE_PTHREAD_MUTEX_TIMEDLOCK
int pthread_mutex_timedlock (pthread_mutex_t *mutex,
			    const struct timespec *timeout);

int
pthread_mutex_timedlock (pthread_mutex_t *mutex, const struct timespec *timeout)
{
	struct timeval timenow;
	struct timespec sleepytime;
	int retcode;
	
	/* This is just to avoid a completely busy wait */
	sleepytime.tv_sec = 0;
	sleepytime.tv_nsec = 10000000;	/* 10ms */
	
	while ((retcode = pthread_mutex_trylock (mutex)) == EBUSY) {
		gettimeofday (&timenow, NULL);
		
		if (timenow.tv_sec >= timeout->tv_sec &&
		    (timenow.tv_usec * 1000) >= timeout->tv_nsec) {
			return ETIMEDOUT;
		}
		
		nanosleep (&sleepytime, NULL);
	}
	
	return retcode;
}
#endif /* HAVE_PTHREAD_MUTEX_TIMEDLOCK */


int
mono_once (mono_once_t *once, void (*once_init) (void))
{
	int thr_ret;
	
	if (!once->complete) {
		pthread_cleanup_push ((void(*)(void *))pthread_mutex_unlock,
				      (void *)&once->mutex);
		thr_ret = pthread_mutex_lock (&once->mutex);
		g_assert (thr_ret == 0);
		
		if (!once->complete) {
			once_init ();
			once->complete = TRUE;
		}
		thr_ret = pthread_mutex_unlock (&once->mutex);
		g_assert (thr_ret == 0);
		
		pthread_cleanup_pop (0);
	}
	
	return 0;
}


#ifdef USE_MONO_MUTEX

int
mono_mutexattr_init (mono_mutexattr_t *attr)
{
	memset (attr, 0, sizeof (mono_mutexattr_t));
	return 0;
}

int
mono_mutexattr_settype (mono_mutexattr_t *attr, int type)
{
	attr->type = type;
	return 0;
}

int
mono_mutexattr_gettype (mono_mutexattr_t *attr, int *type)
{
	*type = attr->type;
	return 0;
}

int
mono_mutexattr_setpshared (mono_mutexattr_t *attr, int pshared)
{
	attr->shared = pshared;
	return 0;
}

int
mono_mutexattr_getpshared (mono_mutexattr_t *attr, int *pshared)
{
	*pshared = attr->shared;
	return 0;
}

int
mono_mutexattr_setprotocol (mono_mutexattr_t *attr, int protocol)
{
	attr->protocol = protocol;
	return 0;
}

int
mono_mutexattr_getprotocol (mono_mutexattr_t *attr, int *protocol)
{
	*protocol = attr->protocol;
	return 0;
}

int
mono_mutexattr_setprioceiling (mono_mutexattr_t *attr, int prioceiling)
{
	attr->priority = prioceiling;
	return 0;
}

int
mono_mutexattr_getprioceiling (mono_mutexattr_t *attr, int *prioceiling)
{
	*prioceiling = attr->priority;
	return 0;
}

int
mono_mutexattr_destroy (mono_mutexattr_t *attr)
{
	return 0;
}


int
mono_mutex_init (mono_mutex_t *mutex, const mono_mutexattr_t *attr)
{
	int ret;
	int thr_ret;
	
	mutex->waiters = 0;
	mutex->depth = 0;
	mutex->owner = MONO_THREAD_NONE;
	
	if (!attr || attr->type == MONO_MUTEX_NORMAL) {
		mutex->type = MONO_MUTEX_NORMAL;
		ret = pthread_mutex_init (&mutex->mutex, NULL);
	} else {
		mutex->type = MONO_MUTEX_RECURSIVE;
		ret = pthread_mutex_init (&mutex->mutex, NULL);
		thr_ret = pthread_cond_init (&mutex->cond, NULL);
		g_assert (thr_ret == 0);
	}
	
	return(ret);
}

int
mono_mutex_lock (mono_mutex_t *mutex)
{
	pthread_t id;
	
	switch (mutex->type) {
	case MONO_MUTEX_NORMAL:
		return pthread_mutex_lock (&mutex->mutex);
	case MONO_MUTEX_RECURSIVE:
		id = pthread_self ();
		if (pthread_mutex_lock (&mutex->mutex) != 0)
			return EINVAL;
		
		while (1) {
			if (mutex->owner == MONO_THREAD_NONE) {
				mutex->owner = id;
				mutex->depth = 1;
				break;
			} else if (mutex->owner == id) {
				mutex->depth++;
				break;
			} else {
				mutex->waiters++;
				if (pthread_cond_wait (&mutex->cond, &mutex->mutex) != 0)
					return EINVAL;
				mutex->waiters--;
			}
		}
		
		return pthread_mutex_unlock (&mutex->mutex);
	}
	
	return EINVAL;
}

int
mono_mutex_trylock (mono_mutex_t *mutex)
{
	pthread_t id;
	
	switch (mutex->type) {
	case MONO_MUTEX_NORMAL:
		return pthread_mutex_trylock (&mutex->mutex);
	case MONO_MUTEX_RECURSIVE:
		id = pthread_self ();
		
		if (pthread_mutex_lock (&mutex->mutex) != 0)
			return EINVAL;
		
		if (mutex->owner != MONO_THREAD_NONE && mutex->owner != id) {
			pthread_mutex_unlock (&mutex->mutex);
			return EBUSY;
		}
		
		while (1) {
			if (mutex->owner == MONO_THREAD_NONE) {
				mutex->owner = id;
				mutex->depth = 1;
				break;
			} else {
				mutex->depth++;
				break;
			}
		}
		
		return pthread_mutex_unlock (&mutex->mutex);
	}
	
	return EINVAL;
}

int
mono_mutex_timedlock (mono_mutex_t *mutex, const struct timespec *timeout)
{
	pthread_t id;
	
	switch (mutex->type) {
	case MONO_MUTEX_NORMAL:
		return pthread_mutex_timedlock (&mutex->mutex, timeout);
	case MONO_MUTEX_RECURSIVE:
		id = pthread_self ();
		
		if (pthread_mutex_timedlock (&mutex->mutex, timeout) != 0)
			return ETIMEDOUT;
		
		while (1) {
			if (mutex->owner == MONO_THREAD_NONE) {
				mutex->owner = id;
				mutex->depth = 1;
				break;
			} else if (mutex->owner == id) {
				mutex->depth++;
				break;
			} else {
				mutex->waiters++;
				if (pthread_cond_timedwait (&mutex->cond, &mutex->mutex, timeout) != 0)
					return ETIMEDOUT;
				mutex->waiters--;
			}
		}
		
		return pthread_mutex_unlock (&mutex->mutex);
	}
	
	return EINVAL;
}

int
mono_mutex_unlock (mono_mutex_t *mutex)
{
	int thr_ret;
	
	switch (mutex->type) {
	case MONO_MUTEX_NORMAL:
		return pthread_mutex_unlock (&mutex->mutex);
	case MONO_MUTEX_RECURSIVE:
		if (pthread_mutex_lock (&mutex->mutex) != 0)
			return EINVAL;
		
		assert (mutex->owner == pthread_self ());
		
		mutex->depth--;
		if (mutex->depth == 0) {
			mutex->owner = MONO_THREAD_NONE;
			if (mutex->waiters > 0) {
				thr_ret = pthread_cond_signal (&mutex->cond);
				g_assert (thr_ret == 0);
			}
		}
		
		return pthread_mutex_unlock (&mutex->mutex);
	}
	
	return EINVAL;
}

int
mono_mutex_destroy (mono_mutex_t *mutex)
{
	int ret = 0;
	int thr_ret;
	
	switch (mutex->type) {
	case MONO_MUTEX_NORMAL:
		ret = pthread_mutex_destroy (&mutex->mutex);
		break;
	case MONO_MUTEX_RECURSIVE:
		if ((ret = pthread_mutex_destroy (&mutex->mutex)) == 0) {
			thr_ret = pthread_cond_destroy (&mutex->cond);
			g_assert (thr_ret == 0);
		}
	}
	
	return ret;
}


int
mono_cond_wait (pthread_cond_t *cond, mono_mutex_t *mutex)
{
	return pthread_cond_wait (cond, &mutex->mutex);
}

int
mono_cond_timedwait (pthread_cond_t *cond, mono_mutex_t *mutex, const struct timespec *timeout)
{
	return pthread_cond_timedwait (cond, &mutex->mutex, timeout);
}

#endif /* USE_MONO_MUTEX */
