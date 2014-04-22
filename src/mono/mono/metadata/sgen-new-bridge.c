/*
 * sgen-bridge.c: Simple generational GC.
 *
 * Copyright 2011 Novell, Inc (http://www.novell.com)
 * Copyright 2011 Xamarin Inc (http://www.xamarin.com)
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 *
 * Copyright 2001-2003 Ximian, Inc
 * Copyright 2003-2010 Novell, Inc.
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

#include "config.h"

#ifdef HAVE_SGEN_GC

#include <stdlib.h>

#include "sgen-gc.h"
#include "sgen-bridge.h"
#include "sgen-hash-table.h"
#include "sgen-qsort.h"
#include "tabledefs.h"
#include "utils/mono-logger-internal.h"
#include "utils/mono-time.h"
#include "utils/mono-compiler.h"

#define NEW_XREFS
#ifdef NEW_XREFS
//#define TEST_NEW_XREFS
#endif

#if !defined(NEW_XREFS) || defined(TEST_NEW_XREFS)
#define OLD_XREFS
#endif

#ifdef NEW_XREFS
#define XREFS new_xrefs
#else
#define XREFS old_xrefs
#endif

typedef struct {
	int size;
	int capacity;
	char *data;
} DynArray;

/*Specializations*/

typedef struct {
	DynArray array;
} DynIntArray;

typedef struct {
	DynArray array;
} DynPtrArray;

typedef struct {
	DynArray array;
} DynSCCArray;


/*
 * FIXME: Optimizations:
 *
 * Don't allocate a scrs array for just one source.  Most objects have
 * just one source, so use the srcs pointer itself.
 */
typedef struct _HashEntry {
	MonoObject *obj;	/* This is a duplicate - it's already stored in the hash table */

	gboolean is_bridge;
	gboolean is_visited;

	int finishing_time;

	DynPtrArray srcs;

	int scc_index;
} HashEntry;

typedef struct {
	HashEntry entry;
	double weight;
} HashEntryWithAccounting;

typedef struct _SCC {
	int index;
	int api_index;
	int num_bridge_entries;
	/*
	 * New and old xrefs are typically mutually exclusive.  Only when TEST_NEW_XREFS is
	 * enabled we do both, and compare the results.  This should only be done for
	 * debugging, obviously.
	 */
#ifdef OLD_XREFS
	DynIntArray old_xrefs;		/* these are incoming, not outgoing */
#endif
#ifdef NEW_XREFS
	gboolean flag;
	DynIntArray new_xrefs;
#endif
} SCC;

static SgenHashTable hash_table = SGEN_HASH_TABLE_INIT (INTERNAL_MEM_BRIDGE_HASH_TABLE, INTERNAL_MEM_BRIDGE_HASH_TABLE_ENTRY, sizeof (HashEntry), mono_aligned_addr_hash, NULL);

static int current_time;

static gboolean bridge_accounting_enabled = FALSE;


/* Core functions */
/* public */

/* private */

static void
dyn_array_init (DynArray *da)
{
	da->size = 0;
	da->capacity = 0;
	da->data = NULL;
}

static void
dyn_array_uninit (DynArray *da, int elem_size)
{
	if (da->capacity <= 0)
		return;

	sgen_free_internal_dynamic (da->data, elem_size * da->capacity, INTERNAL_MEM_BRIDGE_DATA);
	da->data = NULL;
}

static void
dyn_array_ensure_capacity (DynArray *da, int capacity, int elem_size)
{
	int old_capacity = da->capacity;
	char *new_data;

	if (capacity <= old_capacity)
		return;

	if (da->capacity == 0)
		da->capacity = 2;
	while (capacity > da->capacity)
		da->capacity *= 2;

	new_data = sgen_alloc_internal_dynamic (elem_size * da->capacity, INTERNAL_MEM_BRIDGE_DATA, TRUE);
	memcpy (new_data, da->data, elem_size * da->size);
	sgen_free_internal_dynamic (da->data, elem_size * old_capacity, INTERNAL_MEM_BRIDGE_DATA);
	da->data = new_data;
}

static void*
dyn_array_add (DynArray *da, int elem_size)
{
	void *p;

	dyn_array_ensure_capacity (da, da->size + 1, elem_size);

	p = da->data + da->size * elem_size;
	++da->size;
	return p;
}

/* int */
static void
dyn_array_int_init (DynIntArray *da)
{
	dyn_array_init (&da->array);
}

static void
dyn_array_int_uninit (DynIntArray *da)
{
	dyn_array_uninit (&da->array, sizeof (int));
}

static int
dyn_array_int_size (DynIntArray *da)
{
	return da->array.size;
}

static void
dyn_array_int_set_size (DynIntArray *da, int size)
{
	da->array.size = size;
}

static void
dyn_array_int_add (DynIntArray *da, int x)
{
	int *p = dyn_array_add (&da->array, sizeof (int));
	*p = x;
}

static int
dyn_array_int_get (DynIntArray *da, int x)
{
	return ((int*)da->array.data)[x];
}

