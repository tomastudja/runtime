/*
 * daemon.c:  The handle daemon
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *
 * (C) 2002 Ximian, Inc.
 */

#include <config.h>
#include <glib.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>

#include <mono/io-layer/io-layer.h>
#include <mono/io-layer/handles-private.h>
#include <mono/io-layer/wapi-private.h>
#include <mono/io-layer/daemon-messages.h>
#include <mono/io-layer/timefuncs-private.h>
#include <mono/io-layer/daemon-private.h>

#undef DEBUG

/* The shared thread codepath doesn't seem to work yet... */
#undef _POSIX_THREAD_PROCESS_SHARED

/* Keep track of the number of clients */
static int nfds=0;
/* Arrays to keep track of handles that have been referenced by the
 * daemon and clients.  client_handles is indexed by file descriptor
 * value.
 */
static guint32 *daemon_handles=NULL, **client_handles=NULL;
static int client_handles_length=0;

/* The socket which we listen to new connections on */
static int main_sock;

/* Set to TRUE by the SIGCHLD signal handler */
static volatile gboolean check_processes=FALSE;

static gboolean fd_activity (GIOChannel *channel, GIOCondition condition,
			     gpointer data);


/* Deletes the shared memory segment.  If we're exiting on error,
 * clients will get EPIPEs.
 */
static void cleanup (void)
{
	int i;
	
#ifdef NEED_LINK_UNLINK
        unlink(_wapi_shared_data[0]->daemon);
#endif 
	for(i=1; i<_wapi_shared_data[0]->num_segments; i++) {
		unlink (_wapi_shm_file (WAPI_SHM_DATA, i));
	}
	unlink (_wapi_shm_file (WAPI_SHM_DATA, 0));
	
	/* There's only one scratch file */
	unlink (_wapi_shm_file (WAPI_SHM_SCRATCH, 0));
}

/* If there is only one socket, and no child processes, we can exit.
 * We test for child processes by counting handle references held by
 * the daemon.
 */
static void maybe_exit (void)
{
	guint32 i;

#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": Seeing if we should exit");
#endif

	if(nfds>1) {
#ifdef DEBUG
		g_message (G_GNUC_PRETTY_FUNCTION ": Still got clients");
#endif
		return;
	}

	for(i=0;
	    i<_wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT;
	    i++) {
		if(daemon_handles[i]>0) {
#ifdef DEBUG
			g_message (G_GNUC_PRETTY_FUNCTION
				   ": Still got handle references");
#endif
			return;
		}
	}
	
#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": Byebye");
#endif
	
	cleanup ();
	exit (0);
}

/*
 * signal_handler:
 * @unused: unused
 *
 * Called if daemon receives a SIGTERM or SIGINT
 */
static void signal_handler (int unused)
{
	cleanup ();
	exit (-1);
}

/*
 * sigchld_handler:
 * @unused: unused
 *
 * Called if daemon receives a SIGCHLD, and notes that a process needs
 * to be wait()ed for.
 */
static void sigchld_handler (int unused)
{
	/* Notice that a child process died */
	check_processes=TRUE;
}

/*
 * startup:
 *
 * Bind signals and attach to shared memory
 */
static void startup (void)
{
	struct sigaction sa;
	
	sa.sa_handler=signal_handler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags=0;
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGTERM, &sa, NULL);
	
#ifndef HAVE_MSG_NOSIGNAL
	sa.sa_handler=SIG_IGN;
	sigaction (SIGPIPE, &sa, NULL);
#endif

	sa.sa_handler=sigchld_handler;
	sa.sa_flags=SA_NOCLDSTOP;
	sigaction (SIGCHLD, &sa, NULL);
	
#ifdef NEED_LINK_UNLINK
	/* Here's a more portable method... */
	snprintf (_wapi_shared_data[0]->daemon, MONO_SIZEOF_SUNPATH-1,
		  "/tmp/mono-handle-daemon-%d-%ld-%ld", getuid (), random (),
		  time (NULL));
#else
	/* Leave the first byte NULL so we create the socket in the
	 * abstrace namespace, not on the filesystem.  (Lets see how
	 * portable _that_ is :)
	 *
	 * The name is intended to be unique, not cryptographically
	 * secure...
	 */
	snprintf (_wapi_shared_data[0]->daemon+1, MONO_SIZEOF_SUNPATH-2,
		  "mono-handle-daemon-%d-%d-%ld", getuid (), getpid (),
		  time (NULL));
