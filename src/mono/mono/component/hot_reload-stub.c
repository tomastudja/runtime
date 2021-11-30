// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

#include <config.h>
#include <glib.h>
#include "mono/component/component.h"
#include "mono/component/hot_reload.h"
#include "mono/metadata/components.h"
#include "mono/metadata/metadata-update.h"
#include "mono/utils/mono-compiler.h"
#include "mono/utils/mono-error-internals.h"

static bool
hot_reload_stub_available (void);

static void
hot_reload_stub_apply_changes (int origin, MonoImage *base_image, gconstpointer dmeta, uint32_t dmeta_len, gconstpointer dil, uint32_t dil_len, gconstpointer dpdb_bytes_orig, uint32_t dpdb_length, MonoError *error);

static MonoComponentHotReload *
component_hot_reload_stub_init (void);

static void
hot_reload_stub_set_fastpath_data (MonoMetadataUpdateData *ptr);

static gboolean
hot_reload_stub_update_enabled (int *modifiable_assemblies_out);

static gboolean
hot_reload_stub_no_inline (MonoMethod *caller, MonoMethod *callee);

static uint32_t
hot_reload_stub_thread_expose_published (void);

static uint32_t
hot_reload_stub_get_thread_generation (void);

static void
hot_reload_stub_cleanup_on_close (MonoImage *image);

static void
hot_reload_stub_effective_table_slow (const MonoTableInfo **t, int idx);

static void
hot_reload_stub_close_except_pools_all (MonoImage *base_image);

static void
hot_reload_stub_close_all (MonoImage *base_image);

static gpointer
hot_reload_stub_get_updated_method_rva (MonoImage *base_image, uint32_t idx);

static gboolean
hot_reload_stub_table_bounds_check (MonoImage *base_image, int table_index, int token_index);

static gboolean
hot_reload_stub_delta_heap_lookup (MonoImage *base_image, MetadataHeapGetterFunc get_heap, uint32_t orig_index, MonoImage **image_out, uint32_t *index_out);

static gpointer
hot_reload_stub_get_updated_method_ppdb (MonoImage *base_image, uint32_t idx);

static gboolean
hot_reload_stub_has_modified_rows (const MonoTableInfo *table);

static int
hot_reload_stub_table_num_rows_slow (MonoImage *image, int table_index);

static GArray*
hot_reload_stub_get_added_methods (MonoClass *klass);

static uint32_t
hot_reload_stub_method_parent (MonoImage *image, uint32_t method_index);

static MonoComponentHotReload fn_table = {
	{ MONO_COMPONENT_ITF_VERSION, &hot_reload_stub_available },
	&hot_reload_stub_set_fastpath_data,
	&hot_reload_stub_update_enabled,
	&hot_reload_stub_no_inline,
	&hot_reload_stub_thread_expose_published,
	&hot_reload_stub_get_thread_generation,
	&hot_reload_stub_cleanup_on_close,
	&hot_reload_stub_effective_table_slow,
	&hot_reload_stub_apply_changes,
	&hot_reload_stub_close_except_pools_all,
	&hot_reload_stub_close_all,
	&hot_reload_stub_get_updated_method_rva,
	&hot_reload_stub_table_bounds_check,
	&hot_reload_stub_delta_heap_lookup,
	&hot_reload_stub_get_updated_method_ppdb,
	&hot_reload_stub_has_modified_rows,
	&hot_reload_stub_table_num_rows_slow,
	&hot_reload_stub_get_added_methods,
	&hot_reload_stub_method_parent,
};

static bool
hot_reload_stub_available (void)
{
	return false;
}

static MonoComponentHotReload *
component_hot_reload_stub_init (void)
{
	return &fn_table;
}

void
hot_reload_stub_set_fastpath_data (MonoMetadataUpdateData *ptr)
{
}

gboolean
hot_reload_stub_update_enabled (int *modifiable_assemblies_out)
{
	if (modifiable_assemblies_out)
		*modifiable_assemblies_out = MONO_MODIFIABLE_ASSM_NONE;
	return false;
}

static gboolean
hot_reload_stub_no_inline (MonoMethod *caller, MonoMethod *callee)
{
	return false;
}


static uint32_t
hot_reload_stub_thread_expose_published (void)
{
	return 0;
}

uint32_t
hot_reload_stub_get_thread_generation (void)
{
	return 0;
}

void
hot_reload_stub_cleanup_on_close (MonoImage *image)
{
}

void
hot_reload_stub_effective_table_slow (const MonoTableInfo **t, int idx)
{
	g_assert_not_reached ();
}

void
hot_reload_stub_apply_changes (int origin, MonoImage *base_image, gconstpointer dmeta, uint32_t dmeta_len, gconstpointer dil, uint32_t dil_len, gconstpointer dpdb_bytes_orig, uint32_t dpdb_length, MonoError *error)
{
	mono_error_set_not_supported (error, "Hot reload not supported in this runtime.");
}

static void
hot_reload_stub_close_except_pools_all (MonoImage *base_image)
{
}

static void
hot_reload_stub_close_all (MonoImage *base_image)
{
}

gpointer
hot_reload_stub_get_updated_method_rva (MonoImage *base_image, uint32_t idx)
{
	g_assert_not_reached ();
}

gboolean
hot_reload_stub_table_bounds_check (MonoImage *base_image, int table_index, int token_index)
{
	g_assert_not_reached ();
}

static gboolean
hot_reload_stub_delta_heap_lookup (MonoImage *base_image, MetadataHeapGetterFunc get_heap, uint32_t orig_index, MonoImage **image_out, uint32_t *index_out)
{
	g_assert_not_reached ();
}

static gpointer
hot_reload_stub_get_updated_method_ppdb (MonoImage *base_image, uint32_t idx)
{
	g_assert_not_reached ();
}

static gboolean
hot_reload_stub_has_modified_rows (const MonoTableInfo *table)
{
	return FALSE;
}

static int
hot_reload_stub_table_num_rows_slow (MonoImage *image, int table_index)
{
	g_assert_not_reached (); /* should always take the fast path */
}

static GArray*
hot_reload_stub_get_added_methods (MonoClass *klass)
{
	return NULL;
}

static uint32_t
hot_reload_stub_method_parent (MonoImage *image, uint32_t method_index)
{
	return 0;
}


MONO_COMPONENT_EXPORT_ENTRYPOINT
MonoComponentHotReload *
mono_component_hot_reload_init (void)
{
	return component_hot_reload_stub_init ();
}