static void
dyn_array_int_set (DynIntArray *da, int idx, int val)
{
	((int*)da->array.data)[idx] = val;
}

static void
dyn_array_int_ensure_capacity (DynIntArray *da, int capacity)
{
	dyn_array_ensure_capacity (&da->array, capacity, sizeof (int));
}

static void
dyn_array_int_set_all (DynIntArray *dst, DynIntArray *src)
{
	dyn_array_int_ensure_capacity (dst, src->array.size);
	memcpy (dst->array.data, src->array.data, src->array.size * sizeof (int));
	dst->array.size = src->array.size;
}

/* ptr */

static void
dyn_array_ptr_init (DynPtrArray *da)
{
	dyn_array_init (&da->array);
}

static void
dyn_array_ptr_uninit (DynPtrArray *da)
{
	dyn_array_uninit (&da->array, sizeof (void*));
}

static int
dyn_array_ptr_size (DynPtrArray *da)
{
	return da->array.size;
}

static void
dyn_array_ptr_set_size (DynPtrArray *da, int size)
{
	da->array.size = size;
}

static void*
dyn_array_ptr_get (DynPtrArray *da, int x)
{
	return ((void**)da->array.data)[x];
}

static void
dyn_array_ptr_add (DynPtrArray *da, void *ptr)
{
	void **p = dyn_array_add (&da->array, sizeof (void*));
	*p = ptr;
}

#define dyn_array_ptr_push dyn_array_ptr_add

static void*
dyn_array_ptr_pop (DynPtrArray *da)
{
	void *p;
	int size = da->array.size;
	g_assert (size > 0);
	p = dyn_array_ptr_get (da, size - 1);
	--da->array.size;
	return p;
}

/*SCC */

static void
dyn_array_scc_init (DynSCCArray *da)
{
	dyn_array_init (&da->array);
}

static void
dyn_array_scc_uninit (DynSCCArray *da)
{
	dyn_array_uninit (&da->array, sizeof (SCC));
}

static int
dyn_array_scc_size (DynSCCArray *da)
{
	return da->array.size;
}

static SCC*
dyn_array_scc_add (DynSCCArray *da)
{
	return dyn_array_add (&da->array, sizeof (SCC));
}

static SCC*
dyn_array_scc_get_ptr (DynSCCArray *da, int x)
{
	return &((SCC*)da->array.data)[x];
}

/* Merge code*/

static DynIntArray merge_array;

static gboolean
dyn_array_int_contains (DynIntArray *da, int x)
{
	int i;
	for (i = 0; i < dyn_array_int_size (da); ++i)
		if (dyn_array_int_get (da, i) == x)
			return TRUE;
	return FALSE;
}


static void
dyn_array_int_merge (DynIntArray *dst, DynIntArray *src)
{
	int i, j;

	dyn_array_int_ensure_capacity (&merge_array, dyn_array_int_size (dst) + dyn_array_int_size (src));
	dyn_array_int_set_size (&merge_array, 0);

	for (i = j = 0; i < dyn_array_int_size (dst) || j < dyn_array_int_size (src); ) {
		if (i < dyn_array_int_size (dst) && j < dyn_array_int_size (src)) {
			int a = dyn_array_int_get (dst, i); 
			int b = dyn_array_int_get (src, j); 
			if (a < b) {
				dyn_array_int_add (&merge_array, a);
				++i;
			} else if (a == b) {
				dyn_array_int_add (&merge_array, a);
				++i;
				++j;	
			} else {
				dyn_array_int_add (&merge_array, b);
				++j;
			}
		} else if (i < dyn_array_int_size (dst)) {
			dyn_array_int_add (&merge_array, dyn_array_int_get (dst, i));
			++i;
		} else {
			dyn_array_int_add (&merge_array, dyn_array_int_get (src, j));
			++j;
		}
	}

	if (dyn_array_int_size (&merge_array) > dyn_array_int_size (dst)) {
		dyn_array_int_set_all (dst, &merge_array);
	}
}

static void
dyn_array_int_merge_one (DynIntArray *array, int value)
{
	int i;
	int tmp;
	int size = dyn_array_int_size (array);

	for (i = 0; i < size; ++i) {
		if (dyn_array_int_get (array, i) == value)
			return;
		else if (dyn_array_int_get (array, i) > value)
			break;
	}

	dyn_array_int_ensure_capacity (array, size + 1);

	if (i < size) {
		tmp = dyn_array_int_get (array, i);
		for (; i < size; ++i) {
			dyn_array_int_set (array, i, value);
			value = tmp;
			tmp = dyn_array_int_get (array, i + 1);
		}
		dyn_array_int_set (array, size, value);
	} else {
		dyn_array_int_set (array, size, value);
	}

	dyn_array_int_set_size (array, size + 1);
}


