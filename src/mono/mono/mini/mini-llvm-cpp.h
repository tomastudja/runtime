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

G_BEGIN_DECLS

typedef unsigned char * (AllocCodeMemoryCb) (LLVMValueRef function, int size);
typedef void (FunctionEmittedCb) (LLVMValueRef function, void *start, void *end);
typedef void (ExceptionTableCb) (void *data);

LLVMExecutionEngineRef
mono_llvm_create_ee (LLVMModuleProviderRef MP, AllocCodeMemoryCb *alloc_cb, FunctionEmittedCb *emitted_cb, ExceptionTableCb *exception_cb);

G_END_DECLS

#endif /* __MONO_MINI_LLVM_CPP_H__ */  
