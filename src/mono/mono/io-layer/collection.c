/*
 * collection.c:  Garbage collection for handles
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *
 * (C) 2004 Novell, Inc.
 */

#include <config.h>
#include <glib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <mono/io-layer/wapi.h>
#include <mono/io-layer/collection.h>
#include <mono/io-layer/handles-private.h>

#define DEBUG

static pthread_t collection_thread_id;

static gpointer collection_thread (gpointer args) G_GNUC_NORETURN;
static gpointer collection_thread (gpointer unused G_GNUC_UNUSED)
{
	while (1) {
		_wapi_handle_update_refs ();
		_wapi_handle_spin (10000);
	}
}

void _wapi_collection_init (void)
{
	pthread_attr_t attr;
	int ret;
	
	ret = pthread_attr_init (&attr);
	g_assert (ret == 0);
	
#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	ret = pthread_attr_setstacksize (&attr, PTHREAD_STACK_MIN);
	g_assert (ret == 0);
#endif

	ret = pthread_create (&collection_thread_id, &attr, collection_thread,
			      NULL);
	if (ret != 0) {
		g_error ("%s: Couldn't create handle collection thread: %s",
			 __func__, g_strerror (ret));
	}
}

void _wapi_handle_collect (void)
{
	guint32 count = _wapi_shared_layout->collection_count;
	int i;
	
#ifdef DEBUG
	g_message ("%s: (%d) Starting a collection", __func__, getpid ());
#endif

	/* Become the collection master */
	_WAPI_HANDLE_COLLECTION_UNSAFE;
	
#ifdef DEBUG
	g_message ("%s: (%d) Master set", __func__, getpid ());
#endif
	
	/* If count has changed, someone else jumped in as master */
	if (count == _wapi_shared_layout->collection_count) {
		for (i = 0; i < _WAPI_HANDLE_INITIAL_COUNT; i++) {
			struct _WapiHandleShared *shared;
			struct _WapiHandleSharedMetadata *meta;
			guint32 too_old = (guint32)(time(NULL) & 0xFFFFFFFF) - 300; /* 5 minutes without update */
			
			meta = &_wapi_shared_layout->metadata[i];
			if (meta->timestamp < too_old && meta->offset != 0) {
#ifdef DEBUG
				g_message ("%s: (%d) Deleting metadata slot 0x%x handle 0x%x", __func__, getpid (), i, meta->offset);
#endif
				memset (&_wapi_shared_layout->handles[meta->offset], '\0', sizeof(struct _WapiHandleShared));
				memset (&_wapi_shared_layout->metadata[i], '\0', sizeof(struct _WapiHandleSharedMetadata));
			}

			/* Need to blank any handles data that is no
			 * longer pointed to by a metadata entry too
			 */
			shared = &_wapi_shared_layout->handles[i];
			if (shared->stale == TRUE) {
#ifdef DEBUG
				g_message ("%s: (%d) Deleting stale handle 0x%x", __func__, getpid (), i);
#endif
				memset (&_wapi_shared_layout->handles[i], '\0',
					sizeof(struct _WapiHandleShared));
			}
		}

		for (i = 0; i < _wapi_fileshare_layout->hwm; i++) {
			struct _WapiFileShare *file_share = &_wapi_fileshare_layout->share_info[i];
			guint32 too_old = (guint32)(time(NULL) & 0xFFFFFFFF) - 300; /* 5 minutes without update */
			
			if (file_share->timestamp < too_old) {
				memset (file_share, '\0',
					sizeof(struct _WapiFileShare));
			}
		}

		InterlockedIncrement (&_wapi_shared_layout->collection_count);
	}
	
	_WAPI_HANDLE_COLLECTION_SAFE;

#ifdef DEBUG
	g_message ("%s: (%d) Collection done", __func__, getpid ());
#endif
}