static void
enable_accounting (void)
{
	bridge_accounting_enabled = TRUE;
	hash_table = (SgenHashTable)SGEN_HASH_TABLE_INIT (INTERNAL_MEM_BRIDGE_HASH_TABLE, INTERNAL_MEM_BRIDGE_HASH_TABLE_ENTRY, sizeof (HashEntryWithAccounting), mono_aligned_addr_hash, NULL);
}

static MonoGCBridgeObjectKind
class_kind (MonoClass *class)
{
	MonoGCBridgeObjectKind res = bridge_callbacks.bridge_class_kind (class);

	/* If it's a bridge, nothing we can do about it. */
	if (res == GC_BRIDGE_TRANSPARENT_BRIDGE_CLASS || res == GC_BRIDGE_OPAQUE_BRIDGE_CLASS)
		return res;

	/* Non bridge classes with no pointers will never point to a bridge, so we can savely ignore them. */
	if (!class->has_references) {
		SGEN_LOG (6, "class %s is opaque\n", class->name);
		return GC_BRIDGE_OPAQUE_CLASS;
	}

	/* Some arrays can be ignored */
	if (class->rank == 1) {
		MonoClass *elem_class = class->element_class;

		/* FIXME the bridge check can be quite expensive, cache it at the class level. */
		/* An array of a sealed type that is not a bridge will never get to a bridge */
		if ((elem_class->flags & TYPE_ATTRIBUTE_SEALED) && !elem_class->has_references && !bridge_callbacks.bridge_class_kind (elem_class)) {
			SGEN_LOG (6, "class %s is opaque\n", class->name);
			return GC_BRIDGE_OPAQUE_CLASS;
		}
	}

	return GC_BRIDGE_TRANSPARENT_CLASS;
}

static HashEntry*
get_hash_entry (MonoObject *obj, gboolean *existing)
{
	HashEntry *entry = sgen_hash_table_lookup (&hash_table, obj);
	HashEntry new_entry;

	if (entry) {
		if (existing)
			*existing = TRUE;
		return entry;
	}
	if (existing)
		*existing = FALSE;

	memset (&new_entry, 0, sizeof (HashEntry));

	new_entry.obj = obj;
	dyn_array_ptr_init (&new_entry.srcs);
	new_entry.finishing_time = -1;
	new_entry.scc_index = -1;

	sgen_hash_table_replace (&hash_table, obj, &new_entry, NULL);

	return sgen_hash_table_lookup (&hash_table, obj);
}

static void
add_source (HashEntry *entry, HashEntry *src)
{
	dyn_array_ptr_add (&entry->srcs, src);
}

static void
free_data (void)
{
	MonoObject *obj;
	HashEntry *entry;
	int total_srcs = 0;
	int max_srcs = 0;

	SGEN_HASH_TABLE_FOREACH (&hash_table, obj, entry) {
		int entry_size = dyn_array_ptr_size (&entry->srcs);
		total_srcs += entry_size;
		if (entry_size > max_srcs)
			max_srcs = entry_size;
		dyn_array_ptr_uninit (&entry->srcs);
	} SGEN_HASH_TABLE_FOREACH_END;

	sgen_hash_table_clean (&hash_table);

	dyn_array_int_uninit (&merge_array);
	//g_print ("total srcs %d - max %d\n", total_srcs, max_srcs);
}

static HashEntry*
register_bridge_object (MonoObject *obj)
{
	HashEntry *entry = get_hash_entry (obj, NULL);
	entry->is_bridge = TRUE;
	return entry;
}

static void
register_finishing_time (HashEntry *entry, int t)
{
	g_assert (entry->finishing_time < 0);
	entry->finishing_time = t;
}

static int ignored_objects;

static gboolean
is_opaque_object (MonoObject *obj)
{
	if ((obj->vtable->gc_bits & SGEN_GC_BIT_BRIDGE_OPAQUE_OBJECT) == SGEN_GC_BIT_BRIDGE_OPAQUE_OBJECT) {
		SGEN_LOG (6, "ignoring %s\n", obj->vtable->klass->name);
		++ignored_objects;
		return TRUE;
	}
	return FALSE;
}

static gboolean
object_is_live (MonoObject **objp)
{
	MonoObject *obj = *objp;
	MonoObject *fwd = SGEN_OBJECT_IS_FORWARDED (obj);
	if (fwd) {
		*objp = fwd;
		if (is_opaque_object (fwd))
			return TRUE;
		return sgen_hash_table_lookup (&hash_table, fwd) == NULL;
	}
	if (is_opaque_object (obj))
		return TRUE;
	if (!sgen_object_is_live (obj))
		return FALSE;
	return sgen_hash_table_lookup (&hash_table, obj) == NULL;
}

static DynPtrArray registered_bridges;
static DynPtrArray dfs_stack;

static int dfs1_passes, dfs2_passes;


#undef HANDLE_PTR
#define HANDLE_PTR(ptr,obj)	do {					\
		MonoObject *dst = (MonoObject*)*(ptr);			\
		if (dst && !object_is_live (&dst)) {			\
			dyn_array_ptr_push (&dfs_stack, obj_entry);	\
			dyn_array_ptr_push (&dfs_stack, get_hash_entry (dst, NULL)); \
		}							\
	} while (0)

