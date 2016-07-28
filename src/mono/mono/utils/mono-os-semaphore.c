
#include <config.h>
#include <glib.h>

#include <errno.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mono-os-semaphore.h"

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC (1000 * 1000 * 1000)
#endif

#if defined(USE_MACH_SEMA)

void
mono_os_sem_init (MonoSemType *sem, gint value)
{
	kern_return_t res;

	res = semaphore_create (current_task (), sem, SYNC_POLICY_FIFO, value);
	if (G_UNLIKELY (res != KERN_SUCCESS))
		g_error ("%s: semaphore_create failed with error %d", __func__, res);
}

void
mono_os_sem_destroy (MonoSemType *sem)
{
	kern_return_t res;

	res = semaphore_destroy (current_task (), *sem);
	if (G_UNLIKELY (res != KERN_SUCCESS))
		g_error ("%s: semaphore_destroy failed with error %d", __func__, res);
}

gint
mono_os_sem_wait (MonoSemType *sem, MonoSemFlags flags)
{
	kern_return_t res;

retry:
	res = semaphore_wait (*sem);
	if (G_UNLIKELY (res != KERN_SUCCESS && res != KERN_ABORTED))
		g_error ("%s: semaphore_wait failed with error %d", __func__, res);

	if (res == KERN_ABORTED && !(flags & MONO_SEM_FLAGS_ALERTABLE))
		goto retry;

	return res != KERN_SUCCESS ? -1 : 0;
}

