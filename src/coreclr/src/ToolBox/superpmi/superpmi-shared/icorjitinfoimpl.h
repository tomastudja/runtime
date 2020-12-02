// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// DO NOT EDIT THIS FILE! IT IS AUTOGENERATED
// To regenerate run the gen script in src/coreclr/src/tools/Common/JitInterface/ThunkGenerator
// and follow the instructions in docs/project/updating-jitinterface.md

#ifndef _ICorJitInfoImpl
#define _ICorJitInfoImpl

// ICorJitInfoImpl: declare for implementation all the members of the ICorJitInfo interface (which are
// specified as pure virtual methods). This is done once, here, and all implementations share it,
// to avoid duplicated declarations. This file is #include'd within all the ICorJitInfo implementation
// classes.
//
// NOTE: this file is in exactly the same order, with exactly the same whitespace, as the ICorJitInfo
// interface declaration (with the "virtual" and "= 0" syntax removed). This is to make it easy to compare
// against the interface declaration.

/**********************************************************************************/
// clang-format off
/**********************************************************************************/

public:

DWORD getMethodAttribs(
          CORINFO_METHOD_HANDLE ftn);

void setMethodAttribs(
          CORINFO_METHOD_HANDLE ftn,
          CorInfoMethodRuntimeFlags attribs);

void getMethodSig(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_SIG_INFO* sig,
          CORINFO_CLASS_HANDLE memberParent);

bool getMethodInfo(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_METHOD_INFO* info);

CorInfoInline canInline(
          CORINFO_METHOD_HANDLE callerHnd,
          CORINFO_METHOD_HANDLE calleeHnd,
          DWORD* pRestrictions);

void reportInliningDecision(
          CORINFO_METHOD_HANDLE inlinerHnd,
          CORINFO_METHOD_HANDLE inlineeHnd,
          CorInfoInline inlineResult,
          const char* reason);

bool canTailCall(
          CORINFO_METHOD_HANDLE callerHnd,
          CORINFO_METHOD_HANDLE declaredCalleeHnd,
          CORINFO_METHOD_HANDLE exactCalleeHnd,
          bool fIsTailPrefix);

void reportTailCallDecision(
          CORINFO_METHOD_HANDLE callerHnd,
          CORINFO_METHOD_HANDLE calleeHnd,
          bool fIsTailPrefix,
          CorInfoTailCall tailCallResult,
          const char* reason);

void getEHinfo(
          CORINFO_METHOD_HANDLE ftn,
          unsigned EHnumber,
          CORINFO_EH_CLAUSE* clause);

CORINFO_CLASS_HANDLE getMethodClass(
          CORINFO_METHOD_HANDLE method);

CORINFO_MODULE_HANDLE getMethodModule(
          CORINFO_METHOD_HANDLE method);

void getMethodVTableOffset(
          CORINFO_METHOD_HANDLE method,
          unsigned* offsetOfIndirection,
          unsigned* offsetAfterIndirection,
          bool* isRelative);

CORINFO_METHOD_HANDLE resolveVirtualMethod(
          CORINFO_METHOD_HANDLE virtualMethod,
          CORINFO_CLASS_HANDLE implementingClass,
          CORINFO_CONTEXT_HANDLE ownerType);

CORINFO_METHOD_HANDLE getUnboxedEntry(
          CORINFO_METHOD_HANDLE ftn,
          bool* requiresInstMethodTableArg);

CORINFO_CLASS_HANDLE getDefaultEqualityComparerClass(
          CORINFO_CLASS_HANDLE elemType);

void expandRawHandleIntrinsic(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_GENERICHANDLE_RESULT* pResult);

CorInfoIntrinsics getIntrinsicID(
          CORINFO_METHOD_HANDLE method,
          bool* pMustExpand);

bool isIntrinsicType(
          CORINFO_CLASS_HANDLE classHnd);

CorInfoUnmanagedCallConv getUnmanagedCallConv(
          CORINFO_METHOD_HANDLE method);

BOOL pInvokeMarshalingRequired(
          CORINFO_METHOD_HANDLE method,
          CORINFO_SIG_INFO* callSiteSig);

BOOL satisfiesMethodConstraints(
          CORINFO_CLASS_HANDLE parent,
          CORINFO_METHOD_HANDLE method);

BOOL isCompatibleDelegate(
          CORINFO_CLASS_HANDLE objCls,
          CORINFO_CLASS_HANDLE methodParentCls,
          CORINFO_METHOD_HANDLE method,
          CORINFO_CLASS_HANDLE delegateCls,
          BOOL* pfIsOpenDelegate);

