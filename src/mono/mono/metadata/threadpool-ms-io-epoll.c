
#if defined(HAVE_EPOLL)

#include <sys/epoll.h>

#if defined(HOST_WIN32)
/* We assume that epoll is not available on windows */
#error
#endif

#define EPOLL_NEVENTS 128

static gint epoll_fd;
static struct epoll_event *epoll_events;

static gboolean
epoll_init (gint wakeup_pipe_fd)
{
#ifdef EPOOL_CLOEXEC
	epoll_fd = epoll_create1 (EPOLL_CLOEXEC);
#else
	epoll_fd = epoll_create (256);
	fcntl (epoll_fd, F_SETFD, FD_CLOEXEC);
#endif

	if (epoll_fd == -1) {
#ifdef EPOOL_CLOEXEC
		g_warning ("epoll_init: epoll (EPOLL_CLOEXEC) failed, error (%d) %s\n", errno, g_strerror (errno));
#else
		g_warning ("epoll_init: epoll (256) failed, error (%d) %s\n", errno, g_strerror (errno));
#endif
		return FALSE;
	}

	if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, wakeup_pipe_fd, EPOLLIN) == -1) {
		g_warning ("epoll_init: epoll_ctl () failed, error (%d) %s", errno, g_strerror (errno));
		close (epoll_fd);
		return FALSE;
	}

	epoll_events = g_new0 (struct epoll_event, EPOLL_NEVENTS);

	return TRUE;
}

static void
epoll_cleanup (void)
{
	g_free (epoll_events);
	close (epoll_fd);
}

static void
epoll_update_add (ThreadPoolIOUpdate *update)
{
	struct epoll_event event;

	event.data.fd = update->fd;
	if ((update->events & MONO_POLLIN) != 0)
		event.events |= EPOLLIN;
	if ((update->events & MONO_POLLOUT) != 0)
		event.events |= EPOLLOUT;

	if (epoll_ctl (epoll_fd, update->is_new ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, event.data.fd, &event) == -1)
		g_warning ("epoll_update_add: epoll_ctl(%s) failed, error (%d) %s", update->is_new ? "EPOLL_CTL_ADD" : "EPOLL_CTL_MOD", errno, g_strerror (errno));
}

static gint
epoll_event_wait (void)
{
	gint ready;

	ready = epoll_wait (epoll_fd, epoll_events, EPOLL_NEVENTS, -1);
	if (ready == -1) {
		switch (errno) {
		case EINTR:
			check_for_interruption_critical ();
			ready = 0;
			break;
		default:
			g_warning ("epoll_event_wait: epoll_wait () failed, error (%d) %s", errno, g_strerror (errno));
			break;
		}
	}

	return ready;
}

static gint
epoll_event_max (void)
{
	return EPOLL_NEVENTS;
}

static gint
epoll_event_fd_at (guint i)
{
	return epoll_events [i].data.fd;
}

static gboolean
epoll_event_create_sockares_at (guint i, gint fd, MonoMList **list)
{
	struct epoll_event *epoll_event;

	g_assert (list);

	epoll_event = &epoll_events [i];
	g_assert (epoll_event);

	g_assert (fd == epoll_event->data.fd);

	if (*list && (epoll_event->events & (EPOLLIN | EPOLLERR | EPOLLHUP)) != 0) {
		MonoSocketAsyncResult *io_event = get_sockares_for_event (list, MONO_POLLIN);
		if (io_event)
			mono_threadpool_ms_enqueue_work_item (((MonoObject*) io_event)->vtable->domain, (MonoObject*) io_event);
	}
	if (*list && (epoll_event->events & (EPOLLOUT | EPOLLERR | EPOLLHUP)) != 0) {
		MonoSocketAsyncResult *io_event = get_sockares_for_event (list, MONO_POLLOUT);
		if (io_event)
			mono_threadpool_ms_enqueue_work_item (((MonoObject*) io_event)->vtable->domain, (MonoObject*) io_event);
	}

	if (*list) {
		gint events = get_events (*list);

		epoll_event->events = ((events & MONO_POLLOUT) ? EPOLLOUT : 0) | ((events & MONO_POLLIN) ? EPOLLIN : 0);
		if (epoll_ctl (epoll_fd, EPOLL_CTL_MOD, fd, epoll_event) == -1) {
			if (epoll_ctl (epoll_fd, EPOLL_CTL_ADD, fd, epoll_event) == -1)
				g_warning ("epoll_event_create_sockares_at: epoll_ctl () failed, error (%d) %s", errno, g_strerror (errno));
		}
	} else {
		epoll_ctl (epoll_fd, EPOLL_CTL_DEL, fd, epoll_event);
	}

	return TRUE;
}

static ThreadPoolIOBackend backend_epoll = {
	.init = epoll_init,
	.cleanup = epoll_cleanup,
	.update_add = epoll_update_add,
	.event_wait = epoll_event_wait,
	.event_max = epoll_event_max,
	.event_fd_at = epoll_event_fd_at,
	.event_create_sockares_at = epoll_event_create_sockares_at,
};

#endif
