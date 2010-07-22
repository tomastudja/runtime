/*
 * sgen-gc.c: Simple generational GC.
 *
 * Author:
 * 	Paolo Molaro (lupus@ximian.com)
 *
 * Copyright 2005-2010 Novell, Inc (http://www.novell.com)
 *
 * Thread start/stop adapted from Boehm's GC:
 * Copyright (c) 1994 by Xerox Corporation.  All rights reserved.
 * Copyright (c) 1996 by Silicon Graphics.  All rights reserved.
 * Copyright (c) 1998 by Fergus Henderson.  All rights reserved.
 * Copyright (c) 2000-2004 by Hewlett-Packard Company.  All rights reserved.
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

#include "metadata/sgen-gc.h"

/* Pinned objects are allocated in the LOS space if bigger than half a page
 * or from freelists otherwise. We assume that pinned objects are relatively few
 * and they have a slow dying speed (like interned strings, thread objects).
 * As such they will be collected only at major collections.
 * free lists are not global: when we need memory we allocate a PinnedChunk.
 * Each pinned chunk is made of several pages, the first of wich is used
 * internally for bookeeping (here think of a page as 4KB). The bookeeping
 * includes the freelists vectors and info about the object size of each page
 * in the pinned chunk. So, when needed, a free page is found in a pinned chunk,
 * a size is assigned to it, the page is divided in the proper chunks and each
 * chunk is added to the freelist. To not waste space, the remaining space in the
 * first page is used as objects of size 16 or 32 (need to measure which are more
 * common).
 * We use this same structure to allocate memory used internally by the GC, so
 * we never use malloc/free if we need to alloc during collection: the world is stopped
 * and malloc/free will deadlock.
 * When we want to iterate over pinned objects, we just scan a page at a time
 * linearly according to the size of objects in the page: the next pointer used to link
 * the items in the freelist uses the same word as the vtable. Since we keep freelists
 * for each pinned chunk, if the word points outside the pinned chunk it means
 * it is an object.
 * We could avoid this expensive scanning in creative ways. We could have a policy
 * of putting in the pinned space only objects we know about that have no struct fields
 * with references and we can easily use a even expensive write barrier for them,
 * since pointer writes on such objects should be rare.
 * The best compromise is to just alloc interned strings and System.MonoType in them.
 * It would be nice to allocate MonoThread in it, too: must check that we properly
 * use write barriers so we don't have to do any expensive scanning of the whole pinned
 * chunk list during minor collections. We can avoid it now because we alloc in it only
 * reference-free objects.
 */
struct _SgenPinnedChunk {
	SgenBlock block;
	int num_pages;
	int *page_sizes; /* a 0 means the page is still unused */
	void **free_list;
	void *start_data;
	void *data [1]; /* page sizes and free lists are stored here */
};

#define PINNED_FIRST_SLOT_SIZE (sizeof (gpointer) * 4)
#define MAX_FREELIST_SIZE 8192

/* This is a fixed value used for pinned chunks, not the system pagesize */
#define FREELIST_PAGESIZE (16*1024)

/* keep each size a multiple of ALLOC_ALIGN */
/* on 64 bit systems 8 is likely completely unused. */
static const int freelist_sizes [] = {
	8, 16, 24, 32, 40, 48, 64, 80,
	96, 128, 160, 192, 224, 256, 320, 384,
	448, 512, 584, 680, 816, 1024, 1360, 2048,
	2336, 2728, 3272, 4096, 5456, 8192 };
#define FREELIST_NUM_SLOTS (sizeof (freelist_sizes) / sizeof (freelist_sizes [0]))

/* This is also the MAJOR_SECTION_SIZE for the copying major
   collector */
#define PINNED_CHUNK_SIZE	(128 * 1024)

static SgenInternalAllocator unmanaged_allocator;

#define LARGE_INTERNAL_MEM_HEADER_MAGIC	0x7d289f3a