void methodMustBeLoadedBeforeCodeIsRun(
          CORINFO_METHOD_HANDLE method);

CORINFO_METHOD_HANDLE mapMethodDeclToMethodImpl(
          CORINFO_METHOD_HANDLE method);

void getGSCookie(
          GSCookie* pCookieVal,
          GSCookie** ppCookieVal);

void setPatchpointInfo(
          PatchpointInfo* patchpointInfo);

PatchpointInfo* getOSRInfo(
          unsigned* ilOffset);

void resolveToken(
          CORINFO_RESOLVED_TOKEN* pResolvedToken);

bool tryResolveToken(
          CORINFO_RESOLVED_TOKEN* pResolvedToken);

void findSig(
          CORINFO_MODULE_HANDLE module,
          unsigned sigTOK,
          CORINFO_CONTEXT_HANDLE context,
          CORINFO_SIG_INFO* sig);

void findCallSiteSig(
          CORINFO_MODULE_HANDLE module,
          unsigned methTOK,
          CORINFO_CONTEXT_HANDLE context,
          CORINFO_SIG_INFO* sig);

CORINFO_CLASS_HANDLE getTokenTypeAsHandle(
          CORINFO_RESOLVED_TOKEN* pResolvedToken);

BOOL isValidToken(
          CORINFO_MODULE_HANDLE module,
          unsigned metaTOK);

BOOL isValidStringRef(
          CORINFO_MODULE_HANDLE module,
          unsigned metaTOK);

LPCWSTR getStringLiteral(
          CORINFO_MODULE_HANDLE module,
          unsigned metaTOK,
          int* length);

CorInfoType asCorInfoType(
          CORINFO_CLASS_HANDLE cls);

const char* getClassName(
          CORINFO_CLASS_HANDLE cls);

const char* getClassNameFromMetadata(
          CORINFO_CLASS_HANDLE cls,
          const char** namespaceName);

CORINFO_CLASS_HANDLE getTypeInstantiationArgument(
          CORINFO_CLASS_HANDLE cls,
          unsigned index);

int appendClassName(
          WCHAR** ppBuf,
          int* pnBufLen,
          CORINFO_CLASS_HANDLE cls,
          BOOL fNamespace,
          BOOL fFullInst,
          BOOL fAssembly);

BOOL isValueClass(
          CORINFO_CLASS_HANDLE cls);

CorInfoInlineTypeCheck canInlineTypeCheck(
          CORINFO_CLASS_HANDLE cls,
          CorInfoInlineTypeCheckSource source);

DWORD getClassAttribs(
          CORINFO_CLASS_HANDLE cls);

BOOL isStructRequiringStackAllocRetBuf(
          CORINFO_CLASS_HANDLE cls);

CORINFO_MODULE_HANDLE getClassModule(
          CORINFO_CLASS_HANDLE cls);

CORINFO_ASSEMBLY_HANDLE getModuleAssembly(
          CORINFO_MODULE_HANDLE mod);

const char* getAssemblyName(
          CORINFO_ASSEMBLY_HANDLE assem);

void* LongLifetimeMalloc(
          size_t sz);

void LongLifetimeFree(
          void* obj);

size_t getClassModuleIdForStatics(
          CORINFO_CLASS_HANDLE cls,
          CORINFO_MODULE_HANDLE* pModule,
          void** ppIndirection);

unsigned getClassSize(
          CORINFO_CLASS_HANDLE cls);

unsigned getHeapClassSize(
          CORINFO_CLASS_HANDLE cls);

BOOL canAllocateOnStack(
          CORINFO_CLASS_HANDLE cls);

unsigned getClassAlignmentRequirement(
          CORINFO_CLASS_HANDLE cls,
          BOOL fDoubleAlignHint);

unsigned getClassGClayout(
          CORINFO_CLASS_HANDLE cls,
          BYTE* gcPtrs);

unsigned getClassNumInstanceFields(
          CORINFO_CLASS_HANDLE cls);

CORINFO_FIELD_HANDLE getFieldInClass(
          CORINFO_CLASS_HANDLE clsHnd,
          INT num);

BOOL checkMethodModifier(
          CORINFO_METHOD_HANDLE hMethod,
          LPCSTR modifier,
          BOOL fOptional);

CorInfoHelpFunc getNewHelper(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          bool* pHasSideEffects);

CorInfoHelpFunc getNewArrHelper(
          CORINFO_CLASS_HANDLE arrayCls);

CorInfoHelpFunc getCastingHelper(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          bool fThrowing);

CorInfoHelpFunc getSharedCCtorHelper(
          CORINFO_CLASS_HANDLE clsHnd);

