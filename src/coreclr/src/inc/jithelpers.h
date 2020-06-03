// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//
// Allow multiple inclusion.

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
//
// The JIT/EE interface is versioned. By "interface", we mean mean any and all communication between the
// JIT and the EE. Any time a change is made to the interface, the JIT/EE interface version identifier
// must be updated. See code:JITEEVersionIdentifier for more information.
//
// THIS FILE IS PART OF THE JIT-EE INTERFACE.
//
// NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef DYNAMICJITHELPER
//I should never try to generate an alignment stub for a dynamic helper
#define DYNAMICJITHELPER(code,fn,sig) JITHELPER(code,fn,sig)
#endif


// pfnHelper is set to NULL if it is a stubbed helper.
// It will be set in InitJITHelpers2

    JITHELPER(CORINFO_HELP_UNDEF,               NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)

    // Arithmetic
    JITHELPER(CORINFO_HELP_DIV,                 JIT_Div,            CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_MOD,                 JIT_Mod,            CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_UDIV,                JIT_UDiv,           CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_UMOD,                JIT_UMod,           CORINFO_HELP_SIG_8_STACK)

    // CORINFO_HELP_DBL2INT, CORINFO_HELP_DBL2UINT, and CORINFO_HELP_DBL2LONG get
    // patched for CPUs that support SSE2 (P4 and above).
#ifndef TARGET_64BIT
    JITHELPER(CORINFO_HELP_LLSH,                JIT_LLsh,           CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_LRSH,                JIT_LRsh,           CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_LRSZ,                JIT_LRsz,           CORINFO_HELP_SIG_REG_ONLY)
