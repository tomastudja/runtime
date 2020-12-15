// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// DO NOT EDIT THIS FILE! IT IS AUTOGENERATED
// To regenerate run the gen script in src/coreclr/tools/Common/JitInterface/ThunkGenerator
// and follow the instructions in docs/project/updating-jitinterface.md

#include "standardpch.h"
#include "icorjitinfo.h"
#include "superpmi-shim-simple.h"
#include "icorjitcompiler.h"
#include "spmiutil.h"


DWORD interceptor_ICJI::getMethodAttribs(
          CORINFO_METHOD_HANDLE ftn)
{
    return original_ICorJitInfo->getMethodAttribs(ftn);
}

void interceptor_ICJI::setMethodAttribs(
          CORINFO_METHOD_HANDLE ftn,
          CorInfoMethodRuntimeFlags attribs)
{
    original_ICorJitInfo->setMethodAttribs(ftn, attribs);
}

void interceptor_ICJI::getMethodSig(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_SIG_INFO* sig,
          CORINFO_CLASS_HANDLE memberParent)
{
    original_ICorJitInfo->getMethodSig(ftn, sig, memberParent);
}

bool interceptor_ICJI::getMethodInfo(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_METHOD_INFO* info)
{
    return original_ICorJitInfo->getMethodInfo(ftn, info);
}

CorInfoInline interceptor_ICJI::canInline(
          CORINFO_METHOD_HANDLE callerHnd,
          CORINFO_METHOD_HANDLE calleeHnd,
          DWORD* pRestrictions)
{
    return original_ICorJitInfo->canInline(callerHnd, calleeHnd, pRestrictions);
}

void interceptor_ICJI::reportInliningDecision(
          CORINFO_METHOD_HANDLE inlinerHnd,
          CORINFO_METHOD_HANDLE inlineeHnd,
          CorInfoInline inlineResult,
          const char* reason)
{
    original_ICorJitInfo->reportInliningDecision(inlinerHnd, inlineeHnd, inlineResult, reason);
}

bool interceptor_ICJI::canTailCall(
          CORINFO_METHOD_HANDLE callerHnd,
          CORINFO_METHOD_HANDLE declaredCalleeHnd,
          CORINFO_METHOD_HANDLE exactCalleeHnd,
          bool fIsTailPrefix)
{
    return original_ICorJitInfo->canTailCall(callerHnd, declaredCalleeHnd, exactCalleeHnd, fIsTailPrefix);
}

void interceptor_ICJI::reportTailCallDecision(
          CORINFO_METHOD_HANDLE callerHnd,
          CORINFO_METHOD_HANDLE calleeHnd,
          bool fIsTailPrefix,
          CorInfoTailCall tailCallResult,
          const char* reason)
{
    original_ICorJitInfo->reportTailCallDecision(callerHnd, calleeHnd, fIsTailPrefix, tailCallResult, reason);
}