CORINFO_CLASS_HANDLE getTypeForBox(
          CORINFO_CLASS_HANDLE cls);

CorInfoHelpFunc getBoxHelper(
          CORINFO_CLASS_HANDLE cls);

CorInfoHelpFunc getUnBoxHelper(
          CORINFO_CLASS_HANDLE cls);

bool getReadyToRunHelper(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_LOOKUP_KIND* pGenericLookupKind,
          CorInfoHelpFunc id,
          CORINFO_CONST_LOOKUP* pLookup);

void getReadyToRunDelegateCtorHelper(
          CORINFO_RESOLVED_TOKEN* pTargetMethod,
          CORINFO_CLASS_HANDLE delegateType,
          CORINFO_LOOKUP* pLookup);

const char* getHelperName(
          CorInfoHelpFunc helpFunc);

CorInfoInitClassResult initClass(
          CORINFO_FIELD_HANDLE field,
          CORINFO_METHOD_HANDLE method,
          CORINFO_CONTEXT_HANDLE context);

void classMustBeLoadedBeforeCodeIsRun(
          CORINFO_CLASS_HANDLE cls);

CORINFO_CLASS_HANDLE getBuiltinClass(
          CorInfoClassId classId);

CorInfoType getTypeForPrimitiveValueClass(
          CORINFO_CLASS_HANDLE cls);

CorInfoType getTypeForPrimitiveNumericClass(
          CORINFO_CLASS_HANDLE cls);

BOOL canCast(
          CORINFO_CLASS_HANDLE child,
          CORINFO_CLASS_HANDLE parent);

BOOL areTypesEquivalent(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2);

TypeCompareState compareTypesForCast(
          CORINFO_CLASS_HANDLE fromClass,
          CORINFO_CLASS_HANDLE toClass);

TypeCompareState compareTypesForEquality(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2);

CORINFO_CLASS_HANDLE mergeClasses(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2);

BOOL isMoreSpecificType(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2);

CORINFO_CLASS_HANDLE getParentType(
          CORINFO_CLASS_HANDLE cls);

CorInfoType getChildType(
          CORINFO_CLASS_HANDLE clsHnd,
          CORINFO_CLASS_HANDLE* clsRet);

BOOL satisfiesClassConstraints(
          CORINFO_CLASS_HANDLE cls);

BOOL isSDArray(
          CORINFO_CLASS_HANDLE cls);

unsigned getArrayRank(
          CORINFO_CLASS_HANDLE cls);

void* getArrayInitializationData(
          CORINFO_FIELD_HANDLE field,
          DWORD size);

CorInfoIsAccessAllowedResult canAccessClass(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          CORINFO_HELPER_DESC* pAccessHelper);

const char* getFieldName(
          CORINFO_FIELD_HANDLE ftn,
          const char** moduleName);

CORINFO_CLASS_HANDLE getFieldClass(
          CORINFO_FIELD_HANDLE field);

CorInfoType getFieldType(
          CORINFO_FIELD_HANDLE field,
          CORINFO_CLASS_HANDLE* structType,
          CORINFO_CLASS_HANDLE memberParent);

unsigned getFieldOffset(
          CORINFO_FIELD_HANDLE field);

void getFieldInfo(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          CORINFO_ACCESS_FLAGS flags,
          CORINFO_FIELD_INFO* pResult);

bool isFieldStatic(
          CORINFO_FIELD_HANDLE fldHnd);

void getBoundaries(
          CORINFO_METHOD_HANDLE ftn,
          unsigned int* cILOffsets,
          DWORD** pILOffsets,
          ICorDebugInfo::BoundaryTypes* implictBoundaries);

void setBoundaries(
          CORINFO_METHOD_HANDLE ftn,
          ULONG32 cMap,
          ICorDebugInfo::OffsetMapping* pMap);

void getVars(
          CORINFO_METHOD_HANDLE ftn,
          ULONG32* cVars,
          ICorDebugInfo::ILVarInfo** vars,
          bool* extendOthers);

void setVars(
          CORINFO_METHOD_HANDLE ftn,
          ULONG32 cVars,
          ICorDebugInfo::NativeVarInfo* vars);

void* allocateArray(
          size_t cBytes);

void freeArray(
          void* array);

CORINFO_ARG_LIST_HANDLE getArgNext(
          CORINFO_ARG_LIST_HANDLE args);

CorInfoTypeWithMod getArgType(
          CORINFO_SIG_INFO* sig,
          CORINFO_ARG_LIST_HANDLE args,
          CORINFO_CLASS_HANDLE* vcTypeRet);