MonoSemTimedwaitRet
mono_os_sem_timedwait (MonoSemType *sem, guint32 timeout_ms, MonoSemFlags flags)
{
	kern_return_t res;
	gint resint;
	mach_timespec_t ts, copy;
	struct timeval start, current;

	if (timeout_ms == MONO_INFINITE_WAIT)
		return (MonoSemTimedwaitRet) mono_os_sem_wait (sem, flags);

	ts.tv_sec = timeout_ms / 1000;
	ts.tv_nsec = (timeout_ms % 1000) * 1000000;
	while (ts.tv_nsec >= NSEC_PER_SEC) {
		ts.tv_nsec -= NSEC_PER_SEC;
		ts.tv_sec++;
	}

	copy = ts;
	resint = gettimeofday (&start, NULL);
	if (G_UNLIKELY (resint != 0))
		g_error ("%s: gettimeofday failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);

retry:
	res = semaphore_timedwait (*sem, ts);
	if (G_UNLIKELY (res != KERN_SUCCESS && res != KERN_ABORTED && res != KERN_OPERATION_TIMED_OUT))
		g_error ("%s: semaphore_timedwait failed with error %d", __func__, res);

	if (res == KERN_ABORTED && !(flags & MONO_SEM_FLAGS_ALERTABLE)) {
		ts = copy;

		resint = gettimeofday (&current, NULL);
		if (G_UNLIKELY (resint != 0))
			g_error ("%s: gettimeofday failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);

		ts.tv_sec -= (current.tv_sec - start.tv_sec);
		ts.tv_nsec -= (current.tv_usec - start.tv_usec) * 1000;
		if (ts.tv_nsec < 0) {
			if (ts.tv_sec <= 0) {
				ts.tv_nsec = 0;
			} else {
				ts.tv_sec--;
				ts.tv_nsec += NSEC_PER_SEC;
			}
		}
		if (ts.tv_sec < 0) {
			ts.tv_sec = 0;
			ts.tv_nsec = 0;
		}

		goto retry;
	}

	switch (res) {
	case KERN_SUCCESS:
		return MONO_SEM_TIMEDWAIT_RET_SUCCESS;
	case KERN_ABORTED:
		return MONO_SEM_TIMEDWAIT_RET_ALERTED;
	case KERN_OPERATION_TIMED_OUT:
		return MONO_SEM_TIMEDWAIT_RET_TIMEDOUT;
	default:
		g_assert_not_reached ();
	}
}

void
mono_os_sem_post (MonoSemType *sem)
{
	kern_return_t res;

retry:
	res = semaphore_signal (*sem);
	if (G_UNLIKELY (res != KERN_SUCCESS && res != KERN_ABORTED))
		g_error ("%s: semaphore_signal failed with error %d", __func__, res);

	if (res == KERN_ABORTED)
		goto retry;
}

#elif !defined(HOST_WIN32) && defined(HAVE_SEMAPHORE_H)

void
mono_os_sem_init (MonoSemType *sem, gint value)
{
	gint res;

	res = sem_init (sem, 0, value);
	if (G_UNLIKELY (res != 0))
		g_error ("%s: sem_init failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);
}

void
mono_os_sem_destroy (MonoSemType *sem)
{
	gint res;

	res = sem_destroy (sem);
	if (G_UNLIKELY (res != 0))
		g_error ("%s: sem_destroy failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);
}

gint
mono_os_sem_wait (MonoSemType *sem, MonoSemFlags flags)
{
	gint res;

retry:
	res = sem_wait (sem);
	if (G_UNLIKELY (res != 0 && errno != EINTR))
		g_error ("%s: sem_wait failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);

	if (res != 0 && errno == EINTR && !(flags & MONO_SEM_FLAGS_ALERTABLE))
		goto retry;

	return res != 0 ? -1 : 0;
}

MonoSemTimedwaitRet
mono_os_sem_timedwait (MonoSemType *sem, guint32 timeout_ms, MonoSemFlags flags)
{
	struct timespec ts, copy;
	struct timeval t;
	gint res;

	if (timeout_ms == 0) {
		res = sem_trywait (sem);
		if (G_UNLIKELY (res != 0 && errno != EINTR && errno != EAGAIN))
			g_error ("%s: sem_trywait failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);

		if (res == 0)
			return MONO_SEM_TIMEDWAIT_RET_SUCCESS;
		else if (errno == EINTR)
			return MONO_SEM_TIMEDWAIT_RET_ALERTED;
		else if (errno == EAGAIN)
			return MONO_SEM_TIMEDWAIT_RET_TIMEDOUT;
		else
			g_assert_not_reached ();
	}

	if (timeout_ms == MONO_INFINITE_WAIT)
		return (MonoSemTimedwaitRet) mono_os_sem_wait (sem, flags);

	res = gettimeofday (&t, NULL);
	if (G_UNLIKELY (res != 0))
		g_error ("%s: gettimeofday failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);

	ts.tv_sec = timeout_ms / 1000 + t.tv_sec;
	ts.tv_nsec = (timeout_ms % 1000) * 1000000 + t.tv_usec * 1000;
	while (ts.tv_nsec >= NSEC_PER_SEC) {
		ts.tv_nsec -= NSEC_PER_SEC;
		ts.tv_sec++;
	}

	copy = ts;

retry:
	res = sem_timedwait (sem, &ts);
	if (G_UNLIKELY (res != 0 && errno != EINTR && errno != ETIMEDOUT))
		g_error ("%s: sem_timedwait failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);

	if (res != 0 && errno == EINTR && !(flags & MONO_SEM_FLAGS_ALERTABLE)) {
		ts = copy;
		goto retry;
	}

	if (res == 0)
		return MONO_SEM_TIMEDWAIT_RET_SUCCESS;
	else if (errno == EINTR)
		return MONO_SEM_TIMEDWAIT_RET_ALERTED;
	else if (errno == ETIMEDOUT)
		return MONO_SEM_TIMEDWAIT_RET_TIMEDOUT;
	else
		g_assert_not_reached ();
}

void
mono_os_sem_post (MonoSemType *sem)
{
	gint res;

	res = sem_post (sem);
	if (G_UNLIKELY (res != 0))
		g_error ("%s: sem_post failed with \"%s\" (%d)", __func__, g_strerror (errno), errno);
}

#else

void
mono_os_sem_init (MonoSemType *sem, gint value)
{
	*sem = CreateSemaphore (NULL, value, 0x7FFFFFFF, NULL);
	if (G_UNLIKELY (*sem == NULL))
		g_error ("%s: CreateSemaphore failed with error %d", __func__, GetLastError ());
}

void
mono_os_sem_destroy (MonoSemType *sem)
{
	BOOL res;

	res = CloseHandle (*sem);
	if (G_UNLIKELY (res == 0))
		g_error ("%s: CloseHandle failed with error %d", __func__, GetLastError ());
}

MonoSemTimedwaitRet
mono_os_sem_timedwait (MonoSemType *sem, guint32 timeout_ms, MonoSemFlags flags)
{
	BOOL res;

retry:
	res = WaitForSingleObjectEx (*sem, timeout_ms, flags & MONO_SEM_FLAGS_ALERTABLE);
	if (G_UNLIKELY (res != WAIT_OBJECT_0 && res != WAIT_IO_COMPLETION && res != WAIT_TIMEOUT))
		g_error ("%s: WaitForSingleObjectEx failed with error %d", __func__, GetLastError ());

	if (res == WAIT_IO_COMPLETION && !(flags & MONO_SEM_FLAGS_ALERTABLE))
		goto retry;

	switch (res) {
	case WAIT_OBJECT_0:
		return MONO_SEM_TIMEDWAIT_RET_SUCCESS;
	case WAIT_IO_COMPLETION:
		return MONO_SEM_TIMEDWAIT_RET_ALERTED;
	case WAIT_TIMEOUT:
		return MONO_SEM_TIMEDWAIT_RET_TIMEDOUT;
	default:
		g_assert_not_reached ();
	}
}

gint
mono_os_sem_wait (MonoSemType *sem, MonoSemFlags flags)
{
	return mono_os_sem_timedwait (sem, INFINITE, flags) != 0 ? -1 : 0;
}

void
mono_os_sem_post (MonoSemType *sem)
{
	BOOL res;

	res = ReleaseSemaphore (*sem, 1, NULL);
	if (G_UNLIKELY (res == 0))
		g_error ("%s: ReleaseSemaphore failed with error %d", __func__, GetLastError ());
}

#endif
