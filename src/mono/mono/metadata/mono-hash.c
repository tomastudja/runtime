/**
 * \file
 * Hashtable implementation
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
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
#include <config.h>
#include <stdio.h>
#include <math.h>
#include <glib.h>
#include "mono-hash.h"
#include "metadata/gc-internals.h"
#include <mono/utils/checked-build.h>
#include <mono/utils/mono-threads-coop.h>

int mono_g_hash_table_max_chain_length;

#ifdef HAVE_BOEHM_GC
#define mg_new0(type,n)  ((type *) GC_MALLOC(sizeof(type) * (n)))
#define mg_new(type,n)   ((type *) GC_MALLOC(sizeof(type) * (n)))
#define mg_free(x)       do { } while (0)
#else
#define mg_new0(x,n)     g_new0(x,n)
#define mg_new(type,n)   g_new(type,n)
#define mg_free(x)       g_free(x)
#endif

struct _MonoGHashTable {
	GHashFunc      hash_func;
	GEqualFunc     key_equal_func;

	MonoObject **keys;
	MonoObject **values;
	int   table_size;
	int   in_use;
	GDestroyNotify value_destroy_func, key_destroy_func;
	MonoGHashGCType gc_type;
	MonoGCRootSource source;
	const char *msg;
};

#if UNUSED
static gboolean
test_prime (int x)
{
	if ((x & 1) != 0) {
		int n;
		for (n = 3; n< (int)sqrt (x); n += 2) {
			if ((x % n) == 0)
				return FALSE;
		}
		return TRUE;
	}
	// There is only one even prime - 2.
	return (x == 2);
}

static int
calc_prime (int x)
{
	int i;

	for (i = (x & (~1))-1; i< G_MAXINT32; i += 2) {
		if (test_prime (i))
			return i;
	}
	return x;
}
#endif

#define HASH_TABLE_MAX_LOAD_FACTOR 0.7f
/* We didn't really do compaction before, keep it lenient for now */
#define HASH_TABLE_MIN_LOAD_FACTOR 0.05f
/* We triple the table size at rehash time, similar with previous implementation */
#define HASH_TABLE_RESIZE_RATIO 3

static inline void mono_g_hash_table_key_store (MonoGHashTable *hash, int slot, MonoObject* key)
{
	MonoObject **key_addr = &hash->keys [slot];
	if (hash->gc_type & MONO_HASH_KEY_GC)
		mono_gc_wbarrier_generic_store (key_addr, key);
	else
		*key_addr = key;
}

static inline void mono_g_hash_table_value_store (MonoGHashTable *hash, int slot, MonoObject* value)
{
	MonoObject **value_addr = &hash->values [slot];
	if (hash->gc_type & MONO_HASH_VALUE_GC)
		mono_gc_wbarrier_generic_store (value_addr, value);
	else
		*value_addr = value;
}

/* Returns position of key or of an empty slot for it */
static inline int mono_g_hash_table_find_slot (MonoGHashTable *hash, const MonoObject *key)
{
	guint start = ((*hash->hash_func) (key)) % hash->table_size;
	guint i = start;

	if (hash->key_equal_func) {
		GEqualFunc equal = hash->key_equal_func;

		while (hash->keys [i] && !(*equal) (hash->keys [i], key)) {
			i++;
			if (i == hash->table_size)
				i = 0;
		}
	} else {
		while (hash->keys [i] && hash->keys [i] != key) {
			i++;
			if (i == hash->table_size)
				i = 0;
		}
	}

	if (i > start && (i - start) > mono_g_hash_table_max_chain_length)
		mono_g_hash_table_max_chain_length = i - start;
	else if (i < start && (hash->table_size - (start - i)) > mono_g_hash_table_max_chain_length)
		mono_g_hash_table_max_chain_length = hash->table_size - (start - i);
	return i;
}


