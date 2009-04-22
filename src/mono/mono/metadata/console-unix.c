/*
 * console-io.c: ConsoleDriver internal calls for Unix systems.
 *
 * Author:
 *	Gonzalo Paniagua Javier (gonzalo@ximian.com)
 *
 * Copyright (C) 2005-2009 Novell, Inc. (http://www.novell.com)
 */

#include <config.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#endif
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include <mono/metadata/appdomain.h>
#include <mono/metadata/object-internals.h>
#include <mono/metadata/class-internals.h>
#include <mono/metadata/domain-internals.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/threadpool.h>

/* On solaris, curses.h must come before both termios.h and term.h */
#ifdef HAVE_CURSES_H
#    include <curses.h>
#endif
#ifdef HAVE_TERMIOS_H
#    include <termios.h>
#endif
#ifdef HAVE_TERM_H
#    include <term.h>
#endif

/* Needed for FIONREAD under solaris */
#ifdef HAVE_SYS_FILIO_H
#    include <sys/filio.h>
#endif
#ifndef TIOCGWINSZ
#    ifdef HAVE_SYS_IOCTL_H
#        include <sys/ioctl.h>
#    endif
#endif

#include <mono/metadata/console-io.h>
#include <mono/metadata/exception.h>

static gboolean setup_finished;
static gboolean atexit_called;

/* The string used to return the terminal to its previous state */
static gchar *teardown_str;

/* The string used to set the terminal into keypad xmit mode after SIGCONT is received */
static gchar *keypad_xmit_str;

#ifdef HAVE_TERMIOS_H
/* This is the last state used by Mono, used after a CONT signal is received */
static struct termios mono_attr;
#endif

void
mono_console_init (void)
{
	int fd;

	/* Make sure the standard file descriptors are opened */
	fd = open ("/dev/null", O_RDWR);
	while (fd >= 0 && fd < 3) {
		fd = open ("/dev/null", O_RDWR);
	}
	close (fd);
}

static struct termios initial_attr;

MonoBoolean
ves_icall_System_ConsoleDriver_Isatty (HANDLE handle)
{
	MONO_ARCH_SAVE_REGS;

	return isatty (GPOINTER_TO_INT (handle));
}

static MonoBoolean
set_property (gint property, gboolean value)
{
	struct termios attr;
	gboolean callset = FALSE;
	gboolean check;
	
	MONO_ARCH_SAVE_REGS;

	if (tcgetattr (STDIN_FILENO, &attr) == -1)
		return FALSE;

	check = (attr.c_lflag & property) != 0;
	if ((value || check) && !(value && check)) {
		callset = TRUE;
		if (value)
			attr.c_lflag |= property;
		else
			attr.c_lflag &= ~property;
	}

	if (!callset)
		return TRUE;

	if (tcsetattr (STDIN_FILENO, TCSANOW, &attr) == -1)
		return FALSE;

	mono_attr = attr;
	return TRUE;
}

MonoBoolean
ves_icall_System_ConsoleDriver_SetEcho (MonoBoolean want_echo)
{
	
	return set_property (ECHO, want_echo);
}

MonoBoolean
ves_icall_System_ConsoleDriver_SetBreak (MonoBoolean want_break)
{
	return set_property (IGNBRK, !want_break);
}

gint32
ves_icall_System_ConsoleDriver_InternalKeyAvailable (gint32 timeout)
{
	fd_set rfds;
	struct timeval tv;
	struct timeval *tvptr;
	div_t divvy;
	int ret, nbytes;

	MONO_ARCH_SAVE_REGS;

	do {
		FD_ZERO (&rfds);
		FD_SET (STDIN_FILENO, &rfds);
		if (timeout >= 0) {
			divvy = div (timeout, 1000);
			tv.tv_sec = divvy.quot;
			tv.tv_usec = divvy.rem;
			tvptr = &tv;
		} else {
			tvptr = NULL;
		}
		ret = select (STDIN_FILENO + 1, &rfds, NULL, NULL, tvptr);
	} while (ret == -1 && errno == EINTR);

	if (ret > 0) {
		nbytes = 0;
		ret = ioctl (STDIN_FILENO, FIONREAD, &nbytes);
		if (ret >= 0)
			ret = nbytes;
	}

	return (ret > 0) ? ret : 0;
}

static gint32 cols_and_lines;