#endif
}


/*
 * ref_handle:
 * @open_handles: An array of handles referenced by the calling client
 * @handle: handle to inc refcnt
 *
 * Increase ref count of handle for the calling client.  Handle 0 is
 * ignored.
 */
static void ref_handle (guint32 *open_handles, guint32 handle)
{
	guint32 segment, idx;
	
	if(handle==0) {
		return;
	}
	
	_wapi_handle_segment (GUINT_TO_POINTER (handle), &segment, &idx);
	
	_wapi_shared_data[segment]->handles[idx].ref++;
	open_handles[handle]++;
	
#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION
		   ": handle 0x%x ref now %d (%d this process)", handle,
		   _wapi_shared_data[segment]->handles[idx].ref,
		   open_handles[handle]);
#endif
}

/*
 * unref_handle:
 * @open_handles: An array of handles referenced by the calling client
 * @handle: handle to inc refcnt
 *
 * Decrease ref count of handle for the calling client. If global ref
 * count reaches 0 it is free'ed. Return TRUE if the local ref count
 * is 0. Handle 0 is ignored.
 */
static gboolean unref_handle (guint32 *open_handles, guint32 handle)
{
	gboolean destroy=FALSE;
	guint32 segment, idx;
	
	if(handle==0) {
		return(FALSE);
	}
	
	if (open_handles[handle] == 0) {
                g_warning(G_GNUC_PRETTY_FUNCTION
                          ": unref on %d called when ref was already 0", 
                          handle);
                return TRUE;
        }

	_wapi_handle_segment (GUINT_TO_POINTER (handle), &segment, &idx);
	
	_wapi_shared_data[segment]->handles[idx].ref--;
	open_handles[handle]--;
	
#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION
		   ": handle 0x%x ref now %d (%d this process)", handle,
		   _wapi_shared_data[segment]->handles[idx].ref,
		   open_handles[handle]);
#endif

	if(open_handles[handle]==0) {
		/* This client has released the handle */
		destroy=TRUE;
	}
	
	if(_wapi_shared_data[segment]->handles[idx].ref==0) {
		if (open_handles[handle]!=0) {
			g_warning (G_GNUC_PRETTY_FUNCTION ": per-process open_handles mismatch, set to %d, should be 0", open_handles[handle]);
		}
		
#ifdef DEBUG
		g_message (G_GNUC_PRETTY_FUNCTION ": Destroying handle 0x%x",
			   handle);
#endif
		
		_wapi_handle_ops_close_shared (GUINT_TO_POINTER (handle));
		
#if defined(_POSIX_THREAD_PROCESS_SHARED) && _POSIX_THREAD_PROCESS_SHARED != -1
		mono_mutex_destroy (&_wapi_shared_data[segment]->handles[idx].signal_mutex);
		pthread_cond_destroy (&_wapi_shared_data[segment]->handles[idx].signal_cond);
#endif

		memset (&_wapi_shared_data[segment]->handles[idx].u, '\0', sizeof(_wapi_shared_data[segment]->handles[idx].u));
		_wapi_shared_data[segment]->handles[idx].type=WAPI_HANDLE_UNUSED;
	}

	if(open_handles==daemon_handles) {
		/* The daemon released a reference, so see if it's
		 * ready to exit
		 */
		maybe_exit ();
	}
	
	return(destroy);
}

/*
 * add_fd:
 * @fd: Filehandle to add
 *
 * Create a new GIOChannel, and add it to the main loop event sources.
 */
static void add_fd(int fd)
{
	GIOChannel *io_channel;
	guint32 *refs;
	
	io_channel=g_io_channel_unix_new (fd);
	
	/* Turn off all encoding and buffering crap */
	g_io_channel_set_encoding (io_channel, NULL, NULL);
	g_io_channel_set_buffered (io_channel, FALSE);
	
	refs=g_new0 (guint32,_wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT);
	if(daemon_handles==NULL) {
		/* We rely on the daemon channel being created first.
		 * That's safe, because every other channel is the
		 * result of an accept() on the daemon channel.
		 */
		daemon_handles=refs;
	}

	if(fd>=client_handles_length) {
		/* Add a bit of padding, so we dont resize for _every_
		 * new connection
		 */
		int old_len=client_handles_length * sizeof(guint32 *);
		
		client_handles_length=fd+10;
		if(client_handles==NULL) {
			client_handles=g_new0 (guint32 *,
					       client_handles_length);
		} else {
			/* Can't use g_renew here, because the unused
			 * elements must be NULL and g_renew doesn't
			 * initialise the memory it returns
			 */
			client_handles=_wapi_g_renew0 (client_handles, old_len, client_handles_length * sizeof(guint32 *));
		}
	}
	client_handles[fd]=refs;
	
	g_io_add_watch (io_channel, G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL,
			fd_activity, NULL);

	nfds++;
}