CORINFO_CLASS_HANDLE getArgClass(
          CORINFO_SIG_INFO* sig,
          CORINFO_ARG_LIST_HANDLE args);

CorInfoHFAElemType getHFAType(
          CORINFO_CLASS_HANDLE hClass);

HRESULT GetErrorHRESULT(
          struct _EXCEPTION_POINTERS* pExceptionPointers);

ULONG GetErrorMessage(
          LPWSTR buffer,
          ULONG bufferLength);

int FilterException(
          struct _EXCEPTION_POINTERS* pExceptionPointers);

void HandleException(
          struct _EXCEPTION_POINTERS* pExceptionPointers);

void ThrowExceptionForJitResult(
          HRESULT result);

void ThrowExceptionForHelper(
          const CORINFO_HELPER_DESC* throwHelper);

bool runWithErrorTrap(
          ICorJitInfo::errorTrapFunction function,
          void* parameter);

void getEEInfo(
          CORINFO_EE_INFO* pEEInfoOut);

LPCWSTR getJitTimeLogFilename();

mdMethodDef getMethodDefFromMethod(
          CORINFO_METHOD_HANDLE hMethod);

const char* getMethodName(
          CORINFO_METHOD_HANDLE ftn,
          const char** moduleName);

const char* getMethodNameFromMetadata(
          CORINFO_METHOD_HANDLE ftn,
          const char** className,
          const char** namespaceName,
          const char** enclosingClassName);

unsigned getMethodHash(
          CORINFO_METHOD_HANDLE ftn);

size_t findNameOfToken(
          CORINFO_MODULE_HANDLE moduleHandle,
          mdToken token,
          char* szFQName,
          size_t FQNameCapacity);

bool getSystemVAmd64PassStructInRegisterDescriptor(
          CORINFO_CLASS_HANDLE structHnd,
          SYSTEMV_AMD64_CORINFO_STRUCT_REG_PASSING_DESCRIPTOR* structPassInRegDescPtr);

DWORD getThreadTLSIndex(
          void** ppIndirection);

const void* getInlinedCallFrameVptr(
          void** ppIndirection);

LONG* getAddrOfCaptureThreadGlobal(
          void** ppIndirection);

void* getHelperFtn(
          CorInfoHelpFunc ftnNum,
          void** ppIndirection);

void getFunctionEntryPoint(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_CONST_LOOKUP* pResult,
          CORINFO_ACCESS_FLAGS accessFlags);

void getFunctionFixedEntryPoint(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_CONST_LOOKUP* pResult);

void* getMethodSync(
          CORINFO_METHOD_HANDLE ftn,
          void** ppIndirection);

CorInfoHelpFunc getLazyStringLiteralHelper(
          CORINFO_MODULE_HANDLE handle);

CORINFO_MODULE_HANDLE embedModuleHandle(
          CORINFO_MODULE_HANDLE handle,
          void** ppIndirection);

CORINFO_CLASS_HANDLE embedClassHandle(
          CORINFO_CLASS_HANDLE handle,
          void** ppIndirection);

CORINFO_METHOD_HANDLE embedMethodHandle(
          CORINFO_METHOD_HANDLE handle,
          void** ppIndirection);

CORINFO_FIELD_HANDLE embedFieldHandle(
          CORINFO_FIELD_HANDLE handle,
          void** ppIndirection);

void embedGenericHandle(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          BOOL fEmbedParent,
          CORINFO_GENERICHANDLE_RESULT* pResult);

void getLocationOfThisType(
          CORINFO_METHOD_HANDLE context,
          CORINFO_LOOKUP_KIND* pLookupKind);

void getAddressOfPInvokeTarget(
          CORINFO_METHOD_HANDLE method,
          CORINFO_CONST_LOOKUP* pLookup);

LPVOID GetCookieForPInvokeCalliSig(
          CORINFO_SIG_INFO* szMetaSig,
          void** ppIndirection);

bool canGetCookieForPInvokeCalliSig(
          CORINFO_SIG_INFO* szMetaSig);

CORINFO_JUST_MY_CODE_HANDLE getJustMyCodeHandle(
          CORINFO_METHOD_HANDLE method,
          CORINFO_JUST_MY_CODE_HANDLE** ppIndirection);

void GetProfilingHandle(
          BOOL* pbHookFunction,
          void** pProfilerHandle,
          BOOL* pbIndirectedHandles);

void getCallInfo(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_RESOLVED_TOKEN* pConstrainedResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          CORINFO_CALLINFO_FLAGS flags,
          CORINFO_CALL_INFO* pResult);

BOOL canAccessFamily(
          CORINFO_METHOD_HANDLE hCaller,
          CORINFO_CLASS_HANDLE hInstanceType);