MonoGHashTable *
mono_g_hash_table_new_type (GHashFunc hash_func, GEqualFunc key_equal_func, MonoGHashGCType type, MonoGCRootSource source, const char *msg)
{
	MonoGHashTable *hash;

	if (!hash_func)
		hash_func = g_direct_hash;

#ifdef HAVE_SGEN_GC
	hash = mg_new0 (MonoGHashTable, 1);
#else
	hash = mono_gc_alloc_fixed (sizeof (MonoGHashTable), MONO_GC_ROOT_DESCR_FOR_FIXED (sizeof (MonoGHashTable)), source, msg);
#endif

	hash->hash_func = hash_func;
	hash->key_equal_func = key_equal_func;

	hash->table_size = g_spaced_primes_closest (1);
	hash->keys = mg_new0 (MonoObject*, hash->table_size);
	hash->values = mg_new0 (MonoObject*, hash->table_size);

	hash->gc_type = type;
	hash->source = source;
	hash->msg = msg;

	if (type > MONO_HASH_KEY_VALUE_GC)
		g_error ("wrong type for gc hashtable");

#ifdef HAVE_SGEN_GC
	if (hash->gc_type & MONO_HASH_KEY_GC)
		mono_gc_register_root_wbarrier ((char*)hash->keys, sizeof (MonoObject*) * hash->table_size, mono_gc_make_vector_descr (), hash->source, hash->msg);
	if (hash->gc_type & MONO_HASH_VALUE_GC)
		mono_gc_register_root_wbarrier ((char*)hash->values, sizeof (MonoObject*) * hash->table_size, mono_gc_make_vector_descr (), hash->source, hash->msg);
#endif

	return hash;
}

typedef struct {
	MonoGHashTable *hash;
	int new_size;
	MonoObject **keys;
	MonoObject **values;
} RehashData;

static void*
do_rehash (void *_data)
{
	RehashData *data = (RehashData *)_data;
	MonoGHashTable *hash = data->hash;
	int current_size, i;
	MonoObject **old_keys;
	MonoObject **old_values;

	current_size = hash->table_size;
	hash->table_size = data->new_size;
	old_keys = hash->keys;
	old_values = hash->values;
	hash->keys = data->keys;
	hash->values = data->values;

	for (i = 0; i < current_size; i++) {
		if (old_keys [i]) {
			int slot = mono_g_hash_table_find_slot (hash, old_keys [i]);
			mono_g_hash_table_key_store (hash, slot, old_keys [i]);
			mono_g_hash_table_value_store (hash, slot, old_values [i]);
		}
	}
	return NULL;
}

static void
rehash (MonoGHashTable *hash)
{
	MONO_REQ_GC_UNSAFE_MODE; //we must run in unsafe mode to make rehash safe

	RehashData data;
	void *old_keys G_GNUC_UNUSED = hash->keys; /* unused on Boehm */
	void *old_values G_GNUC_UNUSED = hash->values; /* unused on Boehm */

	data.hash = hash;
	/*
	 * Rehash to a size that can fit the current elements. Rehash relative to in_use
	 * to allow also for compaction.
	 */
	data.new_size = g_spaced_primes_closest (hash->in_use / HASH_TABLE_MAX_LOAD_FACTOR * HASH_TABLE_RESIZE_RATIO);
	data.keys = mg_new0 (MonoObject*, data.new_size);
	data.values = mg_new0 (MonoObject*, data.new_size);

#ifdef HAVE_SGEN_GC
	if (hash->gc_type & MONO_HASH_KEY_GC)
		mono_gc_register_root_wbarrier ((char*)data.keys, sizeof (MonoObject*) * data.new_size, mono_gc_make_vector_descr (), hash->source, hash->msg);
	if (hash->gc_type & MONO_HASH_VALUE_GC)
		mono_gc_register_root_wbarrier ((char*)data.values, sizeof (MonoObject*) * data.new_size, mono_gc_make_vector_descr (), hash->source, hash->msg);
#endif

	if (!mono_threads_is_coop_enabled ()) {
		mono_gc_invoke_with_gc_lock (do_rehash, &data);
	} else {
		/* We cannot be preempted */
		do_rehash (&data);
	}

#ifdef HAVE_SGEN_GC
	if (hash->gc_type & MONO_HASH_KEY_GC)
		mono_gc_deregister_root ((char*)old_keys);
	if (hash->gc_type & MONO_HASH_VALUE_GC)
		mono_gc_deregister_root ((char*)old_values);
#endif
	mg_free (old_keys);
	mg_free (old_values);
}

