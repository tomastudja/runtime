/*
 * aot-compiler.c: mono Ahead of Time compiler
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *   Zoltan Varga (vargaz@gmail.com)
 *
 * (C) 2002 Ximian, Inc.
 */

/* Remaining AOT-only work:
 * - optimize the trampolines, generate more code in the arch files.
 * - make things more consistent with how elf works, for example, use ELF 
 *   relocations.
 * Remaining generics sharing work:
 * - optimize the size of the data which is encoded.
 * - optimize the runtime loading of data:
 *   - the trampoline code calls mono_jit_info_table_find () to find the rgctx, 
 *     which loads the debugging+exception handling info for the method. This is a 
 *     huge waste of time and code, since the rgctx structure is currently empty.
 *   - every shared method has a MonoGenericJitInfo structure which is only really
 *     used for handling catch clauses with open types, not a very common use case.
 */
#include "config.h"
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#ifndef HOST_WIN32
#include <sys/time.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include <errno.h>
#include <sys/stat.h>


#include <mono/metadata/tabledefs.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/metadata-internals.h>
#include <mono/metadata/marshal.h>
#include <mono/metadata/gc-internal.h>
#include <mono/metadata/monitor.h>
#include <mono/metadata/mempool-internals.h>
#include <mono/metadata/mono-endian.h>
#include <mono/metadata/threads-types.h>
#include <mono/utils/mono-logger.h>
#include <mono/utils/mono-compiler.h>
#include <mono/utils/mono-time.h>
#include <mono/utils/mono-mmap.h>

#include "mini.h"
#include "image-writer.h"
#include "dwarfwriter.h"

#if !defined(DISABLE_AOT) && !defined(DISABLE_JIT)

#define TV_DECLARE(name) gint64 name
#define TV_GETTIME(tv) tv = mono_100ns_ticks ()
#define TV_ELAPSED(start,end) (((end) - (start)) / 10)

#ifdef TARGET_WIN32
#define SHARED_EXT ".dll"
#elif defined(__ppc__) && defined(__MACH__)
#define SHARED_EXT ".dylib"
#else
#define SHARED_EXT ".so"
#endif

#define ALIGN_TO(val,align) ((((guint64)val) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_PTR_TO(ptr,align) (gpointer)((((gssize)(ptr)) + (align - 1)) & (~(align - 1)))
#define ROUND_DOWN(VALUE,SIZE)	((VALUE) & ~((SIZE) - 1))

typedef struct MonoAotOptions {
	char *outfile;
	gboolean save_temps;
	gboolean write_symbols;
	gboolean metadata_only;
	gboolean bind_to_runtime_version;
	gboolean full_aot;
	gboolean no_dlsym;
	gboolean static_link;
	gboolean asm_only;
	gboolean asm_writer;
	gboolean nodebug;
	gboolean soft_debug;
	int nthreads;
	int ntrampolines;
	int nrgctx_trampolines;
	gboolean print_skipped_methods;
	gboolean stats;
	char *tool_prefix;
	gboolean autoreg;
} MonoAotOptions;

typedef struct MonoAotStats {
	int ccount, mcount, lmfcount, abscount, gcount, ocount, genericcount;
	int code_size, info_size, ex_info_size, unwind_info_size, got_size, class_info_size, got_info_size;
	int methods_without_got_slots, direct_calls, all_calls, llvm_count;
	int got_slots, offsets_size;
	int got_slot_types [MONO_PATCH_INFO_NONE];
	int jit_time, gen_time, link_time;
} MonoAotStats;

typedef struct MonoAotCompile {
	MonoImage *image;
	GPtrArray *methods;
	GHashTable *method_indexes;
	GHashTable *method_depth;
	MonoCompile **cfgs;
	int cfgs_size;
	GHashTable *patch_to_plt_offset;
	GHashTable *plt_offset_to_patch;
	GHashTable *patch_to_got_offset;
	GHashTable **patch_to_got_offset_by_type;
	GPtrArray *got_patches;
	GHashTable *image_hash;
	GHashTable *method_to_cfg;
	GHashTable *token_info_hash;
	GPtrArray *extra_methods;
	GPtrArray *image_table;
	GPtrArray *globals;
	GList *method_order;
	guint32 *plt_got_info_offsets;
	guint32 got_offset, plt_offset, plt_got_offset_base;
	guint32 final_got_size;
	/* Number of GOT entries reserved for trampolines */
	guint32 num_trampoline_got_entries;

	guint32 num_trampolines [MONO_AOT_TRAMP_NUM];
	guint32 trampoline_got_offset_base [MONO_AOT_TRAMP_NUM];
	guint32 trampoline_size [MONO_AOT_TRAMP_NUM];

	MonoAotOptions aot_opts;
	guint32 nmethods;
	guint32 opts;
	MonoMemPool *mempool;
	MonoAotStats stats;
	int method_index;
	char *static_linking_symbol;
	CRITICAL_SECTION mutex;
	gboolean use_bin_writer;
	MonoImageWriter *w;
	MonoDwarfWriter *dwarf;
	FILE *fp;
	char *tmpfname;
	GSList *cie_program;
	GHashTable *unwind_info_offsets;
	GPtrArray *unwind_ops;
	guint32 unwind_info_offset;
	char *got_symbol;
	char *plt_symbol;
	GHashTable *method_label_hash;
	const char *temp_prefix;
	guint32 label_generator;
	gboolean llvm;
	MonoAotFileFlags flags;
	MonoDynamicStream blob;
} MonoAotCompile;

#define mono_acfg_lock(acfg) EnterCriticalSection (&((acfg)->mutex))
#define mono_acfg_unlock(acfg) LeaveCriticalSection (&((acfg)->mutex))

/* This points to the current acfg in LLVM mode */
static MonoAotCompile *llvm_acfg;

#ifdef HAVE_ARRAY_ELEM_INIT
#define MSGSTRFIELD(line) MSGSTRFIELD1(line)
#define MSGSTRFIELD1(line) str##line
static const struct msgstr_t {
#define PATCH_INFO(a,b) char MSGSTRFIELD(__LINE__) [sizeof (b)];
#include "patch-info.h"
#undef PATCH_INFO
} opstr = {
#define PATCH_INFO(a,b) b,
#include "patch-info.h"
#undef PATCH_INFO
};
static const gint16 opidx [] = {
#define PATCH_INFO(a,b) [MONO_PATCH_INFO_ ## a] = offsetof (struct msgstr_t, MSGSTRFIELD(__LINE__)),
#include "patch-info.h"
#undef PATCH_INFO
};

static G_GNUC_UNUSED const char*
get_patch_name (int info)
{
	return (const char*)&opstr + opidx [info];
}

#else
#define PATCH_INFO(a,b) b,
static const char* const
patch_types [MONO_PATCH_INFO_NUM + 1] = {
#include "patch-info.h"
	NULL
};

static G_GNUC_UNUSED const char*
get_patch_name (int info)
{
	return patch_types [info];
}

#endif

/* Wrappers around the image writer functions */

static inline void
emit_section_change (MonoAotCompile *acfg, const char *section_name, int subsection_index)
{
	img_writer_emit_section_change (acfg->w, section_name, subsection_index);
}

static inline void
emit_push_section (MonoAotCompile *acfg, const char *section_name, int subsection)
{
	img_writer_emit_push_section (acfg->w, section_name, subsection);
}

static inline void
emit_pop_section (MonoAotCompile *acfg)
{
	img_writer_emit_pop_section (acfg->w);
}

static inline void
emit_local_symbol (MonoAotCompile *acfg, const char *name, const char *end_label, gboolean func) 
{ 
	img_writer_emit_local_symbol (acfg->w, name, end_label, func); 
}

static inline void
emit_label (MonoAotCompile *acfg, const char *name) 
{ 
	img_writer_emit_label (acfg->w, name); 
}

static inline void
emit_bytes (MonoAotCompile *acfg, const guint8* buf, int size) 
{ 
	img_writer_emit_bytes (acfg->w, buf, size); 
}

static inline void
emit_string (MonoAotCompile *acfg, const char *value) 
{ 
	img_writer_emit_string (acfg->w, value); 
}

static inline void
emit_line (MonoAotCompile *acfg) 
{ 
	img_writer_emit_line (acfg->w); 
}

static inline void
emit_alignment (MonoAotCompile *acfg, int size) 
{ 
	img_writer_emit_alignment (acfg->w, size); 
}

static inline void
emit_pointer_unaligned (MonoAotCompile *acfg, const char *target) 
{ 
	img_writer_emit_pointer_unaligned (acfg->w, target); 
}

static inline void
emit_pointer (MonoAotCompile *acfg, const char *target) 
{ 
	img_writer_emit_pointer (acfg->w, target); 
}

static inline void
emit_int16 (MonoAotCompile *acfg, int value) 
{ 
	img_writer_emit_int16 (acfg->w, value); 
}

static inline void
emit_int32 (MonoAotCompile *acfg, int value) 
{ 
	img_writer_emit_int32 (acfg->w, value); 
}

static inline void
emit_symbol_diff (MonoAotCompile *acfg, const char *end, const char* start, int offset) 
{ 
	img_writer_emit_symbol_diff (acfg->w, end, start, offset); 
}

static inline void
emit_zero_bytes (MonoAotCompile *acfg, int num) 
{ 
	img_writer_emit_zero_bytes (acfg->w, num); 
}

static inline void
emit_byte (MonoAotCompile *acfg, guint8 val) 
{ 
	img_writer_emit_byte (acfg->w, val); 
}

static G_GNUC_UNUSED void
emit_global_inner (MonoAotCompile *acfg, const char *name, gboolean func)
{
	img_writer_emit_global (acfg->w, name, func);
}

static void
emit_global (MonoAotCompile *acfg, const char *name, gboolean func)
{
	if (acfg->aot_opts.no_dlsym) {
		g_ptr_array_add (acfg->globals, g_strdup (name));
		img_writer_emit_local_symbol (acfg->w, name, NULL, func);
	} else {
		img_writer_emit_global (acfg->w, name, func);
	}
}

static void
emit_symbol_size (MonoAotCompile *acfg, const char *name, const char *end_label)
{
	img_writer_emit_symbol_size (acfg->w, name, end_label);
}

static void
emit_string_symbol (MonoAotCompile *acfg, const char *name, const char *value)
{
	img_writer_emit_section_change (acfg->w, ".text", 1);
	emit_global (acfg, name, FALSE);
	img_writer_emit_label (acfg->w, name);
	img_writer_emit_string (acfg->w, value);
}

static G_GNUC_UNUSED void
emit_uleb128 (MonoAotCompile *acfg, guint32 value)
{
	do {
		guint8 b = value & 0x7f;
		value >>= 7;
		if (value != 0) /* more bytes to come */
			b |= 0x80;
		emit_byte (acfg, b);
	} while (value);
}

static G_GNUC_UNUSED void
emit_sleb128 (MonoAotCompile *acfg, gint64 value)
{
	gboolean more = 1;
	gboolean negative = (value < 0);
	guint32 size = 64;
	guint8 byte;

	while (more) {
		byte = value & 0x7f;
		value >>= 7;
		/* the following is unnecessary if the
		 * implementation of >>= uses an arithmetic rather
		 * than logical shift for a signed left operand
		 */
		if (negative)
			/* sign extend */
			value |= - ((gint64)1 <<(size - 7));
		/* sign bit of byte is second high order bit (0x40) */
		if ((value == 0 && !(byte & 0x40)) ||
			(value == -1 && (byte & 0x40)))
			more = 0;
		else
			byte |= 0x80;
		emit_byte (acfg, byte);
	}
}

static G_GNUC_UNUSED void
encode_uleb128 (guint32 value, guint8 *buf, guint8 **endbuf)
{
	guint8 *p = buf;

	do {
		guint8 b = value & 0x7f;
		value >>= 7;
		if (value != 0) /* more bytes to come */
			b |= 0x80;
		*p ++ = b;
	} while (value);

	*endbuf = p;
}

static G_GNUC_UNUSED void
encode_sleb128 (gint32 value, guint8 *buf, guint8 **endbuf)
{
	gboolean more = 1;
	gboolean negative = (value < 0);
	guint32 size = 32;
	guint8 byte;
	guint8 *p = buf;

	while (more) {
		byte = value & 0x7f;
		value >>= 7;
		/* the following is unnecessary if the
		 * implementation of >>= uses an arithmetic rather
		 * than logical shift for a signed left operand
		 */
		if (negative)
			/* sign extend */
			value |= - (1 <<(size - 7));
		/* sign bit of byte is second high order bit (0x40) */
		if ((value == 0 && !(byte & 0x40)) ||
			(value == -1 && (byte & 0x40)))
			more = 0;
		else
			byte |= 0x80;
		*p ++= byte;
	}

	*endbuf = p;
}

/* ARCHITECTURE SPECIFIC CODE */

#if defined(TARGET_X86) || defined(TARGET_AMD64) || defined(TARGET_ARM) || defined(TARGET_POWERPC)
#define EMIT_DWARF_INFO 1
#endif

#if defined(TARGET_ARM)
#define AOT_FUNC_ALIGNMENT 4
#else
#define AOT_FUNC_ALIGNMENT 16
#endif
 
#if defined(TARGET_POWERPC64) && !defined(__mono_ilp32__)
#define PPC_LD_OP "ld"
#define PPC_LDX_OP "ldx"
#else
#define PPC_LD_OP "lwz"
#define PPC_LDX_OP "lwzx"
#endif

/*
 * arch_emit_direct_call:
 *
 *   Emit a direct call to the symbol TARGET. CALL_SIZE is set to the size of the
 * calling code.
 */
static void
arch_emit_direct_call (MonoAotCompile *acfg, const char *target, int *call_size)
{
#if defined(TARGET_X86) || defined(TARGET_AMD64)
	/* Need to make sure this is exactly 5 bytes long */
	emit_byte (acfg, '\xe8');
	emit_symbol_diff (acfg, target, ".", -4);
	*call_size = 5;
#elif defined(TARGET_ARM)
	if (acfg->use_bin_writer) {
		guint8 buf [4];
		guint8 *code;

		code = buf;
		ARM_BL (code, 0);

		img_writer_emit_reloc (acfg->w, R_ARM_CALL, target, -8);
		emit_bytes (acfg, buf, 4);
	} else {
		img_writer_emit_unset_mode (acfg->w);
		fprintf (acfg->fp, "bl %s\n", target);
	}
	*call_size = 4;
#elif defined(TARGET_POWERPC)
	if (acfg->use_bin_writer) {
		g_assert_not_reached ();
	} else {
		img_writer_emit_unset_mode (acfg->w);
		fprintf (acfg->fp, "bl %s\n", target);
		*call_size = 4;
	}
#else
	g_assert_not_reached ();
#endif
}

/*
 * PPC32 design:
 * - we use an approach similar to the x86 abi: reserve a register (r30) to hold 
 *   the GOT pointer.
 * - The full-aot trampolines need access to the GOT of mscorlib, so we store
 *   in in the 2. slot of every GOT, and require every method to place the GOT
 *   address in r30, even when it doesn't access the GOT otherwise. This way,
 *   the trampolines can compute the mscorlib GOT address by loading 4(r30).
 */

/*
 * PPC64 design:
 * PPC64 uses function descriptors which greatly complicate all code, since
 * these are used very inconsistently in the runtime. Some functions like 
 * mono_compile_method () return ftn descriptors, while others like the
 * trampoline creation functions do not.
 * We assume that all GOT slots contain function descriptors, and create 
 * descriptors in aot-runtime.c when needed.
 * The ppc64 abi uses r2 to hold the address of the TOC/GOT, which is loaded
 * from function descriptors, we could do the same, but it would require 
 * rewriting all the ppc/aot code to handle function descriptors properly.
 * So instead, we use the same approach as on PPC32.
 * This is a horrible mess, but fixing it would probably lead to an even bigger
 * one.
 */

#ifdef MONO_ARCH_AOT_SUPPORTED
/*
 * arch_emit_got_offset:
 *
 *   The memory pointed to by CODE should hold native code for computing the GOT
 * address. Emit this code while patching it with the offset between code and
 * the GOT. CODE_SIZE is set to the number of bytes emitted.
 */
static void
arch_emit_got_offset (MonoAotCompile *acfg, guint8 *code, int *code_size)
{
#if defined(TARGET_POWERPC64)
	g_assert (!acfg->use_bin_writer);
	img_writer_emit_unset_mode (acfg->w);
	/* 
	 * The ppc32 code doesn't seem to work on ppc64, the assembler complains about
	 * unsupported relocations. So we store the got address into the .Lgot_addr
	 * symbol which is in the text segment, compute its address, and load it.
	 */
	fprintf (acfg->fp, ".L%d:\n", acfg->label_generator);
	fprintf (acfg->fp, "lis 0, (.Lgot_addr + 4 - .L%d)@h\n", acfg->label_generator);
	fprintf (acfg->fp, "ori 0, 0, (.Lgot_addr + 4 - .L%d)@l\n", acfg->label_generator);
	fprintf (acfg->fp, "add 30, 30, 0\n");
	fprintf (acfg->fp, "%s 30, 0(30)\n", PPC_LD_OP);
	acfg->label_generator ++;
	*code_size = 16;
#elif defined(TARGET_POWERPC)
	g_assert (!acfg->use_bin_writer);
	img_writer_emit_unset_mode (acfg->w);
	fprintf (acfg->fp, ".L%d:\n", acfg->label_generator);
	fprintf (acfg->fp, "lis 0, (%s + 4 - .L%d)@h\n", acfg->got_symbol, acfg->label_generator);
	fprintf (acfg->fp, "ori 0, 0, (%s + 4 - .L%d)@l\n", acfg->got_symbol, acfg->label_generator);
	acfg->label_generator ++;
	*code_size = 8;
#else
	guint32 offset = mono_arch_get_patch_offset (code);
	emit_bytes (acfg, code, offset);
	emit_symbol_diff (acfg, acfg->got_symbol, ".", offset);

	*code_size = offset + 4;
#endif
}

/*
 * arch_emit_got_access:
 *
 *   The memory pointed to by CODE should hold native code for loading a GOT
 * slot. Emit this code while patching it so it accesses the GOT slot GOT_SLOT.
 * CODE_SIZE is set to the number of bytes emitted.
 */
static void
arch_emit_got_access (MonoAotCompile *acfg, guint8 *code, int got_slot, int *code_size)
{
	/* Emit beginning of instruction */
	emit_bytes (acfg, code, mono_arch_get_patch_offset (code));

	/* Emit the offset */
#ifdef TARGET_AMD64
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (unsigned int) ((got_slot * sizeof (gpointer)) - 4));
	*code_size = mono_arch_get_patch_offset (code) + 4;
#elif defined(TARGET_X86)
	emit_int32 (acfg, (unsigned int) ((got_slot * sizeof (gpointer))));
	*code_size = mono_arch_get_patch_offset (code) + 4;
#elif defined(TARGET_ARM)
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (unsigned int) ((got_slot * sizeof (gpointer))) - 12);
	*code_size = mono_arch_get_patch_offset (code) + 4;
#elif defined(TARGET_POWERPC)
	{
		guint8 buf [32];
		guint8 *code;

		code = buf;
		ppc_load32 (code, ppc_r0, got_slot * sizeof (gpointer));
		g_assert (code - buf == 8);
		emit_bytes (acfg, buf, code - buf);
		*code_size = code - buf;
	}
#else
	g_assert_not_reached ();
#endif
}

#endif

/*
 * arch_emit_plt_entry:
 *
 *   Emit code for the PLT entry with index INDEX.
 */
static void
arch_emit_plt_entry (MonoAotCompile *acfg, int index)
{
#if defined(TARGET_X86)
		if (index == 0) {
			/* It is filled up during loading by the AOT loader. */
			emit_zero_bytes (acfg, 16);
		} else {
			/* Need to make sure this is 9 bytes long */
			emit_byte (acfg, '\xe9');
			emit_symbol_diff (acfg, acfg->plt_symbol, ".", -4);
			emit_int32 (acfg, acfg->plt_got_info_offsets [index]);
		}
#elif defined(TARGET_AMD64)
		/*
		 * We can't emit jumps because they are 32 bits only so they can't be patched.
		 * So we make indirect calls through GOT entries which are patched by the AOT 
		 * loader to point to .Lpd entries. 
		 */
		/* jmpq *<offset>(%rip) */
		emit_byte (acfg, '\xff');
		emit_byte (acfg, '\x25');
		emit_symbol_diff (acfg, acfg->got_symbol, ".", ((acfg->plt_got_offset_base + index) * sizeof (gpointer)) -4);
		/* Used by mono_aot_get_plt_info_offset */
		emit_int32 (acfg, acfg->plt_got_info_offsets [index]);
#elif defined(TARGET_ARM)
		guint8 buf [256];
		guint8 *code;

		/* FIXME:
		 * - optimize OP_AOTCONST implementation
		 * - optimize the PLT entries
		 * - optimize SWITCH AOT implementation
		 */
		code = buf;
		if (acfg->use_bin_writer && FALSE) {
			/* FIXME: mono_arch_patch_plt_entry () needs to decode this */
			/* We only emit 1 relocation since we implement it ourselves anyway */
			img_writer_emit_reloc (acfg->w, R_ARM_ALU_PC_G0_NC, acfg->got_symbol, ((acfg->plt_got_offset_base + index) * sizeof (gpointer)) - 8);
			/* FIXME: A 2 instruction encoding is sufficient in most cases */
			ARM_ADD_REG_IMM (code, ARMREG_IP, ARMREG_PC, 0, 0);
			ARM_ADD_REG_IMM (code, ARMREG_IP, ARMREG_IP, 0, 0);
			ARM_LDR_IMM (code, ARMREG_PC, ARMREG_IP, 0);
			emit_bytes (acfg, buf, code - buf);
			/* Used by mono_aot_get_plt_info_offset */
			emit_int32 (acfg, acfg->plt_got_info_offsets [index]);
		} else {
			ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 0);
			ARM_LDR_REG_REG (code, ARMREG_PC, ARMREG_PC, ARMREG_IP);
			emit_bytes (acfg, buf, code - buf);
			emit_symbol_diff (acfg, acfg->got_symbol, ".", ((acfg->plt_got_offset_base + index) * sizeof (gpointer)) - 4);
			/* Used by mono_aot_get_plt_info_offset */
			emit_int32 (acfg, acfg->plt_got_info_offsets [index]);
		}
		/* 
		 * The plt_got_info_offset is computed automatically by 
		 * mono_aot_get_plt_info_offset (), so no need to save it here.
		 */
#elif defined(TARGET_POWERPC)
		guint32 offset = (acfg->plt_got_offset_base + index) * sizeof (gpointer);

		/* The GOT address is guaranteed to be in r30 by OP_LOAD_GOTADDR */
		g_assert (!acfg->use_bin_writer);
		img_writer_emit_unset_mode (acfg->w);
		fprintf (acfg->fp, "lis 11, %d@h\n", offset);
		fprintf (acfg->fp, "ori 11, 11, %d@l\n", offset);
		fprintf (acfg->fp, "add 11, 11, 30\n");
		fprintf (acfg->fp, "%s 11, 0(11)\n", PPC_LD_OP);
#ifdef PPC_USES_FUNCTION_DESCRIPTOR
		fprintf (acfg->fp, "%s 2, %d(11)\n", PPC_LD_OP, (int)sizeof (gpointer));
		fprintf (acfg->fp, "%s 11, 0(11)\n", PPC_LD_OP);
#endif
		fprintf (acfg->fp, "mtctr 11\n");
		fprintf (acfg->fp, "bctr\n");
		emit_int32 (acfg, acfg->plt_got_info_offsets [index]);
#else
		g_assert_not_reached ();
#endif
}

/*
 * arch_emit_specific_trampoline:
 *
 *   Emit code for a specific trampoline. OFFSET is the offset of the first of
 * two GOT slots which contain the generic trampoline address and the trampoline
 * argument. TRAMP_SIZE is set to the size of the emitted trampoline.
 */
static void
arch_emit_specific_trampoline (MonoAotCompile *acfg, int offset, int *tramp_size)
{
	/*
	 * The trampolines created here are variations of the specific 
	 * trampolines created in mono_arch_create_specific_trampoline (). The 
	 * differences are:
	 * - the generic trampoline address is taken from a got slot.
	 * - the offset of the got slot where the trampoline argument is stored
	 *   is embedded in the instruction stream, and the generic trampoline
	 *   can load the argument by loading the offset, adding it to the
	 *   address of the trampoline to get the address of the got slot, and
	 *   loading the argument from there.
	 * - all the trampolines should be of the same length.
	 */
#if defined(TARGET_AMD64)
	/* This should be exactly 16 bytes long */
	*tramp_size = 16;
	/* call *<offset>(%rip) */
	emit_byte (acfg, '\x41');
	emit_byte (acfg, '\xff');
	emit_byte (acfg, '\x15');
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) - 4);
	/* This should be relative to the start of the trampoline */
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) - 4 + 19);
	emit_zero_bytes (acfg, 5);
#elif defined(TARGET_ARM)
	guint8 buf [128];
	guint8 *code;

	/* This should be exactly 20 bytes long */
	*tramp_size = 20;
	code = buf;
	ARM_PUSH (code, 0x5fff);
	ARM_LDR_IMM (code, ARMREG_R1, ARMREG_PC, 4);
	/* Load the value from the GOT */
	ARM_LDR_REG_REG (code, ARMREG_R1, ARMREG_PC, ARMREG_R1);
	/* Branch to it */
	ARM_BLX_REG (code, ARMREG_R1);

	g_assert (code - buf == 16);

	/* Emit it */
	emit_bytes (acfg, buf, code - buf);
	/* 
	 * Only one offset is needed, since the second one would be equal to the
	 * first one.
	 */
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) - 4 + 4);
	//emit_symbol_diff (acfg, acfg->got_symbol, ".", ((offset + 1) * sizeof (gpointer)) - 4 + 8);