static void
dfs1 (HashEntry *obj_entry)
{
	HashEntry *src;
	g_assert (dyn_array_ptr_size (&dfs_stack) == 0);

	dyn_array_ptr_push (&dfs_stack, NULL);
	dyn_array_ptr_push (&dfs_stack, obj_entry);

	do {
		MonoObject *obj;
		char *start;
		++dfs1_passes;

		obj_entry = dyn_array_ptr_pop (&dfs_stack);
		if (obj_entry) {
			src = dyn_array_ptr_pop (&dfs_stack);

			obj = obj_entry->obj;
			start = (char*)obj;

			if (src) {
				//g_print ("link %s -> %s\n", sgen_safe_name (src->obj), sgen_safe_name (obj));
				add_source (obj_entry, src);
			} else {
				//g_print ("starting with %s\n", sgen_safe_name (obj));
			}

			if (obj_entry->is_visited)
				continue;

			obj_entry->is_visited = TRUE;

			dyn_array_ptr_push (&dfs_stack, obj_entry);
			/* NULL marks that the next entry is to be finished */
			dyn_array_ptr_push (&dfs_stack, NULL);

#include "sgen-scan-object.h"
		} else {
			obj_entry = dyn_array_ptr_pop (&dfs_stack);

			//g_print ("finish %s\n", sgen_safe_name (obj_entry->obj));
			register_finishing_time (obj_entry, current_time++);
		}
	} while (dyn_array_ptr_size (&dfs_stack) > 0);
}

/*
 * At the end of bridge processing we need to end up with an (acyclyc) graph of bridge
 * object SCCs, where the links between the nodes (each one an SCC) in that graph represent
 * the presence of a direct or indirect link between those SCCs.  An example:
 *
 *                       D
 *                       |
 *                       v
 *        A -> B -> c -> e -> F
 *
 * A, B, D and F are SCCs that contain bridge objects, c and e don't contain bridge objects.
 * The graph we need to produce from this is:
 *
 *                  D
 *                  |
 *                  v
 *        A -> B -> F
 *
 * Note that we don't need to produce an edge from A to F.  It's sufficient that F is
 * indirectly reachable from A.
 *
 * The old algorithm would create a set, for each SCC, of bridge SCCs that can reach it,
 * directly or indirectly, by merging the ones sets for those that reach it directly.  The
 * sets it would build up are these:
 *
 *   A: {}
 *   B: {A}
 *   c: {B}
 *   D: {}
 *   e: {B,D}
 *   F: {B,D}
 *
 * The merge operations on these sets turned out to be huge time sinks.
 *
 * The new algorithm proceeds in two passes: During DFS2, it only builds up the sets of SCCs
 * that directly point to each SCC:
 *
 *   A: {}
 *   B: {A}
 *   c: {B}
 *   D: {}
 *   e: {c,D}
 *   F: {e}
 *
 * This is the adjacency list for the SCC graph, in other words.  In a separate step
 * afterwards, it does a depth-first traversal of that graph, for each bridge node, to get
 * to the final list.  It uses a flag to avoid traversing any node twice.
 */
static void
scc_add_xref (SCC *src, SCC *dst)
{
	g_assert (src != dst);
	g_assert (src->index != dst->index);

#ifdef NEW_XREFS
	/*
	 * FIXME: Right now we don't even unique the direct ancestors, but just add to the
	 * list.  Doing a containment check slows this algorithm down to almost the speed of
	 * the old one.  Use the flag instead!
	 */
	dyn_array_int_add (&dst->new_xrefs, src->index);
#endif

#ifdef OLD_XREFS
	if (dyn_array_int_contains (&dst->old_xrefs, src->index))
		return;
	if (src->num_bridge_entries) {
		dyn_array_int_merge_one (&dst->old_xrefs, src->index);
	} else {
		int i;
		dyn_array_int_merge (&dst->old_xrefs, &src->old_xrefs);
		for (i = 0; i < dyn_array_int_size (&dst->old_xrefs); ++i)
			g_assert (dyn_array_int_get (&dst->old_xrefs, i) != dst->index);
	}
#endif
}

static void
scc_add_entry (SCC *scc, HashEntry *entry)
{
	g_assert (entry->scc_index < 0);
	entry->scc_index = scc->index;
	if (entry->is_bridge)
		++scc->num_bridge_entries;
}

static DynSCCArray sccs;
static SCC *current_scc;

static void
dfs2 (HashEntry *entry)
{
	int i;

	g_assert (dyn_array_ptr_size (&dfs_stack) == 0);

	dyn_array_ptr_push (&dfs_stack, entry);

	do {
		entry = dyn_array_ptr_pop (&dfs_stack);
		++dfs2_passes;

		if (entry->scc_index >= 0) {
			if (entry->scc_index != current_scc->index)
				scc_add_xref (dyn_array_scc_get_ptr (&sccs, entry->scc_index), current_scc);
			continue;
		}

		scc_add_entry (current_scc, entry);

		for (i = 0; i < dyn_array_ptr_size (&entry->srcs); ++i)
			dyn_array_ptr_push (&dfs_stack, dyn_array_ptr_get (&entry->srcs, i));
	} while (dyn_array_ptr_size (&dfs_stack) > 0);
}

