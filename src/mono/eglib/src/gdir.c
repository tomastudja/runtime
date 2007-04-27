/*
 * Directory utility functions.
 *
 * Author:
 *   Gonzalo Paniagua Javier (gonzalo@novell.com)
 *
 * (C) 2006 Novell, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <dirent.h>
#else
#include <io.h>
#endif

#ifdef G_OS_WIN32
#include <winsock2.h>
#endif

struct _GDir {
#ifdef G_OS_WIN32
	HANDLE handle;
	gchar* current;
	gchar* next;
#else
	DIR *dir;
#endif
};

GDir *
g_dir_open (const gchar *path, guint flags, GError **error)
{
#ifdef G_OS_WIN32
	GDir *dir;
	gunichar2* path_utf16;
	gunichar2* path_utf16_search;
	WIN32_FIND_DATA find_data;

	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);
	dir = g_new0 (GDir, 1);

	path_utf16 = u8to16 (path);

	dir->handle = FindFirstFile (path_utf16, &find_data);
	if (dir->handle == INVALID_HANDLE_VALUE) {
		if (error) {
			gint err = errno;
			*error = g_error_new (G_LOG_DOMAIN, g_file_error_from_errno (err), strerror (err));
		}
		g_free (dir);
		g_free (path_utf16);
		return NULL;
	}

	/* now get files */
	FindClose (dir->handle);
	path_utf16_search = g_malloc ((wcslen(path_utf16) + 3)*sizeof(gunichar2));
	wcscpy (path_utf16_search, path_utf16);
	wcscat (path_utf16_search, L"\\*");

	dir->handle = FindFirstFile (path_utf16_search, &find_data);
	g_free (path_utf16_search);

	while ((wcscmp (find_data.cFileName, L".") == 0) || (wcscmp (find_data.cFileName, L"..") == 0)) {
		if (!FindNextFile (dir->handle, &find_data)) {
			if (error) {
				gint err = errno;
				*error = g_error_new (G_LOG_DOMAIN, g_file_error_from_errno (err), strerror (err));
			}
			g_free (dir);
			g_free (path_utf16);
			return NULL;
		}
	}

	dir->current = NULL;
	dir->next = u16to8 (find_data.cFileName);

	g_free (path_utf16);
	return dir;
#else
	GDir *dir;

	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	(void) flags; /* this is not used */
	dir = g_new (GDir, 1);
	dir->dir = opendir (path);
	if (dir->dir == NULL) {
		if (error) {
			gint err = errno;
			*error = g_error_new (G_LOG_DOMAIN, g_file_error_from_errno (err), strerror (err));
		}
		g_free (dir);
		return NULL;
	}
	return dir;
#endif
}

const gchar *
g_dir_read_name (GDir *dir)
{
#ifdef G_OS_WIN32
	WIN32_FIND_DATA find_data;

	g_return_val_if_fail (dir != NULL && dir->handle != 0, NULL);

	if (dir->current)
		g_free (dir->current);
	dir->current = NULL;

	dir->current = dir->next;

	if (!dir->current)
		return NULL;

	dir->next = NULL;

	do {
		if (!FindNextFile (dir->handle, &find_data)) {
			dir->next = NULL;
			return dir->current;
		}
	} while ((wcscmp (find_data.cFileName, L".") == 0) || (wcscmp (find_data.cFileName, L"..") == 0));

	dir->next = u16to8 (find_data.cFileName);
	return dir->current;
#else
	struct dirent *entry;

	g_return_val_if_fail (dir != NULL && dir->dir != NULL, NULL);
	do {
		entry = readdir (dir->dir);
		if (entry == NULL)
			return NULL;
	} while ((strcmp (entry->d_name, ".") == 0) || (strcmp (entry->d_name, "..") == 0));

	return entry->d_name;
#endif
}

void
g_dir_rewind (GDir *dir)
{
#ifdef G_OS_WIN32
#else
	g_return_if_fail (dir != NULL && dir->dir != NULL);
	rewinddir (dir->dir);
#endif
}

void
g_dir_close (GDir *dir)
{
#ifdef G_OS_WIN32
	g_return_if_fail (dir != NULL && dir->handle != 0);
	
	if (dir->current)
		g_free (dir->current);
	dir->current = NULL;
	if (dir->next)
		g_free (dir->next);
	dir->next = NULL;
	FindClose (dir->handle);
	dir->handle = 0;
	g_free (dir);
#else
	g_return_if_fail (dir != NULL && dir->dir != 0);
	closedir (dir->dir);
	dir->dir = NULL;
	g_free (dir);
#endif
}