#elif defined(TARGET_POWERPC)
	guint8 buf [128];
	guint8 *code;

	*tramp_size = 4;
	code = buf;

	g_assert (!acfg->use_bin_writer);

	/*
	 * PPC has no ip relative addressing, so we need to compute the address
	 * of the mscorlib got. That is slow and complex, so instead, we store it
	 * in the second got slot of every aot image. The caller already computed
	 * the address of its got and placed it into r30.
	 */
	img_writer_emit_unset_mode (acfg->w);
	/* Load mscorlib got address */
	fprintf (acfg->fp, "%s 0, %d(30)\n", PPC_LD_OP, (int)sizeof (gpointer));
	/* Load generic trampoline address */
	fprintf (acfg->fp, "lis 11, %d@h\n", (int)(offset * sizeof (gpointer)));
	fprintf (acfg->fp, "ori 11, 11, %d@l\n", (int)(offset * sizeof (gpointer)));
	fprintf (acfg->fp, "%s 11, 11, 0\n", PPC_LDX_OP);
#ifdef PPC_USES_FUNCTION_DESCRIPTOR
	fprintf (acfg->fp, "%s 11, 0(11)\n", PPC_LD_OP);
#endif
	fprintf (acfg->fp, "mtctr 11\n");
	/* Load trampoline argument */
	/* On ppc, we pass it normally to the generic trampoline */
	fprintf (acfg->fp, "lis 11, %d@h\n", (int)((offset + 1) * sizeof (gpointer)));
	fprintf (acfg->fp, "ori 11, 11, %d@l\n", (int)((offset + 1) * sizeof (gpointer)));
	fprintf (acfg->fp, "%s 0, 11, 0\n", PPC_LDX_OP);
	/* Branch to generic trampoline */
	fprintf (acfg->fp, "bctr\n");

#ifdef PPC_USES_FUNCTION_DESCRIPTOR
	*tramp_size = 10 * 4;
#else
	*tramp_size = 9 * 4;
#endif
#else
	g_assert_not_reached ();
#endif
}

/*
 * arch_emit_unbox_trampoline:
 *
 *   Emit code for the unbox trampoline for METHOD used in the full-aot case.
 * CALL_TARGET is the symbol pointing to the native code of METHOD.
 */
static void
arch_emit_unbox_trampoline (MonoAotCompile *acfg, MonoMethod *method, MonoGenericSharingContext *gsctx, const char *call_target)
{
#if defined(TARGET_AMD64)
	guint8 buf [32];
	guint8 *code;
	int this_reg;

	this_reg = mono_arch_get_this_arg_reg (mono_method_signature (method), gsctx, NULL);
	code = buf;
	amd64_alu_reg_imm (code, X86_ADD, this_reg, sizeof (MonoObject));

	emit_bytes (acfg, buf, code - buf);
	/* jump <method> */
	emit_byte (acfg, '\xe9');
	emit_symbol_diff (acfg, call_target, ".", -4);
#elif defined(TARGET_ARM)
	guint8 buf [128];
	guint8 *code;
	int this_pos = 0;

	code = buf;

	if (MONO_TYPE_ISSTRUCT (mono_method_signature (method)->ret))
		this_pos = 1;

	ARM_ADD_REG_IMM8 (code, this_pos, this_pos, sizeof (MonoObject));

	emit_bytes (acfg, buf, code - buf);
	/* jump to method */
	if (acfg->use_bin_writer) {
		guint8 buf [4];
		guint8 *code;

		code = buf;
		ARM_B (code, 0);

		img_writer_emit_reloc (acfg->w, R_ARM_JUMP24, call_target, -8);
		emit_bytes (acfg, buf, 4);
	} else {
		fprintf (acfg->fp, "\n\tb %s\n", call_target);
	}
#elif defined(TARGET_POWERPC)
	int this_pos = 3;

	if (MONO_TYPE_ISSTRUCT (mono_method_signature (method)->ret))
		this_pos = 4;

	g_assert (!acfg->use_bin_writer);

	fprintf (acfg->fp, "\n\taddi %d, %d, %d\n", this_pos, this_pos, (int)sizeof (MonoObject));
	fprintf (acfg->fp, "\n\tb %s\n", call_target);
#else
	g_assert_not_reached ();
#endif
}

/*
 * arch_emit_static_rgctx_trampoline:
 *
 *   Emit code for a static rgctx trampoline. OFFSET is the offset of the first of
 * two GOT slots which contain the rgctx argument, and the method to jump to.
 * TRAMP_SIZE is set to the size of the emitted trampoline.
 * These kinds of trampolines cannot be enumerated statically, since there could
 * be one trampoline per method instantiation, so we emit the same code for all
 * trampolines, and parameterize them using two GOT slots.
 */
static void
arch_emit_static_rgctx_trampoline (MonoAotCompile *acfg, int offset, int *tramp_size)
{
#if defined(TARGET_AMD64)
	/* This should be exactly 13 bytes long */
	*tramp_size = 13;

	/* mov <OFFSET>(%rip), %r10 */
	emit_byte (acfg, '\x4d');
	emit_byte (acfg, '\x8b');
	emit_byte (acfg, '\x15');
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) - 4);

	/* jmp *<offset>(%rip) */
	emit_byte (acfg, '\xff');
	emit_byte (acfg, '\x25');
	emit_symbol_diff (acfg, acfg->got_symbol, ".", ((offset + 1) * sizeof (gpointer)) - 4);
#elif defined(TARGET_ARM)
	guint8 buf [128];
	guint8 *code;

	/* This should be exactly 24 bytes long */
	*tramp_size = 24;
	code = buf;
	/* Load rgctx value */
	ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 8);
	ARM_LDR_REG_REG (code, MONO_ARCH_RGCTX_REG, ARMREG_PC, ARMREG_IP);
	/* Load branch addr + branch */
	ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 4);
	ARM_LDR_REG_REG (code, ARMREG_PC, ARMREG_PC, ARMREG_IP);

	g_assert (code - buf == 16);

	/* Emit it */
	emit_bytes (acfg, buf, code - buf);
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) - 4 + 8);
	emit_symbol_diff (acfg, acfg->got_symbol, ".", ((offset + 1) * sizeof (gpointer)) - 4 + 4);
#elif defined(TARGET_POWERPC)
	guint8 buf [128];
	guint8 *code;

	*tramp_size = 4;
	code = buf;

	g_assert (!acfg->use_bin_writer);

	/*
	 * PPC has no ip relative addressing, so we need to compute the address
	 * of the mscorlib got. That is slow and complex, so instead, we store it
	 * in the second got slot of every aot image. The caller already computed
	 * the address of its got and placed it into r30.
	 */
	img_writer_emit_unset_mode (acfg->w);
	/* Load mscorlib got address */
	fprintf (acfg->fp, "%s 0, %d(30)\n", PPC_LD_OP, (int)sizeof (gpointer));
	/* Load rgctx */
	fprintf (acfg->fp, "lis 11, %d@h\n", (int)(offset * sizeof (gpointer)));
	fprintf (acfg->fp, "ori 11, 11, %d@l\n", (int)(offset * sizeof (gpointer)));
	fprintf (acfg->fp, "%s %d, 11, 0\n", PPC_LDX_OP, MONO_ARCH_RGCTX_REG);
	/* Load target address */
	fprintf (acfg->fp, "lis 11, %d@h\n", (int)((offset + 1) * sizeof (gpointer)));
	fprintf (acfg->fp, "ori 11, 11, %d@l\n", (int)((offset + 1) * sizeof (gpointer)));
	fprintf (acfg->fp, "%s 11, 11, 0\n", PPC_LDX_OP);
#ifdef PPC_USES_FUNCTION_DESCRIPTOR
	fprintf (acfg->fp, "%s 2, %d(11)\n", PPC_LD_OP, (int)sizeof (gpointer));
	fprintf (acfg->fp, "%s 11, 0(11)\n", PPC_LD_OP);
#endif
	fprintf (acfg->fp, "mtctr 11\n");
	/* Branch to the target address */
	fprintf (acfg->fp, "bctr\n");

#ifdef PPC_USES_FUNCTION_DESCRIPTOR
	*tramp_size = 11 * 4;
#else
	*tramp_size = 9 * 4;
#endif

#else
	g_assert_not_reached ();
#endif
}	

/*
 * arch_emit_imt_thunk:
 *
 *   Emit an IMT thunk usable in full-aot mode. The thunk uses 1 got slot which
 * points to an array of pointer pairs. The pairs of the form [key, ptr], where
 * key is the IMT key, and ptr holds the address of a memory location holding
 * the address to branch to if the IMT arg matches the key. The array is 
 * terminated by a pair whose key is NULL, and whose ptr is the address of the 
 * fail_tramp.
 * TRAMP_SIZE is set to the size of the emitted trampoline.
 */
static void
arch_emit_imt_thunk (MonoAotCompile *acfg, int offset, int *tramp_size)
{
#if defined(TARGET_AMD64)
	guint8 *buf, *code;
	guint8 *labels [3];

	code = buf = g_malloc (256);

	/* FIXME: Optimize this, i.e. use binary search etc. */
	/* Maybe move the body into a separate function (slower, but much smaller) */

	/* R10 is a free register */

	labels [0] = code;
	amd64_alu_membase_imm (code, X86_CMP, AMD64_R10, 0, 0);
	labels [1] = code;
	amd64_branch8 (code, X86_CC_Z, FALSE, 0);

	/* Check key */
	amd64_alu_membase_reg (code, X86_CMP, AMD64_R10, 0, MONO_ARCH_IMT_REG);
	labels [2] = code;
	amd64_branch8 (code, X86_CC_Z, FALSE, 0);

	/* Loop footer */
	amd64_alu_reg_imm (code, X86_ADD, AMD64_R10, 2 * sizeof (gpointer));
	amd64_jump_code (code, labels [0]);

	/* Match */
	mono_amd64_patch (labels [2], code);
	amd64_mov_reg_membase (code, AMD64_R10, AMD64_R10, sizeof (gpointer), 8);
	amd64_jump_membase (code, AMD64_R10, 0);

	/* No match */
	/* FIXME: */
	mono_amd64_patch (labels [1], code);
	x86_breakpoint (code);

	/* mov <OFFSET>(%rip), %r10 */
	emit_byte (acfg, '\x4d');
	emit_byte (acfg, '\x8b');
	emit_byte (acfg, '\x15');
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) - 4);

	emit_bytes (acfg, buf, code - buf);
	
	*tramp_size = code - buf + 7;
#elif defined(TARGET_ARM)
	guint8 buf [128];
	guint8 *code, *code2, *labels [16];

	code = buf;

	/* The IMT method is in v5 */

	/* Need at least two free registers, plus a slot for storing the pc */
	ARM_PUSH (code, (1 << ARMREG_R0)|(1 << ARMREG_R1)|(1 << ARMREG_R2));
	labels [0] = code;
	/* Load the parameter from the GOT */
	ARM_LDR_IMM (code, ARMREG_R0, ARMREG_PC, 0);
	ARM_LDR_REG_REG (code, ARMREG_R0, ARMREG_PC, ARMREG_R0);

	labels [1] = code;
	ARM_LDR_IMM (code, ARMREG_R1, ARMREG_R0, 0);
	ARM_CMP_REG_REG (code, ARMREG_R1, ARMREG_V5);
	labels [2] = code;
	ARM_B_COND (code, ARMCOND_EQ, 0);

	/* End-of-loop check */
	ARM_CMP_REG_IMM (code, ARMREG_R1, 0, 0);
	labels [3] = code;
	ARM_B_COND (code, ARMCOND_EQ, 0);

	/* Loop footer */
	ARM_ADD_REG_IMM8 (code, ARMREG_R0, ARMREG_R0, sizeof (gpointer) * 2);
	labels [4] = code;
	ARM_B (code, 0);
	arm_patch (labels [4], labels [1]);

	/* Match */
	arm_patch (labels [2], code);
	ARM_LDR_IMM (code, ARMREG_R0, ARMREG_R0, 4);
	ARM_LDR_IMM (code, ARMREG_R0, ARMREG_R0, 0);
	/* Save it to the third stack slot */
	ARM_STR_IMM (code, ARMREG_R0, ARMREG_SP, 8);
	/* Restore the registers and branch */
	ARM_POP (code, (1 << ARMREG_R0)|(1 << ARMREG_R1)|(1 << ARMREG_PC));

	/* No match */
	arm_patch (labels [3], code);
	ARM_DBRK (code);

	/* Fixup offset */
	code2 = labels [0];
	ARM_LDR_IMM (code2, ARMREG_R0, ARMREG_PC, (code - (labels [0] + 8)));

	emit_bytes (acfg, buf, code - buf);
	emit_symbol_diff (acfg, acfg->got_symbol, ".", (offset * sizeof (gpointer)) + (code - (labels [0] + 8)) - 4);

	*tramp_size = code - buf + 4;
#elif defined(TARGET_POWERPC)
	guint8 buf [128];
	guint8 *code, *labels [16];

	code = buf;

	/* Load the mscorlib got address */
	ppc_ldptr (code, ppc_r11, sizeof (gpointer), ppc_r30);
	/* Load the parameter from the GOT */
	ppc_load (code, ppc_r0, offset * sizeof (gpointer));
	ppc_ldptr_indexed (code, ppc_r11, ppc_r11, ppc_r0);

	/* Load and check key */
	labels [1] = code;
	ppc_ldptr (code, ppc_r0, 0, ppc_r11);
	ppc_cmp (code, 0, sizeof (gpointer) == 8 ? 1 : 0, ppc_r0, MONO_ARCH_IMT_REG);
	labels [2] = code;
	ppc_bc (code, PPC_BR_TRUE, PPC_BR_EQ, 0);

	/* End-of-loop check */
	ppc_cmpi (code, 0, sizeof (gpointer) == 8 ? 1 : 0, ppc_r0, 0);
	labels [3] = code;
	ppc_bc (code, PPC_BR_TRUE, PPC_BR_EQ, 0);

	/* Loop footer */
	ppc_addi (code, ppc_r11, ppc_r11, 2 * sizeof (gpointer));
	labels [4] = code;
	ppc_b (code, 0);
	mono_ppc_patch (labels [4], labels [1]);

	/* Match */
	mono_ppc_patch (labels [2], code);
	ppc_ldptr (code, ppc_r11, sizeof (gpointer), ppc_r11);
	/* r11 now contains the value of the vtable slot */
	/* this is not a function descriptor on ppc64 */
	ppc_ldptr (code, ppc_r11, 0, ppc_r11);
	ppc_mtctr (code, ppc_r11);
	ppc_bcctr (code, PPC_BR_ALWAYS, 0);

	/* Fail */
	mono_ppc_patch (labels [3], code);
	/* FIXME: */
	ppc_break (code);

	*tramp_size = code - buf;

	emit_bytes (acfg, buf, code - buf);
#else
	g_assert_not_reached ();
#endif
}

static void
arch_emit_autoreg (MonoAotCompile *acfg, char *symbol)
{
#if defined(TARGET_POWERPC) && defined(__mono_ilp32__)
	/* Based on code generated by gcc */
	img_writer_emit_unset_mode (acfg->w);

	fprintf (acfg->fp,
#if defined(_MSC_VER) || defined(MONO_CROSS_COMPILE) 
			 ".section	.ctors,\"aw\",@progbits\n"
			 ".align 2\n"
			 ".globl	%s\n"
			 ".long	%s\n"
			 ".section	.opd,\"aw\"\n"
			 ".align 2\n"
			 "%s:\n"
			 ".long	.%s,.TOC.@tocbase32\n"
			 ".size	%s,.-%s\n"
			 ".section .text\n"
			 ".type	.%s,@function\n"
			 ".align 2\n"
			 ".%s:\n", symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol);
#else
			 ".section	.ctors,\"aw\",@progbits\n"
			 ".align 2\n"
			 ".globl	%1$s\n"
			 ".long	%1$s\n"
			 ".section	.opd,\"aw\"\n"
			 ".align 2\n"
			 "%1$s:\n"
			 ".long	.%1$s,.TOC.@tocbase32\n"
			 ".size	%1$s,.-%1$s\n"
			 ".section .text\n"
			 ".type	.%1$s,@function\n"
			 ".align 2\n"
			 ".%1$s:\n", symbol);
#endif


	fprintf (acfg->fp,
			 "stdu 1,-128(1)\n"
			 "mflr 0\n"
			 "std 31,120(1)\n"
			 "std 0,144(1)\n"

			 ".Lautoreg:\n"
			 "lis 3, .Lglobals@h\n"
			 "ori 3, 3, .Lglobals@l\n"
			 "bl .mono_aot_register_module\n"
			 "ld 11,0(1)\n"
			 "ld 0,16(11)\n"
			 "mtlr 0\n"
			 "ld 31,-8(11)\n"
			 "mr 1,11\n"
			 "blr\n"
			 );
#if defined(_MSC_VER) || defined(MONO_CROSS_COMPILE) 
		fprintf (acfg->fp,
			 ".size	.%s,.-.%s\n", symbol, symbol);
#else
	fprintf (acfg->fp,
			 ".size	.%1$s,.-.%1$s\n", symbol);
#endif
#else
#endif
}

/*
 * mono_arch_get_cie_program:
 *
 *   Get the unwind bytecode for the DWARF CIE.
 */
GSList*
mono_arch_get_cie_program (void)
{
#ifdef TARGET_AMD64
	GSList *l = NULL;

	mono_add_unwind_op_def_cfa (l, (guint8*)NULL, (guint8*)NULL, AMD64_RSP, 8);
	mono_add_unwind_op_offset (l, (guint8*)NULL, (guint8*)NULL, AMD64_RIP, -8);

	return l;
#elif defined(TARGET_POWERPC)
	GSList *l = NULL;

	mono_add_unwind_op_def_cfa (l, (guint8*)NULL, (guint8*)NULL, ppc_r1, 0);

	return l;
#else
	return NULL;
#endif
}

/* END OF ARCH SPECIFIC CODE */

static guint32
mono_get_field_token (MonoClassField *field) 
{
	MonoClass *klass = field->parent;
	int i;

	for (i = 0; i < klass->field.count; ++i) {
		if (field == &klass->fields [i])
			return MONO_TOKEN_FIELD_DEF | (klass->field.first + 1 + i);
	}

	g_assert_not_reached ();
	return 0;
}

static inline void
encode_value (gint32 value, guint8 *buf, guint8 **endbuf)
{
	guint8 *p = buf;

	//printf ("ENCODE: %d 0x%x.\n", value, value);

	/* 
	 * Same encoding as the one used in the metadata, extended to handle values
	 * greater than 0x1fffffff.
	 */
	if ((value >= 0) && (value <= 127))
		*p++ = value;
	else if ((value >= 0) && (value <= 16383)) {
		p [0] = 0x80 | (value >> 8);
		p [1] = value & 0xff;
		p += 2;
	} else if ((value >= 0) && (value <= 0x1fffffff)) {
		p [0] = (value >> 24) | 0xc0;
		p [1] = (value >> 16) & 0xff;
		p [2] = (value >> 8) & 0xff;
		p [3] = value & 0xff;
		p += 4;
	}
	else {
		p [0] = 0xff;
		p [1] = (value >> 24) & 0xff;
		p [2] = (value >> 16) & 0xff;
		p [3] = (value >> 8) & 0xff;
		p [4] = value & 0xff;
		p += 5;
	}
	if (endbuf)
		*endbuf = p;
}

static void
stream_init (MonoDynamicStream *sh)
{
	sh->index = 0;
	sh->alloc_size = 4096;
	sh->data = g_malloc (4096);

	/* So offsets are > 0 */
	sh->index ++;
}

static void
make_room_in_stream (MonoDynamicStream *stream, int size)
{
	if (size <= stream->alloc_size)
		return;
	
	while (stream->alloc_size <= size) {
		if (stream->alloc_size < 4096)
			stream->alloc_size = 4096;
		else
			stream->alloc_size *= 2;
	}
	
	stream->data = g_realloc (stream->data, stream->alloc_size);
}

static guint32
add_stream_data (MonoDynamicStream *stream, const char *data, guint32 len)
{
	guint32 idx;
	
	make_room_in_stream (stream, stream->index + len);
	memcpy (stream->data + stream->index, data, len);
	idx = stream->index;
	stream->index += len;
	return idx;
}

/*
 * add_to_blob:
 *
 *   Add data to the binary blob inside the aot image. Returns the offset inside the
 * blob where the data was stored.
 */
static guint32
add_to_blob (MonoAotCompile *acfg, guint8 *data, guint32 data_len)
{
	if (acfg->blob.alloc_size == 0)
		stream_init (&acfg->blob);

	return add_stream_data (&acfg->blob, (char*)data, data_len);
}

/*
 * emit_offset_table:
 *
 *   Emit a table of increasing offsets in a compact form using differential encoding.
 * There is an index entry for each GROUP_SIZE number of entries. The greater the
 * group size, the more compact the table becomes, but the slower it becomes to compute
 * a given entry. Returns the size of the table.
 */
static guint32
emit_offset_table (MonoAotCompile *acfg, int noffsets, int group_size, gint32 *offsets)
{
	gint32 current_offset;
	int i, buf_size, ngroups, index_entry_size;
	guint8 *p, *buf;
	guint32 *index_offsets;

	ngroups = (noffsets + (group_size - 1)) / group_size;

	index_offsets = g_new0 (guint32, ngroups);

	buf_size = noffsets * 4;
	p = buf = g_malloc0 (buf_size);

	current_offset = 0;
	for (i = 0; i < noffsets; ++i) {
		//printf ("D: %d -> %d\n", i, offsets [i]);
		if ((i % group_size) == 0) {
			index_offsets [i / group_size] = p - buf;
			/* Emit the full value for these entries */
			encode_value (offsets [i], p, &p);
		} else {
			/* The offsets are allowed to be non-increasing */
			//g_assert (offsets [i] >= current_offset);
			encode_value (offsets [i] - current_offset, p, &p);
		}
		current_offset = offsets [i];
	}

	if (ngroups && index_offsets [ngroups - 1] < 65000)
		index_entry_size = 2;
	else
		index_entry_size = 4;

	/* Emit the header */
	emit_int32 (acfg, noffsets);
	emit_int32 (acfg, group_size);
	emit_int32 (acfg, ngroups);
	emit_int32 (acfg, index_entry_size);

	/* Emit the index */
	for (i = 0; i < ngroups; ++i) {
		if (index_entry_size == 2)
			emit_int16 (acfg, index_offsets [i]);
		else
			emit_int32 (acfg, index_offsets [i]);
	}

	/* Emit the data */
	emit_bytes (acfg, buf, p - buf);

    return (int)(p - buf) + (ngroups * 4);
}

static guint32
get_image_index (MonoAotCompile *cfg, MonoImage *image)
{
	guint32 index;

	index = GPOINTER_TO_UINT (g_hash_table_lookup (cfg->image_hash, image));
	if (index)
		return index - 1;
	else {
		index = g_hash_table_size (cfg->image_hash);
		g_hash_table_insert (cfg->image_hash, image, GUINT_TO_POINTER (index + 1));
		g_ptr_array_add (cfg->image_table, image);
		return index;
	}
}

static guint32
find_typespec_for_class (MonoAotCompile *acfg, MonoClass *klass)
{
	int i;
	MonoClass *k = NULL;

	/* FIXME: Search referenced images as well */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_TYPESPEC].rows; ++i) {
		k = mono_class_get_full (acfg->image, MONO_TOKEN_TYPE_SPEC | (i + 1), NULL);
		if (k == klass)
			break;
	}

	if (i < acfg->image->tables [MONO_TABLE_TYPESPEC].rows)
		return MONO_TOKEN_TYPE_SPEC | (i + 1);
	else
		return 0;
}

static void
encode_method_ref (MonoAotCompile *acfg, MonoMethod *method, guint8 *buf, guint8 **endbuf);

/*
 * encode_klass_ref:
 *
 *   Encode a reference to KLASS. We use our home-grown encoding instead of the
 * standard metadata encoding.
 */
static void
encode_klass_ref (MonoAotCompile *acfg, MonoClass *klass, guint8 *buf, guint8 **endbuf)
{
	guint8 *p = buf;

	if (klass->generic_class) {
		guint32 token;
		g_assert (klass->type_token);

		/* Find a typespec for a class if possible */
		token = find_typespec_for_class (acfg, klass);
		if (token) {
			encode_value (token, p, &p);
			encode_value (get_image_index (acfg, acfg->image), p, &p);
		} else {
			MonoClass *gclass = klass->generic_class->container_class;
			MonoGenericInst *inst = klass->generic_class->context.class_inst;
			int i;

			/* Encode it ourselves */
			/* Marker */
			encode_value (MONO_TOKEN_TYPE_SPEC, p, &p);
			encode_value (MONO_TYPE_GENERICINST, p, &p);
			encode_klass_ref (acfg, gclass, p, &p);
			encode_value (inst->type_argc, p, &p);
			for (i = 0; i < inst->type_argc; ++i)
				encode_klass_ref (acfg, mono_class_from_mono_type (inst->type_argv [i]), p, &p);
		}
	} else if (klass->type_token) {
		g_assert (mono_metadata_token_code (klass->type_token) == MONO_TOKEN_TYPE_DEF);
		encode_value (klass->type_token - MONO_TOKEN_TYPE_DEF, p, &p);
		encode_value (get_image_index (acfg, klass->image), p, &p);
	} else if ((klass->byval_arg.type == MONO_TYPE_VAR) || (klass->byval_arg.type == MONO_TYPE_MVAR)) {
		MonoGenericContainer *container = mono_type_get_generic_param_owner (&klass->byval_arg);
		g_assert (container);

		/* Marker */
		encode_value (MONO_TOKEN_TYPE_SPEC, p, &p);
		encode_value (klass->byval_arg.type, p, &p);

		encode_value (mono_type_get_generic_param_num (&klass->byval_arg), p, &p);
		
		encode_value (container->is_method, p, &p);
		if (container->is_method)
			encode_method_ref (acfg, container->owner.method, p, &p);
		else
			encode_klass_ref (acfg, container->owner.klass, p, &p);
	} else {
		/* Array class */
		g_assert (klass->rank > 0);
		encode_value (MONO_TOKEN_TYPE_DEF, p, &p);
		encode_value (get_image_index (acfg, klass->image), p, &p);
		encode_value (klass->rank, p, &p);
		encode_klass_ref (acfg, klass->element_class, p, &p);
	}
	*endbuf = p;
}

