#include <mono/metadata/object-internals.h>
#include <mono/metadata/verify.h>
#include <mono/metadata/verify-internals.h>
#include <mono/metadata/opcodes.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-endian.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/metadata-internals.h>
#include <mono/metadata/class-internals.h>
#include <mono/metadata/tokentype.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

typedef struct {
	const char *data;
	guint32 size;
	GSList *errors;
	int valid;
} VerifyContext;

#define ADD_VERIFY_INFO(__ctx, __msg, __status, __exception)	\
	do {	\
		MonoVerifyInfoExtended *vinfo = g_new (MonoVerifyInfoExtended, 1);	\
		vinfo->info.status = __status;	\
		vinfo->info.message = ( __msg);	\
		vinfo->exception_type = (__exception);	\
		(__ctx)->errors = g_slist_prepend ((__ctx)->errors, vinfo);	\
	} while (0)


#define ADD_ERROR(__ctx, __msg)	\
	do {	\
		ADD_VERIFY_INFO(__ctx, __msg, MONO_VERIFY_ERROR, MONO_EXCEPTION_INVALID_PROGRAM); \
		(__ctx)->valid = 0; \
	} while (0)

#define CHECK_STATE() do { if (!ctx.valid) goto cleanup; } while (0)

static void
verify_msdos_header (VerifyContext *ctx)
{
	if (ctx->size < 128) {
		ADD_ERROR (ctx, g_strdup ("Not enough space for the MS-DOS header"));
		return;
	}
	if (ctx->data [0] != 0x4d || ctx->data [1] != 0x5a)
		ADD_ERROR (ctx,  g_strdup ("Invalid MS-DOS watermark"));
}

GSList*
mono_image_verify (const char *data, guint32 size)
{
	VerifyContext ctx;
	memset (&ctx, 0, sizeof (VerifyContext));
	ctx.data = data;
	ctx.size = size;
	ctx.valid = 1;

	verify_msdos_header (&ctx);
	CHECK_STATE();

cleanup:
	return ctx.errors;
}
