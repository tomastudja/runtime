/*
 * console-io.c: ConsoleDriver internal calls
 *
 * Author:
 *	Gonzalo Paniagua Javier (gonzalo@ximian.com)
 *
 * Copyright (C) 2005 Novell, Inc. (http://www.novell.com)
 */

#include <config.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_TERM_H
#include <term.h>
#endif
#ifndef PLATFORM_WIN32
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif
#endif

#include <mono/metadata/console-io.h>
#include <mono/metadata/exception.h>

static gboolean setup_finished;
static gboolean atexit_called;
static gchar *teardown_str;
static struct termios initial_attr;

MonoBoolean
ves_icall_System_ConsoleDriver_Isatty (HANDLE handle)
{
	MONO_ARCH_SAVE_REGS;

#ifdef PLATFORM_WIN32
	return (GetFileType (handle) == FILE_TYPE_CHAR);
#else
	return isatty (GPOINTER_TO_INT (handle));
#endif
}

static MonoBoolean
set_property (gint property, gboolean value)
{
#ifdef PLATFORM_WIN32
	return FALSE;
#else
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

	return TRUE;
#endif
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
#ifdef PLATFORM_WIN32
	return FALSE;
#else
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
#endif
}

static void
tty_teardown (void)
{
#ifdef PLATFORM_WIN32
#else
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
	setup_finished = FALSE;
#endif
}

MonoBoolean
ves_icall_System_ConsoleDriver_TtySetup (MonoString *teardown)
{
#ifdef PLATFORM_WIN32
	return FALSE;
#else
	struct termios attr;
	
	MONO_ARCH_SAVE_REGS;

	if (tcgetattr (STDIN_FILENO, &initial_attr) == -1)
		return FALSE;

	attr = initial_attr;
	attr.c_lflag &= ~ICANON;
	attr.c_cc [VMIN] = 1;
	attr.c_cc [VTIME] = 0;
	if (tcsetattr (STDIN_FILENO, TCSANOW, &attr) == -1)
		return FALSE;

	setup_finished = TRUE;
	if (!atexit_called) {
		if (teardown != NULL)
			teardown_str = mono_string_to_utf8 (teardown);

		atexit (tty_teardown);
	}

	return TRUE;
#endif
}