/*
 * rem_fd:
 * @channel: GIOChannel to close
 *
 * Closes the IO channel. Closes all handles that it may have open. If
 * only main_sock is left, the daemon is shut down.
 */
static void rem_fd(GIOChannel *channel, guint32 *open_handles)
{
	guint32 handle_count;
	int i, j, fd;
	
	fd=g_io_channel_unix_get_fd (channel);
	
	if(fd == main_sock) {
		/* We shouldn't be deleting the daemon's fd */
		g_warning (G_GNUC_PRETTY_FUNCTION ": Deleting daemon fd!");
		cleanup ();
		exit (-1);
	}
	
#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": Removing client fd %d", fd);
#endif

	g_io_channel_shutdown (channel, TRUE, NULL);

	for(i=0;
	    i<_wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT;
	    i++) {
		handle_count=open_handles[i];
		
		for(j=0; j<handle_count; j++) {
#ifdef DEBUG
			g_message (G_GNUC_PRETTY_FUNCTION ": closing handle 0x%x for client at index %d", i, g_io_channel_unix_get_fd (channel));
#endif
			/* Ignore the hint to the client to destroy
			 * the handle private data
			 */
			unref_handle (open_handles, i);
		}
	}
	
	g_free (open_handles);
	client_handles[fd]=NULL;
	
	nfds--;
	if(nfds==1) {
		/* Just the master socket left, so see if we can
		 * cleanup and exit
		 */
		maybe_exit ();
	}
}