void interceptor_ICJI::getEHinfo(
          CORINFO_METHOD_HANDLE ftn,
          unsigned EHnumber,
          CORINFO_EH_CLAUSE* clause)
{
    original_ICorJitInfo->getEHinfo(ftn, EHnumber, clause);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getMethodClass(
          CORINFO_METHOD_HANDLE method)
{
    return original_ICorJitInfo->getMethodClass(method);
}

CORINFO_MODULE_HANDLE interceptor_ICJI::getMethodModule(
          CORINFO_METHOD_HANDLE method)
{
    return original_ICorJitInfo->getMethodModule(method);
}

void interceptor_ICJI::getMethodVTableOffset(
          CORINFO_METHOD_HANDLE method,
          unsigned* offsetOfIndirection,
          unsigned* offsetAfterIndirection,
          bool* isRelative)
{
    original_ICorJitInfo->getMethodVTableOffset(method, offsetOfIndirection, offsetAfterIndirection, isRelative);
}

bool interceptor_ICJI::resolveVirtualMethod(
          CORINFO_DEVIRTUALIZATION_INFO* info)
{
    return original_ICorJitInfo->resolveVirtualMethod(info);
}

CORINFO_METHOD_HANDLE interceptor_ICJI::getUnboxedEntry(
          CORINFO_METHOD_HANDLE ftn,
          bool* requiresInstMethodTableArg)
{
    return original_ICorJitInfo->getUnboxedEntry(ftn, requiresInstMethodTableArg);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getDefaultEqualityComparerClass(
          CORINFO_CLASS_HANDLE elemType)
{
    return original_ICorJitInfo->getDefaultEqualityComparerClass(elemType);
}

void interceptor_ICJI::expandRawHandleIntrinsic(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_GENERICHANDLE_RESULT* pResult)
{
    original_ICorJitInfo->expandRawHandleIntrinsic(pResolvedToken, pResult);
}

CorInfoIntrinsics interceptor_ICJI::getIntrinsicID(
          CORINFO_METHOD_HANDLE method,
          bool* pMustExpand)
{
    return original_ICorJitInfo->getIntrinsicID(method, pMustExpand);
}

bool interceptor_ICJI::isIntrinsicType(
          CORINFO_CLASS_HANDLE classHnd)
{
    return original_ICorJitInfo->isIntrinsicType(classHnd);
}

CorInfoCallConvExtension interceptor_ICJI::getUnmanagedCallConv(
          CORINFO_METHOD_HANDLE method,
          CORINFO_SIG_INFO* callSiteSig,
          bool* pSuppressGCTransition)
{
    return original_ICorJitInfo->getUnmanagedCallConv(method, callSiteSig, pSuppressGCTransition);
}

bool interceptor_ICJI::pInvokeMarshalingRequired(
          CORINFO_METHOD_HANDLE method,
          CORINFO_SIG_INFO* callSiteSig)
{
    return original_ICorJitInfo->pInvokeMarshalingRequired(method, callSiteSig);
}

bool interceptor_ICJI::satisfiesMethodConstraints(
          CORINFO_CLASS_HANDLE parent,
          CORINFO_METHOD_HANDLE method)
{
    return original_ICorJitInfo->satisfiesMethodConstraints(parent, method);
}

bool interceptor_ICJI::isCompatibleDelegate(
          CORINFO_CLASS_HANDLE objCls,
          CORINFO_CLASS_HANDLE methodParentCls,
          CORINFO_METHOD_HANDLE method,
          CORINFO_CLASS_HANDLE delegateCls,
          bool* pfIsOpenDelegate)
{
    return original_ICorJitInfo->isCompatibleDelegate(objCls, methodParentCls, method, delegateCls, pfIsOpenDelegate);
}

void interceptor_ICJI::methodMustBeLoadedBeforeCodeIsRun(
          CORINFO_METHOD_HANDLE method)
{
    original_ICorJitInfo->methodMustBeLoadedBeforeCodeIsRun(method);
}

CORINFO_METHOD_HANDLE interceptor_ICJI::mapMethodDeclToMethodImpl(
          CORINFO_METHOD_HANDLE method)
{
    return original_ICorJitInfo->mapMethodDeclToMethodImpl(method);
}

void interceptor_ICJI::getGSCookie(
          GSCookie* pCookieVal,
          GSCookie** ppCookieVal)
{
    original_ICorJitInfo->getGSCookie(pCookieVal, ppCookieVal);
}

void interceptor_ICJI::setPatchpointInfo(
          PatchpointInfo* patchpointInfo)
{
    original_ICorJitInfo->setPatchpointInfo(patchpointInfo);
}

PatchpointInfo* interceptor_ICJI::getOSRInfo(
          unsigned* ilOffset)
{
    return original_ICorJitInfo->getOSRInfo(ilOffset);
}

void interceptor_ICJI::resolveToken(
          CORINFO_RESOLVED_TOKEN* pResolvedToken)
{
    original_ICorJitInfo->resolveToken(pResolvedToken);
}

bool interceptor_ICJI::tryResolveToken(
          CORINFO_RESOLVED_TOKEN* pResolvedToken)
{
    return original_ICorJitInfo->tryResolveToken(pResolvedToken);
}

void interceptor_ICJI::findSig(
          CORINFO_MODULE_HANDLE module,
          unsigned sigTOK,
          CORINFO_CONTEXT_HANDLE context,
          CORINFO_SIG_INFO* sig)
{
    original_ICorJitInfo->findSig(module, sigTOK, context, sig);
}

void interceptor_ICJI::findCallSiteSig(
          CORINFO_MODULE_HANDLE module,
          unsigned methTOK,
          CORINFO_CONTEXT_HANDLE context,
          CORINFO_SIG_INFO* sig)
{
    original_ICorJitInfo->findCallSiteSig(module, methTOK, context, sig);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getTokenTypeAsHandle(
          CORINFO_RESOLVED_TOKEN* pResolvedToken)
{
    return original_ICorJitInfo->getTokenTypeAsHandle(pResolvedToken);
}

bool interceptor_ICJI::isValidToken(
          CORINFO_MODULE_HANDLE module,
          unsigned metaTOK)
{
    return original_ICorJitInfo->isValidToken(module, metaTOK);
}

bool interceptor_ICJI::isValidStringRef(
          CORINFO_MODULE_HANDLE module,
          unsigned metaTOK)
{
    return original_ICorJitInfo->isValidStringRef(module, metaTOK);
}

LPCWSTR interceptor_ICJI::getStringLiteral(
          CORINFO_MODULE_HANDLE module,
          unsigned metaTOK,
          int* length)
{
    return original_ICorJitInfo->getStringLiteral(module, metaTOK, length);
}

CorInfoType interceptor_ICJI::asCorInfoType(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->asCorInfoType(cls);
}

const char* interceptor_ICJI::getClassName(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getClassName(cls);
}

const char* interceptor_ICJI::getClassNameFromMetadata(
          CORINFO_CLASS_HANDLE cls,
          const char** namespaceName)
{
    return original_ICorJitInfo->getClassNameFromMetadata(cls, namespaceName);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getTypeInstantiationArgument(
          CORINFO_CLASS_HANDLE cls,
          unsigned index)
{
    return original_ICorJitInfo->getTypeInstantiationArgument(cls, index);
}

int interceptor_ICJI::appendClassName(
          WCHAR** ppBuf,
          int* pnBufLen,
          CORINFO_CLASS_HANDLE cls,
          bool fNamespace,
          bool fFullInst,
          bool fAssembly)
{
    return original_ICorJitInfo->appendClassName(ppBuf, pnBufLen, cls, fNamespace, fFullInst, fAssembly);
}

bool interceptor_ICJI::isValueClass(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->isValueClass(cls);
}

CorInfoInlineTypeCheck interceptor_ICJI::canInlineTypeCheck(
          CORINFO_CLASS_HANDLE cls,
          CorInfoInlineTypeCheckSource source)
{
    return original_ICorJitInfo->canInlineTypeCheck(cls, source);
}

DWORD interceptor_ICJI::getClassAttribs(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getClassAttribs(cls);
}

bool interceptor_ICJI::isStructRequiringStackAllocRetBuf(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->isStructRequiringStackAllocRetBuf(cls);
}

CORINFO_MODULE_HANDLE interceptor_ICJI::getClassModule(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getClassModule(cls);
}

CORINFO_ASSEMBLY_HANDLE interceptor_ICJI::getModuleAssembly(
          CORINFO_MODULE_HANDLE mod)
{
    return original_ICorJitInfo->getModuleAssembly(mod);
}

const char* interceptor_ICJI::getAssemblyName(
          CORINFO_ASSEMBLY_HANDLE assem)
{
    return original_ICorJitInfo->getAssemblyName(assem);
}

void* interceptor_ICJI::LongLifetimeMalloc(
          size_t sz)
{
    return original_ICorJitInfo->LongLifetimeMalloc(sz);
}

void interceptor_ICJI::LongLifetimeFree(
          void* obj)
{
    original_ICorJitInfo->LongLifetimeFree(obj);
}

size_t interceptor_ICJI::getClassModuleIdForStatics(
          CORINFO_CLASS_HANDLE cls,
          CORINFO_MODULE_HANDLE* pModule,
          void** ppIndirection)
{
    return original_ICorJitInfo->getClassModuleIdForStatics(cls, pModule, ppIndirection);
}

unsigned interceptor_ICJI::getClassSize(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getClassSize(cls);
}

unsigned interceptor_ICJI::getHeapClassSize(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getHeapClassSize(cls);
}

bool interceptor_ICJI::canAllocateOnStack(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->canAllocateOnStack(cls);
}

unsigned interceptor_ICJI::getClassAlignmentRequirement(
          CORINFO_CLASS_HANDLE cls,
          bool fDoubleAlignHint)
{
    return original_ICorJitInfo->getClassAlignmentRequirement(cls, fDoubleAlignHint);
}

unsigned interceptor_ICJI::getClassGClayout(
          CORINFO_CLASS_HANDLE cls,
          BYTE* gcPtrs)
{
    return original_ICorJitInfo->getClassGClayout(cls, gcPtrs);
}

unsigned interceptor_ICJI::getClassNumInstanceFields(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getClassNumInstanceFields(cls);
}

CORINFO_FIELD_HANDLE interceptor_ICJI::getFieldInClass(
          CORINFO_CLASS_HANDLE clsHnd,
          INT num)
{
    return original_ICorJitInfo->getFieldInClass(clsHnd, num);
}

bool interceptor_ICJI::checkMethodModifier(
          CORINFO_METHOD_HANDLE hMethod,
          LPCSTR modifier,
          bool fOptional)
{
    return original_ICorJitInfo->checkMethodModifier(hMethod, modifier, fOptional);
}

CorInfoHelpFunc interceptor_ICJI::getNewHelper(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          bool* pHasSideEffects)
{
    return original_ICorJitInfo->getNewHelper(pResolvedToken, callerHandle, pHasSideEffects);
}

CorInfoHelpFunc interceptor_ICJI::getNewArrHelper(
          CORINFO_CLASS_HANDLE arrayCls)
{
    return original_ICorJitInfo->getNewArrHelper(arrayCls);
}

CorInfoHelpFunc interceptor_ICJI::getCastingHelper(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          bool fThrowing)
{
    return original_ICorJitInfo->getCastingHelper(pResolvedToken, fThrowing);
}

CorInfoHelpFunc interceptor_ICJI::getSharedCCtorHelper(
          CORINFO_CLASS_HANDLE clsHnd)
{
    return original_ICorJitInfo->getSharedCCtorHelper(clsHnd);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getTypeForBox(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getTypeForBox(cls);
}

CorInfoHelpFunc interceptor_ICJI::getBoxHelper(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getBoxHelper(cls);
}

CorInfoHelpFunc interceptor_ICJI::getUnBoxHelper(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getUnBoxHelper(cls);
}

bool interceptor_ICJI::getReadyToRunHelper(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_LOOKUP_KIND* pGenericLookupKind,
          CorInfoHelpFunc id,
          CORINFO_CONST_LOOKUP* pLookup)
{
    return original_ICorJitInfo->getReadyToRunHelper(pResolvedToken, pGenericLookupKind, id, pLookup);
}

void interceptor_ICJI::getReadyToRunDelegateCtorHelper(
          CORINFO_RESOLVED_TOKEN* pTargetMethod,
          CORINFO_CLASS_HANDLE delegateType,
          CORINFO_LOOKUP* pLookup)
{
    original_ICorJitInfo->getReadyToRunDelegateCtorHelper(pTargetMethod, delegateType, pLookup);
}

const char* interceptor_ICJI::getHelperName(
          CorInfoHelpFunc helpFunc)
{
    return original_ICorJitInfo->getHelperName(helpFunc);
}

CorInfoInitClassResult interceptor_ICJI::initClass(
          CORINFO_FIELD_HANDLE field,
          CORINFO_METHOD_HANDLE method,
          CORINFO_CONTEXT_HANDLE context)
{
    return original_ICorJitInfo->initClass(field, method, context);
}

void interceptor_ICJI::classMustBeLoadedBeforeCodeIsRun(
          CORINFO_CLASS_HANDLE cls)
{
    original_ICorJitInfo->classMustBeLoadedBeforeCodeIsRun(cls);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getBuiltinClass(
          CorInfoClassId classId)
{
    return original_ICorJitInfo->getBuiltinClass(classId);
}

CorInfoType interceptor_ICJI::getTypeForPrimitiveValueClass(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getTypeForPrimitiveValueClass(cls);
}

CorInfoType interceptor_ICJI::getTypeForPrimitiveNumericClass(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getTypeForPrimitiveNumericClass(cls);
}

bool interceptor_ICJI::canCast(
          CORINFO_CLASS_HANDLE child,
          CORINFO_CLASS_HANDLE parent)
{
    return original_ICorJitInfo->canCast(child, parent);
}

bool interceptor_ICJI::areTypesEquivalent(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2)
{
    return original_ICorJitInfo->areTypesEquivalent(cls1, cls2);
}

TypeCompareState interceptor_ICJI::compareTypesForCast(
          CORINFO_CLASS_HANDLE fromClass,
          CORINFO_CLASS_HANDLE toClass)
{
    return original_ICorJitInfo->compareTypesForCast(fromClass, toClass);
}

TypeCompareState interceptor_ICJI::compareTypesForEquality(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2)
{
    return original_ICorJitInfo->compareTypesForEquality(cls1, cls2);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::mergeClasses(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2)
{
    return original_ICorJitInfo->mergeClasses(cls1, cls2);
}

bool interceptor_ICJI::isMoreSpecificType(
          CORINFO_CLASS_HANDLE cls1,
          CORINFO_CLASS_HANDLE cls2)
{
    return original_ICorJitInfo->isMoreSpecificType(cls1, cls2);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getParentType(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getParentType(cls);
}

CorInfoType interceptor_ICJI::getChildType(
          CORINFO_CLASS_HANDLE clsHnd,
          CORINFO_CLASS_HANDLE* clsRet)
{
    return original_ICorJitInfo->getChildType(clsHnd, clsRet);
}

bool interceptor_ICJI::satisfiesClassConstraints(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->satisfiesClassConstraints(cls);
}

bool interceptor_ICJI::isSDArray(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->isSDArray(cls);
}

unsigned interceptor_ICJI::getArrayRank(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->getArrayRank(cls);
}

void* interceptor_ICJI::getArrayInitializationData(
          CORINFO_FIELD_HANDLE field,
          DWORD size)
{
    return original_ICorJitInfo->getArrayInitializationData(field, size);
}

CorInfoIsAccessAllowedResult interceptor_ICJI::canAccessClass(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          CORINFO_HELPER_DESC* pAccessHelper)
{
    return original_ICorJitInfo->canAccessClass(pResolvedToken, callerHandle, pAccessHelper);
}

const char* interceptor_ICJI::getFieldName(
          CORINFO_FIELD_HANDLE ftn,
          const char** moduleName)
{
    return original_ICorJitInfo->getFieldName(ftn, moduleName);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getFieldClass(
          CORINFO_FIELD_HANDLE field)
{
    return original_ICorJitInfo->getFieldClass(field);
}

CorInfoType interceptor_ICJI::getFieldType(
          CORINFO_FIELD_HANDLE field,
          CORINFO_CLASS_HANDLE* structType,
          CORINFO_CLASS_HANDLE memberParent)
{
    return original_ICorJitInfo->getFieldType(field, structType, memberParent);
}

unsigned interceptor_ICJI::getFieldOffset(
          CORINFO_FIELD_HANDLE field)
{
    return original_ICorJitInfo->getFieldOffset(field);
}

void interceptor_ICJI::getFieldInfo(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          CORINFO_ACCESS_FLAGS flags,
          CORINFO_FIELD_INFO* pResult)
{
    original_ICorJitInfo->getFieldInfo(pResolvedToken, callerHandle, flags, pResult);
}

bool interceptor_ICJI::isFieldStatic(
          CORINFO_FIELD_HANDLE fldHnd)
{
    return original_ICorJitInfo->isFieldStatic(fldHnd);
}

void interceptor_ICJI::getBoundaries(
          CORINFO_METHOD_HANDLE ftn,
          unsigned int* cILOffsets,
          DWORD** pILOffsets,
          ICorDebugInfo::BoundaryTypes* implictBoundaries)
{
    original_ICorJitInfo->getBoundaries(ftn, cILOffsets, pILOffsets, implictBoundaries);
}

void interceptor_ICJI::setBoundaries(
          CORINFO_METHOD_HANDLE ftn,
          ULONG32 cMap,
          ICorDebugInfo::OffsetMapping* pMap)
{
    original_ICorJitInfo->setBoundaries(ftn, cMap, pMap);
}

void interceptor_ICJI::getVars(
          CORINFO_METHOD_HANDLE ftn,
          ULONG32* cVars,
          ICorDebugInfo::ILVarInfo** vars,
          bool* extendOthers)
{
    original_ICorJitInfo->getVars(ftn, cVars, vars, extendOthers);
}

void interceptor_ICJI::setVars(
          CORINFO_METHOD_HANDLE ftn,
          ULONG32 cVars,
          ICorDebugInfo::NativeVarInfo* vars)
{
    original_ICorJitInfo->setVars(ftn, cVars, vars);
}

void* interceptor_ICJI::allocateArray(
          size_t cBytes)
{
    return original_ICorJitInfo->allocateArray(cBytes);
}

void interceptor_ICJI::freeArray(
          void* array)
{
    original_ICorJitInfo->freeArray(array);
}

CORINFO_ARG_LIST_HANDLE interceptor_ICJI::getArgNext(
          CORINFO_ARG_LIST_HANDLE args)
{
    return original_ICorJitInfo->getArgNext(args);
}

CorInfoTypeWithMod interceptor_ICJI::getArgType(
          CORINFO_SIG_INFO* sig,
          CORINFO_ARG_LIST_HANDLE args,
          CORINFO_CLASS_HANDLE* vcTypeRet)
{
    return original_ICorJitInfo->getArgType(sig, args, vcTypeRet);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getArgClass(
          CORINFO_SIG_INFO* sig,
          CORINFO_ARG_LIST_HANDLE args)
{
    return original_ICorJitInfo->getArgClass(sig, args);
}

CorInfoHFAElemType interceptor_ICJI::getHFAType(
          CORINFO_CLASS_HANDLE hClass)
{
    return original_ICorJitInfo->getHFAType(hClass);
}

HRESULT interceptor_ICJI::GetErrorHRESULT(
          struct _EXCEPTION_POINTERS* pExceptionPointers)
{
    return original_ICorJitInfo->GetErrorHRESULT(pExceptionPointers);
}

ULONG interceptor_ICJI::GetErrorMessage(
          LPWSTR buffer,
          ULONG bufferLength)
{
    return original_ICorJitInfo->GetErrorMessage(buffer, bufferLength);
}

int interceptor_ICJI::FilterException(
          struct _EXCEPTION_POINTERS* pExceptionPointers)
{
    return original_ICorJitInfo->FilterException(pExceptionPointers);
}

void interceptor_ICJI::HandleException(
          struct _EXCEPTION_POINTERS* pExceptionPointers)
{
    original_ICorJitInfo->HandleException(pExceptionPointers);
}

void interceptor_ICJI::ThrowExceptionForJitResult(
          HRESULT result)
{
    original_ICorJitInfo->ThrowExceptionForJitResult(result);
}

void interceptor_ICJI::ThrowExceptionForHelper(
          const CORINFO_HELPER_DESC* throwHelper)
{
    original_ICorJitInfo->ThrowExceptionForHelper(throwHelper);
}

bool interceptor_ICJI::runWithErrorTrap(
          ICorJitInfo::errorTrapFunction function,
          void* parameter)
{
    return original_ICorJitInfo->runWithErrorTrap(function, parameter);
}

void interceptor_ICJI::getEEInfo(
          CORINFO_EE_INFO* pEEInfoOut)
{
    original_ICorJitInfo->getEEInfo(pEEInfoOut);
}

LPCWSTR interceptor_ICJI::getJitTimeLogFilename()
{
    return original_ICorJitInfo->getJitTimeLogFilename();
}

mdMethodDef interceptor_ICJI::getMethodDefFromMethod(
          CORINFO_METHOD_HANDLE hMethod)
{
    return original_ICorJitInfo->getMethodDefFromMethod(hMethod);
}

const char* interceptor_ICJI::getMethodName(
          CORINFO_METHOD_HANDLE ftn,
          const char** moduleName)
{
    return original_ICorJitInfo->getMethodName(ftn, moduleName);
}

const char* interceptor_ICJI::getMethodNameFromMetadata(
          CORINFO_METHOD_HANDLE ftn,
          const char** className,
          const char** namespaceName,
          const char** enclosingClassName)
{
    return original_ICorJitInfo->getMethodNameFromMetadata(ftn, className, namespaceName, enclosingClassName);
}

unsigned interceptor_ICJI::getMethodHash(
          CORINFO_METHOD_HANDLE ftn)
{
    return original_ICorJitInfo->getMethodHash(ftn);
}

size_t interceptor_ICJI::findNameOfToken(
          CORINFO_MODULE_HANDLE moduleHandle,
          mdToken token,
          char* szFQName,
          size_t FQNameCapacity)
{
    return original_ICorJitInfo->findNameOfToken(moduleHandle, token, szFQName, FQNameCapacity);
}

bool interceptor_ICJI::getSystemVAmd64PassStructInRegisterDescriptor(
          CORINFO_CLASS_HANDLE structHnd,
          SYSTEMV_AMD64_CORINFO_STRUCT_REG_PASSING_DESCRIPTOR* structPassInRegDescPtr)
{
    return original_ICorJitInfo->getSystemVAmd64PassStructInRegisterDescriptor(structHnd, structPassInRegDescPtr);
}

DWORD interceptor_ICJI::getThreadTLSIndex(
          void** ppIndirection)
{
    return original_ICorJitInfo->getThreadTLSIndex(ppIndirection);
}

const void* interceptor_ICJI::getInlinedCallFrameVptr(
          void** ppIndirection)
{
    return original_ICorJitInfo->getInlinedCallFrameVptr(ppIndirection);
}

LONG* interceptor_ICJI::getAddrOfCaptureThreadGlobal(
          void** ppIndirection)
{
    return original_ICorJitInfo->getAddrOfCaptureThreadGlobal(ppIndirection);
}

void* interceptor_ICJI::getHelperFtn(
          CorInfoHelpFunc ftnNum,
          void** ppIndirection)
{
    return original_ICorJitInfo->getHelperFtn(ftnNum, ppIndirection);
}

void interceptor_ICJI::getFunctionEntryPoint(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_CONST_LOOKUP* pResult,
          CORINFO_ACCESS_FLAGS accessFlags)
{
    original_ICorJitInfo->getFunctionEntryPoint(ftn, pResult, accessFlags);
}

void interceptor_ICJI::getFunctionFixedEntryPoint(
          CORINFO_METHOD_HANDLE ftn,
          CORINFO_CONST_LOOKUP* pResult)
{
    original_ICorJitInfo->getFunctionFixedEntryPoint(ftn, pResult);
}

void* interceptor_ICJI::getMethodSync(
          CORINFO_METHOD_HANDLE ftn,
          void** ppIndirection)
{
    return original_ICorJitInfo->getMethodSync(ftn, ppIndirection);
}

CorInfoHelpFunc interceptor_ICJI::getLazyStringLiteralHelper(
          CORINFO_MODULE_HANDLE handle)
{
    return original_ICorJitInfo->getLazyStringLiteralHelper(handle);
}

CORINFO_MODULE_HANDLE interceptor_ICJI::embedModuleHandle(
          CORINFO_MODULE_HANDLE handle,
          void** ppIndirection)
{
    return original_ICorJitInfo->embedModuleHandle(handle, ppIndirection);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::embedClassHandle(
          CORINFO_CLASS_HANDLE handle,
          void** ppIndirection)
{
    return original_ICorJitInfo->embedClassHandle(handle, ppIndirection);
}

CORINFO_METHOD_HANDLE interceptor_ICJI::embedMethodHandle(
          CORINFO_METHOD_HANDLE handle,
          void** ppIndirection)
{
    return original_ICorJitInfo->embedMethodHandle(handle, ppIndirection);
}

CORINFO_FIELD_HANDLE interceptor_ICJI::embedFieldHandle(
          CORINFO_FIELD_HANDLE handle,
          void** ppIndirection)
{
    return original_ICorJitInfo->embedFieldHandle(handle, ppIndirection);
}

void interceptor_ICJI::embedGenericHandle(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          bool fEmbedParent,
          CORINFO_GENERICHANDLE_RESULT* pResult)
{
    original_ICorJitInfo->embedGenericHandle(pResolvedToken, fEmbedParent, pResult);
}

void interceptor_ICJI::getLocationOfThisType(
          CORINFO_METHOD_HANDLE context,
          CORINFO_LOOKUP_KIND* pLookupKind)
{
    original_ICorJitInfo->getLocationOfThisType(context, pLookupKind);
}

void interceptor_ICJI::getAddressOfPInvokeTarget(
          CORINFO_METHOD_HANDLE method,
          CORINFO_CONST_LOOKUP* pLookup)
{
    original_ICorJitInfo->getAddressOfPInvokeTarget(method, pLookup);
}

LPVOID interceptor_ICJI::GetCookieForPInvokeCalliSig(
          CORINFO_SIG_INFO* szMetaSig,
          void** ppIndirection)
{
    return original_ICorJitInfo->GetCookieForPInvokeCalliSig(szMetaSig, ppIndirection);
}

bool interceptor_ICJI::canGetCookieForPInvokeCalliSig(
          CORINFO_SIG_INFO* szMetaSig)
{
    return original_ICorJitInfo->canGetCookieForPInvokeCalliSig(szMetaSig);
}

CORINFO_JUST_MY_CODE_HANDLE interceptor_ICJI::getJustMyCodeHandle(
          CORINFO_METHOD_HANDLE method,
          CORINFO_JUST_MY_CODE_HANDLE** ppIndirection)
{
    return original_ICorJitInfo->getJustMyCodeHandle(method, ppIndirection);
}

void interceptor_ICJI::GetProfilingHandle(
          bool* pbHookFunction,
          void** pProfilerHandle,
          bool* pbIndirectedHandles)
{
    original_ICorJitInfo->GetProfilingHandle(pbHookFunction, pProfilerHandle, pbIndirectedHandles);
}

void interceptor_ICJI::getCallInfo(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          CORINFO_RESOLVED_TOKEN* pConstrainedResolvedToken,
          CORINFO_METHOD_HANDLE callerHandle,
          CORINFO_CALLINFO_FLAGS flags,
          CORINFO_CALL_INFO* pResult)
{
    original_ICorJitInfo->getCallInfo(pResolvedToken, pConstrainedResolvedToken, callerHandle, flags, pResult);
}

bool interceptor_ICJI::canAccessFamily(
          CORINFO_METHOD_HANDLE hCaller,
          CORINFO_CLASS_HANDLE hInstanceType)
{
    return original_ICorJitInfo->canAccessFamily(hCaller, hInstanceType);
}

bool interceptor_ICJI::isRIDClassDomainID(
          CORINFO_CLASS_HANDLE cls)
{
    return original_ICorJitInfo->isRIDClassDomainID(cls);
}

unsigned interceptor_ICJI::getClassDomainID(
          CORINFO_CLASS_HANDLE cls,
          void** ppIndirection)
{
    return original_ICorJitInfo->getClassDomainID(cls, ppIndirection);
}

void* interceptor_ICJI::getFieldAddress(
          CORINFO_FIELD_HANDLE field,
          void** ppIndirection)
{
    return original_ICorJitInfo->getFieldAddress(field, ppIndirection);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getStaticFieldCurrentClass(
          CORINFO_FIELD_HANDLE field,
          bool* pIsSpeculative)
{
    return original_ICorJitInfo->getStaticFieldCurrentClass(field, pIsSpeculative);
}

CORINFO_VARARGS_HANDLE interceptor_ICJI::getVarArgsHandle(
          CORINFO_SIG_INFO* pSig,
          void** ppIndirection)
{
    return original_ICorJitInfo->getVarArgsHandle(pSig, ppIndirection);
}

bool interceptor_ICJI::canGetVarArgsHandle(
          CORINFO_SIG_INFO* pSig)
{
    return original_ICorJitInfo->canGetVarArgsHandle(pSig);
}

InfoAccessType interceptor_ICJI::constructStringLiteral(
          CORINFO_MODULE_HANDLE module,
          mdToken metaTok,
          void** ppValue)
{
    return original_ICorJitInfo->constructStringLiteral(module, metaTok, ppValue);
}

InfoAccessType interceptor_ICJI::emptyStringLiteral(
          void** ppValue)
{
    return original_ICorJitInfo->emptyStringLiteral(ppValue);
}

DWORD interceptor_ICJI::getFieldThreadLocalStoreID(
          CORINFO_FIELD_HANDLE field,
          void** ppIndirection)
{
    return original_ICorJitInfo->getFieldThreadLocalStoreID(field, ppIndirection);
}

void interceptor_ICJI::setOverride(
          ICorDynamicInfo* pOverride,
          CORINFO_METHOD_HANDLE currentMethod)
{
    original_ICorJitInfo->setOverride(pOverride, currentMethod);
}

void interceptor_ICJI::addActiveDependency(
          CORINFO_MODULE_HANDLE moduleFrom,
          CORINFO_MODULE_HANDLE moduleTo)
{
    original_ICorJitInfo->addActiveDependency(moduleFrom, moduleTo);
}

CORINFO_METHOD_HANDLE interceptor_ICJI::GetDelegateCtor(
          CORINFO_METHOD_HANDLE methHnd,
          CORINFO_CLASS_HANDLE clsHnd,
          CORINFO_METHOD_HANDLE targetMethodHnd,
          DelegateCtorArgs* pCtorData)
{
    return original_ICorJitInfo->GetDelegateCtor(methHnd, clsHnd, targetMethodHnd, pCtorData);
}

void interceptor_ICJI::MethodCompileComplete(
          CORINFO_METHOD_HANDLE methHnd)
{
    original_ICorJitInfo->MethodCompileComplete(methHnd);
}

bool interceptor_ICJI::getTailCallHelpers(
          CORINFO_RESOLVED_TOKEN* callToken,
          CORINFO_SIG_INFO* sig,
          CORINFO_GET_TAILCALL_HELPERS_FLAGS flags,
          CORINFO_TAILCALL_HELPERS* pResult)
{
    return original_ICorJitInfo->getTailCallHelpers(callToken, sig, flags, pResult);
}

bool interceptor_ICJI::convertPInvokeCalliToCall(
          CORINFO_RESOLVED_TOKEN* pResolvedToken,
          bool mustConvert)
{
    return original_ICorJitInfo->convertPInvokeCalliToCall(pResolvedToken, mustConvert);
}

bool interceptor_ICJI::notifyInstructionSetUsage(
          CORINFO_InstructionSet instructionSet,
          bool supportEnabled)
{
    return original_ICorJitInfo->notifyInstructionSetUsage(instructionSet, supportEnabled);
}

void interceptor_ICJI::allocMem(
          ULONG hotCodeSize,
          ULONG coldCodeSize,
          ULONG roDataSize,
          ULONG xcptnsCount,
          CorJitAllocMemFlag flag,
          void** hotCodeBlock,
          void** coldCodeBlock,
          void** roDataBlock)
{
    original_ICorJitInfo->allocMem(hotCodeSize, coldCodeSize, roDataSize, xcptnsCount, flag, hotCodeBlock, coldCodeBlock, roDataBlock);
}

void interceptor_ICJI::reserveUnwindInfo(
          bool isFunclet,
          bool isColdCode,
          ULONG unwindSize)
{
    original_ICorJitInfo->reserveUnwindInfo(isFunclet, isColdCode, unwindSize);
}

void interceptor_ICJI::allocUnwindInfo(
          BYTE* pHotCode,
          BYTE* pColdCode,
          ULONG startOffset,
          ULONG endOffset,
          ULONG unwindSize,
          BYTE* pUnwindBlock,
          CorJitFuncKind funcKind)
{
    original_ICorJitInfo->allocUnwindInfo(pHotCode, pColdCode, startOffset, endOffset, unwindSize, pUnwindBlock, funcKind);
}

void* interceptor_ICJI::allocGCInfo(
          size_t size)
{
    return original_ICorJitInfo->allocGCInfo(size);
}

void interceptor_ICJI::setEHcount(
          unsigned cEH)
{
    original_ICorJitInfo->setEHcount(cEH);
}

void interceptor_ICJI::setEHinfo(
          unsigned EHnumber,
          const CORINFO_EH_CLAUSE* clause)
{
    original_ICorJitInfo->setEHinfo(EHnumber, clause);
}

bool interceptor_ICJI::logMsg(
          unsigned level,
          const char* fmt,
          va_list args)
{
    return original_ICorJitInfo->logMsg(level, fmt, args);
}

int interceptor_ICJI::doAssert(
          const char* szFile,
          int iLine,
          const char* szExpr)
{
    return original_ICorJitInfo->doAssert(szFile, iLine, szExpr);
}

void interceptor_ICJI::reportFatalError(
          CorJitResult result)
{
    original_ICorJitInfo->reportFatalError(result);
}

HRESULT interceptor_ICJI::allocMethodBlockCounts(
          UINT32 count,
          ICorJitInfo::BlockCounts** pBlockCounts)
{
    return original_ICorJitInfo->allocMethodBlockCounts(count, pBlockCounts);
}

HRESULT interceptor_ICJI::getMethodBlockCounts(
          CORINFO_METHOD_HANDLE ftnHnd,
          UINT32* pCount,
          ICorJitInfo::BlockCounts** pBlockCounts,
          UINT32* pNumRuns)
{
    return original_ICorJitInfo->getMethodBlockCounts(ftnHnd, pCount, pBlockCounts, pNumRuns);
}

CORINFO_CLASS_HANDLE interceptor_ICJI::getLikelyClass(
          CORINFO_METHOD_HANDLE ftnHnd,
          CORINFO_CLASS_HANDLE baseHnd,
          UINT32 ilOffset,
          UINT32* pLikelihood,
          UINT32* pNumberOfClasses)
{
    return original_ICorJitInfo->getLikelyClass(ftnHnd, baseHnd, ilOffset, pLikelihood, pNumberOfClasses);
}

void interceptor_ICJI::recordCallSite(
          ULONG instrOffset,
          CORINFO_SIG_INFO* callSig,
          CORINFO_METHOD_HANDLE methodHandle)
{
    original_ICorJitInfo->recordCallSite(instrOffset, callSig, methodHandle);
}

void interceptor_ICJI::recordRelocation(
          void* location,
          void* target,
          WORD fRelocType,
          WORD slotNum,
          INT32 addlDelta)
{
    original_ICorJitInfo->recordRelocation(location, target, fRelocType, slotNum, addlDelta);
}

WORD interceptor_ICJI::getRelocTypeHint(
          void* target)
{
    return original_ICorJitInfo->getRelocTypeHint(target);
}

DWORD interceptor_ICJI::getExpectedTargetArchitecture()
{
    return original_ICorJitInfo->getExpectedTargetArchitecture();
}

DWORD interceptor_ICJI::getJitFlags(
          CORJIT_FLAGS* flags,
          DWORD sizeInBytes)
{
    return original_ICorJitInfo->getJitFlags(flags, sizeInBytes);
}

