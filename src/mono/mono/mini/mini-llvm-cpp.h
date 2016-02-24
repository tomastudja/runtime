/*
 * mini-llvm-cpp.h: LLVM backend
 *
 * Authors:
 *   Zoltan Varga (vargaz@gmail.com)
 *
 * (C) 2009 Novell, Inc.
 */

#ifndef __MONO_MINI_LLVM_CPP_H__
#define __MONO_MINI_LLVM_CPP_H__

#include <glib.h>

#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"

#include <unwind.h>

G_BEGIN_DECLS

/*
 * Keep in sync with the enum in utils/mono-memory-model.h.
 */
typedef enum {
	LLVM_BARRIER_NONE = 0,
	LLVM_BARRIER_ACQ = 1,
	LLVM_BARRIER_REL = 2,
	LLVM_BARRIER_SEQ = 3,
} BarrierKind;

typedef enum {
	LLVM_ATOMICRMW_OP_XCHG = 0,
	LLVM_ATOMICRMW_OP_ADD = 1,
} AtomicRMWOp;

typedef unsigned char * (AllocCodeMemoryCb) (LLVMValueRef function, int size);
typedef void (FunctionEmittedCb) (LLVMValueRef function, void *start, void *end);
typedef void (ExceptionTableCb) (void *data);
typedef char* (DlSymCb) (const char *name, void **symbol);

typedef void* MonoEERef;

MonoEERef
mono_llvm_create_ee (LLVMModuleProviderRef MP, AllocCodeMemoryCb *alloc_cb, FunctionEmittedCb *emitted_cb, ExceptionTableCb *exception_cb, DlSymCb *dlsym_cb, LLVMExecutionEngineRef *ee);

void
mono_llvm_dispose_ee (MonoEERef *mono_ee);

void
mono_llvm_optimize_method (MonoEERef mono_ee, LLVMValueRef method);

void
mono_llvm_dump_value (LLVMValueRef value);

LLVMValueRef
mono_llvm_build_alloca (LLVMBuilderRef builder, LLVMTypeRef Ty, 
						LLVMValueRef ArraySize,
						int alignment, const char *Name);

LLVMValueRef 
mono_llvm_build_load (LLVMBuilderRef builder, LLVMValueRef PointerVal,
					  const char *Name, gboolean is_volatile, BarrierKind barrier);

LLVMValueRef 
mono_llvm_build_aligned_load (LLVMBuilderRef builder, LLVMValueRef PointerVal,
							  const char *Name, gboolean is_volatile, int alignment);

LLVMValueRef 
mono_llvm_build_store (LLVMBuilderRef builder, LLVMValueRef Val, LLVMValueRef PointerVal,
					   gboolean is_volatile, BarrierKind kind);

LLVMValueRef 
mono_llvm_build_aligned_store (LLVMBuilderRef builder, LLVMValueRef Val, LLVMValueRef PointerVal,
							   gboolean is_volatile, int alignment);

LLVMValueRef
mono_llvm_build_atomic_rmw (LLVMBuilderRef builder, AtomicRMWOp op, LLVMValueRef ptr, LLVMValueRef val);

LLVMValueRef
mono_llvm_build_fence (LLVMBuilderRef builder, BarrierKind kind);

void
mono_llvm_replace_uses_of (LLVMValueRef var, LLVMValueRef v);

LLVMValueRef
mono_llvm_build_cmpxchg (LLVMBuilderRef builder, LLVMValueRef addr, LLVMValueRef comparand, LLVMValueRef value);

void
mono_llvm_set_must_tail (LLVMValueRef call_ins);

LLVMValueRef
mono_llvm_create_constant_data_array (const uint8_t *data, int len);

void
mono_llvm_set_is_constant (LLVMValueRef global_var);

void
mono_llvm_set_preserveall_cc (LLVMValueRef func);

void
mono_llvm_set_call_preserveall_cc (LLVMValueRef call);

_Unwind_Reason_Code 
mono_debug_personality (int a, _Unwind_Action b,
	uint64_t c, struct _Unwind_Exception *d, struct _Unwind_Context *e);

void
mono_llvm_set_unhandled_exception_handler (void);

void
default_mono_llvm_unhandled_exception (void);

void*
mono_llvm_create_di_builder (LLVMModuleRef module);

void*
mono_llvm_di_create_function (void *di_builder, void *cu, const char *name, const char *mangled_name, const char *dir, const char *file, int line);

void*
mono_llvm_di_create_compile_unit (void *di_builder, const char *cu_name, const char *dir, const char *producer);

void*
mono_llvm_di_create_file (void *di_builder, const char *dir, const char *file);

void*
mono_llvm_di_create_location (void *di_builder, void *scope, int row, int column);

void
mono_llvm_di_builder_finalize (void *di_builder);

void
mono_llvm_di_set_location (LLVMBuilderRef builder, void *loc_md);

G_END_DECLS

#endif /* __MONO_MINI_LLVM_CPP_H__ */  
