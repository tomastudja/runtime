/*
 * environment.c: System.Environment support internal calls
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *	Sebastien Pouliot (sebastien@ximian.com)
 *
 * Copyright 2002-2003 Ximian, Inc (http://www.ximian.com)
 * Copyright 2004-2009 Novell, Inc (http://www.novell.com)
 */

#include <config.h>
#include <glib.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/exception.h>
#include <mono/io-layer/io-layer.h>

#if !defined(HOST_WIN32) && defined(HAVE_SYS_UTSNAME_H)
#include <sys/utsname.h>
#endif

static gint32 exitcode=0;

gint32 mono_environment_exitcode_get (void)
{
	return(exitcode);
}

void mono_environment_exitcode_set (gint32 value)
{
	exitcode=value;
}

/* note: we better manipulate the string in managed code (easier and safer) */
MonoString*
ves_icall_System_Environment_GetOSVersionString (void)
{
#ifdef HOST_WIN32
	OSVERSIONINFO verinfo;

	MONO_ARCH_SAVE_REGS;

	verinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	if (GetVersionEx (&verinfo)) {
		char version [64];
		/* maximum string length is 35 bytes 
		   3 x 10 bytes per number, 1 byte for 0, 3 x 1 byte for dots, 1 for NULL */
		sprintf (version, "%ld.%ld.%ld.0", 
			verinfo.dwMajorVersion,
			verinfo.dwMinorVersion,
			verinfo.dwBuildNumber);
		return mono_string_new (mono_domain_get (), version);
	}
#elif defined(HAVE_SYS_UTSNAME_H)
	struct utsname name;

	MONO_ARCH_SAVE_REGS;

	if (uname (&name) >= 0) {
		return mono_string_new (mono_domain_get (), name.release);
	}
#endif
	return mono_string_new (mono_domain_get (), "0.0.0.0");
}

