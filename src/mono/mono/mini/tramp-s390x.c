/*------------------------------------------------------------------*/
/* 								    */
/* Name        - tramp-s390.c      			  	    */
/* 								    */
/* Function    - JIT trampoline code for S/390.                     */
/* 								    */
/* Name	       - Neale Ferguson (Neale.Ferguson@SoftwareAG-usa.com) */
/* 								    */
/* Date        - January, 2004					    */
/* 								    */
/* Derivation  - From exceptions-x86 & exceptions-ppc		    */
/* 	         Paolo Molaro (lupus@ximian.com) 		    */
/* 		 Dietmar Maurer (dietmar@ximian.com)		    */
/* 								    */
/* Copyright   - 2001 Ximian, Inc.				    */
/* 								    */
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*                 D e f i n e s                                    */
/*------------------------------------------------------------------*/

#define GR_SAVE_SIZE		4*sizeof(long)
#define FP_SAVE_SIZE		16*sizeof(double)
#define METHOD_SAVE_OFFSET	S390_MINIMAL_STACK_SIZE
#define CREATE_GR_OFFSET	METHOD_SAVE_OFFSET+8
#define CREATE_FP_OFFSET	CREATE_GR_OFFSET+GR_SAVE_SIZE
#define CREATE_LMF_OFFSET	CREATE_FP_OFFSET+FP_SAVE_SIZE
#define CREATE_STACK_SIZE	(CREATE_LMF_OFFSET+2*sizeof(long)+sizeof(MonoLMF))

/*------------------------------------------------------------------*/
/* Method-specific trampoline code fragment sizes		    */
/*------------------------------------------------------------------*/
#define METHOD_TRAMPOLINE_SIZE	96
#define JUMP_TRAMPOLINE_SIZE	96

/*========================= End of Defines =========================*/

/*------------------------------------------------------------------*/
/*                 I n c l u d e s                                  */
/*------------------------------------------------------------------*/

#include <config.h>
#include <glib.h>
#include <string.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/marshal.h>
#include <mono/metadata/tabledefs.h>
#include <mono/arch/s390x/s390x-codegen.h>

#include "mini.h"
#include "mini-s390x.h"

/*========================= End of Includes ========================*/

/*------------------------------------------------------------------*/
/*                 T y p e d e f s                                  */
/*------------------------------------------------------------------*/

/*========================= End of Typedefs ========================*/

/*------------------------------------------------------------------*/
/*                   P r o t o t y p e s                            */
/*------------------------------------------------------------------*/

/*========================= End of Prototypes ======================*/

/*------------------------------------------------------------------*/
/*                 G l o b a l   V a r i a b l e s                  */
/*------------------------------------------------------------------*/


/*====================== End of Global Variables ===================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- get_unbox_trampoline                              */
/*                                                                  */
/* Function	- Return a pointer to a trampoline which does the   */
/*		  unboxing before calling the method.		    */
/*                                                                  */
/*                When value type methods are called through the    */
/*		  vtable we need to unbox the 'this' argument.	    */
/*		                               		 	    */
/* Parameters   - method - Methd pointer			    */
/*		  addr   - Pointer to native code for method	    */
/*		                               		 	    */
/*------------------------------------------------------------------*/

