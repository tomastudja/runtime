#ifndef _MONO_METADATA_ENDIAN_H_
#define _MONO_METADATA_ENDIAN_H_ 1

#include <glib.h>

typedef union {
	guint32 ival;
	float fval;
} mono_rfloat;

typedef union {
	guint64 ival;
	double fval;
	unsigned char cval [8];
} mono_rdouble;

#ifdef ARM_FPU_FPA
#define MONO_DOUBLE_ASSERT_ENDIANITY(dbl_ptr) \
	do { \
		mono_rdouble r;	\
		r.fval = *dbl_ptr;	\
		r.ival = (guint64) *((guint32 *) r.cval) << 32 |	\
				*((guint32 *) (r.cval + 4));	\
		*dbl_ptr = r.fval;	\
	} while (0)
#else
#define MONO_DOUBLE_ASSERT_ENDIANITY(dbl_ptr)
#endif

#if NO_UNALIGNED_ACCESS

guint16 mono_read16 (const unsigned char *x);
guint32 mono_read32 (const unsigned char *x);
guint64 mono_read64 (const unsigned char *x);

#define read16(x) (mono_read16 ((const unsigned char *)(x)))
#define read32(x) (mono_read32 ((const unsigned char *)(x)))
#define read64(x) (mono_read64 ((const unsigned char *)(x)))

#else

#define read16(x) GUINT16_FROM_LE (*((const guint16 *) (x)))
#define read32(x) GUINT32_FROM_LE (*((const guint32 *) (x)))
#define read64(x) GUINT64_FROM_LE (*((const guint64 *) (x)))

#endif

#define readr4(x,dest)	\
	do {	\
		mono_rfloat mf;	\
		mf.ival = read32 ((x));	\
		*(dest) = mf.fval;	\
	} while (0)

#define readr8(x,dest)	\
	do {	\
		mono_rdouble mf;	\
		mf.ival = read64 ((x));	\
		MONO_DOUBLE_ASSERT_ENDIANITY (&mf.fval);	\
		*(dest) = mf.fval;	\
	} while (0)

#endif /* _MONO_METADATA_ENDIAN_H_ */
