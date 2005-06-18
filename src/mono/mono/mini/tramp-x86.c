/*
 * tramp-x86.c: JIT trampoline code for x86
 *
 * Authors:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * (C) 2001 Ximian, Inc.
 */

#include <config.h>
#include <glib.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/metadata-internals.h>
#include <mono/metadata/marshal.h>
#include <mono/metadata/tabledefs.h>
#include <mono/arch/x86/x86-codegen.h>
#include <mono/metadata/mono-debug-debugger.h>

#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
#endif

#include "mini.h"
#include "mini-x86.h"

/*
 * get_unbox_trampoline:
 * @m: method pointer
 * @addr: pointer to native code for @m
 *
 * when value type methods are called through the vtable we need to unbox the
 * this argument. This method returns a pointer to a trampoline which does
 * unboxing before calling the method
 */
static gpointer
get_unbox_trampoline (MonoMethod *m, gpointer addr)
{
	guint8 *code, *start;
	int this_pos = 4;
	MonoDomain *domain = mono_domain_get ();

	if (!mono_method_signature (m)->ret->byref && MONO_TYPE_ISSTRUCT (mono_method_signature (m)->ret))
		this_pos = 8;
	    
	mono_domain_lock (domain);
	start = code = mono_code_manager_reserve (domain->code_mp, 16);
	mono_domain_unlock (domain);

	x86_alu_membase_imm (code, X86_ADD, X86_ESP, this_pos, sizeof (MonoObject));
	x86_jump_code (code, addr);
	g_assert ((code - start) < 16);

	return start;
}

/**
 * x86_magic_trampoline:
 * @eax: saved x86 register 
 * @ecx: saved x86 register 
 * @edx: saved x86 register 
 * @esi: saved x86 register 
 * @edi: saved x86 register 
 * @ebx: saved x86 register
 * @code: pointer into caller code
 * @method: the method to translate
 *
 * This method is called by the trampoline functions for virtual
 * methods. It inspects the caller code to find the address of the
 * vtable slot, then calls the JIT compiler and writes the address
 * of the compiled method back to the vtable. All virtual methods 
 * are called with: x86_call_membase (inst, basereg, disp). We always
 * use 32 bit displacement to ensure that the length of the call 
 * instruction is 6 bytes. We need to get the value of the basereg 
 * and the constant displacement.
 */
static gpointer
x86_magic_trampoline (int eax, int ecx, int edx, int esi, int edi, 
		      int ebx, guint8 *code, MonoMethod *m)
{
	gpointer addr;
	gpointer *vtable_slot;
	int regs [X86_NREG];

	addr = mono_compile_method (m);
	g_assert (addr);

	/* the method was jumped to */
	if (!code)
		return addr;

	regs [X86_EAX] = eax;
	regs [X86_ECX] = ecx;
	regs [X86_EDX] = edx;
	regs [X86_ESI] = esi;
	regs [X86_EDI] = edi;
	regs [X86_EBX] = ebx;

	vtable_slot = mono_arch_get_vcall_slot_addr (code, (gpointer*)regs);
	if (!vtable_slot) {
		/* go to the start of the call instruction
		 *
		 * address_byte = (m << 6) | (o << 3) | reg
		 * call opcode: 0xff address_byte displacement
		 * 0xff m=1,o=2 imm8
		 * 0xff m=2,o=2 imm32
		 */
		code -= 6;
		if ((code [1] == 0xe8)) {
			if (!mono_running_on_valgrind ()) {
				MonoJitInfo *ji = 
					mono_jit_info_table_find (mono_domain_get (), (char*)code);
				MonoJitInfo *target_ji = 
					mono_jit_info_table_find (mono_domain_get (), addr);

				if (mono_method_same_domain (ji, target_ji)) {
					InterlockedExchange ((gint32*)(code + 2), (guint)addr - ((guint)code + 1) - 5);

#ifdef HAVE_VALGRIND_MEMCHECK_H
					/* Tell valgrind to recompile the patched code */
					//VALGRIND_DISCARD_TRANSLATIONS (code + 2, code + 6);
#endif
				}
			}
			return addr;
		} else {
			printf ("Invalid trampoline sequence: %x %x %x %x %x %x %x\n", code [0], code [1], code [2], code [3],
				code [4], code [5], code [6]);
			g_assert_not_reached ();
		}
	}

	if (m->klass->valuetype && !mono_aot_is_got_entry (code, (guint8*)vtable_slot))
		addr = get_unbox_trampoline (m, addr);

	if (mono_aot_is_got_entry (code, (guint8*)vtable_slot) || mono_domain_owns_vtable_slot (mono_domain_get (), vtable_slot))
		*vtable_slot = addr;

	return addr;
}