#else // !TARGET_64BIT
    JITHELPER(CORINFO_HELP_LLSH,                NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_LRSH,                NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_LRSZ,                NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#endif // TARGET_64BIT
    JITHELPER(CORINFO_HELP_LMUL,                JIT_LMul,           CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_LMUL_OVF,            JIT_LMulOvf,        CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_ULMUL_OVF,           JIT_ULMulOvf,       CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_LDIV,                JIT_LDiv,           CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_LMOD,                JIT_LMod,           CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_ULDIV,               JIT_ULDiv,          CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_ULMOD,               JIT_ULMod,          CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_LNG2DBL,             JIT_Lng2Dbl,        CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_ULNG2DBL,            JIT_ULng2Dbl,       CORINFO_HELP_SIG_8_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_DBL2INT,      JIT_Dbl2Lng,        CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBL2INT_OVF,         JIT_Dbl2IntOvf,     CORINFO_HELP_SIG_8_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_DBL2LNG,      JIT_Dbl2Lng,        CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBL2LNG_OVF,         JIT_Dbl2LngOvf,     CORINFO_HELP_SIG_8_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_DBL2UINT,     JIT_Dbl2Lng,        CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBL2UINT_OVF,        JIT_Dbl2UIntOvf,    CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBL2ULNG,            JIT_Dbl2ULng,       CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBL2ULNG_OVF,        JIT_Dbl2ULngOvf,    CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_FLTREM,              JIT_FltRem,         CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBLREM,              JIT_DblRem,         CORINFO_HELP_SIG_16_STACK)
    JITHELPER(CORINFO_HELP_FLTROUND,            JIT_FloatRound,     CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_DBLROUND,            JIT_DoubleRound,    CORINFO_HELP_SIG_16_STACK)

    // Allocating a new object
    JITHELPER(CORINFO_HELP_NEWFAST,                     JIT_New,    CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_NEWSFAST,             JIT_New,    CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_NEWSFAST_FINALIZE,           NULL,       CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_NEWSFAST_ALIGN8,      JIT_New,    CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_NEWSFAST_ALIGN8_VC,          NULL,       CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_NEWSFAST_ALIGN8_FINALIZE,    NULL,       CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_NEW_MDARR,                   JIT_NewMDArr,CORINFO_HELP_SIG_8_VA)
    JITHELPER(CORINFO_HELP_NEW_MDARR_NONVARARG,         JIT_NewMDArrNonVarArg,CORINFO_HELP_SIG_4_STACK)
    JITHELPER(CORINFO_HELP_NEWARR_1_DIRECT,             JIT_NewArr1,CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_NEWARR_1_OBJ,         JIT_NewArr1,CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_NEWARR_1_VC,          JIT_NewArr1,CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_NEWARR_1_ALIGN8,      JIT_NewArr1,CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_STRCNS,              JIT_StrCns,         CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_STRCNS_CURRENT_MODULE, NULL,             CORINFO_HELP_SIG_REG_ONLY)

    // Object model
    JITHELPER(CORINFO_HELP_INITCLASS,           JIT_InitClass,      CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_INITINSTCLASS,       JIT_InitInstantiatedClass, CORINFO_HELP_SIG_REG_ONLY)

    // Casting helpers
    DYNAMICJITHELPER(CORINFO_HELP_ISINSTANCEOFINTERFACE,    NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_ISINSTANCEOFARRAY,        NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_ISINSTANCEOFCLASS,        NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_ISINSTANCEOFANY,          NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_CHKCASTINTERFACE,         NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_CHKCASTARRAY,             NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_CHKCASTCLASS,             NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_CHKCASTANY,               NULL,   CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_CHKCASTCLASS_SPECIAL,     NULL,   CORINFO_HELP_SIG_REG_ONLY)

    DYNAMICJITHELPER(CORINFO_HELP_BOX,          JIT_Box,            CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_BOX_NULLABLE,        JIT_Box,            CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_UNBOX,        NULL,               CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_UNBOX_NULLABLE,      JIT_Unbox_Nullable, CORINFO_HELP_SIG_4_STACK)

    JITHELPER(CORINFO_HELP_GETREFANY,           JIT_GetRefAny,      CORINFO_HELP_SIG_8_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_ARRADDR_ST,   NULL,               CORINFO_HELP_SIG_4_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_LDELEMA_REF,  NULL,               CORINFO_HELP_SIG_4_STACK)

    // Exceptions
    JITHELPER(CORINFO_HELP_THROW,               IL_Throw,           CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_RETHROW,             IL_Rethrow,         CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_USER_BREAKPOINT,     JIT_UserBreakpoint, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_RNGCHKFAIL,          JIT_RngChkFail,     CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_OVERFLOW,            JIT_Overflow,       CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_THROWDIVZERO,        JIT_ThrowDivZero,   CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_THROWNULLREF,        JIT_ThrowNullRef,   CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_INTERNALTHROW,       JIT_InternalThrow,  CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_VERIFICATION,        IL_VerificationError,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SEC_UNMGDCODE_EXCPT, JIT_SecurityUnmanagedCodeException, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_FAIL_FAST,           JIT_FailFast,       CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_METHOD_ACCESS_EXCEPTION,JIT_ThrowMethodAccessException, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_FIELD_ACCESS_EXCEPTION,JIT_ThrowFieldAccessException, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_CLASS_ACCESS_EXCEPTION,JIT_ThrowClassAccessException, CORINFO_HELP_SIG_REG_ONLY)

#ifdef FEATURE_EH_FUNCLETS
    JITHELPER(CORINFO_HELP_ENDCATCH,            NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_ENDCATCH,            JIT_EndCatch,       CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#endif

    JITHELPER(CORINFO_HELP_MON_ENTER,               JIT_MonEnterWorker, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_MON_EXIT,                JIT_MonExitWorker, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_MON_ENTER_STATIC,        JIT_MonEnterStatic,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_MON_EXIT_STATIC,         JIT_MonExitStatic,CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_GETCLASSFROMMETHODPARAM, JIT_GetClassFromMethodParam, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSYNCFROMCLASSHANDLE,  JIT_GetSyncFromClassHandle, CORINFO_HELP_SIG_REG_ONLY)

    // GC support
    DYNAMICJITHELPER(CORINFO_HELP_STOP_FOR_GC,  JIT_RareDisableHelper,  CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_POLL_GC,             JIT_PollGC,         CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_STRESS_GC,           JIT_StressGC,       CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_CHECK_OBJ,           JIT_CheckObj,       CORINFO_HELP_SIG_REG_ONLY)

    // GC Write barrier support
    DYNAMICJITHELPER(CORINFO_HELP_ASSIGN_REF,   JIT_WriteBarrier,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    DYNAMICJITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF, JIT_CheckedWriteBarrier,CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_ENSURE_NONHEAP, JIT_WriteBarrierEnsureNonHeapTarget,CORINFO_HELP_SIG_REG_ONLY)

    DYNAMICJITHELPER(CORINFO_HELP_ASSIGN_BYREF, JIT_ByRefWriteBarrier,CORINFO_HELP_SIG_NO_ALIGN_STUB)

    JITHELPER(CORINFO_HELP_ASSIGN_STRUCT,       JIT_StructWriteBarrier,CORINFO_HELP_SIG_4_STACK)

    // Accessing fields
    JITHELPER(CORINFO_HELP_GETFIELD8,                   JIT_GetField8,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELD8,                   JIT_SetField8,CORINFO_HELP_SIG_4_STACK)
    JITHELPER(CORINFO_HELP_GETFIELD16,                  JIT_GetField16,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELD16,                  JIT_SetField16,CORINFO_HELP_SIG_4_STACK)
    JITHELPER(CORINFO_HELP_GETFIELD32,                  JIT_GetField32,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELD32,                  JIT_SetField32,CORINFO_HELP_SIG_4_STACK)
    JITHELPER(CORINFO_HELP_GETFIELD64,                  JIT_GetField64,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELD64,                  JIT_SetField64,CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_GETFIELDOBJ,                 JIT_GetFieldObj,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELDOBJ,                 JIT_SetFieldObj,CORINFO_HELP_SIG_4_STACK)
    JITHELPER(CORINFO_HELP_GETFIELDSTRUCT,              JIT_GetFieldStruct,CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_SETFIELDSTRUCT,              JIT_SetFieldStruct,CORINFO_HELP_SIG_8_STACK)
    JITHELPER(CORINFO_HELP_GETFIELDFLOAT,               JIT_GetFieldFloat,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELDFLOAT,               JIT_SetFieldFloat,CORINFO_HELP_SIG_4_STACK)
    JITHELPER(CORINFO_HELP_GETFIELDDOUBLE,              JIT_GetFieldDouble,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_SETFIELDDOUBLE,              JIT_SetFieldDouble,CORINFO_HELP_SIG_8_STACK)

    JITHELPER(CORINFO_HELP_GETFIELDADDR,                JIT_GetFieldAddr,CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_GETSTATICFIELDADDR_CONTEXT,  NULL,       CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_GETSTATICFIELDADDR_TLS,      NULL,       CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)

    JITHELPER(CORINFO_HELP_GETGENERICS_GCSTATIC_BASE,   JIT_GetGenericsGCStaticBase,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETGENERICS_NONGCSTATIC_BASE, JIT_GetGenericsNonGCStaticBase,CORINFO_HELP_SIG_REG_ONLY)

#ifdef TARGET_X86
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_GCSTATIC_BASE,          NULL, CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE,       NULL, CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_GCSTATIC_BASE_NOCTOR,   NULL, CORINFO_HELP_SIG_REG_ONLY)
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE_NOCTOR,NULL, CORINFO_HELP_SIG_REG_ONLY)
#else
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_GCSTATIC_BASE,          JIT_GetSharedGCStaticBase, CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE,       JIT_GetSharedNonGCStaticBase, CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_GCSTATIC_BASE_NOCTOR,   JIT_GetSharedGCStaticBaseNoCtor, CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    DYNAMICJITHELPER(CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE_NOCTOR,JIT_GetSharedNonGCStaticBaseNoCtor, CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#endif
    JITHELPER(CORINFO_HELP_GETSHARED_GCSTATIC_BASE_DYNAMICCLASS,    JIT_GetSharedGCStaticBaseDynamicClass,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_NONGCSTATIC_BASE_DYNAMICCLASS, JIT_GetSharedNonGCStaticBaseDynamicClass,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_CLASSINIT_SHARED_DYNAMICCLASS,           JIT_ClassInitDynamicClass,CORINFO_HELP_SIG_REG_ONLY)

    // Thread statics
    JITHELPER(CORINFO_HELP_GETGENERICS_GCTHREADSTATIC_BASE,   JIT_GetGenericsGCThreadStaticBase,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETGENERICS_NONGCTHREADSTATIC_BASE, JIT_GetGenericsNonGCThreadStaticBase,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_GCTHREADSTATIC_BASE,                 JIT_GetSharedGCThreadStaticBase, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_NONGCTHREADSTATIC_BASE,              JIT_GetSharedNonGCThreadStaticBase, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_GCTHREADSTATIC_BASE_NOCTOR,          JIT_GetSharedGCThreadStaticBase, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_NONGCTHREADSTATIC_BASE_NOCTOR,       JIT_GetSharedNonGCThreadStaticBase, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_GCTHREADSTATIC_BASE_DYNAMICCLASS,    JIT_GetSharedGCThreadStaticBaseDynamicClass, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_GETSHARED_NONGCTHREADSTATIC_BASE_DYNAMICCLASS, JIT_GetSharedNonGCThreadStaticBaseDynamicClass, CORINFO_HELP_SIG_REG_ONLY)

    // Debugger
    JITHELPER(CORINFO_HELP_DBG_IS_JUST_MY_CODE, JIT_DbgIsJustMyCode,CORINFO_HELP_SIG_REG_ONLY)

    /* Profiling enter/leave probe addresses */
    DYNAMICJITHELPER(CORINFO_HELP_PROF_FCN_ENTER,    JIT_ProfilerEnterLeaveTailcallStub, CORINFO_HELP_SIG_4_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_PROF_FCN_LEAVE,    JIT_ProfilerEnterLeaveTailcallStub, CORINFO_HELP_SIG_4_STACK)
    DYNAMICJITHELPER(CORINFO_HELP_PROF_FCN_TAILCALL, JIT_ProfilerEnterLeaveTailcallStub, CORINFO_HELP_SIG_4_STACK)

    // Miscellaneous
    JITHELPER(CORINFO_HELP_BBT_FCN_ENTER,       JIT_LogMethodEnter,CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_PINVOKE_CALLI,       GenericPInvokeCalliHelper, CORINFO_HELP_SIG_NO_ALIGN_STUB)

#if defined(TARGET_X86) && !defined(UNIX_X86_ABI)
    JITHELPER(CORINFO_HELP_TAILCALL,            JIT_TailCall,             CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_TAILCALL,            NULL,                     CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#endif

    JITHELPER(CORINFO_HELP_GETCURRENTMANAGEDTHREADID,  JIT_GetCurrentManagedThreadId, CORINFO_HELP_SIG_REG_ONLY)

#ifdef TARGET_64BIT
    JITHELPER(CORINFO_HELP_INIT_PINVOKE_FRAME,  JIT_InitPInvokeFrame,  CORINFO_HELP_SIG_REG_ONLY)
#else
    DYNAMICJITHELPER(CORINFO_HELP_INIT_PINVOKE_FRAME,  NULL,        CORINFO_HELP_SIG_REG_ONLY)
#endif

#ifdef TARGET_X86
    JITHELPER(CORINFO_HELP_MEMSET,              NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_MEMCPY,              NULL,               CORINFO_HELP_SIG_CANNOT_USE_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_MEMSET,              JIT_MemSet,         CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_MEMCPY,              JIT_MemCpy,         CORINFO_HELP_SIG_REG_ONLY)
#endif

    // Generics
    JITHELPER(CORINFO_HELP_RUNTIMEHANDLE_METHOD,    JIT_GenericHandleMethod,        CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_RUNTIMEHANDLE_METHOD_LOG,JIT_GenericHandleMethodLogging, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_RUNTIMEHANDLE_CLASS,     JIT_GenericHandleClass,         CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_RUNTIMEHANDLE_CLASS_LOG, JIT_GenericHandleClassLogging,  CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_TYPEHANDLE_TO_RUNTIMETYPE, JIT_GetRuntimeType,           CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_TYPEHANDLE_TO_RUNTIMETYPE_MAYBENULL, JIT_GetRuntimeType_MaybeNull, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_METHODDESC_TO_STUBRUNTIMEMETHOD, JIT_GetRuntimeMethodStub,CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_FIELDDESC_TO_STUBRUNTIMEFIELD, JIT_GetRuntimeFieldStub,  CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_TYPEHANDLE_TO_RUNTIMETYPEHANDLE, JIT_GetRuntimeType, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_TYPEHANDLE_TO_RUNTIMETYPEHANDLE_MAYBENULL, JIT_GetRuntimeType_MaybeNull, CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_ARE_TYPES_EQUIVALENT, NULL, CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_VIRTUAL_FUNC_PTR,    JIT_VirtualFunctionPointer, CORINFO_HELP_SIG_4_STACK)
    //JITHELPER(CORINFO_HELP_VIRTUAL_FUNC_PTR_LOG,JIT_VirtualFunctionPointerLogging)

    JITHELPER(CORINFO_HELP_READYTORUN_NEW,                 NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_NEWARR_1,            NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_ISINSTANCEOF,        NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_CHKCAST,             NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_STATIC_BASE,         NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_VIRTUAL_FUNC_PTR,    NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_GENERIC_HANDLE,      NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_DELEGATE_CTOR,       NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_READYTORUN_GENERIC_STATIC_BASE, NULL,   CORINFO_HELP_SIG_NO_ALIGN_STUB)

    JITHELPER(CORINFO_HELP_EE_PRESTUB,          ThePreStub,                 CORINFO_HELP_SIG_NO_ALIGN_STUB)

#if defined(HAS_FIXUP_PRECODE)
    JITHELPER(CORINFO_HELP_EE_PRECODE_FIXUP,    PrecodeFixupThunk,          CORINFO_HELP_SIG_NO_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_EE_PRECODE_FIXUP,    NULL,                       CORINFO_HELP_SIG_NO_ALIGN_STUB)
#endif

    JITHELPER(CORINFO_HELP_EE_PINVOKE_FIXUP,    NDirectImportThunk,         CORINFO_HELP_SIG_NO_ALIGN_STUB)

#ifdef FEATURE_PREJIT
    JITHELPER(CORINFO_HELP_EE_VSD_FIXUP,        StubDispatchFixupStub,      CORINFO_HELP_SIG_NO_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_EE_VSD_FIXUP,        NULL,                       CORINFO_HELP_SIG_NO_ALIGN_STUB)
#endif
    JITHELPER(CORINFO_HELP_EE_EXTERNAL_FIXUP,   ExternalMethodFixupStub,    CORINFO_HELP_SIG_NO_ALIGN_STUB)
#ifdef FEATURE_PREJIT
    JITHELPER(CORINFO_HELP_EE_VTABLE_FIXUP,     VirtualMethodFixupStub,     CORINFO_HELP_SIG_NO_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_EE_VTABLE_FIXUP,     NULL,                       CORINFO_HELP_SIG_NO_ALIGN_STUB)
#endif

    JITHELPER(CORINFO_HELP_EE_REMOTING_THUNK,   NULL,                       CORINFO_HELP_SIG_UNDEF)

// We do not need this to be saved in ngen images on Mac64 since the exception dispatch
// is not done via the OS and thus, there wont be any need to know this information
// by anyone.
#ifdef FEATURE_EH_FUNCLETS
    JITHELPER(CORINFO_HELP_EE_PERSONALITY_ROUTINE, ProcessCLRException,               CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_EE_PERSONALITY_ROUTINE_FILTER_FUNCLET, ProcessCLRException,CORINFO_HELP_SIG_UNDEF)
#else // FEATURE_EH_FUNCLETS
    JITHELPER(CORINFO_HELP_EE_PERSONALITY_ROUTINE, NULL,                              CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_EE_PERSONALITY_ROUTINE_FILTER_FUNCLET, NULL,               CORINFO_HELP_SIG_UNDEF)
#endif // !FEATURE_EH_FUNCLETS

#ifdef TARGET_X86
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EAX, JIT_WriteBarrierEAX, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EBX, JIT_WriteBarrierEBX, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_ECX, JIT_WriteBarrierECX, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_ESI, JIT_WriteBarrierESI, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EDI, JIT_WriteBarrierEDI, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EBP, JIT_WriteBarrierEBP, CORINFO_HELP_SIG_NO_ALIGN_STUB)

    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EAX, JIT_CheckedWriteBarrierEAX, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EBX, JIT_CheckedWriteBarrierEBX, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_ECX, JIT_CheckedWriteBarrierECX, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_ESI, JIT_CheckedWriteBarrierESI, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EDI, JIT_CheckedWriteBarrierEDI, CORINFO_HELP_SIG_NO_ALIGN_STUB)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EBP, JIT_CheckedWriteBarrierEBP, CORINFO_HELP_SIG_NO_ALIGN_STUB)
