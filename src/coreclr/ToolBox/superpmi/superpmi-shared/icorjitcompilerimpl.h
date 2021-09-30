// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#ifndef _ICorJitCompilerImpl
#define _ICorJitCompilerImpl

// ICorJitCompilerImpl: declare for implementation all the members of the ICorJitCompiler interface (which are
// specified as pure virtual methods). This is done once, here, and all implementations share it,
// to avoid duplicated declarations. This file is #include'd within all the ICorJitCompiler implementation
// classes.
//
// NOTE: this file is in exactly the same order, with exactly the same whitespace, as the ICorJitCompiler
// interface declaration (with the "virtual" and "= 0" syntax removed). This is to make it easy to compare
// against the interface declaration.

public:
// compileMethod is the main routine to ask the JIT Compiler to create native code for a method. The
// method to be compiled is passed in the 'info' parameter, and the code:ICorJitInfo is used to allow the
// JIT to resolve tokens, and make any other callbacks needed to create the code. nativeEntry, and
// nativeSizeOfCode are just for convenience because the JIT asks the EE for the memory to emit code into
// (see code:ICorJitInfo.allocMem), so really the EE already knows where the method starts and how big
// it is (in fact, it could be in more than one chunk).
//
// * In the 32 bit jit this is implemented by code:CILJit.compileMethod
// * For the 64 bit jit this is implemented by code:PreJit.compileMethod
//
// Note: setTargetOS must be called before this api is used.
CorJitResult compileMethod(ICorJitInfo*                comp,     /* IN */
                           struct CORINFO_METHOD_INFO* info,     /* IN */
                           unsigned /* code:CorJitFlag */ flags, /* IN */
                           uint8_t** nativeEntry,                /* OUT */
                           uint32_t* nativeSizeOfCode            /* OUT */
                           );

// Do any appropriate work at process shutdown.  Default impl is to do nothing.
void ProcessShutdownWork(ICorStaticInfo* info); /* {}; */

// The EE asks the JIT for a "version identifier". This represents the version of the JIT/EE interface.
// If the JIT doesn't implement the same JIT/EE interface expected by the EE (because the JIT doesn't
// return the version identifier that the EE expects), then the EE fails to load the JIT.
//
void getVersionIdentifier(GUID* versionIdentifier /* OUT */
                          );

// When the EE loads the System.Numerics.Vectors assembly, it asks the JIT what length (in bytes) of
// SIMD vector it supports as an intrinsic type.  Zero means that the JIT does not support SIMD
// intrinsics, so the EE should use the default size (i.e. the size of the IL implementation).
unsigned getMaxIntrinsicSIMDVectorLength(CORJIT_FLAGS cpuCompileFlags); /* { return 0; } */

// Some JIT's may support multiple OSs. This api provides a means to specify to the JIT what OS it should
// be trying to compile. This api does not produce any errors, any errors are to be generated by the
// the compileMethod call, which will call back into the VM to ensure bits are correctly setup.
//
// Note: this api MUST be called before the compileMethod is called for the first time in the process.
void setTargetOS(CORINFO_OS os);
#endif