#ifdef TIOCGWINSZ
static int
terminal_get_dimensions (void)
{
	struct winsize ws;

	if (ioctl (STDIN_FILENO, TIOCGWINSZ, &ws) == 0)
		return (ws.ws_col << 16) | ws.ws_row;

	return -1;
}
#else
static int
terminal_get_dimensions (void)
{
	return -1;
}
#endif

static void
tty_teardown (void)
{
	MONO_ARCH_SAVE_REGS;

	if (!setup_finished)
		return;

	if (teardown_str != NULL) {
		write (STDOUT_FILENO, teardown_str, strlen (teardown_str));
		g_free (teardown_str);
		teardown_str = NULL;
	}

	tcflush (STDIN_FILENO, TCIFLUSH);
	tcsetattr (STDIN_FILENO, TCSANOW, &initial_attr);
	set_property (ECHO, TRUE);
	setup_finished = FALSE;
}

static void
do_console_cancel_event (void)
{
	static MonoClassField *cancel_handler_field;
	MonoDomain *domain = mono_domain_get ();
	MonoClass *klass;
	MonoDelegate *load_value;
	MonoMethod *method;
	MonoMethodMessage *msg;
	MonoMethod *im;

	if (!domain->domain)
		return;

	klass = mono_class_from_name (mono_defaults.corlib, "System", "Console");
	if (klass == NULL)
		return;

	if (cancel_handler_field == NULL) {
		cancel_handler_field = mono_class_get_field_from_name (klass, "cancel_handler");
		g_assert (cancel_handler_field);
	}

	mono_field_static_get_value (mono_class_vtable (domain, klass), cancel_handler_field, &load_value);
	if (load_value == NULL)
		return;

	klass = load_value->object.vtable->klass;
	method = mono_class_get_method_from_name (klass, "BeginInvoke", -1);
	g_assert (method != NULL);
	im = mono_get_delegate_invoke (method->klass);
	msg = mono_method_call_message_new (method, NULL, im, NULL, NULL);
	mono_thread_pool_add ((MonoObject *) load_value, msg, NULL, NULL);
}

static gboolean in_sigint;
static void
sigint_handler (int signo)
{
	MONO_ARCH_SAVE_REGS;

	if (in_sigint)
		return;

	in_sigint = TRUE;
	do_console_cancel_event ();
	in_sigint = FALSE;
}

static struct sigaction save_sigcont, save_sigint, save_sigwinch;

static void
sigcont_handler (int signo, void *the_siginfo, void *data)
{
	// Ignore error, there is not much we can do in the sigcont handler.
	tcsetattr (STDIN_FILENO, TCSANOW, &mono_attr);

	if (keypad_xmit_str != NULL)
		write (STDOUT_FILENO, keypad_xmit_str, strlen (keypad_xmit_str));

	// Call previous handler
	if (save_sigcont.sa_sigaction != NULL &&
	    save_sigcont.sa_sigaction != (void *)SIG_DFL &&
	    save_sigcont.sa_sigaction != (void *)SIG_IGN)
		(*save_sigcont.sa_sigaction) (signo, the_siginfo, data);
}

static void
sigwinch_handler (int signo, void *the_siginfo, void *data)
{
	int dims = terminal_get_dimensions ();
	if (dims != -1)
		cols_and_lines = dims;
	
	// Call previous handler
	if (save_sigwinch.sa_sigaction != NULL &&
	    save_sigwinch.sa_sigaction != (void *)SIG_DFL &&
	    save_sigwinch.sa_sigaction != (void *)SIG_IGN)
		(*save_sigwinch.sa_sigaction) (signo, the_siginfo, data);
}

void
console_set_signal_handlers ()
{
	struct sigaction sigcont, sigint, sigwinch;

	memset (&sigcont, 0, sizeof (struct sigaction));
	memset (&sigint, 0, sizeof (struct sigaction));
	memset (&sigwinch, 0, sizeof (struct sigaction));
	
	// Continuing
	sigcont.sa_handler = (void *) sigcont_handler;
	sigcont.sa_flags = 0;
	sigemptyset (&sigcont.sa_mask);
	sigaction (SIGCONT, &sigcont, &save_sigcont);
	
	// Interrupt handler
	sigint.sa_handler = sigint_handler;
	sigint.sa_flags = 0;
	sigemptyset (&sigint.sa_mask);
	sigaction (SIGINT, &sigint, &save_sigint);

	// Window size changed
	sigwinch.sa_handler = (void *) sigwinch_handler;
	sigwinch.sa_flags = 0;
	sigemptyset (&sigwinch.sa_mask);
	sigaction (SIGWINCH, &sigwinch, &save_sigwinch);
}