static gboolean process_compare (gpointer handle, gpointer user_data)
{
	struct _WapiHandle_process *process_handle;
	gboolean ok;
	pid_t pid;
	
	ok=_wapi_lookup_handle (handle, WAPI_HANDLE_PROCESS,
				(gpointer *)&process_handle, NULL);
	if(ok==FALSE) {
		g_warning (G_GNUC_PRETTY_FUNCTION
			   ": error looking up process handle %p", handle);
		return(FALSE);
	}

	pid=GPOINTER_TO_UINT (user_data);
	if(process_handle->id==pid) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

static gboolean process_thread_compare (gpointer handle, gpointer user_data)
{
	struct _WapiHandle_thread *thread_handle;
	gboolean ok;
	guint32 segment, idx;
	
	ok=_wapi_lookup_handle (handle, WAPI_HANDLE_THREAD,
				(gpointer *)&thread_handle, NULL);
	if(ok==FALSE) {
		g_warning (G_GNUC_PRETTY_FUNCTION
			   ": error looking up thread handle %p", handle);
		return(FALSE);
	}

	if(thread_handle->process_handle==user_data) {
		/* Signal the handle.  Don't use
		 * _wapi_handle_set_signal_state() unless we have
		 * process-shared pthread support.
		 */
#ifdef DEBUG
		g_message (G_GNUC_PRETTY_FUNCTION ": Set thread handle %p signalled, because its process died", handle);
#endif

		thread_handle->exitstatus=0;

#if defined(_POSIX_THREAD_PROCESS_SHARED) && _POSIX_THREAD_PROCESS_SHARED != -1
		_wapi_handle_lock_handle (handle);
		_wapi_handle_set_signal_state (handle, TRUE, TRUE);
		_wapi_handle_unlock_handle (handle);
#else
		/* Just tweak the signal state directly.  This is not
		 * recommended behaviour, but it works for threads
		 * because they can never become unsignalled.  There
		 * are some nasty kludges in the handle waiting code
		 * to cope with missing condition signals for when
		 * process-shared pthread support is missing.
		 */
		_wapi_handle_segment (handle, &segment, &idx);
		_wapi_shared_data[segment]->handles[idx].signalled=TRUE;
#endif /* _POSIX_THREAD_PROCESS_SHARED */
	}
	
	/* Return false to keep searching */
	return(FALSE);
}

/* Find the handle associated with pid, mark it dead and record exit
 * status.  Finds all thread handles associated with this process
 * handle, and marks those signalled too, with exitstatus '0'.  It
 * also drops the daemon's reference to the handle, and the thread
 * pointed at by main_thread.
 */
static void process_post_mortem (pid_t pid, int status)
{
	gpointer process_handle;
	struct _WapiHandle_process *process_handle_data;
	guint32 segment, idx;
	
	process_handle=_wapi_search_handle (WAPI_HANDLE_PROCESS,
					    process_compare,
					    GUINT_TO_POINTER (pid),
					    (gpointer *)&process_handle_data,
					    NULL);
	if(process_handle==0) {
		g_warning (G_GNUC_PRETTY_FUNCTION
			   ": Couldn't find handle for process %d!", pid);
	} else {
		/* Signal the handle.  Don't use
		 * _wapi_handle_set_signal_state() unless we have
		 * process-shared pthread support.
		 */
		struct timeval tv;
		
#ifdef DEBUG
		g_message (G_GNUC_PRETTY_FUNCTION
			   ": Set process %d exitstatus to %d", pid,
			   WEXITSTATUS (status));
#endif
		
		/* Technically WEXITSTATUS is only valid if the
		 * process exited normally, but I don't care if the
		 * process caught a signal or not.
		 */
		process_handle_data->exitstatus=WEXITSTATUS (status);

		/* Ignore errors */
		gettimeofday (&tv, NULL);
		_wapi_timeval_to_filetime (&tv,
					   &process_handle_data->exit_time);

#if defined(_POSIX_THREAD_PROCESS_SHARED) && _POSIX_THREAD_PROCESS_SHARED != -1
		_wapi_handle_lock_handle (process_handle);
		_wapi_handle_set_signal_state (process_handle, TRUE, TRUE);
		_wapi_handle_unlock_handle (process_handle);
#else
		/* Just tweak the signal state directly.  This is not
		 * recommended behaviour, but it works for processes
		 * because they can never become unsignalled.  There
		 * are some nasty kludges in the handle waiting code
		 * to cope with missing condition signals for when
		 * process-shared pthread support is missing.
		 */
		_wapi_handle_segment (process_handle, &segment, &idx);
		_wapi_shared_data[segment]->handles[idx].signalled=TRUE;
#endif /* _POSIX_THREAD_PROCESS_SHARED */
	}

	/* Find all threads that have their process
	 * handle==process_handle.  Ignore the return value, all the
	 * work will be done in the compare func
	 */
	(void)_wapi_search_handle (WAPI_HANDLE_THREAD, process_thread_compare,
				   process_handle, NULL, NULL);

	unref_handle (daemon_handles,
		      GPOINTER_TO_UINT (process_handle_data->main_thread));
	unref_handle (daemon_handles, GPOINTER_TO_UINT (process_handle));
}

static void process_died (void)
{
	int status;
	pid_t pid;
	
	check_processes=FALSE;

#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": Reaping processes");
#endif

	while(TRUE) {
		pid=waitpid (-1, &status, WNOHANG);
		if(pid==0 || pid==-1) {
			/* Finished waiting.  I was checking pid==-1
			 * separately but was getting ECHILD when
			 * there were no more child processes (which
			 * doesnt seem to conform to the man page)
			 */
			return;
		} else {
			/* pid contains the ID of a dead process */
#ifdef DEBUG
			g_message (G_GNUC_PRETTY_FUNCTION ": process %d reaped", pid);
#endif
			process_post_mortem (pid, status);
		}
	}
}


/*
 * send_reply:
 * @channel: channel to send reply to
 * @resp: Package to send
 *
 * Send a package to a client
 */
static void send_reply (GIOChannel *channel, WapiHandleResponse *resp)
{
	/* send message */
	_wapi_daemon_response (g_io_channel_unix_get_fd (channel), resp);
}

/*
 * process_new:
 * @channel: The client making the request
 * @open_handles: An array of handles referenced by this client
 * @type: type to init handle to
 *
 * Find a free handle and initialize it to 'type', increase refcnt and
 * send back a reply to the client.
 */
static void process_new (GIOChannel *channel, guint32 *open_handles,
			 WapiHandleType type)
{
	guint32 handle;
	WapiHandleResponse resp={0};
	
	handle=_wapi_handle_new_internal (type);
	if(handle==0) {
		/* Try and allocate a new shared segment, and have
		 * another go
		 */
		guint32 segment=_wapi_shared_data[0]->num_segments;
		int i;
		
		_wapi_handle_ensure_mapped (segment);
		if(_wapi_shared_data[segment]!=NULL) {
			/* Got a new segment */
			gulong old_len, new_len;
			
			old_len=_wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT * sizeof(guint32);
			_wapi_shared_data[0]->num_segments++;
			new_len=_wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT * sizeof(guint32);

			/* Need to expand all the handle reference
			 * count arrays
			 */

			for(i=0; i<client_handles_length; i++) {
				if(client_handles[i]!=NULL) {
					client_handles[i]=_wapi_g_renew0 (client_handles[i], old_len, new_len);
				}
			}

			/* And find the new location for open_handles... */
			i=g_io_channel_unix_get_fd (channel);
			open_handles=client_handles[i];
			daemon_handles=client_handles[main_sock];
			
			handle=_wapi_handle_new_internal (type);
		} else {
			/* Map failed.  Just return 0 meaning "out of
			 * handles"
			 */
		}
	}
	
	/* handle might still be set to 0.  This is handled at the
	 * client end
	 */

	ref_handle (open_handles, handle);

#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": returning new handle 0x%x",
		   handle);
#endif

	resp.type=WapiHandleResponseType_New;
	resp.u.new.type=type;
	resp.u.new.handle=handle;
			
	send_reply (channel, &resp);
}