BOOL isRIDClassDomainID(
          CORINFO_CLASS_HANDLE cls);

unsigned getClassDomainID(
          CORINFO_CLASS_HANDLE cls,
          void** ppIndirection);

void* getFieldAddress(
          CORINFO_FIELD_HANDLE field,
          void** ppIndirection);

CORINFO_CLASS_HANDLE getStaticFieldCurrentClass(
          CORINFO_FIELD_HANDLE field,
          bool* pIsSpeculative);

CORINFO_VARARGS_HANDLE getVarArgsHandle(
          CORINFO_SIG_INFO* pSig,
          void** ppIndirection);

bool canGetVarArgsHandle(
          CORINFO_SIG_INFO* pSig);

InfoAccessType constructStringLiteral(
          CORINFO_MODULE_HANDLE module,
          mdToken metaTok,
          void** ppValue);

InfoAccessType emptyStringLiteral(
          void** ppValue);

DWORD getFieldThreadLocalStoreID(
          CORINFO_FIELD_HANDLE field,
          void** ppIndirection);

void setOverride(
          ICorDynamicInfo* pOverride,
          CORINFO_METHOD_HANDLE currentMethod);

void addActiveDependency(
          CORINFO_MODULE_HANDLE moduleFrom,
          CORINFO_MODULE_HANDLE moduleTo);

CORINFO_METHOD_HANDLE GetDelegateCtor(
          CORINFO_METHOD_HANDLE methHnd,
          CORINFO_CLASS_HANDLE clsHnd,
          CORINFO_METHOD_HANDLE targetMethodHnd,
          DelegateCtorArgs* pCtorData);

void MethodCompileComplete(
          CORINFO_METHOD_HANDLE methHnd);

bool getTailCallHelpers(
          CORINFO_RESOLVED_TOKEN* callToken,
          CORINFO_SIG_INFO* sig,
          CORINFO_GET_TAILCALL_HELPERS_FLAGS flags,
          CORINFO_TAILCALL_HELPERS* pResult);

bool convertPInvokeCalliToCall(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          bool mustConvert);

void notifyInstructionSetUsage(
          CORINFO_InstructionSet instructionSet,
          bool supportEnabled);

void allocMem(
          ULONG hotCodeSize,
          ULONG coldCodeSize,
          ULONG roDataSize,
          ULONG xcptnsCount,
          CorJitAllocMemFlag flag,
          void** hotCodeBlock,
          void** coldCodeBlock,
          void** roDataBlock);

void reserveUnwindInfo(
          BOOL isFunclet,
          BOOL isColdCode,
          ULONG unwindSize);

void allocUnwindInfo(
          BYTE* pHotCode,
          BYTE* pColdCode,
          ULONG startOffset,
          ULONG endOffset,
          ULONG unwindSize,
          BYTE* pUnwindBlock,
          CorJitFuncKind funcKind);

void* allocGCInfo(
          size_t size);

void setEHcount(
          unsigned cEH);

void setEHinfo(
          unsigned EHnumber,
          const CORINFO_EH_CLAUSE* clause);

BOOL logMsg(
          unsigned level,
          const char* fmt,
          va_list args);

int doAssert(
          const char* szFile,
          int iLine,
          const char* szExpr);

void reportFatalError(
          CorJitResult result);

HRESULT allocMethodBlockCounts(
          UINT32 count,
          ICorJitInfo::BlockCounts** pBlockCounts);

HRESULT getMethodBlockCounts(
          CORINFO_METHOD_HANDLE ftnHnd,
          UINT32* pCount,
          ICorJitInfo::BlockCounts** pBlockCounts,
          UINT32* pNumRuns);

CORINFO_CLASS_HANDLE getLikelyClass(
          CORINFO_METHOD_HANDLE ftnHnd,
          CORINFO_CLASS_HANDLE baseHnd,
          UINT32 ilOffset,
          UINT32* pLikelihood,
          UINT32* pNumberOfClasses);

void recordCallSite(
          ULONG instrOffset,
          CORINFO_SIG_INFO* callSig,
          CORINFO_METHOD_HANDLE methodHandle);

void recordRelocation(
          void* location,
          void* target,
          WORD fRelocType,
          WORD slotNum,
          INT32 addlDelta);

WORD getRelocTypeHint(
          void* target);

DWORD getExpectedTargetArchitecture();

DWORD getJitFlags(
          CORJIT_FLAGS* flags,
          DWORD sizeInBytes);

#endif // _ICorJitInfoImpl
/**********************************************************************************/
// clang-format on
/**********************************************************************************/
