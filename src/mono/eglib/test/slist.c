#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "test.h"

RESULT
test_slist_append ()
{
	GSList *list = g_slist_prepend (NULL, "first");
	if (g_slist_length (list) != 1)
		return FAILED ("Prepend failed");

	g_slist_append (list, "second");

	if (g_slist_length (list) != 2)
		return FAILED ("Append failed");

	g_slist_free (list);
	return OK;
}

RESULT
test_slist_concat ()
{
	GSList *foo = g_slist_prepend (NULL, "foo");
	GSList *bar = g_slist_prepend (NULL, "bar");

	GSList *list = g_slist_concat (foo, bar);

	if (g_slist_length (list) != 2)
		return FAILED ("Concat failed.");

	g_slist_free (list);
	return OK;
}

RESULT
test_slist_find ()
{
	GSList *list = g_slist_prepend (NULL, "three");
	GSList *found;
	char *data;
		
	list = g_slist_prepend (list, "two");
	list = g_slist_prepend (list, "one");

	data = "four";
	list = g_slist_append (list, data);

	found = g_slist_find (list, data);

	if (found->data != data)
		return FAILED ("Find failed");

	g_slist_free (list);
	return OK;
}

RESULT
test_slist_remove ()
{
	GSList *list = g_slist_prepend (NULL, "three");
	char *one = "one";
	list = g_slist_prepend (list, "two");
	list = g_slist_prepend (list, one);

	list = g_slist_remove (list, one);

	if (g_slist_length (list) != 2)
		return FAILED ("Remove failed");

	if (strcmp ("two", list->data) != 0)
		return FAILED ("Remove failed");

	g_slist_free (list);
	return OK;
}

RESULT
test_slist_remove_link ()
{
	GSList *foo = g_slist_prepend (NULL, "a");
	GSList *bar = g_slist_prepend (NULL, "b");
	GSList *baz = g_slist_prepend (NULL, "c");
	GSList *list = foo;

	foo = g_slist_concat (foo, bar);
	foo = g_slist_concat (foo, baz);	

	list = g_slist_remove_link (list, bar);

	if (g_slist_length (list) != 2)
		return FAILED ("remove_link failed #1");

	if (bar->next != NULL)
		return FAILED ("remove_link failed #2");

	g_slist_free (list);	
	g_slist_free (bar);

	return OK;
}

static gint
compare (gconstpointer a, gconstpointer b)
{
	char *foo = (char *) a;
	char *bar = (char *) b;

	if (strlen (foo) < strlen (bar))
		return -1;

	return 1;
}

RESULT
test_slist_insert_sorted ()
{
	GSList *list = g_slist_prepend (NULL, "a");
	list = g_slist_append (list, "aaa");

	/* insert at the middle */
	list = g_slist_insert_sorted (list, "aa", compare);
	if (strcmp ("aa", list->next->data))
		return FAILED("insert_sorted failed #1");

	/* insert at the beginning */
	list = g_slist_insert_sorted (list, "", compare);
	if (strcmp ("", list->data))
		return FAILED ("insert_sorted failed #2");

	/* insert at the end */
	list = g_slist_insert_sorted (list, "aaaa", compare);
	if (strcmp ("aaaa", g_slist_last (list)->data))
		return FAILED ("insert_sorted failed #3");

	g_slist_free (list);	
	return OK;
}

RESULT
test_slist_insert_before ()
{
	GSList *foo, *bar, *baz;

	foo = g_slist_prepend (NULL, "foo");
	foo = g_slist_insert_before (foo, NULL, "bar");
	bar = g_slist_last (foo);

	if (strcmp (bar->data, "bar"))
		return FAILED ("1");

	baz = g_slist_insert_before (foo, bar, "baz");
	if (foo != baz)
		return FAILED ("2");

	if (strcmp (foo->next->data, "baz"))
		return FAILED ("3: %s", foo->next->data);

	g_slist_free (foo);
	return OK;
}

#define N_ELEMS 100

static int intcompare (gconstpointer p1, gconstpointer p2)
{
	return GPOINTER_TO_INT (p1) - GPOINTER_TO_INT (p2);
}

static gboolean verify_sort (GSList *list)
{
	list = g_slist_sort (list, intcompare);

	int prev = GPOINTER_TO_INT (list->data);
	for (list = list->next; list; list = list->next) {
		int curr = GPOINTER_TO_INT (list->data);
		if (prev > curr)
			return FALSE;
		prev = curr;
	}
	return TRUE;
}

RESULT
test_slist_sort ()
{
	int i = 0;
	GSList *list = NULL;

	for (i = 0; i < N_ELEMS; ++i)
		list = g_slist_prepend (list, GINT_TO_POINTER (i));
	if (!verify_sort (list))
		return FAILED ("decreasing list");

	g_slist_free (list);

	list = NULL;
	for (i = 0; i < N_ELEMS; ++i)
		list = g_slist_prepend (list, GINT_TO_POINTER (-i));
	if (!verify_sort (list))
		return FAILED ("increasing list");

	g_slist_free (list);

	list = NULL;
	int mul = 1;
	for (i = 0; i < N_ELEMS; ++i) {
		list = g_slist_prepend (list, GINT_TO_POINTER (mul * i));
		mul = -mul;
	}

	if (!verify_sort (list))
		return FAILED ("alternating list");

	g_slist_free (list);

	return OK;
}

static Test slist_tests [] = {
	{"append", test_slist_append},
	{"concat", test_slist_concat},
	{"find", test_slist_find},
	{"remove", test_slist_remove},
	{"remove_link", test_slist_remove_link},
	{"insert_sorted", test_slist_insert_sorted},
	{"insert_before", test_slist_insert_before},
	{"sort", test_slist_sort},
	{NULL, NULL}
};

DEFINE_TEST_GROUP_INIT(slist_tests_init, slist_tests)