/*
 * process_open:
 * @channel: The client making the request
 * @open_handles: An array of handles referenced by this client
 * @handle: handle no.
 *
 * Increase refcnt on a previously created handle and send back a
 * response to the client.
 */
static void process_open (GIOChannel *channel, guint32 *open_handles,
			  guint32 handle)
{
	WapiHandleResponse resp={0};
	guint32 segment, idx;
	struct _WapiHandleShared *shared;

	_wapi_handle_segment (GUINT_TO_POINTER (handle), &segment, &idx);
	shared=&_wapi_shared_data[segment]->handles[idx];
		
	if(shared->type!=WAPI_HANDLE_UNUSED && handle!=0) {
		ref_handle (open_handles, handle);

#ifdef DEBUG
		g_message (G_GNUC_PRETTY_FUNCTION
			   ": returning new handle 0x%x", handle);
#endif

		resp.type=WapiHandleResponseType_Open;
		resp.u.new.type=shared->type;
		resp.u.new.handle=handle;
			
		send_reply (channel, &resp);

		return;
	}

	resp.type=WapiHandleResponseType_Open;
	resp.u.new.handle=0;
			
	send_reply (channel, &resp);
}

/*
 * process_close:
 * @channel: The client making the request
 * @open_handles: An array of handles referenced by this client
 * @handle: handle no.
 *
 * Decrease refcnt on a previously created handle and send back a
 * response to the client with notice of it being destroyed.
 */
static void process_close (GIOChannel *channel, guint32 *open_handles,
			   guint32 handle)
{
	WapiHandleResponse resp={0};
	
	resp.type=WapiHandleResponseType_Close;
	resp.u.close.destroy=unref_handle (open_handles, handle);

#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": unreffing handle 0x%x", handle);
#endif
			
	send_reply (channel, &resp);
}

/*
 * process_scratch:
 * @channel: The client making the request
 * @length: allocate this much scratch space
 *
 * Allocate some scratch space and send a reply to the client.
 */
static void process_scratch (GIOChannel *channel, guint32 length)
{
	WapiHandleResponse resp={0};
	
	resp.type=WapiHandleResponseType_Scratch;
	resp.u.scratch.idx=_wapi_handle_scratch_store_internal (length, &resp.u.scratch.remap);
#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": allocating scratch index 0x%x",
		   resp.u.scratch.idx);
#endif
			
	send_reply (channel, &resp);
}

/*
 * process_scratch_free:
 * @channel: The client making the request
 * @scratch_idx: deallocate this scratch space
 *
 * Deallocate scratch space and send a reply to the client.
 */
static void process_scratch_free (GIOChannel *channel, guint32 scratch_idx)
{
	WapiHandleResponse resp={0};
	
	resp.type=WapiHandleResponseType_ScratchFree;
	_wapi_handle_scratch_delete_internal (scratch_idx);

#ifdef DEBUG
	g_message (G_GNUC_PRETTY_FUNCTION ": deleting scratch index 0x%x",
		   scratch_idx);
#endif
			
	send_reply (channel, &resp);
}