static void
encode_field_info (MonoAotCompile *cfg, MonoClassField *field, guint8 *buf, guint8 **endbuf)
{
	guint32 token = mono_get_field_token (field);
	guint8 *p = buf;

	encode_klass_ref (cfg, field->parent, p, &p);
	g_assert (mono_metadata_token_code (token) == MONO_TOKEN_FIELD_DEF);
	encode_value (token - MONO_TOKEN_FIELD_DEF, p, &p);
	*endbuf = p;
}

static void
encode_generic_context (MonoAotCompile *acfg, MonoGenericContext *context, guint8 *buf, guint8 **endbuf)
{
	guint8 *p = buf;
	int i;
	MonoGenericInst *inst;

	/* Encode the context */
	inst = context->class_inst;
	encode_value (inst ? 1 : 0, p, &p);
	if (inst) {
		encode_value (inst->type_argc, p, &p);
		for (i = 0; i < inst->type_argc; ++i)
			encode_klass_ref (acfg, mono_class_from_mono_type (inst->type_argv [i]), p, &p);
	}
	inst = context->method_inst;
	encode_value (inst ? 1 : 0, p, &p);
	if (inst) {
		encode_value (inst->type_argc, p, &p);
		for (i = 0; i < inst->type_argc; ++i)
			encode_klass_ref (acfg, mono_class_from_mono_type (inst->type_argv [i]), p, &p);
	}

	*endbuf = p;
}

#define MAX_IMAGE_INDEX 250

static void
encode_method_ref (MonoAotCompile *acfg, MonoMethod *method, guint8 *buf, guint8 **endbuf)
{
	guint32 image_index = get_image_index (acfg, method->klass->image);
	guint32 token = method->token;
	MonoJumpInfoToken *ji;
	guint8 *p = buf;
	char *name;

	/*
	 * The encoding for most methods is as follows:
	 * - image index encoded as a leb128
	 * - token index encoded as a leb128
	 * Values of image index >= MONO_AOT_METHODREF_MIN are used to mark additional
	 * types of method encodings.
	 */

	g_assert (image_index < MONO_AOT_METHODREF_MIN);

	/* Mark methods which can't use aot trampolines because they need the further 
	 * processing in mono_magic_trampoline () which requires a MonoMethod*.
	 */
	if ((method->is_generic && (method->flags & METHOD_ATTRIBUTE_VIRTUAL)) ||
		(method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED))
		encode_value ((MONO_AOT_METHODREF_NO_AOT_TRAMPOLINE << 24), p, &p);

	/* 
	 * Some wrapper methods are shared using their signature, encode their 
	 * stringified signature instead.
	 * FIXME: Optimize disk usage
	 */
	name = NULL;
	if (method->wrapper_type) {
		if (method->wrapper_type == MONO_WRAPPER_RUNTIME_INVOKE) {
			char *tmpsig = mono_signature_get_desc (mono_method_signature (method), TRUE);
			if (strcmp (method->name, "runtime_invoke_dynamic")) {
				name = mono_aot_wrapper_name (method);
			} else if (mono_marshal_method_from_wrapper (method) != method) {
				/* Direct wrapper, encode it normally */
			} else {
				name = g_strdup_printf ("(wrapper runtime-invoke):%s (%s)", method->name, tmpsig);
			}
			g_free (tmpsig);
		} else if (method->wrapper_type == MONO_WRAPPER_DELEGATE_INVOKE) {
			char *tmpsig = mono_signature_get_desc (mono_method_signature (method), TRUE);
			name = g_strdup_printf ("(wrapper delegate-invoke):%s (%s)", method->name, tmpsig);
			g_free (tmpsig);
		} else if (method->wrapper_type == MONO_WRAPPER_DELEGATE_BEGIN_INVOKE) {
			char *tmpsig = mono_signature_get_desc (mono_method_signature (method), TRUE);
			name = g_strdup_printf ("(wrapper delegate-begin-invoke):%s (%s)", method->name, tmpsig);
			g_free (tmpsig);
		} else if (method->wrapper_type == MONO_WRAPPER_DELEGATE_END_INVOKE) {
			char *tmpsig = mono_signature_get_desc (mono_method_signature (method), TRUE);
			name = g_strdup_printf ("(wrapper delegate-end-invoke):%s (%s)", method->name, tmpsig);
			g_free (tmpsig);
		}
	}

	if (name) {
		encode_value ((MONO_AOT_METHODREF_WRAPPER_NAME << 24), p, &p);
		strcpy ((char*)p, name);
		p += strlen (name) + 1;
		g_free (name);
	} else if (method->wrapper_type) {
		encode_value ((MONO_AOT_METHODREF_WRAPPER << 24), p, &p);

		encode_value (method->wrapper_type, p, &p);

		switch (method->wrapper_type) {
		case MONO_WRAPPER_REMOTING_INVOKE:
		case MONO_WRAPPER_REMOTING_INVOKE_WITH_CHECK:
		case MONO_WRAPPER_XDOMAIN_INVOKE: {
			MonoMethod *m;

			m = mono_marshal_method_from_wrapper (method);
			g_assert (m);
			encode_method_ref (acfg, m, p, &p);
			break;
		}
		case MONO_WRAPPER_PROXY_ISINST:
		case MONO_WRAPPER_LDFLD:
		case MONO_WRAPPER_LDFLDA:
		case MONO_WRAPPER_STFLD:
		case MONO_WRAPPER_ISINST: {
			MonoClass *proxy_class = mono_marshal_wrapper_info_from_wrapper (method);
			encode_klass_ref (acfg, proxy_class, p, &p);
			break;
		}
		case MONO_WRAPPER_LDFLD_REMOTE:
		case MONO_WRAPPER_STFLD_REMOTE:
			break;
		case MONO_WRAPPER_ALLOC: {
			int alloc_type = mono_gc_get_managed_allocator_type (method);
			g_assert (alloc_type != -1);
			encode_value (alloc_type, p, &p);
			break;
		}
		case MONO_WRAPPER_STELEMREF:
			break;
		case MONO_WRAPPER_UNKNOWN:
			if (strcmp (method->name, "FastMonitorEnter") == 0)
				encode_value (MONO_AOT_WRAPPER_MONO_ENTER, p, &p);
			else if (strcmp (method->name, "FastMonitorExit") == 0)
				encode_value (MONO_AOT_WRAPPER_MONO_EXIT, p, &p);
			else
				g_assert_not_reached ();
			break;
		case MONO_WRAPPER_SYNCHRONIZED:
		case MONO_WRAPPER_MANAGED_TO_NATIVE:
		case MONO_WRAPPER_RUNTIME_INVOKE: {
			MonoMethod *m;

			m = mono_marshal_method_from_wrapper (method);
			g_assert (m);
			g_assert (m != method);
			encode_method_ref (acfg, m, p, &p);
			break;
		}
		case MONO_WRAPPER_MANAGED_TO_MANAGED:
			if (!strcmp (method->name, "ElementAddr")) {
				ElementAddrWrapperInfo *info = mono_marshal_wrapper_info_from_wrapper (method);

				g_assert (info);
				encode_value (MONO_AOT_WRAPPER_ELEMENT_ADDR, p, &p);
				encode_value (info->rank, p, &p);
				encode_value (info->elem_size, p, &p);
			} else {
				g_assert_not_reached ();
			}
			break;
		default:
			g_assert_not_reached ();
		}
	} else if (mono_method_signature (method)->is_inflated) {
		/* 
		 * This is a generic method, find the original token which referenced it and
		 * encode that.
		 * Obtain the token from information recorded by the JIT.
		 */
		ji = g_hash_table_lookup (acfg->token_info_hash, method);
		if (ji) {
			image_index = get_image_index (acfg, ji->image);
			g_assert (image_index < MAX_IMAGE_INDEX);
			token = ji->token;

			encode_value ((MONO_AOT_METHODREF_METHODSPEC << 24), p, &p);
			encode_value (image_index, p, &p);
			encode_value (token, p, &p);
		} else {
			MonoMethod *declaring;
			MonoGenericContext *context = mono_method_get_context (method);

			g_assert (method->is_inflated);
			declaring = ((MonoMethodInflated*)method)->declaring;

			/*
			 * This might be a non-generic method of a generic instance, which 
			 * doesn't have a token since the reference is generated by the JIT 
			 * like Nullable:Box/Unbox, or by generic sharing.
			 */

			encode_value ((MONO_AOT_METHODREF_GINST << 24), p, &p);
			/* Encode the klass */
			encode_klass_ref (acfg, method->klass, p, &p);
			/* Encode the method */
			image_index = get_image_index (acfg, method->klass->image);
			g_assert (image_index < MAX_IMAGE_INDEX);
			g_assert (declaring->token);
			token = declaring->token;
			g_assert (mono_metadata_token_table (token) == MONO_TABLE_METHOD);
			encode_value (image_index, p, &p);
			encode_value (token, p, &p);
			encode_generic_context (acfg, context, p, &p);
		}
	} else if (token == 0) {
		/* This might be a method of a constructed type like int[,].Set */
		/* Obtain the token from information recorded by the JIT */
		ji = g_hash_table_lookup (acfg->token_info_hash, method);
		if (ji) {
			image_index = get_image_index (acfg, ji->image);
			g_assert (image_index < MAX_IMAGE_INDEX);
			token = ji->token;

			encode_value ((MONO_AOT_METHODREF_METHODSPEC << 24), p, &p);
			encode_value (image_index, p, &p);
			encode_value (token, p, &p);
		} else {
			/* Array methods */
			g_assert (method->klass->rank);

			/* Encode directly */
			encode_value ((MONO_AOT_METHODREF_ARRAY << 24), p, &p);
			encode_klass_ref (acfg, method->klass, p, &p);
			if (!strcmp (method->name, ".ctor") && mono_method_signature (method)->param_count == method->klass->rank)
				encode_value (0, p, &p);
			else if (!strcmp (method->name, ".ctor") && mono_method_signature (method)->param_count == method->klass->rank * 2)
				encode_value (1, p, &p);
			else if (!strcmp (method->name, "Get"))
				encode_value (2, p, &p);
			else if (!strcmp (method->name, "Address"))
				encode_value (3, p, &p);
			else if (!strcmp (method->name, "Set"))
				encode_value (4, p, &p);
			else
				g_assert_not_reached ();
		}
	} else {
		g_assert (mono_metadata_token_table (token) == MONO_TABLE_METHOD);
		encode_value ((image_index << 24) | mono_metadata_token_index (token), p, &p);
	}
	*endbuf = p;
}

static gint
compare_patches (gconstpointer a, gconstpointer b)
{
	int i, j;

	i = (*(MonoJumpInfo**)a)->ip.i;
	j = (*(MonoJumpInfo**)b)->ip.i;

	if (i < j)
		return -1;
	else
		if (i > j)
			return 1;
	else
		return 0;
}

/*
 * is_plt_patch:
 *
 *   Return whenever PATCH_INFO refers to a direct call, and thus requires a
 * PLT entry.
 */
static inline gboolean
is_plt_patch (MonoJumpInfo *patch_info)
{
	switch (patch_info->type) {
	case MONO_PATCH_INFO_METHOD:
	case MONO_PATCH_INFO_INTERNAL_METHOD:
	case MONO_PATCH_INFO_JIT_ICALL_ADDR:
	case MONO_PATCH_INFO_ICALL_ADDR:
	case MONO_PATCH_INFO_CLASS_INIT:
	case MONO_PATCH_INFO_RGCTX_FETCH:
	case MONO_PATCH_INFO_GENERIC_CLASS_INIT:
	case MONO_PATCH_INFO_MONITOR_ENTER:
	case MONO_PATCH_INFO_MONITOR_EXIT:
	case MONO_PATCH_INFO_LLVM_IMT_TRAMPOLINE:
		return TRUE;
	default:
		return FALSE;
	}
}

static int
get_plt_offset (MonoAotCompile *acfg, MonoJumpInfo *patch_info)
{
	int res = -1;

	if (is_plt_patch (patch_info)) {
		int idx = GPOINTER_TO_UINT (g_hash_table_lookup (acfg->patch_to_plt_offset, patch_info));

		// FIXME: This breaks the calculation of final_got_size		
		if (!acfg->llvm && patch_info->type == MONO_PATCH_INFO_METHOD && (patch_info->data.method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED)) {
			/* 
			 * Allocate a separate PLT slot for each such patch, since some plt
			 * entries will refer to the method itself, and some will refer to the
			 * wrapper.
			 */
			idx = 0;
		}

		if (idx) {
			res = idx;
		} else {
			MonoJumpInfo *new_ji = mono_patch_info_dup_mp (acfg->mempool, patch_info);

			g_assert (!acfg->final_got_size);

			res = acfg->plt_offset;
			g_hash_table_insert (acfg->plt_offset_to_patch, GUINT_TO_POINTER (res), new_ji);
			g_hash_table_insert (acfg->patch_to_plt_offset, new_ji, GUINT_TO_POINTER (res));
			acfg->plt_offset ++;
		}
	}

	return res;
}

/**
 * get_got_offset:
 *
 *   Returns the offset of the GOT slot where the runtime object resulting from resolving
 * JI could be found if it exists, otherwise allocates a new one.
 */
static guint32
get_got_offset (MonoAotCompile *acfg, MonoJumpInfo *ji)
{
	guint32 got_offset;

	got_offset = GPOINTER_TO_UINT (g_hash_table_lookup (acfg->patch_to_got_offset_by_type [ji->type], ji));
	if (got_offset)
		return got_offset - 1;

	g_assert (!acfg->final_got_size);

	got_offset = acfg->got_offset;
	acfg->got_offset ++;

	acfg->stats.got_slots ++;
	acfg->stats.got_slot_types [ji->type] ++;

	g_hash_table_insert (acfg->patch_to_got_offset, ji, GUINT_TO_POINTER (got_offset + 1));
	g_hash_table_insert (acfg->patch_to_got_offset_by_type [ji->type], ji, GUINT_TO_POINTER (got_offset + 1));
	g_ptr_array_add (acfg->got_patches, ji);

	return got_offset;
}

/* Add a method to the list of methods which need to be emitted */
static void
add_method_with_index (MonoAotCompile *acfg, MonoMethod *method, int index, gboolean extra)
{
	g_assert (method);
	if (!g_hash_table_lookup (acfg->method_indexes, method)) {
		g_ptr_array_add (acfg->methods, method);
		g_hash_table_insert (acfg->method_indexes, method, GUINT_TO_POINTER (index + 1));
		acfg->nmethods = acfg->methods->len + 1;
	}

	if (method->wrapper_type || extra)
		g_ptr_array_add (acfg->extra_methods, method);
}

static guint32
get_method_index (MonoAotCompile *acfg, MonoMethod *method)
{
	int index = GPOINTER_TO_UINT (g_hash_table_lookup (acfg->method_indexes, method));
	
	g_assert (index);

	return index - 1;
}

static int
add_method_full (MonoAotCompile *acfg, MonoMethod *method, gboolean extra, int depth)
{
	int index;

	index = GPOINTER_TO_UINT (g_hash_table_lookup (acfg->method_indexes, method));
	if (index)
		return index - 1;

	index = acfg->method_index;
	add_method_with_index (acfg, method, index, extra);

	/* FIXME: Fix quadratic behavior */
	acfg->method_order = g_list_append (acfg->method_order, GUINT_TO_POINTER (index));

	g_hash_table_insert (acfg->method_depth, method, GUINT_TO_POINTER (depth));

	acfg->method_index ++;

	return index;
}

static int
add_method (MonoAotCompile *acfg, MonoMethod *method)
{
	return add_method_full (acfg, method, FALSE, 0);
}

static void
add_extra_method (MonoAotCompile *acfg, MonoMethod *method)
{
	add_method_full (acfg, method, TRUE, 0);
}

static void
add_extra_method_with_depth (MonoAotCompile *acfg, MonoMethod *method, int depth)
{
	add_method_full (acfg, method, TRUE, depth);
}

static void
add_jit_icall_wrapper (gpointer key, gpointer value, gpointer user_data)
{
	MonoAotCompile *acfg = user_data;
	MonoJitICallInfo *callinfo = value;
	MonoMethod *wrapper;
	char *name;

	if (!callinfo->sig)
		return;

	name = g_strdup_printf ("__icall_wrapper_%s", callinfo->name);
	wrapper = mono_marshal_get_icall_wrapper (callinfo->sig, name, callinfo->func, check_for_pending_exc);
	g_free (name);

	add_method (acfg, wrapper);
}

static MonoMethod*
get_runtime_invoke_sig (MonoMethodSignature *sig)
{
	MonoMethodBuilder *mb;
	MonoMethod *m;

	mb = mono_mb_new (mono_defaults.object_class, "FOO", MONO_WRAPPER_NONE);
	m = mono_mb_create_method (mb, sig, 16);
	return mono_marshal_get_runtime_invoke (m, FALSE);
}

static gboolean
can_marshal_struct (MonoClass *klass)
{
	MonoClassField *field;
	gboolean can_marshal = TRUE;
	gpointer iter = NULL;

	if ((klass->flags & TYPE_ATTRIBUTE_LAYOUT_MASK) == TYPE_ATTRIBUTE_AUTO_LAYOUT)
		return FALSE;

	/* Only allow a few field types to avoid asserts in the marshalling code */
	while ((field = mono_class_get_fields (klass, &iter))) {
		if ((field->type->attrs & FIELD_ATTRIBUTE_STATIC))
			continue;

		switch (field->type->type) {
		case MONO_TYPE_I4:
		case MONO_TYPE_U4:
		case MONO_TYPE_I1:
		case MONO_TYPE_U1:
		case MONO_TYPE_BOOLEAN:
		case MONO_TYPE_I2:
		case MONO_TYPE_U2:
		case MONO_TYPE_CHAR:
		case MONO_TYPE_I8:
		case MONO_TYPE_U8:
		case MONO_TYPE_I:
		case MONO_TYPE_U:
		case MONO_TYPE_PTR:
		case MONO_TYPE_R4:
		case MONO_TYPE_R8:
		case MONO_TYPE_STRING:
			break;
		case MONO_TYPE_VALUETYPE:
			if (!mono_class_from_mono_type (field->type)->enumtype && !can_marshal_struct (mono_class_from_mono_type (field->type)))
				can_marshal = FALSE;
			break;
		default:
			can_marshal = FALSE;
			break;
		}
	}

	/* Special cases */
	/* Its hard to compute whenever these can be marshalled or not */
	if (!strcmp (klass->name_space, "System.Net.NetworkInformation.MacOsStructs"))
		return TRUE;

	return can_marshal;
}

static void
add_wrappers (MonoAotCompile *acfg)
{
	MonoMethod *method, *m;
	int i, j;
	MonoMethodSignature *sig, *csig;
	guint32 token;

	/* 
	 * FIXME: Instead of AOTing all the wrappers, it might be better to redesign them
	 * so there is only one wrapper of a given type, or inlining their contents into their
	 * callers.
	 */

	/* 
	 * FIXME: This depends on the fact that different wrappers have different 
	 * names.
	 */

	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHOD].rows; ++i) {
		MonoMethod *method;
		guint32 token = MONO_TOKEN_METHOD_DEF | (i + 1);
		gboolean skip = FALSE;

		method = mono_get_method (acfg->image, token, NULL);

		if ((method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) ||
			(method->iflags & METHOD_IMPL_ATTRIBUTE_RUNTIME) ||
			(method->flags & METHOD_ATTRIBUTE_ABSTRACT))
			skip = TRUE;

		if (method->is_generic || method->klass->generic_container)
			skip = TRUE;

		/* Skip methods which can not be handled by get_runtime_invoke () */
		sig = mono_method_signature (method);
		if ((sig->ret->type == MONO_TYPE_PTR) ||
			(sig->ret->type == MONO_TYPE_TYPEDBYREF))
			skip = TRUE;

		for (j = 0; j < sig->param_count; j++) {
			if (sig->params [j]->type == MONO_TYPE_TYPEDBYREF)
				skip = TRUE;
		}

#ifdef MONO_ARCH_DYN_CALL_SUPPORTED
		if (!method->klass->contextbound) {
			MonoDynCallInfo *info = mono_arch_dyn_call_prepare (sig);
			gboolean has_nullable = FALSE;

			for (j = 0; j < sig->param_count; j++) {
				if (sig->params [j]->type == MONO_TYPE_GENERICINST && mono_class_is_nullable (mono_class_from_mono_type (sig->params [j])))
					has_nullable = TRUE;
			}

			if (info && !has_nullable) {
				/* Supported by the dynamic runtime-invoke wrapper */
				skip = TRUE;
				g_free (info);
			}
		}
#endif

		if (!skip) {
			//printf ("%s\n", mono_method_full_name (method, TRUE));
			add_method (acfg, mono_marshal_get_runtime_invoke (method, FALSE));
		}
 	}

	if (strcmp (acfg->image->assembly->aname.name, "mscorlib") == 0) {
#ifdef MONO_ARCH_HAVE_TLS_GET
		MonoMethodDesc *desc;
		MonoMethod *orig_method;
		int nallocators;
#endif

		/* Runtime invoke wrappers */

		/* void runtime-invoke () [.cctor] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 0);
		csig->ret = &mono_defaults.void_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* void runtime-invoke () [Finalize] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 0);
		csig->hasthis = 1;
		csig->ret = &mono_defaults.void_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* void runtime-invoke (string) [exception ctor] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 1);
		csig->hasthis = 1;
		csig->ret = &mono_defaults.void_class->byval_arg;
		csig->params [0] = &mono_defaults.string_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* void runtime-invoke (string, string) [exception ctor] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 2);
		csig->hasthis = 1;
		csig->ret = &mono_defaults.void_class->byval_arg;
		csig->params [0] = &mono_defaults.string_class->byval_arg;
		csig->params [1] = &mono_defaults.string_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* string runtime-invoke () [Exception.ToString ()] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 0);
		csig->hasthis = 1;
		csig->ret = &mono_defaults.string_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* void runtime-invoke (string, Exception) [exception ctor] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 2);
		csig->hasthis = 1;
		csig->ret = &mono_defaults.void_class->byval_arg;
		csig->params [0] = &mono_defaults.string_class->byval_arg;
		csig->params [1] = &mono_defaults.exception_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* Assembly runtime-invoke (string, bool) [DoAssemblyResolve] */
		csig = mono_metadata_signature_alloc (mono_defaults.corlib, 2);
		csig->hasthis = 1;
		csig->ret = &(mono_class_from_name (
											mono_defaults.corlib, "System.Reflection", "Assembly"))->byval_arg;
		csig->params [0] = &mono_defaults.string_class->byval_arg;
		csig->params [1] = &mono_defaults.boolean_class->byval_arg;
		add_method (acfg, get_runtime_invoke_sig (csig));

		/* runtime-invoke used by finalizers */
		add_method (acfg, mono_marshal_get_runtime_invoke (mono_class_get_method_from_name_flags (mono_defaults.object_class, "Finalize", 0, 0), TRUE));

		/* This is used by mono_runtime_capture_context () */
		method = mono_get_context_capture_method ();
		if (method)
			add_method (acfg, mono_marshal_get_runtime_invoke (method, FALSE));

#ifdef MONO_ARCH_DYN_CALL_SUPPORTED
		add_method (acfg, mono_marshal_get_runtime_invoke_dynamic ());
#endif

		/* JIT icall wrappers */
		/* FIXME: locking */
		g_hash_table_foreach (mono_get_jit_icall_info (), add_jit_icall_wrapper, acfg);

		/* stelemref */
		add_method (acfg, mono_marshal_get_stelemref ());

#ifdef MONO_ARCH_HAVE_TLS_GET
		/* Managed Allocators */
		nallocators = mono_gc_get_managed_allocator_types ();
		for (i = 0; i < nallocators; ++i) {
			m = mono_gc_get_managed_allocator_by_type (i);
			if (m)
				add_method (acfg, m);
		}

		/* Monitor Enter/Exit */
		desc = mono_method_desc_new ("Monitor:Enter", FALSE);
		orig_method = mono_method_desc_search_in_class (desc, mono_defaults.monitor_class);
		g_assert (orig_method);
		mono_method_desc_free (desc);
		method = mono_monitor_get_fast_path (orig_method);
		if (method)
			add_method (acfg, method);

		desc = mono_method_desc_new ("Monitor:Exit", FALSE);
		orig_method = mono_method_desc_search_in_class (desc, mono_defaults.monitor_class);
		g_assert (orig_method);
		mono_method_desc_free (desc);
		method = mono_monitor_get_fast_path (orig_method);
		if (method)
			add_method (acfg, method);
#endif
	}

	/* 
	 * remoting-invoke-with-check wrappers are very frequent, so avoid emitting them,
	 * we use the original method instead at runtime.
	 * Since full-aot doesn't support remoting, this is not a problem.
	 */
#if 0
	/* remoting-invoke wrappers */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHOD].rows; ++i) {
		MonoMethodSignature *sig;
		
		token = MONO_TOKEN_METHOD_DEF | (i + 1);
		method = mono_get_method (acfg->image, token, NULL);

		sig = mono_method_signature (method);

		if (sig->hasthis && (method->klass->marshalbyref || method->klass == mono_defaults.object_class)) {
			m = mono_marshal_get_remoting_invoke_with_check (method);

			add_method (acfg, m);
		}
	}