typedef struct _LargeInternalMemHeader LargeInternalMemHeader;
struct _LargeInternalMemHeader {
	guint32 magic;
	size_t size;
	double data[0];
};

#ifdef SGEN_PARALLEL_MARK
static LOCK_DECLARE (internal_allocator_mutex);
#define LOCK_INTERNAL_ALLOCATOR pthread_mutex_lock (&internal_allocator_mutex)
#define UNLOCK_INTERNAL_ALLOCATOR pthread_mutex_unlock (&internal_allocator_mutex)
#else
#define LOCK_INTERNAL_ALLOCATOR
#define UNLOCK_INTERNAL_ALLOCATOR
#endif

static long long pinned_chunk_bytes_alloced = 0;
static long long large_internal_bytes_alloced = 0;

#ifdef HEAVY_STATISTICS
static long long stat_internal_alloc = 0;
static long long stat_internal_alloc_loop1 = 0;
static long long stat_internal_alloc_loop2 = 0;
#endif

/*
 * Debug reporting.
 */
static void
report_pinned_chunk (SgenPinnedChunk *chunk, int seq) {
	void **p;
	int i, free_pages, num_free, free_mem;
	free_pages = 0;
	for (i = 0; i < chunk->num_pages; ++i) {
		if (!chunk->page_sizes [i])
			free_pages++;
	}
	printf ("Pinned chunk %d at %p, size: %d, pages: %d, free: %d\n", seq, chunk, chunk->num_pages * FREELIST_PAGESIZE, chunk->num_pages, free_pages);
	free_mem = FREELIST_PAGESIZE * free_pages;
	for (i = 0; i < FREELIST_NUM_SLOTS; ++i) {
		if (!chunk->free_list [i])
			continue;
		num_free = 0;
		p = chunk->free_list [i];
		while (p) {
			num_free++;
			p = *p;
		}
		printf ("\tfree list of size %d, %d items\n", freelist_sizes [i], num_free);
		free_mem += freelist_sizes [i] * num_free;
	}
	printf ("\tfree memory in chunk: %d\n", free_mem);
}

/*
 * Debug reporting.
 */
G_GNUC_UNUSED void
mono_sgen_report_internal_mem_usage (void)
{
	SgenPinnedChunk *chunk;
	int i = 0;
	for (chunk = unmanaged_allocator.chunk_list; chunk; chunk = chunk->block.next)
		report_pinned_chunk (chunk, i++);
}

/*
 * Find the slot number in the freelist for memory chunks that
 * can contain @size objects.
 */
static int
slot_for_size (size_t size)
{
	int slot;
	/* do a binary search or lookup table later. */
	for (slot = 0; slot < FREELIST_NUM_SLOTS; ++slot) {
		if (freelist_sizes [slot] >= size)
			return slot;
	}
	g_assert_not_reached ();
	return -1;
}

/*
 * Build a free list for @size memory chunks from the memory area between
 * start_page and end_page.
 */
static void
build_freelist (SgenPinnedChunk *chunk, int slot, int size, char *start_page, char *end_page)
{
	void **p, **end;
	int count = 0;
	/*g_print ("building freelist for slot %d, size %d in %p\n", slot, size, chunk);*/
	p = (void**)start_page;
	end = (void**)(end_page - size);
	g_assert (!chunk->free_list [slot]);
	chunk->free_list [slot] = p;
	while ((char*)p + size <= (char*)end) {
		count++;
		*p = (void*)((char*)p + size);
		p = *p;
	}
	*p = NULL;
	/*g_print ("%d items created, max: %d\n", count, (end_page - start_page) / size);*/
}