/*
 * process_process_fork:
 * @channel: The client making the request
 * @open_handles: An array of handles referenced by this client
 * @process_fork: Describes the process to fork
 * @fds: stdin, stdout, and stderr for the new process
 *
 * Forks a new process, and returns the process and thread data to the
 * client.
 */
static void process_process_fork (GIOChannel *channel, guint32 *open_handles,
				  WapiHandleRequest_ProcessFork process_fork,
				  int *fds)
{
	WapiHandleResponse resp={0};
	guint32 process_handle, thread_handle;
	struct _WapiHandle_process *process_handle_data;
	struct _WapiHandle_thread *thread_handle_data;
	pid_t pid;
	
	resp.type=WapiHandleResponseType_ProcessFork;
	
	/* Create handles first, so the child process can store exec
	 * errors.  Either handle might be set to 0, if this happens
	 * just reply to the client without bothering to fork.  The
	 * client must check if either handle is 0 and take
	 * appropriate error handling action.
	 */
	process_handle=_wapi_handle_new_internal (WAPI_HANDLE_PROCESS);
	ref_handle (daemon_handles, process_handle);
	ref_handle (open_handles, process_handle);
	
	thread_handle=_wapi_handle_new_internal (WAPI_HANDLE_THREAD);
	ref_handle (daemon_handles, thread_handle);
	ref_handle (open_handles, thread_handle);
	
	if(process_handle==0 || thread_handle==0) {
		/* unref_handle() copes with the handle being 0 */
		unref_handle (daemon_handles, process_handle);
		unref_handle (open_handles, process_handle);
		unref_handle (daemon_handles, thread_handle);
		unref_handle (open_handles, thread_handle);
		process_handle=0;
		thread_handle=0;
	} else {
		char *cmd=NULL, *dir=NULL, **argv, **env;
		GError *gerr=NULL;
		gboolean ret;
		struct timeval tv;
		
		/* Get usable copies of the cmd, dir and env now
		 * rather than in the child process.  This is to
		 * prevent the race condition where the parent can
		 * return the reply to the client, which then promptly
		 * deletes the scratch data before the new process
		 * gets to see it.  Also explode argv here so we can
		 * use it to set the process name.
		 */
		cmd=_wapi_handle_scratch_lookup (process_fork.cmd);
		dir=_wapi_handle_scratch_lookup (process_fork.dir);
		env=_wapi_handle_scratch_lookup (process_fork.env);
		
		ret=g_shell_parse_argv (cmd, NULL, &argv, &gerr);
		if(ret==FALSE) {
			/* FIXME: Could do something with the
			 * GError here
			 */
			process_handle_data->exec_errno=gerr->code;
		} else {
#ifdef DEBUG
			g_message (G_GNUC_PRETTY_FUNCTION ": forking");
#endif

			_wapi_lookup_handle (GUINT_TO_POINTER (process_handle),
					     WAPI_HANDLE_PROCESS,
					     (gpointer *)&process_handle_data,
					     NULL);

			_wapi_lookup_handle (GUINT_TO_POINTER (thread_handle),
					     WAPI_HANDLE_THREAD,
					     (gpointer *)&thread_handle_data,
					     NULL);

			/* Fork, exec cmd with args and optional env,
			 * and return the handles with pid and blank
			 * thread id
			 */
			pid=fork ();
			if(pid==-1) {
				process_handle_data->exec_errno=errno;
			} else if (pid==0) {
				/* child */
				int i;
				
				/* should we detach from the process
				 * group? We're already running
				 * without a controlling tty...
				 */

				/* Connect stdin, stdout and stderr */
				dup2 (fds[0], 0);
				dup2 (fds[1], 1);
				dup2 (fds[2], 2);

				if(process_fork.inherit!=TRUE) {
					/* FIXME: do something here */
				}
				
				/* Close all file descriptors */
				for(i=3; i<getdtablesize (); i++) {
					close (i);
				}
			
#ifdef DEBUG
				g_message (G_GNUC_PRETTY_FUNCTION
					   ": exec()ing [%s] in dir [%s]",
					   cmd, dir);
				{
					i=0;
					while(argv[i]!=NULL) {
						g_message ("arg %d: [%s]",
							   i, argv[i]);
						i++;
					}

					i=0;
					while(env[i]!=NULL) {
						g_message ("env %d: [%s]",
							   i, env[i]);
						i++;
					}
				}
#endif
			
				/* set cwd */
				if(chdir (dir)==-1) {
					process_handle_data->exec_errno=errno;
					exit (-1);
				}
			
				/* pass process and thread handle info
				 * to the child, so it doesn't have to
				 * do an expensive search over the
				 * whole list
				 */
				{
					char *proc_env, *thr_env;
					
					proc_env=g_strdup_printf ("_WAPI_PROCESS_HANDLE=%d", process_handle);
					thr_env=g_strdup_printf ("_WAPI_THREAD_HANDLE=%d", process_handle);
					putenv (proc_env);
					putenv (thr_env);
				}
				
				/* exec */
				execve (argv[0], argv, env);
		
				/* bummer! */
				process_handle_data->exec_errno=errno;
				exit (-1);
			}
		}
		/* parent */

		/* store process name, based on the last section of the cmd */
		{
			char *slash=strrchr (argv[0], '/');
			
			if(slash!=NULL) {
				process_handle_data->proc_name=_wapi_handle_scratch_store (slash+1, strlen (slash+1));
			} else {
				process_handle_data->proc_name=_wapi_handle_scratch_store (argv[0], strlen (argv[0]));
			}
		}
		
		/* These seem to be the defaults on w2k */
		process_handle_data->min_working_set=204800;
		process_handle_data->max_working_set=1413120;
		
		if(cmd!=NULL) {
			g_free (cmd);
		}
		if(dir!=NULL) {
			g_free (dir);
		}
		g_strfreev (argv);
		g_strfreev (env);
		
		/* store pid */
		process_handle_data->id=pid;
		process_handle_data->main_thread=GUINT_TO_POINTER (thread_handle);
		/* Ignore errors */
		gettimeofday (&tv, NULL);
		_wapi_timeval_to_filetime (&tv,
					   &process_handle_data->create_time);
		
		/* FIXME: if env==0, inherit the env from the current
		 * process
		 */
		process_handle_data->env=process_fork.env;

		thread_handle_data->process_handle=GUINT_TO_POINTER (process_handle);

		resp.u.process_fork.pid=pid;
	}
			
	resp.u.process_fork.process_handle=process_handle;
	resp.u.process_fork.thread_handle=thread_handle;

	send_reply (channel, &resp);
}