#endif

	/* delegate-invoke wrappers */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_TYPEDEF].rows; ++i) {
		MonoClass *klass;
		
		token = MONO_TOKEN_TYPE_DEF | (i + 1);
		klass = mono_class_get (acfg->image, token);

		if (klass->delegate && klass != mono_defaults.delegate_class && klass != mono_defaults.multicastdelegate_class && !klass->generic_container) {
			method = mono_get_delegate_invoke (klass);

			m = mono_marshal_get_delegate_invoke (method, NULL);

			add_method (acfg, m);

			method = mono_class_get_method_from_name_flags (klass, "BeginInvoke", -1, 0);
			add_method (acfg, mono_marshal_get_delegate_begin_invoke (method));

			method = mono_class_get_method_from_name_flags (klass, "EndInvoke", -1, 0);
			add_method (acfg, mono_marshal_get_delegate_end_invoke (method));
		}
	}

	/* Synchronized wrappers */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHOD].rows; ++i) {
		token = MONO_TOKEN_METHOD_DEF | (i + 1);
		method = mono_get_method (acfg->image, token, NULL);

		if (method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED && !method->is_generic)
			add_method (acfg, mono_marshal_get_synchronized_wrapper (method));
	}

	/* pinvoke wrappers */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHOD].rows; ++i) {
		MonoMethod *method;
		guint32 token = MONO_TOKEN_METHOD_DEF | (i + 1);

		method = mono_get_method (acfg->image, token, NULL);

		if ((method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) ||
			(method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL)) {
			add_method (acfg, mono_marshal_get_native_wrapper (method, TRUE, TRUE));
		}
	}
 
	/* native-to-managed wrappers */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHOD].rows; ++i) {
		MonoMethod *method;
		guint32 token = MONO_TOKEN_METHOD_DEF | (i + 1);
		MonoCustomAttrInfo *cattr;
		int j;

		method = mono_get_method (acfg->image, token, NULL);

		/* 
		 * Only generate native-to-managed wrappers for methods which have an
		 * attribute named MonoPInvokeCallbackAttribute. We search for the attribute by
		 * name to avoid defining a new assembly to contain it.
		 */
		cattr = mono_custom_attrs_from_method (method);

		if (cattr) {
			for (j = 0; j < cattr->num_attrs; ++j)
				if (cattr->attrs [j].ctor && !strcmp (cattr->attrs [j].ctor->klass->name, "MonoPInvokeCallbackAttribute"))
					break;
			if (j < cattr->num_attrs) {
				MonoCustomAttrEntry *e = &cattr->attrs [j];
				MonoMethodSignature *sig = mono_method_signature (e->ctor);
				const char *p = (const char*)e->data;
				int slen;
				char *n;
				MonoType *t;
				MonoClass *klass;

				g_assert (method->flags & METHOD_ATTRIBUTE_STATIC);

				g_assert (sig->param_count == 1);
				g_assert (sig->params [0]->type == MONO_TYPE_CLASS && !strcmp (mono_class_from_mono_type (sig->params [0])->name, "Type"));

				/* 
				 * Decode the cattr manually since we can't create objects
				 * during aot compilation.
				 */
					
				/* Skip prolog */
				p += 2;

				/* From load_cattr_value () in reflection.c */
				slen = mono_metadata_decode_value (p, &p);
				n = g_memdup (p, slen + 1);
				n [slen] = 0;
				t = mono_reflection_type_from_name (n, acfg->image);
				g_assert (t);
				g_free (n);

				klass = mono_class_from_mono_type (t);
				g_assert (klass->parent == mono_defaults.multicastdelegate_class);

				add_method (acfg, mono_marshal_get_managed_wrapper (method, klass, NULL));
			}
		}

		if ((method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) ||
			(method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL)) {
			add_method (acfg, mono_marshal_get_native_wrapper (method, TRUE, TRUE));
		}
	}

	/* StructureToPtr/PtrToStructure wrappers */
	for (i = 0; i < acfg->image->tables [MONO_TABLE_TYPEDEF].rows; ++i) {
		MonoClass *klass;
		
		token = MONO_TOKEN_TYPE_DEF | (i + 1);
		klass = mono_class_get (acfg->image, token);

		if (klass->valuetype && !klass->generic_container && can_marshal_struct (klass)) {
			add_method (acfg, mono_marshal_get_struct_to_ptr (klass));
			add_method (acfg, mono_marshal_get_ptr_to_struct (klass));
		}
	}
}

static gboolean
has_type_vars (MonoClass *klass)
{
	if ((klass->byval_arg.type == MONO_TYPE_VAR) || (klass->byval_arg.type == MONO_TYPE_MVAR))
		return TRUE;
	if (klass->rank)
		return has_type_vars (klass->element_class);
	if (klass->generic_class) {
		MonoGenericContext *context = &klass->generic_class->context;
		if (context->class_inst) {
			int i;

			for (i = 0; i < context->class_inst->type_argc; ++i)
				if (has_type_vars (mono_class_from_mono_type (context->class_inst->type_argv [i])))
					return TRUE;
		}
	}
	return FALSE;
}

static gboolean
method_has_type_vars (MonoMethod *method)
{
	if (has_type_vars (method->klass))
		return TRUE;

	if (method->is_inflated) {
		MonoGenericContext *context = mono_method_get_context (method);
		if (context->method_inst) {
			int i;

			for (i = 0; i < context->method_inst->type_argc; ++i)
				if (has_type_vars (mono_class_from_mono_type (context->method_inst->type_argv [i])))
					return TRUE;
		}
	}
	return FALSE;
}

/*
 * add_generic_class:
 *
 *   Add all methods of a generic class.
 */
static void
add_generic_class (MonoAotCompile *acfg, MonoClass *klass)
{
	MonoMethod *method;
	gpointer iter;

	mono_class_init (klass);

	if (klass->generic_class && klass->generic_class->context.class_inst->is_open)
		return;

	if (has_type_vars (klass))
		return;

	if (!klass->generic_class && !klass->rank)
		return;

	iter = NULL;
	while ((method = mono_class_get_methods (klass, &iter))) {
		if (mono_method_is_generic_sharable_impl (method, FALSE))
			/* Already added */
			continue;

		if (method->is_generic)
			/* FIXME: */
			continue;

		/*
		 * FIXME: Instances which are referenced by these methods are not added,
		 * for example Array.Resize<int> for List<int>.Add ().
		 */
		add_extra_method (acfg, method);
	}

	if (klass->delegate) {
		method = mono_get_delegate_invoke (klass);

		method = mono_marshal_get_delegate_invoke (method, NULL);

		add_method (acfg, method);
	}

	/* 
	 * For ICollection<T>, add instances of the helper methods
	 * in Array, since a T[] could be cast to ICollection<T>.
	 */
	if (klass->image == mono_defaults.corlib && !strcmp (klass->name_space, "System.Collections.Generic") &&
		(!strcmp(klass->name, "ICollection`1") || !strcmp (klass->name, "IEnumerable`1") || !strcmp (klass->name, "IList`1") || !strcmp (klass->name, "IEnumerator`1"))) {
		MonoClass *tclass = mono_class_from_mono_type (klass->generic_class->context.class_inst->type_argv [0]);
		MonoClass *array_class = mono_bounded_array_class_get (tclass, 1, FALSE);
		gpointer iter;
		char *name_prefix;

		if (!strcmp (klass->name, "IEnumerator`1"))
			name_prefix = g_strdup_printf ("%s.%s", klass->name_space, "IEnumerable`1");
		else
			name_prefix = g_strdup_printf ("%s.%s", klass->name_space, klass->name);

		/* Add the T[]/InternalEnumerator class */
		if (!strcmp (klass->name, "IEnumerable`1") || !strcmp (klass->name, "IEnumerator`1")) {
			MonoClass *nclass;

			iter = NULL;
			while ((nclass = mono_class_get_nested_types (array_class->parent, &iter))) {
				if (!strcmp (nclass->name, "InternalEnumerator`1"))
					break;
			}
			g_assert (nclass);
			nclass = mono_class_inflate_generic_class (nclass, mono_generic_class_get_context (klass->generic_class));
			add_generic_class (acfg, nclass);
		}

		iter = NULL;
		while ((method = mono_class_get_methods (array_class, &iter))) {
			if (strstr (method->name, name_prefix)) {
				MonoMethod *m = mono_aot_get_array_helper_from_wrapper (method);
				add_extra_method (acfg, m);
			}
		}

		g_free (name_prefix);
	}

	/* Add an instance of GenericComparer<T> which is created dynamically by Comparer<T> */
	if (klass->image == mono_defaults.corlib && !strcmp (klass->name_space, "System.Collections.Generic") && !strcmp (klass->name, "Comparer`1")) {
		MonoClass *tclass = mono_class_from_mono_type (klass->generic_class->context.class_inst->type_argv [0]);
		MonoClass *icomparable, *gcomparer;
		MonoGenericContext ctx;
		MonoType *args [16];

		memset (&ctx, 0, sizeof (ctx));

		icomparable = mono_class_from_name (mono_defaults.corlib, "System", "IComparable`1");
		g_assert (icomparable);
		args [0] = &tclass->byval_arg;
		ctx.class_inst = mono_metadata_get_generic_inst (1, args);

		if (mono_class_is_assignable_from (mono_class_inflate_generic_class (icomparable, &ctx), tclass)) {
			gcomparer = mono_class_from_name (mono_defaults.corlib, "System.Collections.Generic", "GenericComparer`1");
			g_assert (gcomparer);
			add_generic_class (acfg, mono_class_inflate_generic_class (gcomparer, &ctx));
		}
	}
}

static void
add_instances_of (MonoAotCompile *acfg, MonoClass *klass, MonoType **insts, int ninsts)
{
	int i;
	MonoGenericContext ctx;
	MonoType *args [16];

	memset (&ctx, 0, sizeof (ctx));

	for (i = 0; i < ninsts; ++i) {
		args [0] = insts [i];
		ctx.class_inst = mono_metadata_get_generic_inst (1, args);
		add_generic_class (acfg, mono_class_inflate_generic_class (klass, &ctx));
	}
}

/*
 * add_generic_instances:
 *
 *   Add instances referenced by the METHODSPEC/TYPESPEC table.
 */
static void
add_generic_instances (MonoAotCompile *acfg)
{
	int i;
	guint32 token;
	MonoMethod *method;
	MonoMethodHeader *header;
	MonoMethodSignature *sig;
	MonoGenericContext *context;

	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHODSPEC].rows; ++i) {
		token = MONO_TOKEN_METHOD_SPEC | (i + 1);
		method = mono_get_method (acfg->image, token, NULL);

		context = mono_method_get_context (method);
		if (context && ((context->class_inst && context->class_inst->is_open) ||
						(context->method_inst && context->method_inst->is_open)))
			continue;

		if (method->klass->image != acfg->image)
			continue;

		if (mono_method_is_generic_sharable_impl (method, FALSE))
			/* Already added */
			continue;

		add_extra_method (acfg, method);
	}

	for (i = 0; i < acfg->image->tables [MONO_TABLE_TYPESPEC].rows; ++i) {
		MonoClass *klass;

		token = MONO_TOKEN_TYPE_SPEC | (i + 1);

		klass = mono_class_get (acfg->image, token);
		if (!klass || klass->rank)
			continue;

		add_generic_class (acfg, klass);
	}

	/* Add types of args/locals */
	for (i = 0; i < acfg->methods->len; ++i) {
		int j;

		method = g_ptr_array_index (acfg->methods, i);

		sig = mono_method_signature (method);

		if (sig) {
			for (j = 0; j < sig->param_count; ++j)
				if (sig->params [j]->type == MONO_TYPE_GENERICINST)
					add_generic_class (acfg, mono_class_from_mono_type (sig->params [j]));
		}

		header = mono_method_get_header (method);

		if (header) {
			for (j = 0; j < header->num_locals; ++j)
				if (header->locals [j]->type == MONO_TYPE_GENERICINST)
					add_generic_class (acfg, mono_class_from_mono_type (header->locals [j]));
		}
	}

	if (acfg->image == mono_defaults.corlib) {
		MonoClass *klass;
		MonoType *insts [256];
		int ninsts = 0;

		insts [ninsts ++] = &mono_defaults.byte_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.sbyte_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.int16_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.uint16_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.int32_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.uint32_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.int64_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.uint64_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.single_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.double_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.char_class->byval_arg;
		insts [ninsts ++] = &mono_defaults.boolean_class->byval_arg;

		/* Add GenericComparer<T> instances for primitive types for Enum.ToString () */
		klass = mono_class_from_name (acfg->image, "System.Collections.Generic", "GenericComparer`1");
		if (klass)
			add_instances_of (acfg, klass, insts, ninsts);
		klass = mono_class_from_name (acfg->image, "System.Collections.Generic", "GenericEqualityComparer`1");
		if (klass)
			add_instances_of (acfg, klass, insts, ninsts);

		/* Add instances of the array generic interfaces for primitive types */
		/* This will add instances of the InternalArray_ helper methods in Array too */
		klass = mono_class_from_name (acfg->image, "System.Collections.Generic", "ICollection`1");
		if (klass)
			add_instances_of (acfg, klass, insts, ninsts);
		klass = mono_class_from_name (acfg->image, "System.Collections.Generic", "IList`1");
		if (klass)
			add_instances_of (acfg, klass, insts, ninsts);
		klass = mono_class_from_name (acfg->image, "System.Collections.Generic", "IEnumerable`1");
		if (klass)
			add_instances_of (acfg, klass, insts, ninsts);

		/* 
		 * Add a managed-to-native wrapper of Array.GetGenericValueImpl<object>, which is
		 * used for all instances of GetGenericValueImpl by the AOT runtime.
		 */
		{
			MonoGenericContext ctx;
			MonoType *args [16];
			MonoMethod *get_method;
			MonoClass *array_klass = mono_array_class_get (mono_defaults.object_class, 1)->parent;

			get_method = mono_class_get_method_from_name (array_klass, "GetGenericValueImpl", 2);

			if (get_method) {
				memset (&ctx, 0, sizeof (ctx));
				args [0] = &mono_defaults.object_class->byval_arg;
				ctx.method_inst = mono_metadata_get_generic_inst (1, args);
				add_extra_method (acfg, mono_marshal_get_native_wrapper (mono_class_inflate_generic_method (get_method, &ctx), TRUE, TRUE));
			}
		}
	}
}

/*
 * is_direct_callable:
 *
 *   Return whenever the method identified by JI is directly callable without 
 * going through the PLT.
 */
static gboolean
is_direct_callable (MonoAotCompile *acfg, MonoMethod *method, MonoJumpInfo *patch_info)
{
	if ((patch_info->type == MONO_PATCH_INFO_METHOD) && (patch_info->data.method->klass->image == acfg->image)) {
		MonoCompile *callee_cfg = g_hash_table_lookup (acfg->method_to_cfg, patch_info->data.method);
		if (callee_cfg) {
			gboolean direct_callable = TRUE;

			if (direct_callable && !(!callee_cfg->has_got_slots && (callee_cfg->method->klass->flags & TYPE_ATTRIBUTE_BEFORE_FIELD_INIT)))
				direct_callable = FALSE;
			if ((callee_cfg->method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED) && (!method || method->wrapper_type != MONO_WRAPPER_SYNCHRONIZED))
				// FIXME: Maybe call the wrapper directly ?
				direct_callable = FALSE;

			if (direct_callable)
				return TRUE;
		}
	}

	return FALSE;
}

/*
 * emit_and_reloc_code:
 *
 *   Emit the native code in CODE, handling relocations along the way. If GOT_ONLY
 * is true, calls are made through the GOT too. This is used for emitting trampolines
 * in full-aot mode, since calls made from trampolines couldn't go through the PLT,
 * since trampolines are needed to make PTL work.
 */
static void
emit_and_reloc_code (MonoAotCompile *acfg, MonoMethod *method, guint8 *code, guint32 code_len, MonoJumpInfo *relocs, gboolean got_only)
{
	int i, pindex, start_index, method_index;
	GPtrArray *patches;
	MonoJumpInfo *patch_info;
	MonoMethodHeader *header;
	gboolean skip, direct_call;
	guint32 got_slot;
	char direct_call_target [128];

	if (method) {
		header = mono_method_get_header (method);

		method_index = get_method_index (acfg, method);
	}

	/* Collect and sort relocations */
	patches = g_ptr_array_new ();
	for (patch_info = relocs; patch_info; patch_info = patch_info->next)
		g_ptr_array_add (patches, patch_info);
	g_ptr_array_sort (patches, compare_patches);

	start_index = 0;
	for (i = 0; i < code_len; i++) {
		patch_info = NULL;
		for (pindex = start_index; pindex < patches->len; ++pindex) {
			patch_info = g_ptr_array_index (patches, pindex);
			if (patch_info->ip.i >= i)
				break;
		}

#ifdef MONO_ARCH_AOT_SUPPORTED
		skip = FALSE;
		if (patch_info && (patch_info->ip.i == i) && (pindex < patches->len)) {
			start_index = pindex;

			switch (patch_info->type) {
			case MONO_PATCH_INFO_NONE:
				break;
			case MONO_PATCH_INFO_GOT_OFFSET: {
				int code_size;
 
				arch_emit_got_offset (acfg, code + i, &code_size);
				i += code_size - 1;
				skip = TRUE;
				patch_info->type = MONO_PATCH_INFO_NONE;
				break;
			}
			default: {
				/*
				 * If this patch is a call, try emitting a direct call instead of
				 * through a PLT entry. This is possible if the called method is in
				 * the same assembly and requires no initialization.
				 */
				direct_call = FALSE;
				if ((patch_info->type == MONO_PATCH_INFO_METHOD) && (patch_info->data.method->klass->image == acfg->image)) {
					if (!got_only && is_direct_callable (acfg, method, patch_info)) {
						MonoCompile *callee_cfg = g_hash_table_lookup (acfg->method_to_cfg, patch_info->data.method);
						//printf ("DIRECT: %s %s\n", method ? mono_method_full_name (method, TRUE) : "", mono_method_full_name (callee_cfg->method, TRUE));
						direct_call = TRUE;
						sprintf (direct_call_target, "%sm_%x", acfg->temp_prefix, get_method_index (acfg, callee_cfg->orig_method));
						patch_info->type = MONO_PATCH_INFO_NONE;
						acfg->stats.direct_calls ++;
					}

					acfg->stats.all_calls ++;
				}

				if (!got_only && !direct_call) {
					int plt_offset = get_plt_offset (acfg, patch_info);
					if (plt_offset != -1) {
						/* This patch has a PLT entry, so we must emit a call to the PLT entry */
						direct_call = TRUE;
						sprintf (direct_call_target, "%sp_%d", acfg->temp_prefix, plt_offset);
		
						/* Nullify the patch */
						patch_info->type = MONO_PATCH_INFO_NONE;
					}
				}

				if (direct_call) {
					int call_size;

					arch_emit_direct_call (acfg, direct_call_target, &call_size);
					i += call_size - 1;
				} else {
					int code_size;

					got_slot = get_got_offset (acfg, patch_info);

					arch_emit_got_access (acfg, code + i, got_slot, &code_size);
					i += code_size - 1;
				}
				skip = TRUE;
			}
			}
		}
#endif /* MONO_ARCH_AOT_SUPPORTED */

		if (!skip) {
			/* Find next patch */
			patch_info = NULL;
			for (pindex = start_index; pindex < patches->len; ++pindex) {
				patch_info = g_ptr_array_index (patches, pindex);
				if (patch_info->ip.i >= i)
					break;
			}

			/* Try to emit multiple bytes at once */
			if (pindex < patches->len && patch_info->ip.i > i) {
				emit_bytes (acfg, code + i, patch_info->ip.i - i);
				i = patch_info->ip.i - 1;
			} else {
				emit_bytes (acfg, code + i, 1);
			}
		}
	}
}

/*
 * sanitize_symbol:
 *
 *   Modify SYMBOL so it only includes characters permissible in symbols.
 */
static void
sanitize_symbol (char *symbol)
{
	int i, len = strlen (symbol);

	for (i = 0; i < len; ++i)
		if (!isalnum (symbol [i]) && (symbol [i] != '_'))
			symbol [i] = '_';
}

static char*
get_debug_sym (MonoMethod *method, const char *prefix, GHashTable *cache)
{
	char *name1, *name2, *cached;
	int i, j, len, count;
		
	name1 = mono_method_full_name (method, TRUE);
	len = strlen (name1);
	name2 = malloc (strlen (prefix) + len + 16);
	memcpy (name2, prefix, strlen (prefix));
	j = strlen (prefix);
	for (i = 0; i < len; ++i) {
		if (isalnum (name1 [i])) {
			name2 [j ++] = name1 [i];
		} else if (name1 [i] == ' ' && name1 [i + 1] == '(' && name1 [i + 2] == ')') {
			i += 2;
		} else if (name1 [i] == ',' && name1 [i + 1] == ' ') {
			name2 [j ++] = '_';
			i++;
		} else if (name1 [i] == '(' || name1 [i] == ')' || name1 [i] == '>') {
		} else
			name2 [j ++] = '_';
	}
	name2 [j] = '\0';

	g_free (name1);

	count = 0;
	while (g_hash_table_lookup (cache, name2)) {
		sprintf (name2 + j, "_%d", count);
		count ++;
	}

	cached = g_strdup (name2);
	g_hash_table_insert (cache, cached, cached);

	return name2;
}

static void
emit_method_code (MonoAotCompile *acfg, MonoCompile *cfg)
{
	MonoMethod *method;
	int method_index;
	guint8 *code;
	char *debug_sym = NULL;
	char symbol [128];
	int func_alignment = AOT_FUNC_ALIGNMENT;
	MonoMethodHeader *header;

	method = cfg->orig_method;
	code = cfg->native_code;
	header = mono_method_get_header (method);

	method_index = get_method_index (acfg, method);

	/* Emit unbox trampoline */
	if (acfg->aot_opts.full_aot && cfg->orig_method->klass->valuetype && (method->flags & METHOD_ATTRIBUTE_VIRTUAL)) {
		char call_target [256];

		if (!method->wrapper_type && !method->is_inflated) {
			g_assert (method->token);
			sprintf (symbol, "ut_%d", mono_metadata_token_index (method->token) - 1);
		} else {
			sprintf (symbol, "ut_e_%d", get_method_index (acfg, method));
		}

		emit_section_change (acfg, ".text", 0);
		emit_global (acfg, symbol, TRUE);
		emit_label (acfg, symbol);

		sprintf (call_target, "%sm_%x", acfg->temp_prefix, method_index);

		arch_emit_unbox_trampoline (acfg, cfg->orig_method, cfg->generic_sharing_context, call_target);
	}

	/* Make the labels local */
	sprintf (symbol, "%sm_%x", acfg->temp_prefix, method_index);

	emit_section_change (acfg, ".text", 0);
	emit_alignment (acfg, func_alignment);
	emit_label (acfg, symbol);

	if (acfg->aot_opts.write_symbols) {
		/* 
		 * Write a C style symbol for every method, this has two uses:
		 * - it works on platforms where the dwarf debugging info is not
		 *   yet supported.
		 * - it allows the setting of breakpoints of aot-ed methods.
		 */
		debug_sym = get_debug_sym (method, "", acfg->method_label_hash);

		sprintf (symbol, "%sme_%x", acfg->temp_prefix, method_index);
		emit_local_symbol (acfg, debug_sym, symbol, TRUE);
		emit_label (acfg, debug_sym);
	}

	if (cfg->verbose_level > 0)
		g_print ("Method %s emitted as %s\n", mono_method_full_name (method, TRUE), symbol);

	acfg->stats.code_size += cfg->code_len;

	acfg->cfgs [method_index]->got_offset = acfg->got_offset;

	emit_and_reloc_code (acfg, method, code, cfg->code_len, cfg->patch_info, FALSE);

	emit_line (acfg);

	if (acfg->aot_opts.write_symbols) {
		emit_symbol_size (acfg, debug_sym, ".");
		g_free (debug_sym);
	}

	sprintf (symbol, "%sme_%x", acfg->temp_prefix, method_index);
	emit_label (acfg, symbol);
}

/**
 * encode_patch:
 *
 *  Encode PATCH_INFO into its disk representation.
 */
static void
encode_patch (MonoAotCompile *acfg, MonoJumpInfo *patch_info, guint8 *buf, guint8 **endbuf)
{
	guint8 *p = buf;

	switch (patch_info->type) {
	case MONO_PATCH_INFO_NONE:
		break;
	case MONO_PATCH_INFO_IMAGE:
		encode_value (get_image_index (acfg, patch_info->data.image), p, &p);
		break;
	case MONO_PATCH_INFO_MSCORLIB_GOT_ADDR:
		break;
	case MONO_PATCH_INFO_METHOD_REL:
		encode_value ((gint)patch_info->data.offset, p, &p);
		break;
	case MONO_PATCH_INFO_SWITCH: {
		gpointer *table = (gpointer *)patch_info->data.table->table;
		int k;

		encode_value (patch_info->data.table->table_size, p, &p);
		for (k = 0; k < patch_info->data.table->table_size; k++)
			encode_value ((int)(gssize)table [k], p, &p);
		break;
	}
	case MONO_PATCH_INFO_METHODCONST:
	case MONO_PATCH_INFO_METHOD:
	case MONO_PATCH_INFO_METHOD_JUMP:
	case MONO_PATCH_INFO_ICALL_ADDR:
	case MONO_PATCH_INFO_METHOD_RGCTX:
		encode_method_ref (acfg, patch_info->data.method, p, &p);
		break;
	case MONO_PATCH_INFO_INTERNAL_METHOD:
	case MONO_PATCH_INFO_JIT_ICALL_ADDR: {
		guint32 len = strlen (patch_info->data.name);

		encode_value (len, p, &p);

		memcpy (p, patch_info->data.name, len);
		p += len;
		*p++ = '\0';
		break;
	}
	case MONO_PATCH_INFO_LDSTR: {
		guint32 image_index = get_image_index (acfg, patch_info->data.token->image);
		guint32 token = patch_info->data.token->token;
		g_assert (mono_metadata_token_code (token) == MONO_TOKEN_STRING);
		encode_value (image_index, p, &p);
		encode_value (patch_info->data.token->token - MONO_TOKEN_STRING, p, &p);
		break;
	}
	case MONO_PATCH_INFO_RVA:
	case MONO_PATCH_INFO_DECLSEC:
	case MONO_PATCH_INFO_LDTOKEN:
	case MONO_PATCH_INFO_TYPE_FROM_HANDLE:
		encode_value (get_image_index (acfg, patch_info->data.token->image), p, &p);
		encode_value (patch_info->data.token->token, p, &p);
		encode_value (patch_info->data.token->has_context, p, &p);
		if (patch_info->data.token->has_context)
			encode_generic_context (acfg, &patch_info->data.token->context, p, &p);
		break;
	case MONO_PATCH_INFO_EXC_NAME: {
		MonoClass *ex_class;

		ex_class =
			mono_class_from_name (mono_defaults.exception_class->image,
								  "System", patch_info->data.target);
		g_assert (ex_class);
		encode_klass_ref (acfg, ex_class, p, &p);
		break;
	}
	case MONO_PATCH_INFO_R4:
		encode_value (*((guint32 *)patch_info->data.target), p, &p);
		break;
	case MONO_PATCH_INFO_R8:
		encode_value (((guint32 *)patch_info->data.target) [MINI_LS_WORD_IDX], p, &p);
		encode_value (((guint32 *)patch_info->data.target) [MINI_MS_WORD_IDX], p, &p);
		break;
	case MONO_PATCH_INFO_VTABLE:
	case MONO_PATCH_INFO_CLASS:
	case MONO_PATCH_INFO_IID:
	case MONO_PATCH_INFO_ADJUSTED_IID:
		encode_klass_ref (acfg, patch_info->data.klass, p, &p);
		break;
	case MONO_PATCH_INFO_CLASS_INIT:
	case MONO_PATCH_INFO_DELEGATE_TRAMPOLINE:
		encode_klass_ref (acfg, patch_info->data.klass, p, &p);
		break;
	case MONO_PATCH_INFO_FIELD:
	case MONO_PATCH_INFO_SFLDA:
		encode_field_info (acfg, patch_info->data.field, p, &p);
		break;
	case MONO_PATCH_INFO_INTERRUPTION_REQUEST_FLAG:
		break;
	case MONO_PATCH_INFO_RGCTX_FETCH: {
		MonoJumpInfoRgctxEntry *entry = patch_info->data.rgctx_entry;

		encode_method_ref (acfg, entry->method, p, &p);
		encode_value (entry->in_mrgctx, p, &p);
		encode_value (entry->info_type, p, &p);
		encode_value (entry->data->type, p, &p);
		encode_patch (acfg, entry->data, p, &p);
		break;
	}
	case MONO_PATCH_INFO_GENERIC_CLASS_INIT:
	case MONO_PATCH_INFO_MONITOR_ENTER:
	case MONO_PATCH_INFO_MONITOR_EXIT:
	case MONO_PATCH_INFO_SEQ_POINT_INFO:
		break;
	case MONO_PATCH_INFO_LLVM_IMT_TRAMPOLINE:
		encode_method_ref (acfg, patch_info->data.imt_tramp->method, p, &p);
		encode_value (patch_info->data.imt_tramp->vt_offset, p, &p);
		break;
	default:
		g_warning ("unable to handle jump info %d", patch_info->type);
		g_assert_not_reached ();
	}

	*endbuf = p;
}