/*
 * x86_aot_trampoline:
 *
 *   This trampoline handles calls made from AOT code. We try to bypass the 
 * normal JIT compilation logic to avoid loading the metadata for the method.
 */
static gpointer
x86_aot_trampoline (int eax, int ecx, int edx, int esi, int edi, 
					int ebx, guint8 *code, guint8 *token_info)
{
	MonoImage *image;
	guint32 token;
	MonoMethod *method;
	gpointer addr;
	gpointer *vtable_slot;
	int regs [X86_NREG];

	image = *(gpointer*)token_info;
	token_info += sizeof (gpointer);
	token = *(guint32*)token_info;

	/* Later we could avoid allocating the MonoMethod */
	method = mono_get_method (image, token, NULL);
	g_assert (method);

	if (method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED)
		method = mono_marshal_get_synchronized_wrapper (method);

	addr = mono_compile_method (method);
	g_assert (addr);

	regs [X86_EAX] = eax;
	regs [X86_ECX] = ecx;
	regs [X86_EDX] = edx;
	regs [X86_ESI] = esi;
	regs [X86_EDI] = edi;
	regs [X86_EBX] = ebx;

	vtable_slot = mono_arch_get_vcall_slot_addr (code, (gpointer*)regs);
	g_assert (vtable_slot);

	if (method->klass->valuetype)
		addr = get_unbox_trampoline (method, addr);

	if (mono_domain_owns_vtable_slot (mono_domain_get (), vtable_slot))
		*vtable_slot = addr;

	return addr;
}	

/**
 * x86_class_init_trampoline:
 * @eax: saved x86 register 
 * @ecx: saved x86 register 
 * @edx: saved x86 register 
 * @esi: saved x86 register 
 * @edi: saved x86 register 
 * @ebx: saved x86 register
 * @code: pointer into caller code
 * @vtable: the type to initialize
 *
 * This method calls mono_runtime_class_init () to run the static constructor
 * for the type, then patches the caller code so it is not called again.
 */
static void
x86_class_init_trampoline (int eax, int ecx, int edx, int esi, int edi, 
						   int ebx, guint8 *code, MonoVTable *vtable)
{
	mono_runtime_class_init (vtable);

	code -= 5;
	if (code [0] == 0xe8) {
		if (!mono_running_on_valgrind ()) {
			guint32 ops;
			/*
			 * Thread safe code patching using the algorithm from the paper
			 * 'Practicing JUDO: Java Under Dynamic Optimizations'
			 */
			/* 
			 * First atomically change the the first 2 bytes of the call to a
			 * spinning jump.
			 */
			ops = 0xfeeb;
			InterlockedExchange ((gint32*)code, ops);

			/* Then change the other bytes to a nop */
			code [2] = 0x90;
			code [3] = 0x90;
			code [4] = 0x90;

			/* Then atomically change the first 4 bytes to a nop as well */
			ops = 0x90909090;
			InterlockedExchange ((gint32*)code, ops);

#ifdef HAVE_VALGRIND_MEMCHECK_H
			/* FIXME: the calltree skin trips on the self modifying code above */

			/* Tell valgrind to recompile the patched code */
			//VALGRIND_DISCARD_TRANSLATIONS (code, code + 8);
#endif
		}
	} else if (code [0] == 0x90 || code [0] == 0xeb) {
		/* Already changed by another thread */
		;
	} else if ((code [-1] == 0xff) && (x86_modrm_reg (code [0]) == 0x2)) {
		/* call *<OFFSET>(<REG>) -> Call made from AOT code */
		/* FIXME: Patch up the trampoline */
		;
	} else {
			printf ("Invalid trampoline sequence: %x %x %x %x %x %x %x\n", code [0], code [1], code [2], code [3],
				code [4], code [5], code [6]);
			g_assert_not_reached ();
		}
}