guint
mono_g_hash_table_size (MonoGHashTable *hash)
{
	g_return_val_if_fail (hash != NULL, 0);

	return hash->in_use;
}

gpointer
mono_g_hash_table_lookup (MonoGHashTable *hash, gconstpointer key)
{
	gpointer orig_key, value;

	if (mono_g_hash_table_lookup_extended (hash, key, &orig_key, &value))
		return value;
	else
		return NULL;
}

gboolean
mono_g_hash_table_lookup_extended (MonoGHashTable *hash, gconstpointer key, gpointer *orig_key, gpointer *value)
{
	int slot;

	g_return_val_if_fail (hash != NULL, FALSE);

	slot = mono_g_hash_table_find_slot (hash, key);

	if (hash->keys [slot]) {
		*orig_key = hash->keys [slot];
		*value = hash->values [slot];
		return TRUE;
	}

	return FALSE;
}

void
mono_g_hash_table_foreach (MonoGHashTable *hash, GHFunc func, gpointer user_data)
{
	int i;

	g_return_if_fail (hash != NULL);
	g_return_if_fail (func != NULL);

	for (i = 0; i < hash->table_size; i++) {
		if (hash->keys [i])
			(*func)(hash->keys [i], hash->values [i], user_data);
	}
}

gpointer
mono_g_hash_table_find (MonoGHashTable *hash, GHRFunc predicate, gpointer user_data)
{
	int i;

	g_return_val_if_fail (hash != NULL, NULL);
	g_return_val_if_fail (predicate != NULL, NULL);

	for (i = 0; i < hash->table_size; i++) {
		if (hash->keys [i] && (*predicate)(hash->keys [i], hash->values [i], user_data))
			return hash->values [i];
	}
	return NULL;
}

gboolean
mono_g_hash_table_remove (MonoGHashTable *hash, gconstpointer key)
{
	int slot, last_clear_slot;

	g_return_val_if_fail (hash != NULL, FALSE);
	slot = mono_g_hash_table_find_slot (hash, key);

	if (!hash->keys [slot])
		return FALSE;

	if (hash->key_destroy_func)
		(*hash->key_destroy_func)(hash->keys [slot]);
	hash->keys [slot] = NULL;
	if (hash->value_destroy_func)
		(*hash->value_destroy_func)(hash->values [slot]);
	hash->values [slot] = NULL;
	hash->in_use--;

	/*
	 * When we insert in the hashtable, if the required position is occupied we
	 * consecutively try out following positions. In order to be able to find
	 * if a key exists or not in the array (without traversing the entire hash)
	 * we maintain the constraint that there can be no free slots between two
	 * entries that are hashed to the same position. This means that, at search
	 * time, when we encounter a free slot we can stop looking for collissions.
	 * Similarly, at remove time, we need to shift all following slots to their
	 * normal slot, until we reach an empty slot.
	 */
	last_clear_slot = slot;
	slot = (slot + 1) % hash->table_size;
	while (hash->keys [slot]) {
		guint hashcode = ((*hash->hash_func)(hash->keys [slot])) % hash->table_size;
		/*
		 * We try to move the current element to last_clear_slot, but only if
		 * it brings it closer to its normal position (hashcode)
		 */
		if ((last_clear_slot < slot && (hashcode > slot || hashcode <= last_clear_slot)) ||
				(last_clear_slot > slot && (hashcode > slot && hashcode <= last_clear_slot))) {
			mono_g_hash_table_key_store (hash, last_clear_slot, hash->keys [slot]);
			mono_g_hash_table_value_store (hash, last_clear_slot, hash->values [slot]);
			hash->keys [slot] = NULL;
			hash->values [slot] = NULL;
			last_clear_slot = slot;
		}
		slot++;
		if (slot == hash->table_size)
			slot = 0;
	}
	return TRUE;
}

