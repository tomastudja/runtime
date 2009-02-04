#ifndef __MONO_UTILS_DL_H__
#define __MONO_UTILS_DL_H__

#include "mono/utils/mono-compiler.h"

enum {
	MONO_DL_LAZY  = 1,
	MONO_DL_LOCAL = 2,
	MONO_DL_MASK  = 3
};

typedef struct _MonoDl MonoDl;

MonoDl*     mono_dl_open       (const char *name, int flags, char **error_msg) MONO_INTERNAL;
char*       mono_dl_symbol     (MonoDl *module, const char *name, void **symbol) MONO_INTERNAL;
void        mono_dl_close      (MonoDl *module) MONO_INTERNAL;

char*       mono_dl_build_path (const char *directory, const char *name, void **iter) MONO_INTERNAL;


#endif /* __MONO_UTILS_DL_H__ */