#ifdef NEW_XREFS
static void
gather_xrefs (SCC *scc)
{
	int i;
	for (i = 0; i < dyn_array_int_size (&scc->new_xrefs); ++i) {
		int index = dyn_array_int_get (&scc->new_xrefs, i);
		SCC *src = dyn_array_scc_get_ptr (&sccs, index);
		if (src->flag)
			continue;
		src->flag = TRUE;
		if (src->num_bridge_entries)
			dyn_array_int_add (&merge_array, index);
		else
			gather_xrefs (src);
	}
}

static void
reset_flags (SCC *scc)
{
	int i;
	for (i = 0; i < dyn_array_int_size (&scc->new_xrefs); ++i) {
		int index = dyn_array_int_get (&scc->new_xrefs, i);
		SCC *src = dyn_array_scc_get_ptr (&sccs, index);
		if (!src->flag)
			continue;
		src->flag = FALSE;
		if (!src->num_bridge_entries)
			reset_flags (src);
	}
}
#endif

static int
compare_hash_entries (const HashEntry *e1, const HashEntry *e2)
{
	return e2->finishing_time - e1->finishing_time;
}

DEF_QSORT_INLINE(hash_entries, HashEntry*, compare_hash_entries)

static unsigned long step_1, step_2, step_3, step_4, step_5, step_6, step_7, step_8;
static int fist_pass_links, second_pass_links, sccs_links;
static int max_sccs_links = 0;

static void
register_finalized_object (MonoObject *obj)
{
	g_assert (sgen_need_bridge_processing ());
	dyn_array_ptr_push (&registered_bridges, obj);
}

static void
reset_data (void)
{
	dyn_array_ptr_set_size (&registered_bridges, 0);
}

static void
processing_stw_step (void)
{
	int i;
	int bridge_count;
	SGEN_TV_DECLARE (atv);
	SGEN_TV_DECLARE (btv);

	if (!dyn_array_ptr_size (&registered_bridges))
		return;

	/*
	 * bridge_processing_in_progress must be set with the world
	 * stopped.  If not there would be race conditions.
	 */
	bridge_processing_in_progress = TRUE;

	SGEN_TV_GETTIME (btv);

	/* first DFS pass */

	dyn_array_ptr_init (&dfs_stack);
	dyn_array_int_init (&merge_array);

	current_time = 0;
	/*
	First we insert all bridges into the hash table and then we do dfs1.

	It must be done in 2 steps since the bridge arrays doesn't come in reverse topological order,
	which means that we can have entry N pointing to entry N + 1.

	If we dfs1 entry N before N + 1 is registered we'll not consider N + 1 for this bridge
	pass and not create the required xref between the two.
	*/
	bridge_count = dyn_array_ptr_size (&registered_bridges);
	for (i = 0; i < bridge_count ; ++i)
		register_bridge_object (dyn_array_ptr_get (&registered_bridges, i));

	for (i = 0; i < bridge_count; ++i)
		dfs1 (get_hash_entry (dyn_array_ptr_get (&registered_bridges, i), NULL));

	SGEN_TV_GETTIME (atv);
	step_2 = SGEN_TV_ELAPSED (btv, atv);
}

static mono_bool
is_bridge_object_alive (MonoObject *obj, void *data)
{
	SgenHashTable *table = data;
	unsigned char *value = sgen_hash_table_lookup (table, obj);
	if (!value)
		return TRUE;
	return *value;
}