#else
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EAX, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EBX, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_ECX, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_ESI, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EDI, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_ASSIGN_REF_EBP, NULL, CORINFO_HELP_SIG_UNDEF)

    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EAX, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EBX, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_ECX, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_ESI, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EDI, NULL, CORINFO_HELP_SIG_UNDEF)
    JITHELPER(CORINFO_HELP_CHECKED_ASSIGN_REF_EBP, NULL, CORINFO_HELP_SIG_UNDEF)
#endif

    JITHELPER(CORINFO_HELP_LOOP_CLONE_CHOICE_ADDR, JIT_LoopCloneChoiceAddr, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_DEBUG_LOG_LOOP_CLONING, JIT_DebugLogLoopCloning, CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_THROW_ARGUMENTEXCEPTION,           JIT_ThrowArgumentException,             CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_THROW_ARGUMENTOUTOFRANGEEXCEPTION, JIT_ThrowArgumentOutOfRangeException,   CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_THROW_NOT_IMPLEMENTED,             JIT_ThrowNotImplementedException,       CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_THROW_PLATFORM_NOT_SUPPORTED,      JIT_ThrowPlatformNotSupportedException, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_THROW_TYPE_NOT_SUPPORTED,          JIT_ThrowTypeNotSupportedException,     CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_JIT_PINVOKE_BEGIN,         JIT_PInvokeBegin,     CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_JIT_PINVOKE_END,           JIT_PInvokeEnd,       CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_JIT_REVERSE_PINVOKE_ENTER, JIT_ReversePInvokeEnter, CORINFO_HELP_SIG_REG_ONLY)
    JITHELPER(CORINFO_HELP_JIT_REVERSE_PINVOKE_EXIT,  JIT_ReversePInvokeExit, CORINFO_HELP_SIG_REG_ONLY)

    JITHELPER(CORINFO_HELP_GVMLOOKUP_FOR_SLOT, NULL, CORINFO_HELP_SIG_NO_ALIGN_STUB)

#ifndef TARGET_ARM64
    JITHELPER(CORINFO_HELP_STACK_PROBE, JIT_StackProbe, CORINFO_HELP_SIG_REG_ONLY)
#else
    JITHELPER(CORINFO_HELP_STACK_PROBE, NULL, CORINFO_HELP_SIG_UNDEF)
#endif

    JITHELPER(CORINFO_HELP_PATCHPOINT, JIT_Patchpoint, CORINFO_HELP_SIG_REG_ONLY)

#undef JITHELPER
#undef DYNAMICJITHELPER
#undef JITHELPER
#undef DYNAMICJITHELPER