static void
encode_patch_list (MonoAotCompile *acfg, GPtrArray *patches, int n_patches, int first_got_offset, guint8 *buf, guint8 **endbuf)
{
	guint8 *p = buf;
	guint32 pindex, offset;
	MonoJumpInfo *patch_info;

	encode_value (n_patches, p, &p);

	for (pindex = 0; pindex < patches->len; ++pindex) {
		patch_info = g_ptr_array_index (patches, pindex);

		if (patch_info->type == MONO_PATCH_INFO_NONE || patch_info->type == MONO_PATCH_INFO_BB)
			/* Nothing to do */
			continue;

		offset = get_got_offset (acfg, patch_info);
		encode_value (offset, p, &p);
	}

	*endbuf = p;
}

static void
emit_method_info (MonoAotCompile *acfg, MonoCompile *cfg)
{
	MonoMethod *method;
	GList *l;
	int pindex, buf_size, n_patches;
	guint8 *code;
	GPtrArray *patches;
	MonoJumpInfo *patch_info;
	MonoMethodHeader *header;
	guint32 method_index;
	guint8 *p, *buf;
	guint32 first_got_offset;

	method = cfg->orig_method;
	code = cfg->native_code;
	header = mono_method_get_header (method);

	method_index = get_method_index (acfg, method);

	/* Sort relocations */
	patches = g_ptr_array_new ();
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next)
		g_ptr_array_add (patches, patch_info);
	g_ptr_array_sort (patches, compare_patches);

	first_got_offset = acfg->cfgs [method_index]->got_offset;

	/**********************/
	/* Encode method info */
	/**********************/

	buf_size = (patches->len < 1000) ? 40960 : 40960 + (patches->len * 64);
	p = buf = g_malloc (buf_size);

	if (mono_class_get_cctor (method->klass))
		encode_klass_ref (acfg, method->klass, p, &p);
	else
		/* Not needed when loading the method */
		encode_value (0, p, &p);

	/* String table */
	if (cfg->opt & MONO_OPT_SHARED) {
		encode_value (g_list_length (cfg->ldstr_list), p, &p);
		for (l = cfg->ldstr_list; l; l = l->next) {
			encode_value ((long)l->data, p, &p);
		}
	}
	else
		/* Used only in shared mode */
		g_assert (!cfg->ldstr_list);

	n_patches = 0;
	for (pindex = 0; pindex < patches->len; ++pindex) {
		patch_info = g_ptr_array_index (patches, pindex);
		
		if ((patch_info->type == MONO_PATCH_INFO_GOT_OFFSET) ||
			(patch_info->type == MONO_PATCH_INFO_NONE)) {
			patch_info->type = MONO_PATCH_INFO_NONE;
			/* Nothing to do */
			continue;
		}

		if ((patch_info->type == MONO_PATCH_INFO_IMAGE) && (patch_info->data.image == acfg->image)) {
			/* Stored in a GOT slot initialized at module load time */
			patch_info->type = MONO_PATCH_INFO_NONE;
			continue;
		}

		if (is_plt_patch (patch_info)) {
			/* Calls are made through the PLT */
			patch_info->type = MONO_PATCH_INFO_NONE;
			continue;
		}

		n_patches ++;
	}

	if (n_patches)
		g_assert (cfg->has_got_slots);

	encode_patch_list (acfg, patches, n_patches, first_got_offset, p, &p);

	acfg->stats.info_size += p - buf;

	g_assert (p - buf < buf_size);

	cfg->method_info_offset = add_to_blob (acfg, buf, p - buf);
	g_free (buf);
}

static guint32
get_unwind_info_offset (MonoAotCompile *acfg, guint8 *encoded, guint32 encoded_len)
{
	guint32 cache_index;
	guint32 offset;

	/* Reuse the unwind module to canonize and store unwind info entries */
	cache_index = mono_cache_unwind_info (encoded, encoded_len);

	/* Use +/- 1 to distinguish 0s from missing entries */
	offset = GPOINTER_TO_UINT (g_hash_table_lookup (acfg->unwind_info_offsets, GUINT_TO_POINTER (cache_index + 1)));
	if (offset)
		return offset - 1;
	else {
		guint8 buf [16];
		guint8 *p;

		/* 
		 * It would be easier to use assembler symbols, but the caller needs an
		 * offset now.
		 */
		offset = acfg->unwind_info_offset;
		g_hash_table_insert (acfg->unwind_info_offsets, GUINT_TO_POINTER (cache_index + 1), GUINT_TO_POINTER (offset + 1));
		g_ptr_array_add (acfg->unwind_ops, GUINT_TO_POINTER (cache_index));

		p = buf;
		encode_value (encoded_len, p, &p);

		acfg->unwind_info_offset += encoded_len + (p - buf);
		return offset;
	}
}

static void
emit_exception_debug_info (MonoAotCompile *acfg, MonoCompile *cfg)
{
	MonoMethod *method;
	int i, k, buf_size, method_index;
	guint32 debug_info_size;
	guint8 *code;
	MonoMethodHeader *header;
	guint8 *p, *buf, *debug_info;
	MonoJitInfo *jinfo = cfg->jit_info;
	guint32 flags;
	gboolean use_unwind_ops = FALSE;
	MonoSeqPointInfo *seq_points;

	method = cfg->orig_method;
	code = cfg->native_code;
	header = mono_method_get_header (method);

	method_index = get_method_index (acfg, method);

	if (!acfg->aot_opts.nodebug) {
		mono_debug_serialize_debug_info (cfg, &debug_info, &debug_info_size);
	} else {
		debug_info = NULL;
		debug_info_size = 0;
	}

	seq_points = cfg->seq_point_info;

	buf_size = header->num_clauses * 256 + debug_info_size + 1024 + (seq_points ? (seq_points->len * 64) : 0);
	p = buf = g_malloc (buf_size);

#ifdef MONO_ARCH_HAVE_XP_UNWIND
	use_unwind_ops = cfg->unwind_ops != NULL;
#endif

	flags = (jinfo->has_generic_jit_info ? 1 : 0) | (use_unwind_ops ? 2 : 0) | (header->num_clauses ? 4 : 0) | (seq_points ? 8 : 0) | (cfg->compile_llvm ? 16 : 0);

	encode_value (flags, p, &p);

	if (use_unwind_ops) {
		guint32 encoded_len;
		guint8 *encoded;

		/* 
		 * This is a duplicate of the data in the .debug_frame section, but that
		 * section cannot be accessed using the dl interface.
		 */
		encoded = mono_unwind_ops_encode (cfg->unwind_ops, &encoded_len);
		encode_value (get_unwind_info_offset (acfg, encoded, encoded_len), p, &p);
		g_free (encoded);
	} else {
		encode_value (jinfo->used_regs, p, &p);
	}

	/* Exception table */
	if (jinfo->num_clauses)
		encode_value (jinfo->num_clauses, p, &p);

	for (k = 0; k < jinfo->num_clauses; ++k) {
		MonoJitExceptionInfo *ei = &jinfo->clauses [k];

		encode_value (ei->flags, p, &p);
		encode_value (ei->exvar_offset, p, &p);

		if (ei->flags == MONO_EXCEPTION_CLAUSE_FILTER)
			encode_value ((gint)((guint8*)ei->data.filter - code), p, &p);
		else {
			if (ei->data.catch_class) {
				encode_value (1, p, &p);
				encode_klass_ref (acfg, ei->data.catch_class, p, &p);
			} else {
				encode_value (0, p, &p);
			}
		}

		encode_value ((gint)((guint8*)ei->try_start - code), p, &p);
		encode_value ((gint)((guint8*)ei->try_end - code), p, &p);
		encode_value ((gint)((guint8*)ei->handler_start - code), p, &p);
	}

	if (jinfo->has_generic_jit_info) {
		MonoGenericJitInfo *gi = mono_jit_info_get_generic_jit_info (jinfo);

		encode_value (gi->has_this ? 1 : 0, p, &p);
		encode_value (gi->this_reg, p, &p);
		encode_value (gi->this_offset, p, &p);

		/* 
		 * Need to encode jinfo->method too, since it is not equal to 'method'
		 * when using generic sharing.
		 */
		encode_method_ref (acfg, jinfo->method, p, &p);
	}

	if (seq_points) {
		int il_offset, native_offset, last_il_offset, last_native_offset, j;

		encode_value (seq_points->len, p, &p);
		last_il_offset = last_native_offset = 0;
		for (i = 0; i < seq_points->len; ++i) {
			SeqPoint *sp = &seq_points->seq_points [i];
			il_offset = sp->il_offset;
			native_offset = sp->native_offset;
			encode_value (il_offset - last_il_offset, p, &p);
			encode_value (native_offset - last_native_offset, p, &p);
			last_il_offset = il_offset;
			last_native_offset = native_offset;

			encode_value (sp->next_len, p, &p);
			for (j = 0; j < sp->next_len; ++j)
				encode_value (sp->next [j], p, &p);
		}
	}
		

	g_assert (debug_info_size < buf_size);

	encode_value (debug_info_size, p, &p);
	if (debug_info_size) {
		memcpy (p, debug_info, debug_info_size);
		p += debug_info_size;
		g_free (debug_info);
	}

	acfg->stats.ex_info_size += p - buf;

	g_assert (p - buf < buf_size);

	/* Emit info */
	cfg->ex_info_offset = add_to_blob (acfg, buf, p - buf);
	g_free (buf);
}

static guint32
emit_klass_info (MonoAotCompile *acfg, guint32 token)
{
	MonoClass *klass = mono_class_get (acfg->image, token);
	guint8 *p, *buf;
	int i, buf_size, res;
	gboolean no_special_static, cant_encode;
	gpointer iter = NULL;

	buf_size = 10240 + (klass->vtable_size * 16);
	p = buf = g_malloc (buf_size);

	g_assert (klass);

	mono_class_init (klass);

	mono_class_get_nested_types (klass, &iter);
	g_assert (klass->nested_classes_inited);

	mono_class_setup_vtable (klass);

	/* 
	 * Emit all the information which is required for creating vtables so
	 * the runtime does not need to create the MonoMethod structures which
	 * take up a lot of space.
	 */

	no_special_static = !mono_class_has_special_static_fields (klass);

	/* Check whenever we have enough info to encode the vtable */
	cant_encode = FALSE;
	for (i = 0; i < klass->vtable_size; ++i) {
		MonoMethod *cm = klass->vtable [i];

		if (cm && mono_method_signature (cm)->is_inflated && !g_hash_table_lookup (acfg->token_info_hash, cm))
			cant_encode = TRUE;
	}

	if (klass->generic_container || cant_encode) {
		encode_value (-1, p, &p);
	} else {
		encode_value (klass->vtable_size, p, &p);
		encode_value ((klass->generic_container ? (1 << 8) : 0) | (no_special_static << 7) | (klass->has_static_refs << 6) | (klass->has_references << 5) | ((klass->blittable << 4) | ((klass->ext && klass->ext->nested_classes) ? 1 : 0) << 3) | (klass->has_cctor << 2) | (klass->has_finalize << 1) | klass->ghcimpl, p, &p);
		if (klass->has_cctor)
			encode_method_ref (acfg, mono_class_get_cctor (klass), p, &p);
		if (klass->has_finalize)
			encode_method_ref (acfg, mono_class_get_finalizer (klass), p, &p);
 
		encode_value (klass->instance_size, p, &p);
		encode_value (mono_class_data_size (klass), p, &p);
		encode_value (klass->packing_size, p, &p);
		encode_value (klass->min_align, p, &p);

		for (i = 0; i < klass->vtable_size; ++i) {
			MonoMethod *cm = klass->vtable [i];

			if (cm)
				encode_method_ref (acfg, cm, p, &p);
			else
				encode_value (0, p, &p);
		}
	}

	acfg->stats.class_info_size += p - buf;

	g_assert (p - buf < buf_size);
	res = add_to_blob (acfg, buf, p - buf);
	g_free (buf);

	return res;
}

/*
 * Calls made from AOTed code are routed through a table of jumps similar to the
 * ELF PLT (Program Linkage Table). The differences are the following:
 * - the ELF PLT entries make an indirect jump though the GOT so they expect the
 *   GOT pointer to be in EBX. We want to avoid this, so our table contains direct
 *   jumps. This means the jumps need to be patched when the address of the callee is
 *   known. Initially the PLT entries jump to code which transfers control to the
 *   AOT runtime through the first PLT entry.
 */
static void
emit_plt (MonoAotCompile *acfg)
{
	char symbol [128];
	int i;
	GHashTable *cache;

	cache = g_hash_table_new (g_str_hash, g_str_equal);

	emit_line (acfg);
	sprintf (symbol, "plt");

	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, symbol, TRUE);
#ifdef TARGET_X86
	/* This section will be made read-write by the AOT loader */
	emit_alignment (acfg, mono_pagesize ());
#else
	emit_alignment (acfg, 16);
#endif
	emit_label (acfg, symbol);
	emit_label (acfg, acfg->plt_symbol);

	for (i = 0; i < acfg->plt_offset; ++i) {
		char label [128];
		char *debug_sym = NULL;
		MonoJumpInfo *ji;

		sprintf (label, "%sp_%d", acfg->temp_prefix, i);

		if (acfg->llvm) {
			/*
			 * If the target is directly callable, alias the plt symbol to point to
			 * the method code.
			 * FIXME: Use this to simplify emit_and_reloc_code ().
			 * FIXME: Avoid the got slot.
			 * FIXME: Add support to the binary writer.
			 */
			ji = g_hash_table_lookup (acfg->plt_offset_to_patch, GUINT_TO_POINTER (i));
			if (ji && is_direct_callable (acfg, NULL, ji) && !acfg->use_bin_writer) {
				MonoCompile *callee_cfg = g_hash_table_lookup (acfg->method_to_cfg, ji->data.method);
				fprintf (acfg->fp, "\n.set %s, .Lm_%x\n", label, get_method_index (acfg, callee_cfg->orig_method));
				continue;
			}
		}

		emit_label (acfg, label);

		if (acfg->aot_opts.write_symbols) {
			MonoJumpInfo *ji = g_hash_table_lookup (acfg->plt_offset_to_patch, GUINT_TO_POINTER (i));

			if (ji) {
				switch (ji->type) {
				case MONO_PATCH_INFO_METHOD:
					debug_sym = get_debug_sym (ji->data.method, "plt_", cache);
					break;
				case MONO_PATCH_INFO_INTERNAL_METHOD:
					debug_sym = g_strdup_printf ("plt__jit_icall_%s", ji->data.name);
					break;
				case MONO_PATCH_INFO_CLASS_INIT:
					debug_sym = g_strdup_printf ("plt__class_init_%s", mono_type_get_name (&ji->data.klass->byval_arg));
					sanitize_symbol (debug_sym);
					break;
				case MONO_PATCH_INFO_RGCTX_FETCH:
					debug_sym = g_strdup_printf ("plt__rgctx_fetch_%d", acfg->label_generator ++);
					break;
				case MONO_PATCH_INFO_ICALL_ADDR: {
					char *s = get_debug_sym (ji->data.method, "", cache);
					
					debug_sym = g_strdup_printf ("plt__icall_native_%s", s);
					g_free (s);
					break;
				}
				case MONO_PATCH_INFO_JIT_ICALL_ADDR:
					debug_sym = g_strdup_printf ("plt__jit_icall_native_%s", ji->data.name);
					break;
				case MONO_PATCH_INFO_GENERIC_CLASS_INIT:
					debug_sym = g_strdup_printf ("plt__generic_class_init");
					break;
				default:
					break;
				}

				if (debug_sym) {
					emit_local_symbol (acfg, debug_sym, NULL, TRUE);
					emit_label (acfg, debug_sym);
				}
			}
		}

		/* 
		 * The first plt entry is used to transfer code to the AOT loader. 
		 */
		arch_emit_plt_entry (acfg, i);

		if (debug_sym) {
			emit_symbol_size (acfg, debug_sym, ".");
			g_free (debug_sym);
		}
	}

	emit_symbol_size (acfg, acfg->plt_symbol, ".");

	sprintf (symbol, "plt_end");
	emit_global (acfg, symbol, TRUE);
	emit_label (acfg, symbol);

	g_hash_table_destroy (cache);
}

static G_GNUC_UNUSED void
emit_trampoline (MonoAotCompile *acfg, const char *name, guint8 *code, 
				 guint32 code_size, int got_offset, MonoJumpInfo *ji, GSList *unwind_ops)
{
	char start_symbol [256];
	char symbol [256];
	guint32 buf_size, info_offset;
	MonoJumpInfo *patch_info;
	guint8 *buf, *p;
	GPtrArray *patches;

	/* Emit code */

	sprintf (start_symbol, "%s", name);

	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, start_symbol, TRUE);
	emit_alignment (acfg, 16);
	emit_label (acfg, start_symbol);

	sprintf (symbol, "%snamed_%s", acfg->temp_prefix, name);
	emit_label (acfg, symbol);

	/* 
	 * The code should access everything through the GOT, so we pass
	 * TRUE here.
	 */
	emit_and_reloc_code (acfg, NULL, code, code_size, ji, TRUE);

	emit_symbol_size (acfg, start_symbol, ".");

	/* Emit info */

	/* Sort relocations */
	patches = g_ptr_array_new ();
	for (patch_info = ji; patch_info; patch_info = patch_info->next)
		if (patch_info->type != MONO_PATCH_INFO_NONE)
			g_ptr_array_add (patches, patch_info);
	g_ptr_array_sort (patches, compare_patches);

	buf_size = patches->len * 128 + 128;
	buf = g_malloc (buf_size);
	p = buf;

	encode_patch_list (acfg, patches, patches->len, got_offset, p, &p);
	g_assert (p - buf < buf_size);

	sprintf (symbol, "%s_p", name);

	info_offset = add_to_blob (acfg, buf, p - buf);

	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, symbol, FALSE);
	emit_label (acfg, symbol);

	emit_int32 (acfg, info_offset);

	/* Emit debug info */
	if (unwind_ops) {
		char symbol2 [256];

		sprintf (symbol, "%s", name);
		sprintf (symbol2, "%snamed_%s", acfg->temp_prefix, name);

		if (acfg->dwarf)
			mono_dwarf_writer_emit_trampoline (acfg->dwarf, symbol, symbol2, NULL, NULL, code_size, unwind_ops);
	}
}

static void
emit_trampolines (MonoAotCompile *acfg)
{
	char symbol [256];
	int i, tramp_got_offset;
	MonoAotTrampoline ntype;
#ifdef MONO_ARCH_HAVE_FULL_AOT_TRAMPOLINES
	int tramp_type;
	guint32 code_size;
	MonoJumpInfo *ji;
	guint8 *code;
	GSList *unwind_ops;
#endif

	if (!acfg->aot_opts.full_aot)
		return;
	
	g_assert (acfg->image->assembly);

	/* Currently, we only emit most trampolines into the mscorlib AOT image. */
	if (strcmp (acfg->image->assembly->aname.name, "mscorlib") == 0) {
#ifdef MONO_ARCH_HAVE_FULL_AOT_TRAMPOLINES
		/*
		 * Emit the generic trampolines.
		 *
		 * We could save some code by treating the generic trampolines as a wrapper
		 * method, but that approach has its own complexities, so we choose the simpler
		 * method.
		 */
		for (tramp_type = 0; tramp_type < MONO_TRAMPOLINE_NUM; ++tramp_type) {
			code = mono_arch_create_trampoline_code_full (tramp_type, &code_size, &ji, &unwind_ops, TRUE);

			/* Emit trampoline code */

			sprintf (symbol, "generic_trampoline_%d", tramp_type);

			emit_trampoline (acfg, symbol, code, code_size, acfg->got_offset, ji, unwind_ops);
		}

		code = mono_arch_get_nullified_class_init_trampoline (&code_size);
		emit_trampoline (acfg, "nullified_class_init_trampoline", code, code_size, acfg->got_offset, NULL, NULL);
#if defined(TARGET_AMD64) && defined(MONO_ARCH_MONITOR_OBJECT_REG)
		code = mono_arch_create_monitor_enter_trampoline_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "monitor_enter_trampoline", code, code_size, acfg->got_offset, ji, NULL);
		code = mono_arch_create_monitor_exit_trampoline_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "monitor_exit_trampoline", code, code_size, acfg->got_offset, ji, NULL);
#endif

		code = mono_arch_create_generic_class_init_trampoline_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "generic_class_init_trampoline", code, code_size, acfg->got_offset, ji, NULL);

		/* Emit the exception related code pieces */
		code = mono_arch_get_restore_context_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "restore_context", code, code_size, acfg->got_offset, ji, NULL);
		code = mono_arch_get_call_filter_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "call_filter", code, code_size, acfg->got_offset, ji, NULL);
		code = mono_arch_get_throw_exception_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "throw_exception", code, code_size, acfg->got_offset, ji, NULL);
		code = mono_arch_get_rethrow_exception_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "rethrow_exception", code, code_size, acfg->got_offset, ji, NULL);
#ifdef MONO_ARCH_HAVE_THROW_EXCEPTION_BY_NAME
		code = mono_arch_get_throw_exception_by_name_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "throw_exception_by_name", code, code_size, acfg->got_offset, ji, NULL);
#endif
		code = mono_arch_get_throw_corlib_exception_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "throw_corlib_exception", code, code_size, acfg->got_offset, ji, NULL);

#if defined(TARGET_AMD64)
		code = mono_arch_get_throw_pending_exception_full (&code_size, &ji, TRUE);
		emit_trampoline (acfg, "throw_pending_exception", code, code_size, acfg->got_offset, ji, NULL);
#endif

		for (i = 0; i < 128; ++i) {
			int offset;

			offset = MONO_RGCTX_SLOT_MAKE_RGCTX (i);
			code = mono_arch_create_rgctx_lazy_fetch_trampoline_full (offset, &code_size, &ji, TRUE);
			sprintf (symbol, "rgctx_fetch_trampoline_%u", offset);
			emit_trampoline (acfg, symbol, code, code_size, acfg->got_offset, ji, NULL);

			offset = MONO_RGCTX_SLOT_MAKE_MRGCTX (i);
			code = mono_arch_create_rgctx_lazy_fetch_trampoline_full (offset, &code_size, &ji, TRUE);
			sprintf (symbol, "rgctx_fetch_trampoline_%u", offset);
			emit_trampoline (acfg, symbol, code, code_size, acfg->got_offset, ji, NULL);
		}

		{
			GSList *l;

			/* delegate_invoke_impl trampolines */
			l = mono_arch_get_delegate_invoke_impls ();
			while (l) {
				MonoAotTrampInfo *info = l->data;

				emit_trampoline (acfg, info->name, info->code, info->code_size, acfg->got_offset, NULL, NULL);
				l = l->next;
			}
		}