/*
 * read_message:
 * @channel: The client to read the request from
 * @open_handles: An array of handles referenced by this client
 *
 * Read a message (A WapiHandleRequest) from a client and dispatch
 * whatever it wants to the process_* calls.
 */
static void read_message (GIOChannel *channel, guint32 *open_handles)
{
	WapiHandleRequest req;
	int fds[3]={0, 1, 2};
	int ret;
	gboolean has_fds=FALSE;
	
	/* Reading data */
	ret=_wapi_daemon_request (g_io_channel_unix_get_fd (channel), &req,
				  fds, &has_fds);
	if(ret==0) {
		/* Other end went away */
#ifdef DEBUG
		g_message ("Read 0 bytes on fd %d, closing it",
			   g_io_channel_unix_get_fd (channel));
#endif

		rem_fd (channel, open_handles);
		return;
	}
	
	switch(req.type) {
	case WapiHandleRequestType_New:
		process_new (channel, open_handles, req.u.new.type);
		break;
	case WapiHandleRequestType_Open:
#ifdef DEBUG
		g_assert(req.u.open.handle < _wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT);
#endif
		process_open (channel, open_handles, req.u.open.handle);
		break;
	case WapiHandleRequestType_Close:
#ifdef DEBUG
		g_assert(req.u.close.handle < _wapi_shared_data[0]->num_segments * _WAPI_HANDLES_PER_SEGMENT);
#endif
		process_close (channel, open_handles, req.u.close.handle);
		break;
	case WapiHandleRequestType_Scratch:
		process_scratch (channel, req.u.scratch.length);
		break;
	case WapiHandleRequestType_ScratchFree:
		process_scratch_free (channel, req.u.scratch_free.idx);
		break;
	case WapiHandleRequestType_ProcessFork:
		process_process_fork (channel, open_handles,
				      req.u.process_fork, fds);
		break;
	case WapiHandleRequestType_Error:
		/* fall through */
	default:
		/* Catch bogus requests */
		/* FIXME: call rem_fd? */
		break;
	}

	if(has_fds==TRUE) {
#ifdef DEBUG
		g_message (G_GNUC_PRETTY_FUNCTION ": closing %d", fds[0]);
		g_message (G_GNUC_PRETTY_FUNCTION ": closing %d", fds[1]);
		g_message (G_GNUC_PRETTY_FUNCTION ": closing %d", fds[2]);
#endif
		
		close (fds[0]);
		close (fds[1]);
		close (fds[2]);
	}
}