//
// Currently unused, should we ever call the restore handler?
// Perhaps before calling into Process.Start?
//
void
console_restore_signal_handlers ()
{
	sigaction (SIGCONT, &save_sigcont, NULL);
	sigaction (SIGINT, &save_sigint, NULL);
	sigaction (SIGWINCH, &save_sigwinch, NULL);
}

static void
set_control_chars (MonoArray *control_chars, const guchar *cc)
{
	/* The index into the array comes from corlib/System/ControlCharacters.cs */
#ifdef VINTR
	mono_array_set (control_chars, gchar, 0, cc [VINTR]);
#endif
#ifdef VQUIT
	mono_array_set (control_chars, gchar, 1, cc [VQUIT]);
#endif
#ifdef VERASE
	mono_array_set (control_chars, gchar, 2, cc [VERASE]);
#endif
#ifdef VKILL
	mono_array_set (control_chars, gchar, 3, cc [VKILL]);
#endif
#ifdef VEOF
	mono_array_set (control_chars, gchar, 4, cc [VEOF]);
#endif
#ifdef VTIME
	mono_array_set (control_chars, gchar, 5, cc [VTIME]);
#endif
#ifdef VMIN
	mono_array_set (control_chars, gchar, 6, cc [VMIN]);
#endif
#ifdef VSWTC
	mono_array_set (control_chars, gchar, 7, cc [VSWTC]);
#endif
#ifdef VSTART
	mono_array_set (control_chars, gchar, 8, cc [VSTART]);
#endif
#ifdef VSTOP
	mono_array_set (control_chars, gchar, 9, cc [VSTOP]);
#endif
#ifdef VSUSP
	mono_array_set (control_chars, gchar, 10, cc [VSUSP]);
#endif
#ifdef VEOL
	mono_array_set (control_chars, gchar, 11, cc [VEOL]);
#endif
#ifdef VREPRINT
	mono_array_set (control_chars, gchar, 12, cc [VREPRINT]);
#endif
#ifdef VDISCARD
	mono_array_set (control_chars, gchar, 13, cc [VDISCARD]);
#endif
#ifdef VWERASE
	mono_array_set (control_chars, gchar, 14, cc [VWERASE]);
#endif
#ifdef VLNEXT
	mono_array_set (control_chars, gchar, 15, cc [VLNEXT]);
#endif
#ifdef VEOL2
	mono_array_set (control_chars, gchar, 16, cc [VEOL2]);
#endif
}

MonoBoolean
ves_icall_System_ConsoleDriver_TtySetup (MonoString *keypad, MonoString *teardown, MonoArray **control_chars, int **size)
{
	int dims;

	MONO_ARCH_SAVE_REGS;

	dims = terminal_get_dimensions ();
	if (dims == -1){
		int cols = 0, rows = 0;
				      
		char *str = getenv ("COLUMNS");
		if (str != NULL)
			cols = atoi (str);
		str = getenv ("LINES");
		if (str != NULL)
			rows = atoi (str);

		if (cols != 0 && rows != 0)
			cols_and_lines = (cols << 16) | rows;
		else
			cols_and_lines = -1;
	} else {
		cols_and_lines = dims;
	}
	
	*size = &cols_and_lines;

	/* 17 is the number of entries set in set_control_chars() above.
	 * NCCS is the total size, but, by now, we only care about those 17 values*/
	*control_chars = mono_array_new (mono_domain_get (), mono_defaults.byte_class, 17);
	if (tcgetattr (STDIN_FILENO, &initial_attr) == -1)
		return FALSE;

	mono_attr = initial_attr;
	mono_attr.c_lflag &= ~(ICANON);
	mono_attr.c_iflag &= ~(IXON|IXOFF);
	mono_attr.c_cc [VMIN] = 1;
	mono_attr.c_cc [VTIME] = 0;
	if (tcsetattr (STDIN_FILENO, TCSANOW, &mono_attr) == -1)
		return FALSE;

	set_control_chars (*control_chars, mono_attr.c_cc);
	/* If initialized from another appdomain... */
	if (setup_finished)
		return TRUE;

	keypad_xmit_str = keypad != NULL ? mono_string_to_utf8 (keypad) : NULL;
	
	console_set_signal_handlers ();
	setup_finished = TRUE;
	if (!atexit_called) {
		if (teardown != NULL)
			teardown_str = mono_string_to_utf8 (teardown);

		atexit (tty_teardown);
	}

	return TRUE;
}