guint
mono_g_hash_table_foreach_remove (MonoGHashTable *hash, GHRFunc func, gpointer user_data)
{
	int i;
	int count = 0;

	g_return_val_if_fail (hash != NULL, 0);
	g_return_val_if_fail (func != NULL, 0);

	for (i = 0; i < hash->table_size; i++) {
		if (hash->keys [i] && (*func)(hash->keys [i], hash->values [i], user_data)) {
			mono_g_hash_table_remove (hash, hash->keys [i]);
			count++;
			/* Retry current slot in case the removal shifted elements */
			i--;
		}
	}
	if (hash->in_use < hash->table_size * HASH_TABLE_MIN_LOAD_FACTOR)
		rehash (hash);
	return count;
}

void
mono_g_hash_table_destroy (MonoGHashTable *hash)
{
	int i;

	g_return_if_fail (hash != NULL);

#ifdef HAVE_SGEN_GC
	if (hash->gc_type & MONO_HASH_KEY_GC)
		mono_gc_deregister_root ((char*)hash->keys);
	if (hash->gc_type & MONO_HASH_VALUE_GC)
		mono_gc_deregister_root ((char*)hash->values);
#endif

	for (i = 0; i < hash->table_size; i++) {
		if (hash->keys [i]) {
			if (hash->key_destroy_func)
				(*hash->key_destroy_func)(hash->keys [i]);
			if (hash->value_destroy_func)
				(*hash->value_destroy_func)(hash->values [i]);
		}
	}
	mg_free (hash->keys);
	mg_free (hash->values);
#ifdef HAVE_SGEN_GC
	mg_free (hash);
#else
	mono_gc_free_fixed (hash);
#endif
}

static void
mono_g_hash_table_insert_replace (MonoGHashTable *hash, gpointer key, gpointer value, gboolean replace)
{
	int slot;
	g_return_if_fail (hash != NULL);

	if (hash->in_use > (hash->table_size * HASH_TABLE_MAX_LOAD_FACTOR))
		rehash (hash);

	slot = mono_g_hash_table_find_slot (hash, key);

	if (hash->keys [slot]) {
		if (replace) {
			if (hash->key_destroy_func)
				(*hash->key_destroy_func)(hash->keys [slot]);
			mono_g_hash_table_key_store (hash, slot, (MonoObject*)key);
		}
		if (hash->value_destroy_func)
			(*hash->value_destroy_func) (hash->values [slot]);
		mono_g_hash_table_value_store (hash, slot, (MonoObject*)value);
	} else {
		mono_g_hash_table_key_store (hash, slot, (MonoObject*)key);
		mono_g_hash_table_value_store (hash, slot, (MonoObject*)value);
		hash->in_use++;
	}
}

void
mono_g_hash_table_insert (MonoGHashTable *h, gpointer k, gpointer v)
{
	mono_g_hash_table_insert_replace (h, k, v, FALSE);
}

void
mono_g_hash_table_replace(MonoGHashTable *h, gpointer k, gpointer v)
{
	mono_g_hash_table_insert_replace (h, k, v, TRUE);
}

void
mono_g_hash_table_print_stats (MonoGHashTable *hash)
{
	int i = 0, chain_size = 0, max_chain_size = 0;
	gboolean wrapped_around = FALSE;

	while (TRUE) {
		if (hash->keys [i]) {
			chain_size++;
		} else {
			max_chain_size = MAX(max_chain_size, chain_size);
			chain_size = 0;
			if (wrapped_around)
				break;
		}

		if (i == (hash->table_size - 1)) {
			wrapped_around = TRUE;
			i = 0;
		} else {
			i++;
		}
	}
	/* Rehash to a size that can fit the current elements */
	printf ("Size: %d Table Size: %d Max Chain Length: %d\n", hash->in_use, hash->table_size, max_chain_size);
}