/*
 * fd_activity:
 * @channel: The IO channel that is active
 * @condition: The condition that has been satisfied
 * @data: A pointer to an array of handles referenced by this client
 *
 * The callback called by the main loop when there is activity on an
 * IO channel.
 */
static gboolean fd_activity (GIOChannel *channel, GIOCondition condition,
			     gpointer data)
{
	int fd=g_io_channel_unix_get_fd (channel);
	guint32 *open_handles=client_handles[fd];
	
	if(condition & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
#ifdef DEBUG
		g_message ("fd %d error", fd);
#endif

		rem_fd (channel, open_handles);
		return(FALSE);
	}

	if(condition & (G_IO_IN | G_IO_PRI)) {
		if(fd==main_sock) {
			int newsock;
			struct sockaddr addr;
			socklen_t addrlen=sizeof(struct sockaddr);
			
			newsock=accept (main_sock, &addr, &addrlen);
			if(newsock==-1) {
				g_critical ("accept error: %s",
					    g_strerror (errno));
				cleanup ();
				exit (-1);
			}

#ifdef DEBUG
			g_message ("accept returning %d", newsock);
#endif

			add_fd (newsock);
		} else {
#ifdef DEBUG
			g_message ("reading data on fd %d", fd);
#endif

			read_message (channel, open_handles);
		}
		return(TRUE);
	}
	
	return(FALSE);	/* remove source */
}

/*
 * _wapi_daemon_main:
 *
 * Open socket, create shared mem segment and begin listening for
 * clients.
 */
void _wapi_daemon_main(gpointer data, gpointer scratch)
{
	struct sockaddr_un main_socket_address;
	int ret;

#ifdef DEBUG
	g_message ("Starting up...");
#endif

	_wapi_shared_data[0]=data;
	_wapi_shared_scratch=scratch;
	_wapi_shared_scratch->is_shared=TRUE;
	
	/* Note that we've got the starting segment already */
	_wapi_shared_data[0]->num_segments=1;
	_wapi_shm_mapped_segments=1;
	
	startup ();
	
	main_sock=socket(PF_UNIX, SOCK_STREAM, 0);

	main_socket_address.sun_family=AF_UNIX;
	memcpy(main_socket_address.sun_path, _wapi_shared_data[0]->daemon,
	       MONO_SIZEOF_SUNPATH);

	ret=bind(main_sock, (struct sockaddr *)&main_socket_address,
		 sizeof(struct sockaddr_un));
	if(ret==-1) {
		g_critical ("bind failed: %s", g_strerror (errno));
		_wapi_shared_data[0]->daemon_running=DAEMON_DIED_AT_STARTUP;
		exit(-1);
	}

#ifdef DEBUG
	g_message("bound");
#endif

	ret=listen(main_sock, 5);
	if(ret==-1) {
		g_critical ("listen failed: %s", g_strerror (errno));
		_wapi_shared_data[0]->daemon_running=DAEMON_DIED_AT_STARTUP;
		exit(-1);
	}

#ifdef DEBUG
	g_message("listening");
#endif

	add_fd(main_sock);

	/* We're finished setting up, let everyone else know we're
	 * ready.  From now on, it's up to us to delete the shared
	 * memory segment when appropriate.
	 */
	_wapi_shared_data[0]->daemon_running=DAEMON_RUNNING;

	while(TRUE) {
		if(check_processes==TRUE) {
			process_died ();
		}
		
#ifdef DEBUG
		g_message ("polling");
#endif

		/* Block until something happens. We don't use
		 * g_main_loop_run() because we rely on the SIGCHLD
		 * signal interrupting poll() so we can reap child
		 * processes as soon as they die, without burning cpu
		 * time by polling the flag.
		 */
		g_main_context_iteration (g_main_context_default (), TRUE);
	}
}

