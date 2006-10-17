/*
 * Pointer Array
 *
 * Author:
 *   Aaron Bockover (abockover@novell.com)
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
 
#include <stdlib.h>
#include <glib.h>

typedef struct _GPtrArrayPriv {
	gpointer *pdata;
	guint len;
	guint size;
} GPtrArrayPriv;

static void 
g_ptr_array_grow(GPtrArrayPriv *array, guint length)
{
	gint new_length = array->len + length;

	g_return_if_fail(array != NULL);

	if(new_length <= array->size) {
		return;
	}

	array->size = 1;

	while(array->size < new_length) {
		array->size <<= 1;
	}

	array->size = MAX(array->size, 16);
	array->pdata = g_realloc(array->pdata, array->size * sizeof(gpointer));
}

GPtrArray *
g_ptr_array_new()
{
	return g_ptr_array_sized_new(0);
}

GPtrArray *
g_ptr_array_sized_new(guint reserved_size)
{
	GPtrArrayPriv *array = g_new0(GPtrArrayPriv, 1);

	array->pdata = NULL;
	array->len = 0;
	array->size = 0;

	if(reserved_size > 0) {
		g_ptr_array_grow(array, reserved_size);
	}

	return (GPtrArray *)array;
}

gpointer *
g_ptr_array_free(GPtrArray *array, gboolean free_seg)
{
	gpointer *data = NULL;
	
	g_return_val_if_fail(array != NULL, NULL);

	if(free_seg) {
		g_free(array->pdata);
	} else {
		data = array->pdata;
	}

	g_free(array);
	
	return data;
}

void
g_ptr_array_set_size(GPtrArray *array, gint length)
{
	g_return_if_fail(array != NULL);

	if(length > array->len) {
		g_ptr_array_grow((GPtrArrayPriv *)array, length);
		memset(array->pdata + array->len, 0, (length - array->len) 
			* sizeof(gpointer));
	}

	array->len = length;
}

void
g_ptr_array_add(GPtrArray *array, gpointer data)
{
	g_return_if_fail(array != NULL);
	g_ptr_array_grow((GPtrArrayPriv *)array, 1);
	array->pdata[array->len++] = data;
}

gpointer
g_ptr_array_remove_index(GPtrArray *array, guint index)
{
	gpointer removed_node;
	
	g_return_val_if_fail(array != NULL, NULL);
	g_return_val_if_fail(index >= 0 || index < array->len, NULL);

	removed_node = array->pdata[index];

	if(index != array->len - 1) {
		g_memmove(array->pdata + index, array->pdata + index + 1,
			(array->len - index - 1) * sizeof(gpointer));
	}
	
	array->len--;
	array->pdata[array->len] = NULL;

	return removed_node;
}

gboolean
g_ptr_array_remove(GPtrArray *array, gpointer data)
{
	guint i;

	g_return_val_if_fail(array != NULL, FALSE);

	for(i = 0; i < array->len; i++) {
		if(array->pdata[i] == data) {
			g_ptr_array_remove_index(array, i);
			return TRUE;
		}
	}

	return FALSE;
}

void 
g_ptr_array_foreach(GPtrArray *array, GFunc func, gpointer user_data)
{
	gint i;

	for(i = 0; i < array->len; i++) {
		func(g_ptr_array_index(array, i), user_data);
	}
}

void
g_ptr_array_sort(GPtrArray *array, GCompareFunc compare_func)
{
	g_return_if_fail(array != NULL);
	qsort(array->pdata, array->len, sizeof(gpointer), compare_func);
}

void
g_ptr_array_sort_with_data(GPtrArray *array, GCompareDataFunc compare_func, 
	gpointer user_data)
{
}