#endif /* #ifdef MONO_ARCH_HAVE_FULL_AOT_TRAMPOLINES */

		/* Emit trampolines which are numerous */

		/*
		 * These include the following:
		 * - specific trampolines
		 * - static rgctx invoke trampolines
		 * - imt thunks
		 * These trampolines have the same code, they are parameterized by GOT 
		 * slots. 
		 * They are defined in this file, in the arch_... routines instead of
		 * in tramp-<ARCH>.c, since it is easier to do it this way.
		 */

		/*
		 * When running in aot-only mode, we can't create specific trampolines at 
		 * runtime, so we create a few, and save them in the AOT file. 
		 * Normal trampolines embed their argument as a literal inside the 
		 * trampoline code, we can't do that here, so instead we embed an offset
		 * which needs to be added to the trampoline address to get the address of
		 * the GOT slot which contains the argument value.
		 * The generated trampolines jump to the generic trampolines using another
		 * GOT slot, which will be setup by the AOT loader to point to the 
		 * generic trampoline code of the given type.
		 */

		/*
		 * FIXME: Maybe we should use more specific trampolines (i.e. one class init for
		 * each class).
		 */

		emit_section_change (acfg, ".text", 0);

		tramp_got_offset = acfg->got_offset;

		for (ntype = 0; ntype < MONO_AOT_TRAMP_NUM; ++ntype) {
			switch (ntype) {
			case MONO_AOT_TRAMP_SPECIFIC:
				sprintf (symbol, "specific_trampolines");
				break;
			case MONO_AOT_TRAMP_STATIC_RGCTX:
				sprintf (symbol, "static_rgctx_trampolines");
				break;
			case MONO_AOT_TRAMP_IMT_THUNK:
				sprintf (symbol, "imt_thunks");
				break;
			default:
				g_assert_not_reached ();
			}

			emit_global (acfg, symbol, TRUE);
			emit_alignment (acfg, 16);
			emit_label (acfg, symbol);

			acfg->trampoline_got_offset_base [ntype] = tramp_got_offset;

			for (i = 0; i < acfg->num_trampolines [ntype]; ++i) {
				int tramp_size = 0;

				switch (ntype) {
				case MONO_AOT_TRAMP_SPECIFIC:
					arch_emit_specific_trampoline (acfg, tramp_got_offset, &tramp_size);
					tramp_got_offset += 2;
				break;
				case MONO_AOT_TRAMP_STATIC_RGCTX:
					arch_emit_static_rgctx_trampoline (acfg, tramp_got_offset, &tramp_size);				
					tramp_got_offset += 2;
					break;
				case MONO_AOT_TRAMP_IMT_THUNK:
					arch_emit_imt_thunk (acfg, tramp_got_offset, &tramp_size);
					tramp_got_offset += 1;
					break;
				default:
					g_assert_not_reached ();
				}

				if (!acfg->trampoline_size [ntype]) {
					g_assert (tramp_size);
					acfg->trampoline_size [ntype] = tramp_size;
				}
			}
		}

		/* Reserve some entries at the end of the GOT for our use */
		acfg->num_trampoline_got_entries = tramp_got_offset - acfg->got_offset;
	}

	acfg->got_offset += acfg->num_trampoline_got_entries;
}

static gboolean
str_begins_with (const char *str1, const char *str2)
{
	int len = strlen (str2);
	return strncmp (str1, str2, len) == 0;
}

static void
mono_aot_parse_options (const char *aot_options, MonoAotOptions *opts)
{
	gchar **args, **ptr;

	args = g_strsplit (aot_options ? aot_options : "", ",", -1);
	for (ptr = args; ptr && *ptr; ptr ++) {
		const char *arg = *ptr;

		if (str_begins_with (arg, "outfile=")) {
			opts->outfile = g_strdup (arg + strlen ("outfile="));
		} else if (str_begins_with (arg, "save-temps")) {
			opts->save_temps = TRUE;
		} else if (str_begins_with (arg, "keep-temps")) {
			opts->save_temps = TRUE;
		} else if (str_begins_with (arg, "write-symbols")) {
			opts->write_symbols = TRUE;
		} else if (str_begins_with (arg, "metadata-only")) {
			opts->metadata_only = TRUE;
		} else if (str_begins_with (arg, "bind-to-runtime-version")) {
			opts->bind_to_runtime_version = TRUE;
		} else if (str_begins_with (arg, "full")) {
			opts->full_aot = TRUE;
		} else if (str_begins_with (arg, "threads=")) {
			opts->nthreads = atoi (arg + strlen ("threads="));
		} else if (str_begins_with (arg, "static")) {
			opts->static_link = TRUE;
			opts->no_dlsym = TRUE;
		} else if (str_begins_with (arg, "asmonly")) {
			opts->asm_only = TRUE;
		} else if (str_begins_with (arg, "asmwriter")) {
			opts->asm_writer = TRUE;
		} else if (str_begins_with (arg, "nodebug")) {
			opts->nodebug = TRUE;
		} else if (str_begins_with (arg, "ntrampolines=")) {
			opts->ntrampolines = atoi (arg + strlen ("ntrampolines="));
		} else if (str_begins_with (arg, "nrgctx-trampolines=")) {
			opts->nrgctx_trampolines = atoi (arg + strlen ("nrgctx-trampolines="));
		} else if (str_begins_with (arg, "autoreg")) {
			opts->autoreg = TRUE;
		} else if (str_begins_with (arg, "tool-prefix=")) {
			opts->tool_prefix = g_strdup (arg + strlen ("tool-prefix="));
		} else if (str_begins_with (arg, "soft-debug")) {
			opts->soft_debug = TRUE;
		} else if (str_begins_with (arg, "print-skipped")) {
			opts->print_skipped_methods = TRUE;
		} else if (str_begins_with (arg, "stats")) {
			opts->stats = TRUE;
		} else {
			fprintf (stderr, "AOT : Unknown argument '%s'.\n", arg);
			exit (1);
		}
	}

	g_strfreev (args);
}

static void
add_token_info_hash (gpointer key, gpointer value, gpointer user_data)
{
	MonoMethod *method = (MonoMethod*)key;
	MonoJumpInfoToken *ji = (MonoJumpInfoToken*)value;
	MonoJumpInfoToken *new_ji = g_new0 (MonoJumpInfoToken, 1);
	MonoAotCompile *acfg = user_data;

	new_ji->image = ji->image;
	new_ji->token = ji->token;
	g_hash_table_insert (acfg->token_info_hash, method, new_ji);
}

static gboolean
can_encode_class (MonoAotCompile *acfg, MonoClass *klass)
{
	if (klass->type_token)
		return TRUE;
	if ((klass->byval_arg.type == MONO_TYPE_VAR) || (klass->byval_arg.type == MONO_TYPE_MVAR))
		return TRUE;
	if (klass->rank)
		return can_encode_class (acfg, klass->element_class);
	return FALSE;
}

static gboolean
can_encode_patch (MonoAotCompile *acfg, MonoJumpInfo *patch_info)
{
	switch (patch_info->type) {
	case MONO_PATCH_INFO_METHOD:
	case MONO_PATCH_INFO_METHODCONST: {
		MonoMethod *method = patch_info->data.method;

		if (method->wrapper_type) {
			switch (method->wrapper_type) {
			case MONO_WRAPPER_NONE:
			case MONO_WRAPPER_REMOTING_INVOKE_WITH_CHECK:
			case MONO_WRAPPER_XDOMAIN_INVOKE:
			case MONO_WRAPPER_STFLD:
			case MONO_WRAPPER_LDFLD:
			case MONO_WRAPPER_LDFLDA:
			case MONO_WRAPPER_LDFLD_REMOTE:
			case MONO_WRAPPER_STFLD_REMOTE:
			case MONO_WRAPPER_STELEMREF:
			case MONO_WRAPPER_ISINST:
			case MONO_WRAPPER_PROXY_ISINST:
			case MONO_WRAPPER_ALLOC:
			case MONO_WRAPPER_REMOTING_INVOKE:
			case MONO_WRAPPER_UNKNOWN:
				break;
			case MONO_WRAPPER_MANAGED_TO_MANAGED:
				if (!strcmp (method->name, "ElementAddr"))
					return TRUE;
				else
					return FALSE;
			default:
				//printf ("Skip (wrapper call): %d -> %s\n", patch_info->type, mono_method_full_name (patch_info->data.method, TRUE));
				return FALSE;
			}
		} else {
			if (!method->token) {
				/* The method is part of a constructed type like Int[,].Set (). */
				if (!g_hash_table_lookup (acfg->token_info_hash, method)) {
					if (method->klass->rank)
						return TRUE;
					return FALSE;
				}
			}
		}
		break;
	}
	case MONO_PATCH_INFO_VTABLE:
	case MONO_PATCH_INFO_CLASS_INIT:
	case MONO_PATCH_INFO_DELEGATE_TRAMPOLINE:
	case MONO_PATCH_INFO_CLASS:
	case MONO_PATCH_INFO_IID:
	case MONO_PATCH_INFO_ADJUSTED_IID:
		if (!can_encode_class (acfg, patch_info->data.klass)) {
			//printf ("Skip: %s\n", mono_type_full_name (&patch_info->data.klass->byval_arg));
			return FALSE;
		}
		break;
	case MONO_PATCH_INFO_RGCTX_FETCH: {
		MonoJumpInfoRgctxEntry *entry = patch_info->data.rgctx_entry;

		if (!can_encode_patch (acfg, entry->data))
			return FALSE;
		break;
	}
	default:
		break;
	}

	return TRUE;
}

static void
add_generic_class (MonoAotCompile *acfg, MonoClass *klass);

/*
 * compile_method:
 *
 *   AOT compile a given method.
 * This function might be called by multiple threads, so it must be thread-safe.
 */
static void
compile_method (MonoAotCompile *acfg, MonoMethod *method)
{
	MonoCompile *cfg;
	MonoJumpInfo *patch_info;
	gboolean skip;
	int index, depth;
	MonoMethod *wrapped;

	if (acfg->aot_opts.metadata_only)
		return;

	mono_acfg_lock (acfg);
	index = get_method_index (acfg, method);
	mono_acfg_unlock (acfg);

	/* fixme: maybe we can also precompile wrapper methods */
	if ((method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) ||
		(method->iflags & METHOD_IMPL_ATTRIBUTE_RUNTIME) ||
		(method->flags & METHOD_ATTRIBUTE_ABSTRACT)) {
		//printf ("Skip (impossible): %s\n", mono_method_full_name (method, TRUE));
		return;
	}

	if (method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL)
		return;

	wrapped = mono_marshal_method_from_wrapper (method);
	if (wrapped && (wrapped->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL) && wrapped->is_generic)
		// FIXME: The wrapper should be generic too, but it is not
		return;

	InterlockedIncrement (&acfg->stats.mcount);

#if 0
	if (method->is_generic || method->klass->generic_container) {
		InterlockedIncrement (&acfg->stats.genericcount);
		return;
	}
#endif

	//acfg->aot_opts.print_skipped_methods = TRUE;

	/*
	 * Since these methods are the only ones which are compiled with
	 * AOT support, and they are not used by runtime startup/shutdown code,
	 * the runtime will not see AOT methods during AOT compilation,so it
	 * does not need to support them by creating a fake GOT etc.
	 */
	cfg = mini_method_compile (method, acfg->opts, mono_get_root_domain (), FALSE, TRUE, 0);
	if (cfg->exception_type == MONO_EXCEPTION_GENERIC_SHARING_FAILED) {
		//printf ("F: %s\n", mono_method_full_name (method, TRUE));
		InterlockedIncrement (&acfg->stats.genericcount);
		return;
	}
	if (cfg->exception_type != MONO_EXCEPTION_NONE) {
		/* Let the exception happen at runtime */
		return;
	}

	if (cfg->disable_aot) {
		if (acfg->aot_opts.print_skipped_methods)
			printf ("Skip (disabled): %s\n", mono_method_full_name (method, TRUE));
		InterlockedIncrement (&acfg->stats.ocount);
		mono_destroy_compile (cfg);
		return;
	}

	/* Nullify patches which need no aot processing */
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
		switch (patch_info->type) {
		case MONO_PATCH_INFO_LABEL:
		case MONO_PATCH_INFO_BB:
			patch_info->type = MONO_PATCH_INFO_NONE;
			break;
		default:
			break;
		}
	}

	/* Collect method->token associations from the cfg */
	mono_acfg_lock (acfg);
	g_hash_table_foreach (cfg->token_info_hash, add_token_info_hash, acfg);
	mono_acfg_unlock (acfg);

	/*
	 * Check for absolute addresses.
	 */
	skip = FALSE;
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
		switch (patch_info->type) {
		case MONO_PATCH_INFO_ABS:
			/* unable to handle this */
			skip = TRUE;	
			break;
		default:
			break;
		}
	}

	if (skip) {
		if (acfg->aot_opts.print_skipped_methods)
			printf ("Skip (abs call): %s\n", mono_method_full_name (method, TRUE));
		InterlockedIncrement (&acfg->stats.abscount);
		mono_destroy_compile (cfg);
		return;
	}

	/* Lock for the rest of the code */
	mono_acfg_lock (acfg);

	/*
	 * Check for methods/klasses we can't encode.
	 */
	skip = FALSE;
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
		if (!can_encode_patch (acfg, patch_info))
			skip = TRUE;
	}

	if (skip) {
		if (acfg->aot_opts.print_skipped_methods)
			printf ("Skip (patches): %s\n", mono_method_full_name (method, TRUE));
		acfg->stats.ocount++;
		mono_destroy_compile (cfg);
		mono_acfg_unlock (acfg);
		return;
	}

	/* Adds generic instances referenced by this method */
	/* 
	 * The depth is used to avoid infinite loops when generic virtual recursion is 
	 * encountered.
	 */
	depth = GPOINTER_TO_UINT (g_hash_table_lookup (acfg->method_depth, method));
	if (depth < 32) {
		for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
			switch (patch_info->type) {
			case MONO_PATCH_INFO_METHOD: {
				MonoMethod *m = patch_info->data.method;
				if (m->is_inflated) {
					if (!(mono_class_generic_sharing_enabled (m->klass) &&
						  mono_method_is_generic_sharable_impl (m, FALSE)) &&
						!method_has_type_vars (m)) {
						if (m->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL) {
							if (acfg->aot_opts.full_aot)
								add_extra_method_with_depth (acfg, mono_marshal_get_native_wrapper (m, TRUE, TRUE), depth + 1);
						} else {
							add_extra_method_with_depth (acfg, m, depth + 1);
						}
					}
					add_generic_class (acfg, m->klass);
				}
				if (m->wrapper_type == MONO_WRAPPER_MANAGED_TO_MANAGED && !strcmp (m->name, "ElementAddr"))
					add_extra_method_with_depth (acfg, m, depth + 1);
				break;
			}
			case MONO_PATCH_INFO_VTABLE: {
				MonoClass *klass = patch_info->data.klass;

				if (klass->generic_class && !mono_generic_context_is_sharable (&klass->generic_class->context, FALSE))
					add_generic_class (acfg, klass);
				break;
			}
			default:
				break;
			}
		}
	}

	/* Determine whenever the method has GOT slots */
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
		switch (patch_info->type) {
		case MONO_PATCH_INFO_GOT_OFFSET:
		case MONO_PATCH_INFO_NONE:
			break;
		case MONO_PATCH_INFO_IMAGE:
			/* The assembly is stored in GOT slot 0 */
			if (patch_info->data.image != acfg->image)
				cfg->has_got_slots = TRUE;
			break;
		default:
			if (!is_plt_patch (patch_info))
				cfg->has_got_slots = TRUE;
			break;
		}
	}

	if (!cfg->has_got_slots)
		InterlockedIncrement (&acfg->stats.methods_without_got_slots);

	/* 
	 * FIXME: Instead of this mess, allocate the patches from the aot mempool.
	 */
	/* Make a copy of the patch info which is in the mempool */
	{
		MonoJumpInfo *patches = NULL, *patches_end = NULL;

		for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
			MonoJumpInfo *new_patch_info = mono_patch_info_dup_mp (acfg->mempool, patch_info);

			if (!patches)
				patches = new_patch_info;
			else
				patches_end->next = new_patch_info;
			patches_end = new_patch_info;
		}
		cfg->patch_info = patches;
	}
	/* Make a copy of the unwind info */
	{
		GSList *l, *unwind_ops;
		MonoUnwindOp *op;

		unwind_ops = NULL;
		for (l = cfg->unwind_ops; l; l = l->next) {
			op = mono_mempool_alloc (acfg->mempool, sizeof (MonoUnwindOp));
			memcpy (op, l->data, sizeof (MonoUnwindOp));
			unwind_ops = g_slist_prepend_mempool (acfg->mempool, unwind_ops, op);
		}
		cfg->unwind_ops = g_slist_reverse (unwind_ops);
	}
	/* Make a copy of the argument/local info */
	{
		MonoInst **args, **locals;
		MonoMethodSignature *sig;
		MonoMethodHeader *header;
		int i;
		
		sig = mono_method_signature (method);
		args = mono_mempool_alloc (acfg->mempool, sizeof (MonoInst*) * (sig->param_count + sig->hasthis));
		for (i = 0; i < sig->param_count + sig->hasthis; ++i) {
			args [i] = mono_mempool_alloc (acfg->mempool, sizeof (MonoInst));
			memcpy (args [i], cfg->args [i], sizeof (MonoInst));
		}
		cfg->args = args;

		header = mono_method_get_header (method);
		locals = mono_mempool_alloc (acfg->mempool, sizeof (MonoInst*) * header->num_locals);
		for (i = 0; i < header->num_locals; ++i) {
			locals [i] = mono_mempool_alloc (acfg->mempool, sizeof (MonoInst));
			memcpy (locals [i], cfg->locals [i], sizeof (MonoInst));
		}
		cfg->locals = locals;
	}

	/* Free some fields used by cfg to conserve memory */
	mono_mempool_destroy (cfg->mempool);
	cfg->mempool = NULL;
	g_free (cfg->varinfo);
	cfg->varinfo = NULL;
	g_free (cfg->vars);
	cfg->vars = NULL;
	if (cfg->rs) {
		mono_regstate_free (cfg->rs);
		cfg->rs = NULL;
	}

	//printf ("Compile:           %s\n", mono_method_full_name (method, TRUE));

	while (index >= acfg->cfgs_size) {
		MonoCompile **new_cfgs;
		int new_size;

		new_size = acfg->cfgs_size * 2;
		new_cfgs = g_new0 (MonoCompile*, new_size);
		memcpy (new_cfgs, acfg->cfgs, sizeof (MonoCompile*) * acfg->cfgs_size);
		g_free (acfg->cfgs);
		acfg->cfgs = new_cfgs;
		acfg->cfgs_size = new_size;
	}
	acfg->cfgs [index] = cfg;

	g_hash_table_insert (acfg->method_to_cfg, cfg->orig_method, cfg);

	/*
	if (cfg->orig_method->wrapper_type)
		g_ptr_array_add (acfg->extra_methods, cfg->orig_method);
	*/

	mono_acfg_unlock (acfg);

	InterlockedIncrement (&acfg->stats.ccount);
}
 
static void
compile_thread_main (gpointer *user_data)
{
	MonoDomain *domain = user_data [0];
	MonoAotCompile *acfg = user_data [1];
	GPtrArray *methods = user_data [2];
	int i;

	mono_thread_attach (domain);

	for (i = 0; i < methods->len; ++i)
		compile_method (acfg, g_ptr_array_index (methods, i));
}

static void
load_profile_files (MonoAotCompile *acfg)
{
	FILE *infile;
	char *tmp;
	int file_index, res, method_index, i;
	char ver [256];
	guint32 token;
	GList *unordered;

	file_index = 0;
	while (TRUE) {
		tmp = g_strdup_printf ("%s/.mono/aot-profile-data/%s-%s-%d", g_get_home_dir (), acfg->image->assembly_name, acfg->image->guid, file_index);

		if (!g_file_test (tmp, G_FILE_TEST_IS_REGULAR)) {
			g_free (tmp);
			break;
		}

		infile = fopen (tmp, "r");
		g_assert (infile);

		printf ("Using profile data file '%s'\n", tmp);
		g_free (tmp);

		file_index ++;

		res = fscanf (infile, "%32s\n", ver);
		if ((res != 1) || strcmp (ver, "#VER:1") != 0) {
			printf ("Profile file has wrong version or invalid.\n");
			fclose (infile);
			continue;
		}

		while (TRUE) {
			res = fscanf (infile, "%d\n", &token);
			if (res < 1)
				break;

			method_index = mono_metadata_token_index (token) - 1;

			if (!g_list_find (acfg->method_order, GUINT_TO_POINTER (method_index)))
				acfg->method_order = g_list_append (acfg->method_order, GUINT_TO_POINTER (method_index));
		}
		fclose (infile);
	}

	/* Add missing methods */
	unordered = NULL;
	for (i = 0; i < acfg->image->tables [MONO_TABLE_METHOD].rows; ++i) {
		if (!g_list_find (acfg->method_order, GUINT_TO_POINTER (i)))
			unordered = g_list_prepend (unordered, GUINT_TO_POINTER (i));
	}
	unordered = g_list_reverse (unordered);
	if (acfg->method_order)
		g_list_last (acfg->method_order)->next = unordered;
	else
		acfg->method_order = unordered;
}
 
/* Used by the LLVM backend */
guint32
mono_aot_get_got_offset (MonoJumpInfo *ji)
{
	return get_got_offset (llvm_acfg, ji);
}

char*
mono_aot_get_method_name (MonoCompile *cfg)
{
	guint32 method_index = get_method_index (llvm_acfg, cfg->orig_method);

	/* LLVM converts these to .Lm_%x */
	return g_strdup_printf ("m_%x", method_index);
}

char*
mono_aot_get_method_debug_name (MonoCompile *cfg)
{
	return get_debug_sym (cfg->orig_method, "", llvm_acfg->method_label_hash);
}

char*
mono_aot_get_plt_symbol (MonoJumpInfoType type, gconstpointer data)
{
	MonoJumpInfo *ji = mono_mempool_alloc (llvm_acfg->mempool, sizeof (MonoJumpInfo));
	int offset;

	ji->type = type;
	ji->data.target = data;

	if (!can_encode_patch (llvm_acfg, ji))
		return NULL;

	offset = get_plt_offset (llvm_acfg, ji);

	return g_strdup_printf (".Lp_%d", offset);
}

MonoJumpInfo*
mono_aot_patch_info_dup (MonoJumpInfo* ji)
{
	MonoJumpInfo *res;

	mono_acfg_lock (llvm_acfg);
	res = mono_patch_info_dup_mp (llvm_acfg->mempool, ji);
	mono_acfg_unlock (llvm_acfg);

	return res;
}

#ifdef ENABLE_LLVM

/*
 * emit_llvm_file:
 *
 *   Emit the LLVM code into an LLVM bytecode file, and compile it using the LLVM
 * tools.
 */
static void
emit_llvm_file (MonoAotCompile *acfg)
{
	char *command, *opts;
	int i;
	MonoJumpInfo *patch_info;

	/*
	 * When using LLVM, we let llvm emit the got since the LLVM IL needs to refer
	 * to it.
	 */

	/* Compute the final size of the got */
	for (i = 0; i < acfg->nmethods; ++i) {
		if (acfg->cfgs [i]) {
			for (patch_info = acfg->cfgs [i]->patch_info; patch_info; patch_info = patch_info->next) {
				if (patch_info->type != MONO_PATCH_INFO_NONE) {
					if (!is_plt_patch (patch_info))
						get_got_offset (acfg, patch_info);
					else
						get_plt_offset (acfg, patch_info);
				}
			}
		}
	}
	acfg->final_got_size = acfg->got_offset + acfg->plt_offset;

	mono_llvm_emit_aot_module ("temp.bc", acfg->final_got_size);

	/*
	 * FIXME: Experiment with adding optimizations, the -std-compile-opts set takes
	 * a lot of time, and doesn't seem to save much space.
	 * The following optimizations cannot be enabled:
	 * - 'tailcallelim'
	 */
	opts = g_strdup ("-instcombine -simplifycfg");
#if 1
	command = g_strdup_printf ("opt -f %s -o temp.opt.bc temp.bc", opts);
	printf ("Executing opt: %s\n", command);
	if (system (command) != 0) {
		exit (1);
	}
#endif
	g_free (opts);

	//command = g_strdup_printf ("llc -march=arm -mtriple=arm-linux-gnueabi -f -relocation-model=pic -unwind-tables temp.bc");
	command = g_strdup_printf ("llc -f -relocation-model=pic -unwind-tables -o temp.s temp.opt.bc");
	printf ("Executing llc: %s\n", command);

	if (system (command) != 0) {
		exit (1);
	}
}
#endif

static void
emit_code (MonoAotCompile *acfg)
{
	int i;
	char symbol [256];
	GList *l;

#if defined(TARGET_POWERPC64)
	sprintf (symbol, ".Lgot_addr");
	emit_section_change (acfg, ".text", 0);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);
	emit_pointer (acfg, acfg->got_symbol);
#endif

	if (!acfg->llvm) {
		sprintf (symbol, "methods");
		emit_section_change (acfg, ".text", 0);
		emit_global (acfg, symbol, TRUE);
		emit_alignment (acfg, 8);
		emit_label (acfg, symbol);
	}

	/* 
	 * Emit some padding so the local symbol for the first method doesn't have the
	 * same address as 'methods'.
	 */
	emit_zero_bytes (acfg, 16);

	for (l = acfg->method_order; l != NULL; l = l->next) {
		i = GPOINTER_TO_UINT (l->data);

		if (acfg->cfgs [i]) {
			if (acfg->cfgs [i]->compile_llvm)
				acfg->stats.llvm_count ++;
			else
				emit_method_code (acfg, acfg->cfgs [i]);
		}
	}

	sprintf (symbol, "methods_end");
	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	sprintf (symbol, "code_offsets");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	acfg->stats.offsets_size += acfg->nmethods * 4;

	for (i = 0; i < acfg->nmethods; ++i) {
		if (acfg->cfgs [i]) {
			sprintf (symbol, "%sm_%x", acfg->temp_prefix, i);
			emit_symbol_diff (acfg, symbol, "methods", 0);
		} else {
			emit_int32 (acfg, 0xffffffff);
		}
	}
	emit_line (acfg);
}

static void
emit_info (MonoAotCompile *acfg)
{
	int i;
	char symbol [256];
	GList *l;
	gint32 *offsets;

	offsets = g_new0 (gint32, acfg->nmethods);

	for (l = acfg->method_order; l != NULL; l = l->next) {
		i = GPOINTER_TO_UINT (l->data);

		if (acfg->cfgs [i]) {
			emit_method_info (acfg, acfg->cfgs [i]);
			offsets [i] = acfg->cfgs [i]->method_info_offset;
		} else {
			offsets [i] = 0;
		}
	}

	sprintf (symbol, "method_info_offsets");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	acfg->stats.offsets_size += emit_offset_table (acfg, acfg->nmethods, 10, offsets);

	g_free (offsets);
}