/* LOCKING: if !managed, requires the internal allocator lock to be held */
static SgenPinnedChunk*
alloc_pinned_chunk (gboolean managed)
{
	SgenPinnedChunk *chunk;
	int offset;
	int size = PINNED_CHUNK_SIZE;

	if (managed)
		LOCK_INTERNAL_ALLOCATOR;

	chunk = mono_sgen_alloc_os_memory_aligned (size, size, TRUE);
	chunk->block.role = MEMORY_ROLE_PINNED;

	if (managed)
		mono_sgen_update_heap_boundaries ((mword)chunk, ((mword)chunk + size));
	pinned_chunk_bytes_alloced += size;

	/* setup the bookeeping fields */
	chunk->num_pages = size / FREELIST_PAGESIZE;
	offset = G_STRUCT_OFFSET (SgenPinnedChunk, data);
	chunk->page_sizes = (void*)((char*)chunk + offset);
	offset += sizeof (int) * chunk->num_pages;
	offset = SGEN_ALIGN_UP (offset);
	chunk->free_list = (void*)((char*)chunk + offset);
	offset += sizeof (void*) * FREELIST_NUM_SLOTS;
	offset = SGEN_ALIGN_UP (offset);
	chunk->start_data = (void*)((char*)chunk + offset);

	/* allocate the first page to the freelist */
	chunk->page_sizes [0] = PINNED_FIRST_SLOT_SIZE;
	build_freelist (chunk, slot_for_size (PINNED_FIRST_SLOT_SIZE), PINNED_FIRST_SLOT_SIZE, chunk->start_data, ((char*)chunk + FREELIST_PAGESIZE));
	mono_sgen_debug_printf (4, "Allocated pinned chunk %p, size: %d\n", chunk, size);

	if (managed)
		UNLOCK_INTERNAL_ALLOCATOR;

	return chunk;
}

/* assumes freelist for slot is empty, so try to alloc a new page */
static void*
get_chunk_freelist (SgenPinnedChunk *chunk, int slot)
{
	int i;
	void **p;
	p = chunk->free_list [slot];
	if (p) {
		chunk->free_list [slot] = *p;
		return p;
	}
	for (i = 0; i < chunk->num_pages; ++i) {
		int size;
		if (chunk->page_sizes [i])
			continue;
		size = freelist_sizes [slot];
		chunk->page_sizes [i] = size;
		build_freelist (chunk, slot, size, (char*)chunk + FREELIST_PAGESIZE * i, (char*)chunk + FREELIST_PAGESIZE * (i + 1));
		break;
	}
	/* try again */
	p = chunk->free_list [slot];
	if (p) {
		chunk->free_list [slot] = *p;
		return p;
	}
	return NULL;
}

/* used for the GC-internal data structures */
void*
mono_sgen_alloc_internal_full (SgenInternalAllocator *alc, size_t size, int type)
{
	int slot;
	void *res = NULL;
	SgenPinnedChunk *pchunk;

	LOCK_INTERNAL_ALLOCATOR;

	HEAVY_STAT (++stat_internal_alloc);

	if (size > freelist_sizes [FREELIST_NUM_SLOTS - 1]) {
		LargeInternalMemHeader *mh;

		UNLOCK_INTERNAL_ALLOCATOR;

		size += sizeof (LargeInternalMemHeader);
		mh = mono_sgen_alloc_os_memory (size, TRUE);
		mh->magic = LARGE_INTERNAL_MEM_HEADER_MAGIC;
		mh->size = size;
		/* FIXME: do a CAS here */
		large_internal_bytes_alloced += size;
		return mh->data;
	}

	slot = slot_for_size (size);
	g_assert (size <= freelist_sizes [slot]);

	alc->small_internal_mem_bytes [type] += freelist_sizes [slot];

	for (pchunk = alc->chunk_list; pchunk; pchunk = pchunk->block.next) {
		void **p = pchunk->free_list [slot];
		HEAVY_STAT (++stat_internal_alloc_loop1);
		if (p) {
			pchunk->free_list [slot] = *p;

			UNLOCK_INTERNAL_ALLOCATOR;

			memset (p, 0, size);
			return p;
		}
	}
	for (pchunk = alc->chunk_list; pchunk; pchunk = pchunk->block.next) {
		HEAVY_STAT (++stat_internal_alloc_loop2);
		res = get_chunk_freelist (pchunk, slot);
		if (res) {
			UNLOCK_INTERNAL_ALLOCATOR;

			memset (res, 0, size);
			return res;
		}
	}
	pchunk = alloc_pinned_chunk (FALSE);
	/* FIXME: handle OOM */
	pchunk->block.next = alc->chunk_list;
	alc->chunk_list = pchunk;
	res = get_chunk_freelist (pchunk, slot);

	UNLOCK_INTERNAL_ALLOCATOR;

	memset (res, 0, size);
	return res;
}