guchar*
mono_arch_create_trampoline_code (MonoTrampolineType tramp_type)
{
	guint8 *buf, *code;

	code = buf = mono_global_codeman_reserve (256);

	/* save caller save regs because we need to do a call */ 
	x86_push_reg (buf, X86_EDX);
	x86_push_reg (buf, X86_EAX);
	x86_push_reg (buf, X86_ECX);

	/* save LMF begin */

	/* save the IP (caller ip) */
	if (tramp_type == MONO_TRAMPOLINE_JUMP)
		x86_push_imm (buf, 0);
	else
		x86_push_membase (buf, X86_ESP, 16);

	x86_push_reg (buf, X86_EBP);
	x86_push_reg (buf, X86_ESI);
	x86_push_reg (buf, X86_EDI);
	x86_push_reg (buf, X86_EBX);

	/* save method info */
	x86_push_membase (buf, X86_ESP, 32);
	/* get the address of lmf for the current thread */
	x86_call_code (buf, mono_get_lmf_addr);
	/* push lmf */
	x86_push_reg (buf, X86_EAX); 
	/* push *lfm (previous_lmf) */
	x86_push_membase (buf, X86_EAX, 0);
	/* *(lmf) = ESP */
	x86_mov_membase_reg (buf, X86_EAX, 0, X86_ESP, 4);
	/* save LFM end */

	/* push the method info */
	x86_push_membase (buf, X86_ESP, 44);
	/* push the return address onto the stack */
	if (tramp_type == MONO_TRAMPOLINE_JUMP)
		x86_push_imm (buf, 0);
	else
		x86_push_membase (buf, X86_ESP, 52);

	/* save all register values */
	x86_push_reg (buf, X86_EBX);
	x86_push_reg (buf, X86_EDI);
	x86_push_reg (buf, X86_ESI);
	x86_push_membase (buf, X86_ESP, 64); /* EDX */
	x86_push_membase (buf, X86_ESP, 64); /* ECX */
	x86_push_membase (buf, X86_ESP, 64); /* EAX */

	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		x86_call_code (buf, x86_class_init_trampoline);
	else if (tramp_type == MONO_TRAMPOLINE_AOT)
		x86_call_code (buf, x86_aot_trampoline);
	else
		x86_call_code (buf, x86_magic_trampoline);
	x86_alu_reg_imm (buf, X86_ADD, X86_ESP, 8*4);

	/* restore LMF start */
	/* ebx = previous_lmf */
	x86_pop_reg (buf, X86_EBX);
	/* edi = lmf */
	x86_pop_reg (buf, X86_EDI);
	/* *(lmf) = previous_lmf */
	x86_mov_membase_reg (buf, X86_EDI, 0, X86_EBX, 4);
	/* discard method info */
	x86_pop_reg (buf, X86_ESI);
	/* restore caller saved regs */
	x86_pop_reg (buf, X86_EBX);
	x86_pop_reg (buf, X86_EDI);
	x86_pop_reg (buf, X86_ESI);
	x86_pop_reg (buf, X86_EBP);

	/* discard save IP */
	x86_alu_reg_imm (buf, X86_ADD, X86_ESP, 4);		
	/* restore LMF end */

	x86_alu_reg_imm (buf, X86_ADD, X86_ESP, 16);

	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		x86_ret (buf);
	else
		/* call the compiled method */
		x86_jump_reg (buf, X86_EAX);

	g_assert ((buf - code) <= 256);

	return code;
}