#endif /* #if !defined(DISABLE_AOT) && !defined(DISABLE_JIT) */

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
#define mix(a,b,c) { \
	a -= c;  a ^= rot(c, 4);  c += b; \
	b -= a;  b ^= rot(a, 6);  a += c; \
	c -= b;  c ^= rot(b, 8);  b += a; \
	a -= c;  a ^= rot(c,16);  c += b; \
	b -= a;  b ^= rot(a,19);  a += c; \
	c -= b;  c ^= rot(b, 4);  b += a; \
}
#define final(a,b,c) { \
	c ^= b; c -= rot(b,14); \
	a ^= c; a -= rot(c,11); \
	b ^= a; b -= rot(a,25); \
	c ^= b; c -= rot(b,16); \
	a ^= c; a -= rot(c,4);  \
	b ^= a; b -= rot(a,14); \
	c ^= b; c -= rot(b,24); \
}

static guint
mono_aot_type_hash (MonoType *t1)
{
	guint hash = t1->type;

	hash |= t1->byref << 6; /* do not collide with t1->type values */
	switch (t1->type) {
	case MONO_TYPE_VALUETYPE:
	case MONO_TYPE_CLASS:
	case MONO_TYPE_SZARRAY:
		/* check if the distribution is good enough */
		return ((hash << 5) - hash) ^ mono_metadata_str_hash (t1->data.klass->name);
	case MONO_TYPE_PTR:
		return ((hash << 5) - hash) ^ mono_metadata_type_hash (t1->data.type);
	case MONO_TYPE_ARRAY:
		return ((hash << 5) - hash) ^ mono_metadata_type_hash (&t1->data.array->eklass->byval_arg);
	case MONO_TYPE_GENERICINST:
		return ((hash << 5) - hash) ^ 0;
	}
	return hash;
}

/*
 * mono_aot_method_hash:
 *
 *   Return a hash code for methods which only depends on metadata.
 */
guint32
mono_aot_method_hash (MonoMethod *method)
{
	MonoMethodSignature *sig;
	MonoClass *klass;
	int i;
	int hashes_count;
	guint32 *hashes_start, *hashes;
	guint32 a, b, c;

	/* Similar to the hash in mono_method_get_imt_slot () */

	sig = mono_method_signature (method);

	hashes_count = sig->param_count + 5;
	hashes_start = malloc (hashes_count * sizeof (guint32));
	hashes = hashes_start;

	/* Some wrappers are assigned to random classes */
	if (!method->wrapper_type || method->wrapper_type == MONO_WRAPPER_REMOTING_INVOKE_WITH_CHECK)
		klass = method->klass;
	else
		klass = mono_defaults.object_class;

	if (!method->wrapper_type) {
		char *full_name = mono_type_full_name (&klass->byval_arg);

		hashes [0] = mono_metadata_str_hash (full_name);
		hashes [1] = 0;
		g_free (full_name);
	} else {
		hashes [0] = mono_metadata_str_hash (klass->name);
		hashes [1] = mono_metadata_str_hash (klass->name_space);
	}
	if (method->wrapper_type == MONO_WRAPPER_STFLD || method->wrapper_type == MONO_WRAPPER_LDFLD || method->wrapper_type == MONO_WRAPPER_LDFLDA)
		/* The method name includes a stringified pointer */
		hashes [2] = 0;
	else
		hashes [2] = mono_metadata_str_hash (method->name);
	hashes [3] = method->wrapper_type;
	hashes [4] = mono_aot_type_hash (sig->ret);
	for (i = 0; i < sig->param_count; i++) {
		hashes [5 + i] = mono_aot_type_hash (sig->params [i]);
	}
	
	/* Setup internal state */
	a = b = c = 0xdeadbeef + (((guint32)hashes_count)<<2);

	/* Handle most of the hashes */
	while (hashes_count > 3) {
		a += hashes [0];
		b += hashes [1];
		c += hashes [2];
		mix (a,b,c);
		hashes_count -= 3;
		hashes += 3;
	}

	/* Handle the last 3 hashes (all the case statements fall through) */
	switch (hashes_count) { 
	case 3 : c += hashes [2];
	case 2 : b += hashes [1];
	case 1 : a += hashes [0];
		final (a,b,c);
	case 0: /* nothing left to add */
		break;
	}
	
	free (hashes_start);
	
	return c;
}
#undef rot
#undef mix
#undef final

/*
 * mono_aot_wrapper_name:
 *
 *   Return a string which uniqely identifies the given wrapper method.
 */
char*
mono_aot_wrapper_name (MonoMethod *method)
{
	char *name, *tmpsig, *klass_desc;

	tmpsig = mono_signature_get_desc (mono_method_signature (method), TRUE);

	switch (method->wrapper_type) {
	case MONO_WRAPPER_RUNTIME_INVOKE:
		if (!strcmp (method->name, "runtime_invoke_dynamic"))
			name = g_strdup_printf ("(wrapper runtime-invoke-dynamic)");
		else
			name = g_strdup_printf ("%s (%s)", method->name, tmpsig);
		break;
	default:
		klass_desc = mono_type_full_name (&method->klass->byval_arg);
		name = g_strdup_printf ("%s:%s (%s)", klass_desc, method->name, tmpsig);
		g_free (klass_desc);
		break;
	}

	g_free (tmpsig);

	return name;
}

/*
 * mono_aot_get_array_helper_from_wrapper;
 *
 * Get the helper method in Array called by an array wrapper method.
 */
MonoMethod*
mono_aot_get_array_helper_from_wrapper (MonoMethod *method)
{
	MonoMethod *m;
	const char *prefix;
	MonoGenericContext ctx;
	MonoType *args [16];
	char *mname, *iname, *s, *s2, *helper_name = NULL;

	prefix = "System.Collections.Generic";
	s = g_strdup_printf ("%s", method->name + strlen (prefix) + 1);
	s2 = strstr (s, "`1.");
	g_assert (s2);
	s2 [0] = '\0';
	iname = s;
	mname = s2 + 3;

	//printf ("X: %s %s\n", iname, mname);

	if (!strcmp (iname, "IList"))
		helper_name = g_strdup_printf ("InternalArray__%s", mname);
	else
		helper_name = g_strdup_printf ("InternalArray__%s_%s", iname, mname);
	m = mono_class_get_method_from_name (mono_defaults.array_class, helper_name, mono_method_signature (method)->param_count);
	g_assert (m);
	g_free (helper_name);
	g_free (s);

	if (m->is_generic) {
		memset (&ctx, 0, sizeof (ctx));
		args [0] = &method->klass->element_class->byval_arg;
		ctx.method_inst = mono_metadata_get_generic_inst (1, args);
		m = mono_class_inflate_generic_method (m, &ctx);
	}

	return m;
}

/*
 * mono_aot_tramp_info_create:
 *
 *   Create a MonoAotTrampInfo structure from the arguments.
 */
MonoAotTrampInfo*
mono_aot_tramp_info_create (const char *name, guint8 *code, guint32 code_size)
{
	MonoAotTrampInfo *info = g_new0 (MonoAotTrampInfo, 1);

	info->name = (char*)name;
	info->code = code;
	info->code_size = code_size;

	return info;
}

#if !defined(DISABLE_AOT) && !defined(DISABLE_JIT)

typedef struct HashEntry {
    guint32 key, value, index;
	struct HashEntry *next;
} HashEntry;

/*
 * emit_extra_methods:
 *
 * Emit methods which are not in the METHOD table, like wrappers.
 */
static void
emit_extra_methods (MonoAotCompile *acfg)
{
	int i, table_size, buf_size;
	char symbol [256];
	guint8 *p, *buf;
	guint32 *info_offsets;
	guint32 hash;
	GPtrArray *table;
	HashEntry *entry, *new_entry;
	int nmethods, max_chain_length;
	int *chain_lengths;

	info_offsets = g_new0 (guint32, acfg->extra_methods->len);

	/* Emit method info */
	nmethods = 0;
	for (i = 0; i < acfg->extra_methods->len; ++i) {
		MonoMethod *method = g_ptr_array_index (acfg->extra_methods, i);
		MonoCompile *cfg = g_hash_table_lookup (acfg->method_to_cfg, method);
		char *name;

		if (!cfg)
			continue;

		buf_size = 512;
		p = buf = g_malloc (buf_size);

		nmethods ++;

		name = NULL;
		if (method->wrapper_type) {
			/* 
			 * We encode some wrappers using their name, since encoding them
			 * directly would be difficult. This also avoids creating the wrapper
			 * methods at runtime, since they are not needed anyway.
			 */
			switch (method->wrapper_type) {
			case MONO_WRAPPER_REMOTING_INVOKE_WITH_CHECK:
			case MONO_WRAPPER_SYNCHRONIZED:
				/* encode_method_ref () can handle these */
				break;
			case MONO_WRAPPER_RUNTIME_INVOKE:
				if (mono_marshal_method_from_wrapper (method) != method && !strstr (method->name, "virtual"))
					/* Direct wrapper, encode normally */
					break;
				/* Fall through */
			default:
				name = mono_aot_wrapper_name (method);
				break;
			}
		}

		if (name) {
			encode_value (1, p, &p);
			encode_value (method->wrapper_type, p, &p);
			strcpy ((char*)p, name);
			p += strlen (name ) + 1;
			g_free (name);
		} else {
			encode_value (0, p, &p);
			encode_method_ref (acfg, method, p, &p);
		}

		g_assert ((p - buf) < buf_size);

		info_offsets [i] = add_to_blob (acfg, buf, p - buf);
		g_free (buf);
	}

	/*
	 * Construct a chained hash table for mapping indexes in extra_method_info to
	 * method indexes.
	 */
	table_size = g_spaced_primes_closest ((int)(nmethods * 1.5));
	table = g_ptr_array_sized_new (table_size);
	for (i = 0; i < table_size; ++i)
		g_ptr_array_add (table, NULL);
	chain_lengths = g_new0 (int, table_size);
	max_chain_length = 0;
	for (i = 0; i < acfg->extra_methods->len; ++i) {
		MonoMethod *method = g_ptr_array_index (acfg->extra_methods, i);
		MonoCompile *cfg = g_hash_table_lookup (acfg->method_to_cfg, method);
		guint32 key, value;

		if (!cfg)
			continue;

		key = info_offsets [i];
		value = get_method_index (acfg, method);

		hash = mono_aot_method_hash (method) % table_size;

		chain_lengths [hash] ++;
		max_chain_length = MAX (max_chain_length, chain_lengths [hash]);

		new_entry = mono_mempool_alloc0 (acfg->mempool, sizeof (HashEntry));
		new_entry->key = key;
		new_entry->value = value;

		entry = g_ptr_array_index (table, hash);
		if (entry == NULL) {
			new_entry->index = hash;
			g_ptr_array_index (table, hash) = new_entry;
		} else {
			while (entry->next)
				entry = entry->next;
			
			entry->next = new_entry;
			new_entry->index = table->len;
			g_ptr_array_add (table, new_entry);
		}
	}

	//printf ("MAX: %d\n", max_chain_length);

	/* Emit the table */
	sprintf (symbol, "extra_method_table");
	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	emit_int32 (acfg, table_size);
	for (i = 0; i < table->len; ++i) {
		HashEntry *entry = g_ptr_array_index (table, i);

		if (entry == NULL) {
			emit_int32 (acfg, 0);
			emit_int32 (acfg, 0);
			emit_int32 (acfg, 0);
		} else {
			//g_assert (entry->key > 0);
			emit_int32 (acfg, entry->key);
			emit_int32 (acfg, entry->value);
			if (entry->next)
				emit_int32 (acfg, entry->next->index);
			else
				emit_int32 (acfg, 0);
		}
	}

	/* 
	 * Emit a table reverse mapping method indexes to their index in extra_method_info.
	 * This is used by mono_aot_find_jit_info ().
	 */
	sprintf (symbol, "extra_method_info_offsets");
	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	emit_int32 (acfg, acfg->extra_methods->len);
	for (i = 0; i < acfg->extra_methods->len; ++i) {
		MonoMethod *method = g_ptr_array_index (acfg->extra_methods, i);

		emit_int32 (acfg, get_method_index (acfg, method));
		emit_int32 (acfg, info_offsets [i]);
	}
}	

static void
emit_exception_info (MonoAotCompile *acfg)
{
	int i;
	char symbol [256];
	gint32 *offsets;

	offsets = g_new0 (gint32, acfg->nmethods);
	for (i = 0; i < acfg->nmethods; ++i) {
		if (acfg->cfgs [i]) {
			emit_exception_debug_info (acfg, acfg->cfgs [i]);
			offsets [i] = acfg->cfgs [i]->ex_info_offset;
		} else {
			offsets [i] = 0;
		}
	}

	sprintf (symbol, "ex_info_offsets");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	acfg->stats.offsets_size += emit_offset_table (acfg, acfg->nmethods, 10, offsets);
	g_free (offsets);
}

static void
emit_unwind_info (MonoAotCompile *acfg)
{
	int i;
	char symbol [128];

	/* 
	 * The unwind info contains a lot of duplicates so we emit each unique
	 * entry once, and only store the offset from the start of the table in the
	 * exception info.
	 */

	sprintf (symbol, "unwind_info");
	emit_section_change (acfg, ".text", 1);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);
	emit_global (acfg, symbol, FALSE);

	for (i = 0; i < acfg->unwind_ops->len; ++i) {
		guint32 index = GPOINTER_TO_UINT (g_ptr_array_index (acfg->unwind_ops, i));
		guint8 *unwind_info;
		guint32 unwind_info_len;
		guint8 buf [16];
		guint8 *p;

		unwind_info = mono_get_cached_unwind_info (index, &unwind_info_len);

		p = buf;
		encode_value (unwind_info_len, p, &p);
		emit_bytes (acfg, buf, p - buf);
		emit_bytes (acfg, unwind_info, unwind_info_len);

		acfg->stats.unwind_info_size += (p - buf) + unwind_info_len;
	}
}

static void
emit_class_info (MonoAotCompile *acfg)
{
	int i;
	char symbol [256];
	gint32 *offsets;

	offsets = g_new0 (gint32, acfg->image->tables [MONO_TABLE_TYPEDEF].rows);
	for (i = 0; i < acfg->image->tables [MONO_TABLE_TYPEDEF].rows; ++i)
		offsets [i] = emit_klass_info (acfg, MONO_TOKEN_TYPE_DEF | (i + 1));

	sprintf (symbol, "class_info_offsets");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	acfg->stats.offsets_size += emit_offset_table (acfg, acfg->image->tables [MONO_TABLE_TYPEDEF].rows, 10, offsets);
	g_free (offsets);
}

typedef struct ClassNameTableEntry {
	guint32 token, index;
	struct ClassNameTableEntry *next;
} ClassNameTableEntry;

static void
emit_class_name_table (MonoAotCompile *acfg)
{
	int i, table_size;
	guint32 token, hash;
	MonoClass *klass;
	GPtrArray *table;
	char *full_name;
	char symbol [256];
	ClassNameTableEntry *entry, *new_entry;

	/*
	 * Construct a chained hash table for mapping class names to typedef tokens.
	 */
	table_size = g_spaced_primes_closest ((int)(acfg->image->tables [MONO_TABLE_TYPEDEF].rows * 1.5));
	table = g_ptr_array_sized_new (table_size);
	for (i = 0; i < table_size; ++i)
		g_ptr_array_add (table, NULL);
	for (i = 0; i < acfg->image->tables [MONO_TABLE_TYPEDEF].rows; ++i) {
		token = MONO_TOKEN_TYPE_DEF | (i + 1);
		klass = mono_class_get (acfg->image, token);
		full_name = mono_type_get_name_full (mono_class_get_type (klass), MONO_TYPE_NAME_FORMAT_FULL_NAME);
		hash = mono_metadata_str_hash (full_name) % table_size;
		g_free (full_name);

		/* FIXME: Allocate from the mempool */
		new_entry = g_new0 (ClassNameTableEntry, 1);
		new_entry->token = token;

		entry = g_ptr_array_index (table, hash);
		if (entry == NULL) {
			new_entry->index = hash;
			g_ptr_array_index (table, hash) = new_entry;
		} else {
			while (entry->next)
				entry = entry->next;
			
			entry->next = new_entry;
			new_entry->index = table->len;
			g_ptr_array_add (table, new_entry);
		}
	}

	/* Emit the table */
	sprintf (symbol, "class_name_table");
	emit_section_change (acfg, ".text", 0);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	/* FIXME: Optimize memory usage */
	g_assert (table_size < 65000);
	emit_int16 (acfg, table_size);
	g_assert (table->len < 65000);
	for (i = 0; i < table->len; ++i) {
		ClassNameTableEntry *entry = g_ptr_array_index (table, i);

		if (entry == NULL) {
			emit_int16 (acfg, 0);
			emit_int16 (acfg, 0);
		} else {
			emit_int16 (acfg, mono_metadata_token_index (entry->token));
			if (entry->next)
				emit_int16 (acfg, entry->next->index);
			else
				emit_int16 (acfg, 0);
		}
	}
}

static void
emit_image_table (MonoAotCompile *acfg)
{
	int i;
	char symbol [256];

	/*
	 * The image table is small but referenced in a lot of places.
	 * So we emit it at once, and reference its elements by an index.
	 */

	sprintf (symbol, "mono_image_table");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	emit_int32 (acfg, acfg->image_table->len);
	for (i = 0; i < acfg->image_table->len; i++) {
		MonoImage *image = (MonoImage*)g_ptr_array_index (acfg->image_table, i);
		MonoAssemblyName *aname = &image->assembly->aname;

		/* FIXME: Support multi-module assemblies */
		g_assert (image->assembly->image == image);

		emit_string (acfg, image->assembly_name);
		emit_string (acfg, image->guid);
		emit_string (acfg, aname->culture ? aname->culture : "");
		emit_string (acfg, (const char*)aname->public_key_token);

		emit_alignment (acfg, 8);
		emit_int32 (acfg, aname->flags);
		emit_int32 (acfg, aname->major);
		emit_int32 (acfg, aname->minor);
		emit_int32 (acfg, aname->build);
		emit_int32 (acfg, aname->revision);
	}
}

static void
emit_got_info (MonoAotCompile *acfg)
{
	char symbol [256];
	int i, first_plt_got_patch, buf_size;
	guint8 *p, *buf;
	guint32 *got_info_offsets;

	/* Add the patches needed by the PLT to the GOT */
	acfg->plt_got_offset_base = acfg->got_offset;
	first_plt_got_patch = acfg->got_patches->len;
	for (i = 1; i < acfg->plt_offset; ++i) {
		MonoJumpInfo *patch_info = g_hash_table_lookup (acfg->plt_offset_to_patch, GUINT_TO_POINTER (i));

		g_ptr_array_add (acfg->got_patches, patch_info);
	}

	acfg->got_offset += acfg->plt_offset;

	/**
	 * FIXME: 
	 * - optimize offsets table.
	 * - reduce number of exported symbols.
	 * - emit info for a klass only once.
	 * - determine when a method uses a GOT slot which is guaranteed to be already 
	 *   initialized.
	 * - clean up and document the code.
	 * - use String.Empty in class libs.
	 */

	/* Encode info required to decode shared GOT entries */
	buf_size = acfg->got_patches->len * 64;
	p = buf = mono_mempool_alloc (acfg->mempool, buf_size);
	got_info_offsets = mono_mempool_alloc (acfg->mempool, acfg->got_patches->len * sizeof (guint32));
	acfg->plt_got_info_offsets = mono_mempool_alloc (acfg->mempool, acfg->plt_offset * sizeof (guint32));
	/* Unused */
	if (acfg->plt_offset)
		acfg->plt_got_info_offsets [0] = 0;
	for (i = 0; i < acfg->got_patches->len; ++i) {
		MonoJumpInfo *ji = g_ptr_array_index (acfg->got_patches, i);

		p = buf;

		encode_value (ji->type, p, &p);
		encode_patch (acfg, ji, p, &p);

		g_assert (p - buf <= buf_size);
		got_info_offsets [i] = add_to_blob (acfg, buf, p - buf);

		if (i >= first_plt_got_patch)
			acfg->plt_got_info_offsets [i - first_plt_got_patch + 1] = got_info_offsets [i];
		acfg->stats.got_info_size += p - buf;
	}

	/* Emit got_info_offsets table */
	sprintf (symbol, "got_info_offsets");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	/* No need to emit offsets for the got plt entries, the plt embeds them directly */
	acfg->stats.offsets_size += emit_offset_table (acfg, first_plt_got_patch, 10, (gint32*)got_info_offsets);
}

static void
emit_got (MonoAotCompile *acfg)
{
	char symbol [256];

	if (!acfg->llvm) {
		/* Don't make GOT global so accesses to it don't need relocations */
		sprintf (symbol, "%s", acfg->got_symbol);
		emit_section_change (acfg, ".bss", 0);
		emit_alignment (acfg, 8);
		emit_local_symbol (acfg, symbol, "got_end", FALSE);
		emit_label (acfg, symbol);
		if (acfg->got_offset > 0)
			emit_zero_bytes (acfg, (int)(acfg->got_offset * sizeof (gpointer)));

		sprintf (symbol, "got_end");
		emit_label (acfg, symbol);
	}

	sprintf (symbol, "mono_aot_got_addr");
	emit_section_change (acfg, ".data", 0);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);
	emit_pointer (acfg, acfg->got_symbol);
}

typedef struct GlobalsTableEntry {
	guint32 value, index;
	struct GlobalsTableEntry *next;
} GlobalsTableEntry;

static void
emit_globals_table (MonoAotCompile *acfg)
{
	int i, table_size;
	guint32 hash;
	GPtrArray *table;
	char symbol [256];
	GlobalsTableEntry *entry, *new_entry;

	/*
	 * Construct a chained hash table for mapping global names to their index in
	 * the globals table.
	 */
	table_size = g_spaced_primes_closest ((int)(acfg->globals->len * 1.5));
	table = g_ptr_array_sized_new (table_size);
	for (i = 0; i < table_size; ++i)
		g_ptr_array_add (table, NULL);
	for (i = 0; i < acfg->globals->len; ++i) {
		char *name = g_ptr_array_index (acfg->globals, i);

		hash = mono_metadata_str_hash (name) % table_size;

		/* FIXME: Allocate from the mempool */
		new_entry = g_new0 (GlobalsTableEntry, 1);
		new_entry->value = i;

		entry = g_ptr_array_index (table, hash);
		if (entry == NULL) {
			new_entry->index = hash;
			g_ptr_array_index (table, hash) = new_entry;
		} else {
			while (entry->next)
				entry = entry->next;
			
			entry->next = new_entry;
			new_entry->index = table->len;
			g_ptr_array_add (table, new_entry);
		}
	}

	/* Emit the table */
	sprintf (symbol, ".Lglobals_hash");
	emit_section_change (acfg, ".text", 0);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	/* FIXME: Optimize memory usage */
	g_assert (table_size < 65000);
	emit_int16 (acfg, table_size);
	for (i = 0; i < table->len; ++i) {
		GlobalsTableEntry *entry = g_ptr_array_index (table, i);

		if (entry == NULL) {
			emit_int16 (acfg, 0);
			emit_int16 (acfg, 0);
		} else {
			emit_int16 (acfg, entry->value + 1);
			if (entry->next)
				emit_int16 (acfg, entry->next->index);
			else
				emit_int16 (acfg, 0);
		}
	}

	/* Emit the names */
	for (i = 0; i < acfg->globals->len; ++i) {
		char *name = g_ptr_array_index (acfg->globals, i);

		sprintf (symbol, "name_%d", i);
		emit_section_change (acfg, ".text", 1);
		emit_label (acfg, symbol);
		emit_string (acfg, name);
	}

	/* Emit the globals table */
	sprintf (symbol, ".Lglobals");
	emit_section_change (acfg, ".data", 0);
	/* This is not a global, since it is accessed by the init function */
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	sprintf (symbol, "%sglobals_hash", acfg->temp_prefix);
	emit_pointer (acfg, symbol);

	for (i = 0; i < acfg->globals->len; ++i) {
		char *name = g_ptr_array_index (acfg->globals, i);

		sprintf (symbol, "name_%d", i);
		emit_pointer (acfg, symbol);

		sprintf (symbol, "%s", name);
		emit_pointer (acfg, symbol);
	}
	/* Null terminate the table */
	emit_int32 (acfg, 0);
	emit_int32 (acfg, 0);
}

static void
emit_globals (MonoAotCompile *acfg)
{
	char *build_info;

	emit_string_symbol (acfg, "mono_assembly_guid" , acfg->image->guid);

	emit_string_symbol (acfg, "mono_aot_version", MONO_AOT_FILE_VERSION);

	if (acfg->aot_opts.bind_to_runtime_version) {
		build_info = mono_get_runtime_build_info ();
		emit_string_symbol (acfg, "mono_runtime_version", build_info);
		g_free (build_info);
	} else {
		emit_string_symbol (acfg, "mono_runtime_version", "");
	}

	/* 
	 * When static linking, we emit a global which will point to the symbol table.
	 */
	if (acfg->aot_opts.static_link) {
		char symbol [256];
		char *p;

		/* Emit a string holding the assembly name */
		emit_string_symbol (acfg, "mono_aot_assembly_name", acfg->image->assembly->aname.name);

		emit_globals_table (acfg);

		/* 
		 * Emit a global symbol which can be passed by an embedding app to
		 * mono_aot_register_module ().
		 */
#if defined(__MACH__)
		sprintf (symbol, "_mono_aot_module_%s_info", acfg->image->assembly->aname.name);
#else
		sprintf (symbol, "mono_aot_module_%s_info", acfg->image->assembly->aname.name);
#endif

		/* Get rid of characters which cannot occur in symbols */
		p = symbol;
		for (p = symbol; *p; ++p) {
			if (!(isalnum (*p) || *p == '_'))
				*p = '_';
		}
		acfg->static_linking_symbol = g_strdup (symbol);
		emit_global_inner (acfg, symbol, FALSE);
		emit_alignment (acfg, 8);
		emit_label (acfg, symbol);
		sprintf (symbol, "%sglobals", acfg->temp_prefix);
		emit_pointer (acfg, symbol);
	}
}

static void
emit_autoreg (MonoAotCompile *acfg)
{
	char *symbol;

	/*
	 * Emit a function into the .ctor section which will be called by the ELF
	 * loader to register this module with the runtime.
	 */
	if (! (!acfg->use_bin_writer && acfg->aot_opts.static_link && acfg->aot_opts.autoreg))
		return;

	symbol = g_strdup_printf ("_%s_autoreg", acfg->static_linking_symbol);

	arch_emit_autoreg (acfg, symbol);

	g_free (symbol);
}	

