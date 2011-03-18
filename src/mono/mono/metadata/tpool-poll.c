/*
 * tpool-poll.c: poll related stuff
 *
 * Authors:
 *   Dietmar Maurer (dietmar@ximian.com)
 *   Gonzalo Paniagua Javier (gonzalo@ximian.com)
 *
 * Copyright 2001-2003 Ximian, Inc (http://www.ximian.com)
 * Copyright 2004-2011 Novell, Inc (http://www.novell.com)
 */

#define INIT_POLLFD(a, b, c) {(a)->fd = b; (a)->events = c; (a)->revents = 0;}
struct _tp_poll_data {
	int pipe [2];
	MonoSemType new_sem;
	mono_pollfd newpfd;
};

typedef struct _tp_poll_data tp_poll_data;

static void tp_poll_shutdown (gpointer event_data);
static void tp_poll_modify (gpointer event_data, int fd, int operation, int events, gboolean is_new);
static void tp_poll_wait (gpointer p);

#ifdef HOST_WIN32
static void
connect_hack (gpointer x)
{
	struct sockaddr_in *addr = (struct sockaddr_in *) x;
	tp_poll_data *data = socket_io_data.event_data;
	int count = 0;

	while (connect ((SOCKET) data->pipe [1], (SOCKADDR *) addr, sizeof (struct sockaddr_in))) {
		Sleep (500);
		if (++count > 3) {
			g_warning ("Error initializing async. sockets %d.", WSAGetLastError ());
			g_assert (WSAGetLastError ());
		}
	}
}
#endif

#ifdef HOST_WIN32
#define PIPE(a) _pipe ((a), 256, O_BINARY)
#else
#define PIPE(a) pipe (a)
#endif
static gpointer
tp_poll_init (SocketIOData *data)
{
	tp_poll_data *result;

	result = g_new0 (tp_poll_data, 1);
	if (PIPE (result->pipe) != 0) {
		int err = errno;
		perror ("mono");
		g_assert (err);
	}
	MONO_SEM_INIT (&result->new_sem, 1);
	data->shutdown = tp_poll_shutdown;
	data->modify = tp_poll_modify;
	data->wait = tp_poll_wait;
	return result;
}

static void
tp_poll_modify (gpointer event_data, int fd, int operation, int events, gboolean is_new)
{
	tp_poll_data *data = event_data;
	char msg [1];
	int w;

	MONO_SEM_WAIT (&data->new_sem);
	INIT_POLLFD (&data->newpfd, GPOINTER_TO_INT (fd), events);
	*msg = (char) operation;
#ifndef HOST_WIN32
	w = write (data->pipe [1], msg, 1);
#else
	send ((SOCKET) data->pipe [1], msg, 1, 0);
#endif
}

static void
tp_poll_shutdown (gpointer event_data)
{
	tp_poll_data *data = event_data;

#ifdef HOST_WIN32
	closesocket (data->pipe [0]);
	closesocket (data->pipe [1]);
#else
	if (data->pipe [0] > -1)
		close (data->pipe [0]);
	if (data->pipe [1] > -1)
		close (data->pipe [1]);
#endif
	data->pipe [0] = -1;
	data->pipe [1] = -1;
	MONO_SEM_DESTROY (&data->new_sem);
}

static int
mark_bad_fds (mono_pollfd *pfds, int nfds)
{
	int i, ret;
	mono_pollfd *pfd;
	int count = 0;

	for (i = 0; i < nfds; i++) {
		pfd = &pfds [i];
		if (pfd->fd == -1)
			continue;

		ret = mono_poll (pfd, 1, 0);
		if (ret == -1 && errno == EBADF) {
			pfd->revents |= MONO_POLLNVAL;
			count++;
		} else if (ret == 1) {
			count++;
		}
	}

	return count;
}