static gpointer
get_unbox_trampoline (MonoMethod *method, gpointer addr)
{
	guint8 *code, *start;
	int this_pos = s390_r2;

	start = addr;
	if ((!mono_method_signature (method)->ret->byref) && 
	    (MONO_TYPE_ISSTRUCT (mono_method_signature (method)->ret)))
		this_pos = s390_r3;
    
	start = code = mono_global_codeman_reserve (28);

	s390_basr (code, s390_r1, 0);
	s390_j	  (code, 6);
	s390_llong(code, addr);
	s390_lg   (code, s390_r1, 0, s390_r1, 4);
	s390_aghi (code, this_pos, sizeof(MonoObject));
	s390_br   (code, s390_r1);

	g_assert ((code - start) <= 28);

	return start;
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- s390_magic_trampoline                             */
/*                                                                  */
/* Function	- This method is called by the function             */
/*                "arch_create_jit_trampoline", which in turn is    */
/*                called by the trampoline functions for virtual    */
/*                methods. After having called the JIT compiler to  */
/*                compile the method, it inspects the caller code   */
/*                to find the address of the method-specific part   */
/*                of the trampoline vtable slot for this method,    */
/*                updates it with a fragment that calls the newly   */
/*                compiled code and returns this address. The calls */
/*                generated by mono for S/390 will look like either:*/
/*                1. l     %r1,xxx(%rx)                             */
/*                   bras  %r14,%r1                                 */
/*                2. brasl %r14,xxxxxx                              */
/*                                                                  */
/* Parameters   - code   - Pointer into caller code                 */
/*                method - The method to compile                    */
/*                sp     - Stack pointer                            */
/*                                                                  */
/*------------------------------------------------------------------*/

static gpointer
s390_magic_trampoline (MonoMethod *method, guchar *code, char *sp)
{
	gpointer addr;
	gint64 displace;
	guchar reg, lkReg;
	guchar *base;
	unsigned short opcode;
	MonoJitInfo *codeJi, 
		    *addrJi;

	addr = mono_compile_method(method);
	g_assert(addr);

	if (code) {

		codeJi = mono_jit_info_table_find (mono_domain_get(), code);
		addrJi = mono_jit_info_table_find (mono_domain_get(), addr);
		if (mono_method_same_domain (codeJi, addrJi)) {

			opcode = *((unsigned short *) (code - 6));
			if (opcode == 0xc0e5) {
				/* This is the 'brasl' instruction */
				code    -= 4;
				displace = ((gint64) addr - (gint64) (code - 2)) / 2;
				if (mono_method_same_domain (codeJi, addrJi)) {
					s390_patch_rel (code, displace);
					mono_arch_flush_icache (code, 4);
				}
			} else {
				/*-----------------------------------*/
				/* This is a bras r14,Rz instruction */
				/* If it's preceded by a LG Rx,d(Ry) */
				/* If Rz == 1 then we check if unbox-*/
				/* is required. We patch the address */
				/* by determining the location desc- */
				/* cribed by *Ry+d.                  */ 
				/*-----------------------------------*/
				code    -= 6;

				/*-----------------------------------*/
				/* If call is preceded by LGR then   */
				/* there's nothing to patch          */
				/*-----------------------------------*/
				if ((code[0] == 0xb9) &&
				    (code[1] == 0x04))
					return addr;

				/*-----------------------------------*/
				/* We back up until we're pointing at*/
				/* the base/displacement portion of  */
				/* the LG instruction                */
				/*-----------------------------------*/
				lkReg    = code[5] & 0x0f;

				/*-----------------------------------*/
				/* The LG instruction has format:    */
				/* E3x0ylllhh04 - where:             */
				/* x = Rx; y = Ry;                   */
				/* lll = low 12 bits of displacement */
				/* hh  = high 8 bits of displacement */
				/*-----------------------------------*/
				reg      = code[0] >> 4;
				displace = (code[2] << 12) +
					   ((code[0] & 0x0f) << 8) + 
					   code[1];

				if (reg > 5) 
					base = *((guchar **) (sp + S390_REG_SAVE_OFFSET +
							       sizeof(long)*(reg-6)));
				else
					base = *((guchar **) ((sp - CREATE_STACK_SIZE) + 
							       CREATE_GR_OFFSET +
							       sizeof(long)*(reg-2)));

				/* Calls that need unboxing use R1 */
				if (lkReg == 1)  {
					if ((method->klass->valuetype) && 
					    (!mono_aot_is_got_entry(code, base)))
						addr = get_unbox_trampoline(method, addr);

					code = base + displace;
					if (mono_domain_owns_vtable_slot(mono_domain_get(), 
									 code))
						s390_patch_addr(code, addr);
				} else {
					code = base + displace;
					s390_patch_addr(code, addr);
				}
			}
		}
	}

	return addr;
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- s390_class_init_trampoline                        */
/*                                                                  */
/* Function	- Initialize a class and then no-op the call to     */
/*                the trampoline.                                   */
/*                                                                  */
/*------------------------------------------------------------------*/

static void
s390_class_init_trampoline (void *vtable, guchar *code, char *sp)
{
	char patch[2] = {0x07, 0x00};

	mono_runtime_class_init (vtable);

	code = code - 2;

	memcpy(code, patch, sizeof(patch));
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- mono_arch_create_trampoline_code                            */
/*                                                                  */
/* Function	- Create the designated type of trampoline according*/
/*                to the 'tramp_type' parameter.                    */
/*                                                                  */
/*------------------------------------------------------------------*/

guchar *
mono_arch_create_trampoline_code (MonoTrampolineType tramp_type)
{

	guint8 *buf, *code = NULL;
	int i, offset, lmfOffset;

	if(!code) {
		/* Now we'll create in 'buf' the S/390 trampoline code. This
		 is the trampoline code common to all methods  */
		
		code = buf = mono_global_codeman_reserve(512);
		
		/*-----------------------------------------------------------
		STEP 0: First create a non-standard function prologue with a
		stack size big enough to save our registers.
		-----------------------------------------------------------*/
		
		s390_stmg (buf, s390_r6, s390_r14, STK_BASE, S390_REG_SAVE_OFFSET);
		s390_lgr  (buf, s390_r11, s390_r15);
		s390_aghi (buf, STK_BASE, -CREATE_STACK_SIZE);
		s390_stg  (buf, s390_r11, 0, STK_BASE, 0);
		s390_stg  (buf, s390_r1, 0, STK_BASE, METHOD_SAVE_OFFSET);
		s390_stmg (buf, s390_r2, s390_r5, STK_BASE, CREATE_GR_OFFSET);

		/* Save the FP registers */
		offset = CREATE_FP_OFFSET;
		for (i = s390_f0; i <= s390_f15; ++i) {
			s390_std  (buf, i, 0, STK_BASE, offset);
			offset += 8;
		}

		/*----------------------------------------------------------
		STEP 1: call 'mono_get_lmf_addr()' to get the address of our
		LMF. We'll need to restore it after the call to
		's390_magic_trampoline' and before the call to the native
		method.
		----------------------------------------------------------*/
				
		s390_basr (buf, s390_r13, 0);
		s390_j	  (buf, 6);
		s390_llong(buf, mono_get_lmf_addr);
		s390_lg   (buf, s390_r1, 0, s390_r13, 4);
		s390_basr (buf, s390_r14, s390_r1);

		/*---------------------------------------------------------------*/
		/* we build the MonoLMF structure on the stack - see mini-s390.h */
		/* Keep in sync with the code in mono_arch_emit_prolog 		 */
		/*---------------------------------------------------------------*/
		lmfOffset = CREATE_STACK_SIZE - sizeof(MonoLMF);
											
		s390_lgr   (buf, s390_r13, STK_BASE);
		s390_aghi  (buf, s390_r13, lmfOffset);	
											
		/*---------------------------------------------------------------*/	
		/* Set lmf.lmf_addr = jit_tls->lmf				 */	
		/*---------------------------------------------------------------*/	
		s390_stg   (buf, s390_r2, 0, s390_r13, 				
			    G_STRUCT_OFFSET(MonoLMF, lmf_addr));			
											
		/*---------------------------------------------------------------*/	
		/* Get current lmf						 */	
		/*---------------------------------------------------------------*/	
		s390_lg    (buf, s390_r0, 0, s390_r2, 0);				
											
		/*---------------------------------------------------------------*/	
		/* Set our lmf as the current lmf				 */	
		/*---------------------------------------------------------------*/	
		s390_stg   (buf, s390_r13, 0, s390_r2, 0);				
											
		/*---------------------------------------------------------------*/	
		/* Have our lmf.previous_lmf point to the last lmf		 */	
		/*---------------------------------------------------------------*/	
		s390_stg   (buf, s390_r0, 0, s390_r13, 				
			    G_STRUCT_OFFSET(MonoLMF, previous_lmf));			
											
		/*---------------------------------------------------------------*/	
		/* save method info						 */	
		/*---------------------------------------------------------------*/	
		s390_lg    (buf, s390_r1, 0, STK_BASE, METHOD_SAVE_OFFSET);
		s390_stg   (buf, s390_r1, 0, s390_r13, 				
			    G_STRUCT_OFFSET(MonoLMF, method));				
									
		/*---------------------------------------------------------------*/	
		/* save the current SP						 */	
		/*---------------------------------------------------------------*/	
		s390_lg    (buf, s390_r1, 0, STK_BASE, 0);
		s390_stg   (buf, s390_r1, 0, s390_r13, G_STRUCT_OFFSET(MonoLMF, ebp));	
									
		/*---------------------------------------------------------------*/	
		/* save the current IP						 */	
		/*---------------------------------------------------------------*/	
		if (tramp_type == MONO_TRAMPOLINE_JUMP) {
			s390_lghi  (buf, s390_r1, 0);
		} else {
			s390_lg    (buf, s390_r1, 0, s390_r1, S390_RET_ADDR_OFFSET);
//			s390_la    (buf, s390_r1, 0, s390_r1, 0);
		}
		s390_stg   (buf, s390_r1, 0, s390_r13, G_STRUCT_OFFSET(MonoLMF, eip));	
											
		/*---------------------------------------------------------------*/	
		/* Save general and floating point registers			 */	
		/*---------------------------------------------------------------*/	
		s390_stmg  (buf, s390_r2, s390_r12, s390_r13, 				
			    G_STRUCT_OFFSET(MonoLMF, gregs[2]));			
		for (i = 0; i < 16; i++) {						
			s390_std  (buf, i, 0, s390_r13, 				
				   G_STRUCT_OFFSET(MonoLMF, fregs[i]));			
		}									

		/*---------------------------------------------------------------*/
		/* STEP 2: call 's390_magic_trampoline()', who will compile the  */
		/* code and fix the method vtable entry for us			 */
		/*---------------------------------------------------------------*/
				
		/* Set arguments */
		
		/* Arg 1: MonoMethod *method. It was put in r1 by the
		method-specific trampoline code, and then saved before the call
		to mono_get_lmf_addr()'. */
		s390_lg (buf, s390_r2, 0, STK_BASE, METHOD_SAVE_OFFSET);
		
		/* Arg 2: code (next address to the instruction that called us) */
		if (tramp_type == MONO_TRAMPOLINE_JUMP) {
			s390_lghi (buf, s390_r3, 0);
		} else {
			s390_lg   (buf, s390_r3, 0, s390_r11, S390_RET_ADDR_OFFSET);
		}
		
		/* Arg 3: stack pointer */
		s390_lgr  (buf, s390_r4, STK_BASE);
		s390_ahi  (buf, s390_r4, CREATE_STACK_SIZE);
		
		/* Calculate call address and call
		's390_magic_trampoline'. Return value will be in r2 */
		s390_basr (buf, s390_r13, 0);
		s390_j	  (buf, 6);
		if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT) {
			s390_llong(buf, s390_class_init_trampoline);
		} else {
			s390_llong(buf, s390_magic_trampoline);
		}
		s390_lg   (buf, s390_r1, 0, s390_r13, 4);
		s390_basr (buf, s390_r14, s390_r1);
		
		/* OK, code address is now on r2. Move it to r1, so that we
		can restore r2 and use it from r1 later */
		s390_lgr  (buf, s390_r1, s390_r2);

		/*----------------------------------------------------------
		STEP 3: Restore the LMF
		----------------------------------------------------------*/
		restoreLMF(buf, STK_BASE, CREATE_STACK_SIZE);
	
		/*----------------------------------------------------------
		STEP 4: call the compiled method
		----------------------------------------------------------*/
		
		/* Restore registers */

		s390_lmg  (buf, s390_r2, s390_r5, STK_BASE, CREATE_GR_OFFSET);
		
		/* Restore the FP registers */
		offset = CREATE_FP_OFFSET;
		for (i = s390_f0; i <= s390_f15; ++i) {
			s390_ld  (buf, i, 0, STK_BASE, offset);
			offset += 8;
		}

		/* Restore stack pointer and jump to the code - 
		   R14 contains the return address to our caller */
		s390_lgr  (buf, STK_BASE, s390_r11);
		s390_lmg  (buf, s390_r6, s390_r14, STK_BASE, S390_REG_SAVE_OFFSET);
		s390_br   (buf, s390_r1);

		/* Flush instruction cache, since we've generated code */
		mono_arch_flush_icache (code, buf - code);
	
		/* Sanity check */
		g_assert ((buf - code) <= 512);
	}

	return code;
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- mono_arch_create_jump_trampoline                  */
/*                                                                  */
/* Function	- Create the designated type of trampoline according*/
/*                to the 'tramp_type' parameter.                    */
/*                                                                  */
/*------------------------------------------------------------------*/

MonoJitInfo *
mono_arch_create_jump_trampoline (MonoMethod *method)
{
	guint8 *code, *buf, *tramp = NULL;
	MonoJitInfo *ji;
	MonoDomain *domain = mono_domain_get();
	gint32 displace;

	tramp = mono_get_trampoline_code (MONO_TRAMPOLINE_JUMP);

	mono_domain_lock (domain);
	code = buf = mono_code_manager_reserve (domain->code_mp, JUMP_TRAMPOLINE_SIZE);
	mono_domain_unlock (domain);

	s390_basr (buf, s390_r1, 0);
	s390_j	  (buf, 6);
	s390_llong(buf, method);
	s390_lg   (buf, s390_r1, 0, s390_r1, 4);
	displace = (tramp - buf) / 2;
	s390_jcl  (buf, S390_CC_UN, displace);

	mono_arch_flush_icache (code, buf-code);

	g_assert ((buf - code) <= JUMP_TRAMPOLINE_SIZE);
	
	ji 		= g_new0 (MonoJitInfo, 1);
	ji->method 	= method;
	ji->code_start 	= code;
	ji->code_size  	= buf - code;
	
	mono_jit_stats.method_trampolines++;

	return ji;
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- mono_arch_create_jit_trampoline                   */
/*                                                                  */
/* Function	- Creates a trampoline function for virtual methods.*/
/*                If the created code is called it first starts JIT */
/*                compilation and then calls the newly created      */
/*                method. It also replaces the corresponding vtable */
/*                entry (see s390_magic_trampoline).                */
/*                                                                  */
/*                A trampoline consists of two parts: a main        */
/*                fragment, shared by all method trampolines, and   */
/*                and some code specific to each method, which      */
/*                hard-codes a reference to that method and then    */
/*                calls the main fragment.                          */
/*                                                                  */
/*                The main fragment contains a call to              */
/*                's390_magic_trampoline', which performs a call    */
/*                to the JIT compiler and substitutes the method-   */
/*                specific fragment with some code that directly    */
/*                calls the JIT-compiled method.                    */
/*                                                                  */
/* Parameter    - method - Pointer to the method information        */
/*                                                                  */
/* Returns      - A pointer to the newly created code               */
/*                                                                  */
/*------------------------------------------------------------------*/

gpointer
mono_arch_create_jit_trampoline (MonoMethod *method)
{
	guint8 *code, *buf;
	static guint8 *vc = NULL;
	gint32 displace;

	vc = mono_get_trampoline_code (MONO_TRAMPOLINE_GENERIC);

	/* This is the method-specific part of the trampoline. Its purpose is
	to provide the generic part with the MonoMethod *method pointer. We'll
	use r1 to keep that value, for instance. */
	code = buf = mono_global_codeman_reserve(METHOD_TRAMPOLINE_SIZE);

	s390_basr (buf, s390_r1, 0);
	s390_j	  (buf, 6);
	s390_llong(buf, method);
	s390_lg   (buf, s390_r1, 0, s390_r1, 4);
	displace = (vc - buf) / 2;
	s390_jcl  (buf, S390_CC_UN, displace);

	/* Flush instruction cache, since we've generated code */
	mono_arch_flush_icache (code, buf - code);
		
	/* Sanity check */
	g_assert ((buf - code) <= METHOD_TRAMPOLINE_SIZE);
	
	return code;
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- mono_arch_create_class_init_trampoline            */
/*                                                                  */
/* Function	- Creates a trampoline function to run a type init- */
/*                ializer. If the trampoline is called, it calls    */
/*                mono_runtime_class_init with the given vtable,    */
/*                then patches the caller code so it does not get   */
/*                called any more.                                  */
/*                                                                  */
/* Parameter    - vtable - The type to initialize                   */
/*                                                                  */
/* Returns      - A pointer to the newly created code               */
/*                                                                  */
/*------------------------------------------------------------------*/

gpointer
mono_arch_create_class_init_trampoline (MonoVTable *vtable)
{
	guint8 *code, *buf, *tramp;

	tramp = mono_get_trampoline_code (MONO_TRAMPOLINE_CLASS_INIT);

	/*-----------------------------------------------------------*/
	/* This is the method-specific part of the trampoline. Its   */
	/* purpose is to provide the generic part with the MonoMethod*/
	/* *method pointer.                                          */
	/*-----------------------------------------------------------*/
	code = buf = mono_global_codeman_reserve(METHOD_TRAMPOLINE_SIZE);

	s390_stg  (buf, s390_r14, 0, STK_BASE, S390_RET_ADDR_OFFSET);
	s390_aghi (buf, STK_BASE, -S390_MINIMAL_STACK_SIZE);

	s390_basr (buf, s390_r1, 0);
	s390_j	  (buf, 10);
	s390_llong(buf, vtable);
        s390_llong(buf, s390_class_init_trampoline);
	s390_lgr  (buf, s390_r3, s390_r14);
	s390_lg   (buf, s390_r2, 0, s390_r1, 4);
	s390_lghi (buf, s390_r4, 0);
	s390_lg   (buf, s390_r1, 0, s390_r1, 12);
	s390_basr (buf, s390_r14, s390_r1);

	s390_aghi (buf, STK_BASE, S390_MINIMAL_STACK_SIZE);
	s390_lg   (buf, s390_r14, 0, STK_BASE, S390_RET_ADDR_OFFSET);
	s390_br   (buf, s390_r14);

	/* Flush instruction cache, since we've generated code */
	mono_arch_flush_icache (code, buf - code);
		
	/* Sanity check */
	g_assert ((buf - code) <= METHOD_TRAMPOLINE_SIZE);

	mono_jit_stats.method_trampolines++;

	return code;
}

/*========================= End of Function ========================*/

/*------------------------------------------------------------------*/
/*                                                                  */
/* Name		- mono_debuger_create_notification_function	    */
/*                                                                  */
/* Function	- This method is only called when running in the    */
/*                Mono debugger. It returns a pointer to the        */
/*                arch specific notification function.              */
/*                                                                  */
/*------------------------------------------------------------------*/

gpointer
mono_debugger_create_notification_function (void)
{
	guint8 *ptr, *buf;

	ptr = buf = mono_global_codeman_reserve (16);
	s390_break (buf);
	s390_br (buf, s390_r14);

	return ptr;
}

/*========================= End of Function ========================*/