static void
emit_mem_end (MonoAotCompile *acfg)
{
	char symbol [128];

	sprintf (symbol, "mem_end");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);
}

/*
 * Emit a structure containing all the information not stored elsewhere.
 */
static void
emit_file_info (MonoAotCompile *acfg)
{
	char symbol [128];
	int i;

	sprintf (symbol, "mono_aot_file_info");
	emit_section_change (acfg, ".data", 0);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);
	emit_global (acfg, symbol, FALSE);

	/* The data emitted here must match MonoAotFileInfo in aot-runtime.c. */
	emit_int32 (acfg, acfg->plt_got_offset_base);
	emit_int32 (acfg, (int)(acfg->got_offset * sizeof (gpointer)));
	emit_int32 (acfg, acfg->plt_offset);
	emit_int32 (acfg, acfg->nmethods);
	emit_int32 (acfg, acfg->flags);
	emit_int32 (acfg, acfg->opts);

	for (i = 0; i < MONO_AOT_TRAMP_NUM; ++i)
		emit_int32 (acfg, acfg->num_trampolines [i]);
	for (i = 0; i < MONO_AOT_TRAMP_NUM; ++i)
		emit_int32 (acfg, acfg->trampoline_got_offset_base [i]);
	for (i = 0; i < MONO_AOT_TRAMP_NUM; ++i)
		emit_int32 (acfg, acfg->trampoline_size [i]);
}

static void
emit_blob (MonoAotCompile *acfg)
{
	char symbol [128];

	sprintf (symbol, "blob");
	emit_section_change (acfg, ".text", 1);
	emit_global (acfg, symbol, FALSE);
	emit_alignment (acfg, 8);
	emit_label (acfg, symbol);

	emit_bytes (acfg, (guint8*)acfg->blob.data, acfg->blob.index);
}

static void
emit_dwarf_info (MonoAotCompile *acfg)
{
#ifdef EMIT_DWARF_INFO
	int i;
	char symbol [128], symbol2 [128];

	/* DIEs for methods */
	for (i = 0; i < acfg->nmethods; ++i) {
		MonoCompile *cfg = acfg->cfgs [i];

		if (!cfg)
			continue;

		// FIXME: LLVM doesn't define .Lme_...
		if (cfg->compile_llvm)
			continue;

		sprintf (symbol, "%sm_%x", acfg->temp_prefix, i);
		sprintf (symbol2, "%sme_%x", acfg->temp_prefix, i);

		mono_dwarf_writer_emit_method (acfg->dwarf, cfg, cfg->method, symbol, symbol2, cfg->jit_info->code_start, cfg->jit_info->code_size, cfg->args, cfg->locals, cfg->unwind_ops, mono_debug_find_method (cfg->jit_info->method, mono_domain_get ()));
	}
#endif
}

static void
collect_methods (MonoAotCompile *acfg)
{
	int i;
	MonoImage *image = acfg->image;

	/* Collect methods */
	for (i = 0; i < image->tables [MONO_TABLE_METHOD].rows; ++i) {
		MonoMethod *method;
		guint32 token = MONO_TOKEN_METHOD_DEF | (i + 1);

		method = mono_get_method (acfg->image, token, NULL);

		if (!method) {
			printf ("Failed to load method 0x%x from '%s'.\n", token, image->name);
			exit (1);
		}
			
		/* Load all methods eagerly to skip the slower lazy loading code */
		mono_class_setup_methods (method->klass);

		if (acfg->aot_opts.full_aot && method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL) {
			/* Compile the wrapper instead */
			/* We do this here instead of add_wrappers () because it is easy to do it here */
			MonoMethod *wrapper = mono_marshal_get_native_wrapper (method, check_for_pending_exc, TRUE);
			method = wrapper;
		}

		/* FIXME: Some mscorlib methods don't have debug info */
		/*
		if (acfg->aot_opts.soft_debug && !method->wrapper_type) {
			if (!((method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL) ||
				  (method->iflags & METHOD_IMPL_ATTRIBUTE_RUNTIME) ||
				  (method->flags & METHOD_ATTRIBUTE_ABSTRACT) ||
				  (method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL))) {
				if (!mono_debug_lookup_method (method)) {
					fprintf (stderr, "Method %s has no debug info, probably the .mdb file for the assembly is missing.\n", mono_method_full_name (method, TRUE));
					exit (1);
				}
			}
		}
		*/

		/* Since we add the normal methods first, their index will be equal to their zero based token index */
		add_method_with_index (acfg, method, i, FALSE);
		acfg->method_index ++;
	}

	add_generic_instances (acfg);

	if (acfg->aot_opts.full_aot)
		add_wrappers (acfg);
}

static void
compile_methods (MonoAotCompile *acfg)
{
	int i, methods_len;

	if (acfg->aot_opts.nthreads > 0) {
		GPtrArray *frag;
		int len, j;
		GPtrArray *threads;
		HANDLE handle;
		gpointer *user_data;
		MonoMethod **methods;

		methods_len = acfg->methods->len;

		len = acfg->methods->len / acfg->aot_opts.nthreads;
		g_assert (len > 0);
		/* 
		 * Partition the list of methods into fragments, and hand it to threads to
		 * process.
		 */
		threads = g_ptr_array_new ();
		/* Make a copy since acfg->methods is modified by compile_method () */
		methods = g_new0 (MonoMethod*, methods_len);
		//memcpy (methods, g_ptr_array_index (acfg->methods, 0), sizeof (MonoMethod*) * methods_len);
		for (i = 0; i < methods_len; ++i)
			methods [i] = g_ptr_array_index (acfg->methods, i);
		i = 0;
		while (i < methods_len) {
			frag = g_ptr_array_new ();
			for (j = 0; j < len; ++j) {
				if (i < methods_len) {
					g_ptr_array_add (frag, methods [i]);
					i ++;
				}
			}

			user_data = g_new0 (gpointer, 3);
			user_data [0] = mono_domain_get ();
			user_data [1] = acfg;
			user_data [2] = frag;
			
			handle = mono_create_thread (NULL, 0, (gpointer)compile_thread_main, user_data, 0, NULL);
			g_ptr_array_add (threads, handle);
		}
		g_free (methods);

		for (i = 0; i < threads->len; ++i) {
			WaitForSingleObjectEx (g_ptr_array_index (threads, i), INFINITE, FALSE);
		}
	} else {
		methods_len = 0;
	}

	/* Compile methods added by compile_method () or all methods if nthreads == 0 */
	for (i = methods_len; i < acfg->methods->len; ++i) {
		/* This can new methods to acfg->methods */
		compile_method (acfg, g_ptr_array_index (acfg->methods, i));
	}
}

static int
compile_asm (MonoAotCompile *acfg)
{
	char *command, *objfile;
	char *outfile_name, *tmp_outfile_name;
	const char *tool_prefix = acfg->aot_opts.tool_prefix ? acfg->aot_opts.tool_prefix : "";

#if defined(TARGET_AMD64)
#define AS_OPTIONS "--64"
#elif defined(TARGET_POWERPC64)
#define AS_OPTIONS "-a64 -mppc64"
#define LD_OPTIONS "-m elf64ppc"
#elif defined(sparc) && SIZEOF_VOID_P == 8
#define AS_OPTIONS "-xarch=v9"
#else
#define AS_OPTIONS ""
#endif

#ifndef LD_OPTIONS
#define LD_OPTIONS ""
#endif

#ifdef ENABLE_LLVM
#define EH_LD_OPTIONS "--eh-frame-hdr"
#else
#define EH_LD_OPTIONS ""
#endif

	if (acfg->aot_opts.asm_only) {
		printf ("Output file: '%s'.\n", acfg->tmpfname);
		if (acfg->aot_opts.static_link)
			printf ("Linking symbol: '%s'.\n", acfg->static_linking_symbol);
		return 0;
	}

	if (acfg->aot_opts.static_link) {
		if (acfg->aot_opts.outfile)
			objfile = g_strdup_printf ("%s", acfg->aot_opts.outfile);
		else
			objfile = g_strdup_printf ("%s.o", acfg->image->name);
	} else {
		objfile = g_strdup_printf ("%s.o", acfg->tmpfname);
	}
	command = g_strdup_printf ("%sas %s %s -o %s", tool_prefix, AS_OPTIONS, acfg->tmpfname, objfile);
	printf ("Executing the native assembler: %s\n", command);
	if (system (command) != 0) {
		g_free (command);
		g_free (objfile);
		return 1;
	}

	g_free (command);

	if (acfg->aot_opts.static_link) {
		printf ("Output file: '%s'.\n", objfile);
		printf ("Linking symbol: '%s'.\n", acfg->static_linking_symbol);
		g_free (objfile);
		return 0;
	}

	if (acfg->aot_opts.outfile)
		outfile_name = g_strdup_printf ("%s", acfg->aot_opts.outfile);
	else
		outfile_name = g_strdup_printf ("%s%s", acfg->image->name, SHARED_EXT);

	tmp_outfile_name = g_strdup_printf ("%s.tmp", outfile_name);

#if defined(sparc)
	command = g_strdup_printf ("ld -shared -G -o %s %s.o", tmp_outfile_name, acfg->tmpfname);
#elif defined(__ppc__) && defined(__MACH__)
	command = g_strdup_printf ("gcc -dynamiclib -o %s %s.o", tmp_outfile_name, acfg->tmpfname);
#elif defined(HOST_WIN32)
	command = g_strdup_printf ("gcc -shared --dll -mno-cygwin -o %s %s.o", tmp_outfile_name, acfg->tmpfname);
#else
	command = g_strdup_printf ("%sld %s %s -shared -o %s %s.o", tool_prefix, EH_LD_OPTIONS, LD_OPTIONS, tmp_outfile_name, acfg->tmpfname);
#endif
	printf ("Executing the native linker: %s\n", command);
	if (system (command) != 0) {
		g_free (tmp_outfile_name);
		g_free (outfile_name);
		g_free (command);
		g_free (objfile);
		return 1;
	}

	g_free (command);
	unlink (objfile);
	/*com = g_strdup_printf ("strip --strip-unneeded %s%s", acfg->image->name, SHARED_EXT);
	printf ("Stripping the binary: %s\n", com);
	system (com);
	g_free (com);*/

#if defined(TARGET_ARM) && !defined(__MACH__)
	/* 
	 * gas generates 'mapping symbols' each time code and data is mixed, which 
	 * happens a lot in emit_and_reloc_code (), so we need to get rid of them.
	 */
	command = g_strdup_printf ("%sstrip --strip-symbol=\\$a --strip-symbol=\\$d %s", tool_prefix, tmp_outfile_name);
	printf ("Stripping the binary: %s\n", command);
	if (system (command) != 0) {
		g_free (tmp_outfile_name);
		g_free (outfile_name);
		g_free (command);
		g_free (objfile);
		return 1;
	}
#endif

	rename (tmp_outfile_name, outfile_name);

	g_free (tmp_outfile_name);
	g_free (outfile_name);
	g_free (objfile);

	if (acfg->aot_opts.save_temps)
		printf ("Retained input file.\n");
	else
		unlink (acfg->tmpfname);

	return 0;
}

static MonoAotCompile*
acfg_create (MonoAssembly *ass, guint32 opts)
{
	MonoImage *image = ass->image;
	MonoAotCompile *acfg;
	int i;

	acfg = g_new0 (MonoAotCompile, 1);
	acfg->methods = g_ptr_array_new ();
	acfg->method_indexes = g_hash_table_new (NULL, NULL);
	acfg->method_depth = g_hash_table_new (NULL, NULL);
	acfg->plt_offset_to_patch = g_hash_table_new (NULL, NULL);
	acfg->patch_to_plt_offset = g_hash_table_new (mono_patch_info_hash, mono_patch_info_equal);
	acfg->patch_to_got_offset = g_hash_table_new (mono_patch_info_hash, mono_patch_info_equal);
	acfg->patch_to_got_offset_by_type = g_new0 (GHashTable*, MONO_PATCH_INFO_NUM);
	for (i = 0; i < MONO_PATCH_INFO_NUM; ++i)
		acfg->patch_to_got_offset_by_type [i] = g_hash_table_new (mono_patch_info_hash, mono_patch_info_equal);
	acfg->got_patches = g_ptr_array_new ();
	acfg->method_to_cfg = g_hash_table_new (NULL, NULL);
	acfg->token_info_hash = g_hash_table_new_full (NULL, NULL, NULL, g_free);
	acfg->image_hash = g_hash_table_new (NULL, NULL);
	acfg->image_table = g_ptr_array_new ();
	acfg->globals = g_ptr_array_new ();
	acfg->image = image;
	acfg->opts = opts;
	acfg->mempool = mono_mempool_new ();
	acfg->extra_methods = g_ptr_array_new ();
	acfg->unwind_info_offsets = g_hash_table_new (NULL, NULL);
	acfg->unwind_ops = g_ptr_array_new ();
	acfg->method_label_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	InitializeCriticalSection (&acfg->mutex);

	return acfg;
}

static void
acfg_free (MonoAotCompile *acfg)
{
	int i;

	img_writer_destroy (acfg->w);
	for (i = 0; i < acfg->nmethods; ++i)
		if (acfg->cfgs [i])
			g_free (acfg->cfgs [i]);
	g_free (acfg->cfgs);
	g_free (acfg->static_linking_symbol);
	g_free (acfg->got_symbol);
	g_free (acfg->plt_symbol);
	g_ptr_array_free (acfg->methods, TRUE);
	g_ptr_array_free (acfg->got_patches, TRUE);
	g_ptr_array_free (acfg->image_table, TRUE);
	g_ptr_array_free (acfg->globals, TRUE);
	g_ptr_array_free (acfg->unwind_ops, TRUE);
	g_hash_table_destroy (acfg->method_indexes);
	g_hash_table_destroy (acfg->method_depth);
	g_hash_table_destroy (acfg->plt_offset_to_patch);
	g_hash_table_destroy (acfg->patch_to_plt_offset);
	g_hash_table_destroy (acfg->patch_to_got_offset);
	g_hash_table_destroy (acfg->method_to_cfg);
	g_hash_table_destroy (acfg->token_info_hash);
	g_hash_table_destroy (acfg->image_hash);
	g_hash_table_destroy (acfg->unwind_info_offsets);
	g_hash_table_destroy (acfg->method_label_hash);
	for (i = 0; i < MONO_PATCH_INFO_NUM; ++i)
		g_hash_table_destroy (acfg->patch_to_got_offset_by_type [i]);
	g_free (acfg->patch_to_got_offset_by_type);
	mono_mempool_destroy (acfg->mempool);
	g_free (acfg);
}

int
mono_compile_assembly (MonoAssembly *ass, guint32 opts, const char *aot_options)
{
	MonoImage *image = ass->image;
	int res;
	MonoAotCompile *acfg;
	char *outfile_name, *tmp_outfile_name, *p;
	TV_DECLARE (atv);
	TV_DECLARE (btv);

	printf ("Mono Ahead of Time compiler - compiling assembly %s\n", image->name);

	acfg = acfg_create (ass, opts);

	memset (&acfg->aot_opts, 0, sizeof (acfg->aot_opts));
	acfg->aot_opts.write_symbols = TRUE;
	acfg->aot_opts.ntrampolines = 1024;
	acfg->aot_opts.nrgctx_trampolines = 1024;

	mono_aot_parse_options (aot_options, &acfg->aot_opts);

	if (acfg->aot_opts.static_link)
		acfg->aot_opts.autoreg = TRUE;

	//acfg->aot_opts.print_skipped_methods = TRUE;

#ifndef MONO_ARCH_HAVE_FULL_AOT_TRAMPOLINES
	if (acfg->aot_opts.full_aot) {
		printf ("--aot=full is not supported on this platform.\n");
		return 1;
	}
#endif

	if (acfg->aot_opts.static_link)
		acfg->aot_opts.asm_writer = TRUE;

	if (acfg->aot_opts.soft_debug) {
		MonoDebugOptions *opt = mini_get_debug_options ();

		opt->mdb_optimizations = TRUE;
		opt->gen_seq_points = TRUE;

		if (mono_debug_format == MONO_DEBUG_FORMAT_NONE) {
			fprintf (stderr, "The soft-debug AOT option requires the --debug option.\n");
			return 1;
		}
	}

#ifdef ENABLE_LLVM
	acfg->llvm = TRUE;
 	acfg->aot_opts.asm_writer = TRUE;
	acfg->flags |= MONO_AOT_FILE_FLAG_WITH_LLVM;
#endif

	if (acfg->aot_opts.full_aot)
		acfg->flags |= MONO_AOT_FILE_FLAG_FULL_AOT;

	load_profile_files (acfg);

	acfg->num_trampolines [MONO_AOT_TRAMP_SPECIFIC] = acfg->aot_opts.full_aot ? acfg->aot_opts.ntrampolines : 0;
#ifdef MONO_ARCH_HAVE_STATIC_RGCTX_TRAMPOLINE
	acfg->num_trampolines [MONO_AOT_TRAMP_STATIC_RGCTX] = acfg->aot_opts.full_aot ? acfg->aot_opts.nrgctx_trampolines : 0;
#endif
	acfg->num_trampolines [MONO_AOT_TRAMP_IMT_THUNK] = acfg->aot_opts.full_aot ? 128 : 0;

	acfg->got_symbol = g_strdup_printf ("mono_aot_%s_got", acfg->image->assembly->aname.name);
	acfg->plt_symbol = g_strdup_printf ("mono_aot_%s_plt", acfg->image->assembly->aname.name);

	/* Get rid of characters which cannot occur in symbols */
	for (p = acfg->got_symbol; *p; ++p) {
		if (!(isalnum (*p) || *p == '_'))
			*p = '_';
	}
	for (p = acfg->plt_symbol; *p; ++p) {
		if (!(isalnum (*p) || *p == '_'))
			*p = '_';
	}

	acfg->method_index = 1;

	collect_methods (acfg);

	acfg->cfgs_size = acfg->methods->len + 32;
	acfg->cfgs = g_new0 (MonoCompile*, acfg->cfgs_size);

	/* PLT offset 0 is reserved for the PLT trampoline */
	acfg->plt_offset = 1;

#ifdef ENABLE_LLVM
	llvm_acfg = acfg;
	mono_llvm_create_aot_module (acfg->got_symbol);
#endif

	/* GOT offset 0 is reserved for the address of the current assembly */
	{
		MonoJumpInfo *ji;

		ji = mono_mempool_alloc0 (acfg->mempool, sizeof (MonoAotCompile));
		ji->type = MONO_PATCH_INFO_IMAGE;
		ji->data.image = acfg->image;

		get_got_offset (acfg, ji);

		/* Slot 1 is reserved for the mscorlib got addr */
		ji = mono_mempool_alloc0 (acfg->mempool, sizeof (MonoAotCompile));
		ji->type = MONO_PATCH_INFO_MSCORLIB_GOT_ADDR;
		get_got_offset (acfg, ji);
	}

	TV_GETTIME (atv);

	compile_methods (acfg);

	TV_GETTIME (btv);

	acfg->stats.jit_time = TV_ELAPSED (atv, btv);

	TV_GETTIME (atv);

#ifdef ENABLE_LLVM
	emit_llvm_file (acfg);
#endif

	if (!acfg->aot_opts.asm_only && !acfg->aot_opts.asm_writer && bin_writer_supported ()) {
		if (acfg->aot_opts.outfile)
			outfile_name = g_strdup_printf ("%s", acfg->aot_opts.outfile);
		else
			outfile_name = g_strdup_printf ("%s%s", acfg->image->name, SHARED_EXT);

		/* 
		 * Can't use g_file_open_tmp () as it will be deleted at exit, and
		 * it might be in another file system so the rename () won't work.
		 */
		tmp_outfile_name = g_strdup_printf ("%s.tmp", outfile_name);

		acfg->fp = fopen (tmp_outfile_name, "w");
		if (!acfg->fp) {
			printf ("Unable to create temporary file '%s': %s\n", tmp_outfile_name, strerror (errno));
			return 1;
		}

		acfg->w = img_writer_create (acfg->fp, TRUE);
		acfg->use_bin_writer = TRUE;
	} else {
		if (acfg->llvm) {
			/* Append to the .s file created by llvm */
			/* FIXME: Use multiple files instead */
			acfg->tmpfname = g_strdup ("temp.s");
			acfg->fp = fopen (acfg->tmpfname, "a");
		} else {
			if (acfg->aot_opts.asm_only) {
				if (acfg->aot_opts.outfile)
					acfg->tmpfname = g_strdup_printf ("%s", acfg->aot_opts.outfile);
				else
					acfg->tmpfname = g_strdup_printf ("%s.s", acfg->image->name);
				acfg->fp = fopen (acfg->tmpfname, "w+");
			} else {
				int i = g_file_open_tmp ("mono_aot_XXXXXX", &acfg->tmpfname, NULL);
				acfg->fp = fdopen (i, "w+");
			}
			g_assert (acfg->fp);
		}
		acfg->w = img_writer_create (acfg->fp, FALSE);
		
		tmp_outfile_name = NULL;
		outfile_name = NULL;
	}

	acfg->temp_prefix = img_writer_get_temp_label_prefix (acfg->w);

	if (!acfg->aot_opts.nodebug)
		acfg->dwarf = mono_dwarf_writer_create (acfg->w, NULL, 0, FALSE);

	img_writer_emit_start (acfg->w);

	if (acfg->dwarf)
		mono_dwarf_writer_emit_base_info (acfg->dwarf, mono_arch_get_cie_program ());

	emit_code (acfg);

	emit_info (acfg);

	emit_extra_methods (acfg);

	emit_trampolines (acfg);

	emit_class_name_table (acfg);

	emit_got_info (acfg);

	emit_exception_info (acfg);

	emit_unwind_info (acfg);

	emit_class_info (acfg);

	emit_plt (acfg);

	emit_image_table (acfg);

	emit_got (acfg);

	emit_file_info (acfg);

	emit_blob (acfg);

	emit_globals (acfg);

	emit_autoreg (acfg);

	if (acfg->dwarf) {
		emit_dwarf_info (acfg);
		mono_dwarf_writer_close (acfg->dwarf);
	}

	emit_mem_end (acfg);

	TV_GETTIME (btv);

	acfg->stats.gen_time = TV_ELAPSED (atv, btv);

	if (acfg->llvm)
		g_assert (acfg->got_offset == acfg->final_got_size);

	printf ("Code: %d Info: %d Ex Info: %d Unwind Info: %d Class Info: %d PLT: %d GOT Info: %d GOT: %d Offsets: %d\n", acfg->stats.code_size, acfg->stats.info_size, acfg->stats.ex_info_size, acfg->stats.unwind_info_size, acfg->stats.class_info_size, acfg->plt_offset, acfg->stats.got_info_size, (int)(acfg->got_offset * sizeof (gpointer)), acfg->stats.offsets_size);

	TV_GETTIME (atv);
	res = img_writer_emit_writeout (acfg->w);
	if (res != 0) {
		acfg_free (acfg);
		return res;
	}
	if (acfg->use_bin_writer) {
		int err = rename (tmp_outfile_name, outfile_name);

		if (err) {
			printf ("Unable to rename '%s' to '%s': %s\n", tmp_outfile_name, outfile_name, strerror (errno));
			return 1;
		}
	} else {
		res = compile_asm (acfg);
		if (res != 0) {
			acfg_free (acfg);
			return res;
		}
	}
	TV_GETTIME (btv);
	acfg->stats.link_time = TV_ELAPSED (atv, btv);

	printf ("Compiled %d out of %d methods (%d%%)\n", acfg->stats.ccount, acfg->stats.mcount, acfg->stats.mcount ? (acfg->stats.ccount * 100) / acfg->stats.mcount : 100);
	if (acfg->stats.genericcount)
		printf ("%d methods are generic (%d%%)\n", acfg->stats.genericcount, acfg->stats.mcount ? (acfg->stats.genericcount * 100) / acfg->stats.mcount : 100);
	if (acfg->stats.abscount)
		printf ("%d methods contain absolute addresses (%d%%)\n", acfg->stats.abscount, acfg->stats.mcount ? (acfg->stats.abscount * 100) / acfg->stats.mcount : 100);
	if (acfg->stats.lmfcount)
		printf ("%d methods contain lmf pointers (%d%%)\n", acfg->stats.lmfcount, acfg->stats.mcount ? (acfg->stats.lmfcount * 100) / acfg->stats.mcount : 100);
	if (acfg->stats.ocount)
		printf ("%d methods have other problems (%d%%)\n", acfg->stats.ocount, acfg->stats.mcount ? (acfg->stats.ocount * 100) / acfg->stats.mcount : 100);
	if (acfg->llvm)
		printf ("Methods compiled with LLVM: %d (%d%%)\n", acfg->stats.llvm_count, acfg->stats.mcount ? (acfg->stats.llvm_count * 100) / acfg->stats.mcount : 100);
	printf ("Methods without GOT slots: %d (%d%%)\n", acfg->stats.methods_without_got_slots, acfg->stats.mcount ? (acfg->stats.methods_without_got_slots * 100) / acfg->stats.mcount : 100);
	printf ("Direct calls: %d (%d%%)\n", acfg->stats.direct_calls, acfg->stats.all_calls ? (acfg->stats.direct_calls * 100) / acfg->stats.all_calls : 100);

	if (acfg->aot_opts.stats) {
		int i;

		printf ("GOT slot distribution:\n");
		for (i = 0; i < MONO_PATCH_INFO_NONE; ++i)
			if (acfg->stats.got_slot_types [i])
				printf ("\t%s: %d\n", get_patch_name (i), acfg->stats.got_slot_types [i]);
	}

	printf ("JIT time: %d ms, Generation time: %d ms, Assembly+Link time: %d ms.\n", acfg->stats.jit_time / 1000, acfg->stats.gen_time / 1000, acfg->stats.link_time / 1000);

	acfg_free (acfg);
	
	return 0;
}

#else

/* AOT disabled */

int
mono_compile_assembly (MonoAssembly *ass, guint32 opts, const char *aot_options)
{
	return 0;
}

#endif
