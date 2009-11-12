/*
 * mono-path.c: Routines for handling path names.
 * 
 * Authors:
 * 	Gonzalo Paniagua Javier (gonzalo@novell.com)
 * 	Miguel de Icaza (miguel@novell.com)
 *
 * (C) 2006 Novell, Inc.  http://www.novell.com
 *
 */
#include <config.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
/* This is only needed for the mono_path_canonicalize code, MAXSYMLINKS, could be moved */
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include "mono-path.h"

/* Embedded systems lack MAXSYMLINKS */
#ifndef MAXSYMLINKS
#define MAXSYMLINKS 3
#endif

/* Resolves '..' and '.' references in a path. If the path provided is relative,
 * it will be relative to the current directory */
gchar *
mono_path_canonicalize (const char *path)
{
	gchar *abspath, *pos, *lastpos, *dest;
	int backc;

	if (g_path_is_absolute (path)) {
		abspath = g_strdup (path);
	} else {
		gchar *tmpdir = g_get_current_dir ();
		abspath = g_build_filename (tmpdir, path, NULL);
		g_free (tmpdir);
	}

#ifdef PLATFORM_WIN32
	g_strdelimit (abspath, "/", '\\');
#endif
	abspath = g_strreverse (abspath);

	backc = 0;
	dest = lastpos = abspath;
	pos = strchr (lastpos, G_DIR_SEPARATOR);

	while (pos != NULL) {
		int len = pos - lastpos;
		if (len == 1 && lastpos [0] == '.') {
			// nop
		} else if (len == 2 && lastpos [0] == '.' && lastpos [1] == '.') {
			backc++;
		} else if (len > 0) {
			if (backc > 0) {
				backc--;
			} else {
				if (dest != lastpos) 
					/* The two strings can overlap */
					memmove (dest, lastpos, len + 1);
				dest += len + 1;
			}
		}
		lastpos = pos + 1;
		pos = strchr (lastpos, G_DIR_SEPARATOR);
	}

#ifdef PLATFORM_WIN32 /* For UNC paths the first '\' is removed. */
	if (*(lastpos-1) == G_DIR_SEPARATOR && *(lastpos-2) == G_DIR_SEPARATOR)
		lastpos = lastpos-1;
#endif
	
	if (dest != lastpos) strcpy (dest, lastpos);
	return g_strreverse (abspath);
}

/*
 * This ensures that the path that we store points to the final file
 * not a path to a symlink.
 */
#if !defined(PLATFORM_NO_SYMLINKS)
static gchar *
resolve_symlink (const char *path)
{
	char *p, *concat, *dir;
	char buffer [PATH_MAX+1];
	int n, iterations = 0;

	p = g_strdup (path);
	do {
		iterations++;
		n = readlink (p, buffer, sizeof (buffer)-1);
		if (n < 0){
			char *copy = p;
			p = mono_path_canonicalize (copy);
			g_free (copy);
			return p;
		}
		
		buffer [n] = 0;
		if (!g_path_is_absolute (buffer)) {
			dir = g_path_get_dirname (p);
			concat = g_build_filename (dir, buffer, NULL);
			g_free (dir);
		} else {
			concat = g_strdup (buffer);
		}
		g_free (p);
		p = mono_path_canonicalize (concat);
		g_free (concat);
	} while (iterations < MAXSYMLINKS);

	return p;
}
#endif

gchar *
mono_path_resolve_symlinks (const char *path)
{
#if defined(PLATFORM_NO_SYMLINKS)
	return mono_path_canonicalize (path);
#else
	gchar **split = g_strsplit (path, G_DIR_SEPARATOR_S, -1);
	gchar *p = g_strdup_printf ("");
	int i;

	for (i = 0; split [i] != NULL; i++) {
		gchar *tmp = NULL;

		// resolve_symlink of "" goes into canonicalize which resolves to cwd
		if (strcmp (split [i], "") != 0) {
			tmp = g_strdup_printf ("%s%s", p, split [i]);
			g_free (p);
			p = resolve_symlink (tmp);
			g_free (tmp);
		}

		if (split [i+1] != NULL) {
			tmp = g_strdup_printf ("%s%s", p, G_DIR_SEPARATOR_S);
			g_free (p);
			p = tmp;
		}
	}

	g_free (split);
	return p;
#endif
}