#define TRAMPOLINE_SIZE 10

static gpointer
create_specific_trampoline (gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain, guint32 *code_len)
{
	guint8 *code, *buf, *tramp;
	
	tramp = mono_get_trampoline_code (tramp_type);

	mono_domain_lock (domain);
	code = buf = mono_code_manager_reserve (domain->code_mp, TRAMPOLINE_SIZE);
	mono_domain_unlock (domain);

	x86_push_imm (buf, arg1);
	x86_jump_code (buf, tramp);
	g_assert ((buf - code) <= TRAMPOLINE_SIZE);

	mono_arch_flush_icache (code, buf - code);

	mono_jit_stats.method_trampolines++;

	if (code_len)
		*code_len = buf - code;

	return code;
}

MonoJitInfo*
mono_arch_create_jump_trampoline (MonoMethod *method)
{
	MonoJitInfo *ji;
	gpointer code;
	guint32 code_size;

	code = create_specific_trampoline (method, MONO_TRAMPOLINE_JUMP, mono_domain_get (), &code_size);

	ji = g_new0 (MonoJitInfo, 1);
	ji->code_start = code;
	ji->code_size = code_size;
	ji->method = method;

	return ji;
}

/**
 * mono_arch_create_jit_trampoline:
 * @method: pointer to the method info
 *
 * Creates a trampoline function for virtual methods. If the created
 * code is called it first starts JIT compilation of method,
 * and then calls the newly created method. I also replaces the
 * corresponding vtable entry (see x86_magic_trampoline).
 * 
 * Returns: a pointer to the newly created code 
 */
gpointer
mono_arch_create_jit_trampoline (MonoMethod *method)
{
	return create_specific_trampoline (method, MONO_TRAMPOLINE_GENERIC, mono_domain_get (), NULL);
}

gpointer
mono_arch_create_jit_trampoline_from_token (MonoImage *image, guint32 token)
{
	MonoDomain *domain = mono_domain_get ();
	guint8 *buf, *start;

	mono_domain_lock (domain);
	buf = start = mono_code_manager_reserve (domain->code_mp, 2 * sizeof (gpointer));
	mono_domain_unlock (domain);

	*(gpointer*)buf = image;
	buf += sizeof (gpointer);
	*(guint32*)buf = token;

	return create_specific_trampoline (start, MONO_TRAMPOLINE_AOT, domain, NULL);
}

/**
 * mono_arch_create_class_init_trampoline:
 *  @vtable: the type to initialize
 *
 * Creates a trampoline function to run a type initializer. 
 * If the trampoline is called, it calls mono_runtime_class_init with the
 * given vtable, then patches the caller code so it does not get called any
 * more.
 * 
 * Returns: a pointer to the newly created code 
 */
gpointer
mono_arch_create_class_init_trampoline (MonoVTable *vtable)
{
	return create_specific_trampoline (vtable, MONO_TRAMPOLINE_CLASS_INIT, vtable->domain, NULL);
}

void
mono_arch_invalidate_method (MonoJitInfo *ji, void *func, gpointer func_arg)
{
	/* FIXME: This is not thread safe */
	guint8 *code = ji->code_start;

	x86_push_imm (code, func_arg);
	x86_call_code (code, (guint8*)func);
}

/*
 * This method is only called when running in the Mono Debugger.
 */
gpointer
mono_debugger_create_notification_function (gpointer *notification_address)
{
	guint8 *ptr, *buf;

	ptr = buf = mono_global_codeman_reserve (16);

	x86_breakpoint (buf);
	if (notification_address)
		*notification_address = buf;
	x86_ret (buf);

	return ptr;
}
