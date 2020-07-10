#include "pinvoke.h"

#include <stdint.h>

/*
 * The table header contain autogenerated function declarations, so avoid including standard headers
 * to avoid incompatible declarations.
 */
#define NULL ((void*)0)
int strcmp (const char *s1, const char *s2);
void mono_wasm_printerr (const char *s);

#ifdef GEN_PINVOKE
#include "pinvoke-table.h"
#else
#include "pinvoke-tables-default.h"
#endif

void
mono_wasm_pinvoke_vararg_stub (void)
{
	/* This is just a stub used to mark vararg pinvokes */
}

void*
wasm_dl_lookup_pinvoke_table (const char *name)
{
	for (int i = 0; i < sizeof (pinvoke_tables) / sizeof (void*); ++i) {
		if (!strcmp (name, pinvoke_names [i]))
			return pinvoke_tables [i];
	}
	return NULL;
}

int
wasm_dl_is_pinvoke_table (void *handle)
{
	for (int i = 0; i < sizeof (pinvoke_tables) / sizeof (void*); ++i) {
		if (pinvoke_tables [i] == handle) {
			return 1;
		}
	}
	return 0;
}

void*
wasm_dl_get_native_to_interp (const char *key, void *extra_arg)
{
#ifdef GEN_PINVOKE
	for (int i = 0; i < sizeof (wasm_native_to_interp_map) / sizeof (void*); ++i) {
		if (!strcmp (wasm_native_to_interp_map [i], key)) {
			void *addr = wasm_native_to_interp_funcs [i];
			wasm_native_to_interp_ftndescs [i] = *(InterpFtnDesc*)extra_arg;
			return addr;
		}
	}
	return NULL;
#else
	return NULL;
#endif
}