void*
mono_sgen_alloc_internal (size_t size, int type)
{
	return mono_sgen_alloc_internal_full (&unmanaged_allocator, size, type);
}

void
mono_sgen_free_internal_full (SgenInternalAllocator *alc, void *addr, int type)
{
	SgenPinnedChunk *pchunk;
	LargeInternalMemHeader *mh;
	if (!addr)
		return;

	LOCK_INTERNAL_ALLOCATOR;

	for (pchunk = alc->chunk_list; pchunk; pchunk = pchunk->block.next) {
		/*printf ("trying to free %p in %p (pages: %d)\n", addr, pchunk, pchunk->num_pages);*/
		if (addr >= (void*)pchunk && (char*)addr < (char*)pchunk + pchunk->num_pages * FREELIST_PAGESIZE) {
			int offset = (char*)addr - (char*)pchunk;
			int page = offset / FREELIST_PAGESIZE;
			int slot = slot_for_size (pchunk->page_sizes [page]);
			void **p = addr;
			*p = pchunk->free_list [slot];
			pchunk->free_list [slot] = p;

			alc->small_internal_mem_bytes [type] -= freelist_sizes [slot];

			UNLOCK_INTERNAL_ALLOCATOR;

			return;
		}
	}

	UNLOCK_INTERNAL_ALLOCATOR;

	mh = (LargeInternalMemHeader*)((char*)addr - G_STRUCT_OFFSET (LargeInternalMemHeader, data));
	g_assert (mh->magic == LARGE_INTERNAL_MEM_HEADER_MAGIC);
	/* FIXME: do a CAS */
	large_internal_bytes_alloced -= mh->size;
	mono_sgen_free_os_memory (mh, mh->size);
}

void
mono_sgen_free_internal (void *addr, int type)
{
	mono_sgen_free_internal_full (&unmanaged_allocator, addr, type);
}

void
mono_sgen_dump_internal_mem_usage (FILE *heap_dump_file)
{
	static char const *internal_mem_names [] = { "pin-queue", "fragment", "section", "scan-starts",
						     "fin-table", "finalize-entry", "dislink-table",
						     "dislink", "roots-table", "root-record", "statistics",
						     "remset", "gray-queue", "store-remset", "marksweep-tables",
						     "marksweep-block-info", "ephemeron-link" };

	int i;

	fprintf (heap_dump_file, "<other-mem-usage type=\"large-internal\" size=\"%lld\"/>\n", large_internal_bytes_alloced);
	fprintf (heap_dump_file, "<other-mem-usage type=\"pinned-chunks\" size=\"%lld\"/>\n", pinned_chunk_bytes_alloced);
	for (i = 0; i < INTERNAL_MEM_MAX; ++i) {
		fprintf (heap_dump_file, "<other-mem-usage type=\"%s\" size=\"%ld\"/>\n",
				internal_mem_names [i], unmanaged_allocator.small_internal_mem_bytes [i]);
	}
}

void
mono_sgen_init_internal_allocator (void)
{
#ifdef HEAVY_STATISTICS
	mono_counters_register ("Internal allocs", MONO_COUNTER_GC | MONO_COUNTER_LONG, &stat_internal_alloc);
	mono_counters_register ("Internal alloc loop1", MONO_COUNTER_GC | MONO_COUNTER_LONG, &stat_internal_alloc_loop1);
	mono_counters_register ("Internal alloc loop2", MONO_COUNTER_GC | MONO_COUNTER_LONG, &stat_internal_alloc_loop2);
#endif
}