static void
processing_finish (int generation)
{
	int i, j;
	int num_sccs, num_xrefs;
	int max_entries, max_xrefs;
	int hash_table_size, sccs_size;
	MonoObject *obj;
	HashEntry *entry;
	int num_registered_bridges;
	HashEntry **all_entries;
	MonoGCBridgeSCC **api_sccs;
	MonoGCBridgeXRef *api_xrefs;
	SgenHashTable alive_hash = SGEN_HASH_TABLE_INIT (INTERNAL_MEM_BRIDGE_ALIVE_HASH_TABLE, INTERNAL_MEM_BRIDGE_ALIVE_HASH_TABLE_ENTRY, 1, mono_aligned_addr_hash, NULL);
	SGEN_TV_DECLARE (atv);
	SGEN_TV_DECLARE (btv);

	if (!dyn_array_ptr_size (&registered_bridges))
		return;

	g_assert (bridge_processing_in_progress);

	SGEN_TV_GETTIME (atv);

	/* alloc and fill array of all entries */

	all_entries = sgen_alloc_internal_dynamic (sizeof (HashEntry*) * hash_table.num_entries, INTERNAL_MEM_BRIDGE_DATA, TRUE);

	j = 0;
	SGEN_HASH_TABLE_FOREACH (&hash_table, obj, entry) {
		g_assert (entry->finishing_time >= 0);
		all_entries [j++] = entry;
		fist_pass_links += dyn_array_ptr_size (&entry->srcs);
	} SGEN_HASH_TABLE_FOREACH_END;
	g_assert (j == hash_table.num_entries);
	hash_table_size = hash_table.num_entries;

	/* sort array according to decreasing finishing time */
	qsort_hash_entries (all_entries, hash_table.num_entries);

	SGEN_TV_GETTIME (btv);
	step_3 = SGEN_TV_ELAPSED (atv, btv);

	/* second DFS pass */

	dyn_array_scc_init (&sccs);
	for (i = 0; i < hash_table.num_entries; ++i) {
		HashEntry *entry = all_entries [i];
		if (entry->scc_index < 0) {
			int index = dyn_array_scc_size (&sccs);
			current_scc = dyn_array_scc_add (&sccs);
			current_scc->index = index;
			current_scc->num_bridge_entries = 0;
#ifdef NEW_XREFS
			current_scc->flag = FALSE;
			dyn_array_int_init (&current_scc->new_xrefs);
#endif
#ifdef OLD_XREFS
			dyn_array_int_init (&current_scc->old_xrefs);
#endif
			current_scc->api_index = -1;

			dfs2 (entry);

#ifdef NEW_XREFS
			/*
			 * If a node has only one incoming edge, we just copy the source's
			 * xrefs array, effectively removing the source from the graph.
			 * This takes care of long linked lists.
			 */
			if (!current_scc->num_bridge_entries && dyn_array_int_size (&current_scc->new_xrefs) == 1) {
				SCC *src;
				j = dyn_array_int_get (&current_scc->new_xrefs, 0);
				src = dyn_array_scc_get_ptr (&sccs, j);
				if (src->num_bridge_entries)
					dyn_array_int_set (&current_scc->new_xrefs, 0, j);
				else
					dyn_array_int_set_all (&current_scc->new_xrefs, &src->new_xrefs);
			}
#endif
		}
	}

#ifdef NEW_XREFS
#ifdef TEST_NEW_XREFS
	for (j = 0; j < dyn_array_scc_size (&sccs); ++j) {
		SCC *scc = dyn_array_scc_get_ptr (&sccs, j);
		g_assert (!scc->flag);
	}
#endif

	for (i = 0; i < dyn_array_scc_size (&sccs); ++i) {
		SCC *scc = dyn_array_scc_get_ptr (&sccs, i);
		g_assert (scc->index == i);
		if (!scc->num_bridge_entries)
			continue;

		dyn_array_int_set_size (&merge_array, 0);
		gather_xrefs (scc);
		reset_flags (scc);
		dyn_array_int_set_all (&scc->new_xrefs, &merge_array);

#ifdef TEST_NEW_XREFS
		for (j = 0; j < dyn_array_scc_size (&sccs); ++j) {
			SCC *scc = dyn_array_scc_get_ptr (&sccs, j);
			g_assert (!scc->flag);
		}
#endif
	}

#ifdef TEST_NEW_XREFS
	for (i = 0; i < dyn_array_scc_size (&sccs); ++i) {
		SCC *scc = dyn_array_scc_get_ptr (&sccs, i);
		g_assert (scc->index == i);
		if (!scc->num_bridge_entries)
			continue;

		g_assert (dyn_array_int_size (&scc->new_xrefs) == dyn_array_int_size (&scc->old_xrefs));
		for (j = 0; j < dyn_array_int_size (&scc->new_xrefs); ++j)
			g_assert (dyn_array_int_contains (&scc->old_xrefs, dyn_array_int_get (&scc->new_xrefs, j)));
	}
#endif
#endif

	/*
	 * Compute the weight of each object. The weight of an object is its size plus the size of all
	 * objects it points do. When the an object is pointed by multiple objects we distribute it's weight
	 * equally among them. This distribution gives a rough estimate of the real impact of making the object
	 * go away.
	 *
	 * The reasoning for this model is that complex graphs with single roots will have a bridge with very high
	 * value in comparison to others.
	 *
	 * The all_entries array has all objects topologically sorted. To correctly propagate the weights it must be
	 * done in reverse topological order - so we calculate the weight of the pointed-to objects before processing
	 * pointer-from objects.
	 *
	 * We log those objects in the opposite order for no particular reason. The other constrain is that it should use the same
	 * direction as the other logging loop that records live/dead information.
	 */
	if (bridge_accounting_enabled) {
		for (i = hash_table.num_entries - 1; i >= 0; --i) {
			double w;
			HashEntryWithAccounting *entry = (HashEntryWithAccounting*)all_entries [i];

			entry->weight += (double)sgen_safe_object_get_size (entry->entry.obj);
			w = entry->weight / dyn_array_ptr_size (&entry->entry.srcs);
			for (j = 0; j < dyn_array_ptr_size (&entry->entry.srcs); ++j) {
				HashEntryWithAccounting *other = (HashEntryWithAccounting *)dyn_array_ptr_get (&entry->entry.srcs, j);
				other->weight += w;
			}
		}
		for (i = 0; i < hash_table.num_entries; ++i) {
			HashEntryWithAccounting *entry = (HashEntryWithAccounting*)all_entries [i];
			if (entry->entry.is_bridge) {
				MonoClass *klass = ((MonoVTable*)SGEN_LOAD_VTABLE (entry->entry.obj))->klass;
				mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_GC, "OBJECT %s::%s (%p) weight %f", klass->name_space, klass->name, entry->entry.obj, entry->weight);
			}
		}
	}

	sccs_size = dyn_array_scc_size (&sccs);

	for (i = 0; i < hash_table.num_entries; ++i) {
		HashEntry *entry = all_entries [i];
		second_pass_links += dyn_array_ptr_size (&entry->srcs);
	}

	SGEN_TV_GETTIME (atv);
	step_4 = SGEN_TV_ELAPSED (btv, atv);

	//g_print ("%d sccs\n", sccs.size);

	dyn_array_ptr_uninit (&dfs_stack);

	/* init data for callback */

	num_sccs = 0;
	for (i = 0; i < dyn_array_scc_size (&sccs); ++i) {
		SCC *scc = dyn_array_scc_get_ptr (&sccs, i);
		g_assert (scc->index == i);
		if (scc->num_bridge_entries)
			++num_sccs;
		sccs_links += dyn_array_int_size (&scc->XREFS);
		max_sccs_links = MAX (max_sccs_links, dyn_array_int_size (&scc->XREFS));
	}

	api_sccs = sgen_alloc_internal_dynamic (sizeof (MonoGCBridgeSCC*) * num_sccs, INTERNAL_MEM_BRIDGE_DATA, TRUE);
	num_xrefs = 0;
	j = 0;
	for (i = 0; i < dyn_array_scc_size (&sccs); ++i) {
		SCC *scc = dyn_array_scc_get_ptr (&sccs, i);
		if (!scc->num_bridge_entries)
			continue;

		api_sccs [j] = sgen_alloc_internal_dynamic (sizeof (MonoGCBridgeSCC) + sizeof (MonoObject*) * scc->num_bridge_entries, INTERNAL_MEM_BRIDGE_DATA, TRUE);
		api_sccs [j]->is_alive = FALSE;
		api_sccs [j]->num_objs = scc->num_bridge_entries;
		scc->num_bridge_entries = 0;
		scc->api_index = j++;

		num_xrefs += dyn_array_int_size (&scc->XREFS);
	}

	SGEN_HASH_TABLE_FOREACH (&hash_table, obj, entry) {
		if (entry->is_bridge) {
			SCC *scc = dyn_array_scc_get_ptr (&sccs, entry->scc_index);
			api_sccs [scc->api_index]->objs [scc->num_bridge_entries++] = entry->obj;
		}
	} SGEN_HASH_TABLE_FOREACH_END;

	api_xrefs = sgen_alloc_internal_dynamic (sizeof (MonoGCBridgeXRef) * num_xrefs, INTERNAL_MEM_BRIDGE_DATA, TRUE);
	j = 0;
	for (i = 0; i < dyn_array_scc_size (&sccs); ++i) {
		int k;
		SCC *scc = dyn_array_scc_get_ptr (&sccs, i);
		if (!scc->num_bridge_entries)
			continue;
		for (k = 0; k < dyn_array_int_size (&scc->XREFS); ++k) {
			SCC *src_scc = dyn_array_scc_get_ptr (&sccs, dyn_array_int_get (&scc->XREFS, k));
			if (!src_scc->num_bridge_entries)
				continue;
			api_xrefs [j].src_scc_index = src_scc->api_index;
			api_xrefs [j].dst_scc_index = scc->api_index;
			++j;
		}
	}

	SGEN_TV_GETTIME (btv);
	step_5 = SGEN_TV_ELAPSED (atv, btv);

	/* free data */

	j = 0;
	max_entries = max_xrefs = 0;
	for (i = 0; i < dyn_array_scc_size (&sccs); ++i) {
		SCC *scc = dyn_array_scc_get_ptr (&sccs, i);
		if (scc->num_bridge_entries)
			++j;
		if (scc->num_bridge_entries > max_entries)
			max_entries = scc->num_bridge_entries;
		if (dyn_array_int_size (&scc->XREFS) > max_xrefs)
			max_xrefs = dyn_array_int_size (&scc->XREFS);
#ifdef NEW_XREFS
		dyn_array_int_uninit (&scc->new_xrefs);
#endif
#ifdef OLD_XREFS
		dyn_array_int_uninit (&scc->old_xrefs);
#endif

	}
	dyn_array_scc_uninit (&sccs);

	sgen_free_internal_dynamic (all_entries, sizeof (HashEntry*) * hash_table.num_entries, INTERNAL_MEM_BRIDGE_DATA);

	free_data ();
	/* Empty the registered bridges array */
	num_registered_bridges = dyn_array_ptr_size (&registered_bridges);
	dyn_array_ptr_set_size (&registered_bridges, 0);

	SGEN_TV_GETTIME (atv);
	step_6 = SGEN_TV_ELAPSED (btv, atv);

	//g_print ("%d sccs containing bridges - %d max bridge objects - %d max xrefs\n", j, max_entries, max_xrefs);

	/* callback */

	bridge_callbacks.cross_references (num_sccs, api_sccs, num_xrefs, api_xrefs);

	/* Release for finalization those objects we no longer care. */
	SGEN_TV_GETTIME (btv);
	step_7 = SGEN_TV_ELAPSED (atv, btv);

	for (i = 0; i < num_sccs; ++i) {
		unsigned char alive = api_sccs [i]->is_alive ? 1 : 0;
		for (j = 0; j < api_sccs [i]->num_objs; ++j) {
			/* Build hash table for nulling weak links. */
			sgen_hash_table_replace (&alive_hash, api_sccs [i]->objs [j], &alive, NULL);

			/* Release for finalization those objects we no longer care. */
			if (!api_sccs [i]->is_alive)
				sgen_mark_bridge_object (api_sccs [i]->objs [j]);
		}
	}

	/* Null weak links to dead objects. */
	sgen_null_links_with_predicate (GENERATION_NURSERY, is_bridge_object_alive, &alive_hash);
	if (generation == GENERATION_OLD)
		sgen_null_links_with_predicate (GENERATION_OLD, is_bridge_object_alive, &alive_hash);

	sgen_hash_table_clean (&alive_hash);

	if (bridge_accounting_enabled) {
		for (i = 0; i < num_sccs; ++i) {
			for (j = 0; j < api_sccs [i]->num_objs; ++j)
				mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_GC,
					"OBJECT %s (%p) SCC [%d] %s",
						sgen_safe_name (api_sccs [i]->objs [j]), api_sccs [i]->objs [j],
						i,
						api_sccs [i]->is_alive  ? "ALIVE" : "DEAD");
		}
	}

	/* free callback data */

	for (i = 0; i < num_sccs; ++i) {
		sgen_free_internal_dynamic (api_sccs [i],
				sizeof (MonoGCBridgeSCC) + sizeof (MonoObject*) * api_sccs [i]->num_objs,
				INTERNAL_MEM_BRIDGE_DATA);
	}
	sgen_free_internal_dynamic (api_sccs, sizeof (MonoGCBridgeSCC*) * num_sccs, INTERNAL_MEM_BRIDGE_DATA);

	sgen_free_internal_dynamic (api_xrefs, sizeof (MonoGCBridgeXRef) * num_xrefs, INTERNAL_MEM_BRIDGE_DATA);

	SGEN_TV_GETTIME (atv);
	step_8 = SGEN_TV_ELAPSED (btv, atv);

	mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_GC, "GC_BRIDGE num-objects %d num_hash_entries %d sccs size %d init %.2fms df1 %.2fms sort %.2fms dfs2 %.2fms setup-cb %.2fms free-data %.2fms user-cb %.2fms clenanup %.2fms links %d/%d/%d/%d dfs passes %d/%d ignored %d",
		num_registered_bridges, hash_table_size, dyn_array_scc_size (&sccs),
		step_1 / 10000.0f,
		step_2 / 10000.0f,
		step_3 / 10000.0f,
		step_4 / 10000.0f,
		step_5 / 10000.0f,
		step_6 / 10000.0f,
		step_7 / 10000.0f,
		step_8 / 10000.f,
		fist_pass_links, second_pass_links, sccs_links, max_sccs_links,
		dfs1_passes, dfs2_passes, ignored_objects);

	step_1 = 0; /* We must cleanup since this value is used as an accumulator. */
	fist_pass_links = second_pass_links = sccs_links = max_sccs_links = 0;
	dfs1_passes = dfs2_passes = ignored_objects = 0;

	bridge_processing_in_progress = FALSE;
}

static void
describe_pointer (MonoObject *obj)
{
	HashEntry *entry;
	int i;

	for (i = 0; i < dyn_array_ptr_size (&registered_bridges); ++i) {
		if (obj == dyn_array_ptr_get (&registered_bridges, i)) {
			printf ("Pointer is a registered bridge object.\n");
			break;
		}
	}

	entry = sgen_hash_table_lookup (&hash_table, obj);
	if (!entry)
		return;

	printf ("Bridge hash table entry %p:\n", entry);
	printf ("  is bridge: %d\n", (int)entry->is_bridge);
	printf ("  is visited: %d\n", (int)entry->is_visited);
}

void
sgen_new_bridge_init (SgenBridgeProcessor *collector)
{
	collector->reset_data = reset_data;
	collector->processing_stw_step = processing_stw_step;
	collector->processing_finish = processing_finish;
	collector->class_kind = class_kind;
	collector->register_finalized_object = register_finalized_object;
	collector->describe_pointer = describe_pointer;
	collector->enable_accounting = enable_accounting;
}

#endif