static void
tp_poll_wait (gpointer p)
{
#if MONO_SMALL_CONFIG
#define INITIAL_POLLFD_SIZE	128
#else
#define INITIAL_POLLFD_SIZE	1024
#endif
#define POLL_ERRORS (MONO_POLLERR | MONO_POLLHUP | MONO_POLLNVAL)
	mono_pollfd *pfds;
	gint maxfd = 1;
	gint allocated;
	gint i;
	MonoInternalThread *thread;
	tp_poll_data *data;
	SocketIOData *socket_io_data = p;
	gpointer *async_results;
	gint nresults;

	thread = mono_thread_internal_current ();

	data = socket_io_data->event_data;
	allocated = INITIAL_POLLFD_SIZE;
	pfds = g_new0 (mono_pollfd, allocated);
	async_results = g_new0 (gpointer, allocated * 2);
	INIT_POLLFD (pfds, data->pipe [0], MONO_POLLIN);
	for (i = 1; i < allocated; i++)
		INIT_POLLFD (&pfds [i], -1, 0);

	while (1) {
		int nsock = 0;
		mono_pollfd *pfd;
		char one [1];
		MonoMList *list;
		MonoObject *ares;

		do {
			if (nsock == -1) {
				if (THREAD_WANTS_A_BREAK (thread))
					mono_thread_interruption_checkpoint ();
			}

			nsock = mono_poll (pfds, maxfd, -1);
		} while (nsock == -1 && errno == EINTR);

		/* 
		 * Apart from EINTR, we only check EBADF, for the rest:
		 *  EINVAL: mono_poll() 'protects' us from descriptor
		 *      numbers above the limit if using select() by marking
		 *      then as MONO_POLLERR.  If a system poll() is being
		 *      used, the number of descriptor we're passing will not
		 *      be over sysconf(_SC_OPEN_MAX), as the error would have
		 *      happened when opening.
		 *
		 *  EFAULT: we own the memory pointed by pfds.
		 *  ENOMEM: we're doomed anyway
		 *
		 */

		if (nsock == -1 && errno == EBADF) {
			pfds->revents = 0; /* Just in case... */
			nsock = mark_bad_fds (pfds, maxfd);
		}

		if ((pfds->revents & POLL_ERRORS) != 0) {
			/* We're supposed to die now, as the pipe has been closed */
			g_free (pfds);
			g_free (async_results);
			socket_io_cleanup (socket_io_data);
			return;
		}

		/* Got a new socket */
		if ((pfds->revents & MONO_POLLIN) != 0) {
			int nread;

			for (i = 1; i < allocated; i++) {
				pfd = &pfds [i];
				if (pfd->fd == -1 || pfd->fd == data->newpfd.fd)
					break;
			}

			if (i == allocated) {
				mono_pollfd *oldfd;

				oldfd = pfds;
				i = allocated;
				allocated = allocated * 2;
				pfds = g_renew (mono_pollfd, oldfd, allocated);
				g_free (oldfd);
				for (; i < allocated; i++)
					INIT_POLLFD (&pfds [i], -1, 0);
				async_results = g_renew (gpointer, async_results, allocated * 2);
			}
#ifndef HOST_WIN32
			nread = read (data->pipe [0], one, 1);
#else
			nread = recv ((SOCKET) data->pipe [0], one, 1, 0);
#endif
			if (nread <= 0) {
				g_free (pfds);
				g_free (async_results);
				return; /* we're closed */
			}

			INIT_POLLFD (&pfds [i], data->newpfd.fd, data->newpfd.events);
			memset (&data->newpfd, 0, sizeof (mono_pollfd));
			MONO_SEM_POST (&data->new_sem);
			if (i >= maxfd)
				maxfd = i + 1;
			nsock--;
		}

		if (nsock == 0)
			continue;

		EnterCriticalSection (&socket_io_data->io_lock);
		if (socket_io_data->inited == 3) {
			g_free (pfds);
			g_free (async_results);
			LeaveCriticalSection (&socket_io_data->io_lock);
			return; /* cleanup called */
		}

		nresults = 0;
		for (i = 1; i < maxfd && nsock > 0; i++) {
			pfd = &pfds [i];
			if (pfd->fd == -1 || pfd->revents == 0)
				continue;

			nsock--;
			list = mono_g_hash_table_lookup (socket_io_data->sock_to_state, GINT_TO_POINTER (pfd->fd));
			if (list != NULL && (pfd->revents & (MONO_POLLIN | POLL_ERRORS)) != 0) {
				ares = get_io_event (&list, MONO_POLLIN);
				if (ares != NULL)
					async_results [nresults++] = ares;
			}

			if (list != NULL && (pfd->revents & (MONO_POLLOUT | POLL_ERRORS)) != 0) {
				ares = get_io_event (&list, MONO_POLLOUT);
				if (ares != NULL)
					async_results [nresults++] = ares;
			}

			if (list != NULL) {
				mono_g_hash_table_replace (socket_io_data->sock_to_state, GINT_TO_POINTER (pfd->fd), list);
				pfd->events = get_events_from_list (list);
			} else {
				mono_g_hash_table_remove (socket_io_data->sock_to_state, GINT_TO_POINTER (pfd->fd));
				pfd->fd = -1;
				if (i == maxfd - 1)
					maxfd--;
			}
		}
		LeaveCriticalSection (&socket_io_data->io_lock);
		threadpool_append_jobs (&async_io_tp, (MonoObject **) async_results, nresults);
		memset (async_results, 0, sizeof (gpointer) * nresults);
	}
}

