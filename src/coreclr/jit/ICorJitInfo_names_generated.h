// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// DO NOT EDIT THIS FILE! IT IS AUTOGENERATED
// To regenerate run the gen script in src/coreclr/tools/Common/JitInterface/ThunkGenerator
// and follow the instructions in docs/project/updating-jitinterface.md
DEF_CLR_API(isIntrinsic)
DEF_CLR_API(getMethodAttribs)
DEF_CLR_API(setMethodAttribs)
DEF_CLR_API(getMethodSig)
DEF_CLR_API(getMethodInfo)
DEF_CLR_API(canInline)
DEF_CLR_API(beginInlining)
DEF_CLR_API(reportInliningDecision)
DEF_CLR_API(canTailCall)
DEF_CLR_API(reportTailCallDecision)
DEF_CLR_API(getEHinfo)
DEF_CLR_API(getMethodClass)
DEF_CLR_API(getMethodModule)
DEF_CLR_API(getMethodVTableOffset)
DEF_CLR_API(resolveVirtualMethod)
DEF_CLR_API(getUnboxedEntry)
DEF_CLR_API(getDefaultComparerClass)
DEF_CLR_API(getDefaultEqualityComparerClass)
DEF_CLR_API(expandRawHandleIntrinsic)
DEF_CLR_API(isIntrinsicType)
DEF_CLR_API(getUnmanagedCallConv)
DEF_CLR_API(pInvokeMarshalingRequired)
DEF_CLR_API(satisfiesMethodConstraints)
DEF_CLR_API(methodMustBeLoadedBeforeCodeIsRun)
DEF_CLR_API(mapMethodDeclToMethodImpl)
DEF_CLR_API(getGSCookie)
DEF_CLR_API(setPatchpointInfo)
DEF_CLR_API(getOSRInfo)
DEF_CLR_API(resolveToken)
DEF_CLR_API(tryResolveToken)
DEF_CLR_API(findSig)
DEF_CLR_API(findCallSiteSig)
DEF_CLR_API(getTokenTypeAsHandle)
DEF_CLR_API(isValidToken)
DEF_CLR_API(isValidStringRef)
DEF_CLR_API(getStringLiteral)
DEF_CLR_API(printObjectDescription)
DEF_CLR_API(asCorInfoType)
DEF_CLR_API(getClassNameFromMetadata)
DEF_CLR_API(getTypeInstantiationArgument)
DEF_CLR_API(printClassName)
DEF_CLR_API(isValueClass)
DEF_CLR_API(canInlineTypeCheck)
DEF_CLR_API(getClassAttribs)
DEF_CLR_API(getClassModule)
DEF_CLR_API(getModuleAssembly)
DEF_CLR_API(getAssemblyName)
DEF_CLR_API(LongLifetimeMalloc)
DEF_CLR_API(LongLifetimeFree)
DEF_CLR_API(getClassModuleIdForStatics)
DEF_CLR_API(getIsClassInitedFlagAddress)
DEF_CLR_API(getStaticBaseAddress)
DEF_CLR_API(getClassSize)
DEF_CLR_API(getHeapClassSize)
DEF_CLR_API(canAllocateOnStack)
DEF_CLR_API(getClassAlignmentRequirement)
DEF_CLR_API(getClassGClayout)
DEF_CLR_API(getClassNumInstanceFields)
DEF_CLR_API(getFieldInClass)
DEF_CLR_API(checkMethodModifier)
DEF_CLR_API(getNewHelper)
DEF_CLR_API(getNewArrHelper)
DEF_CLR_API(getCastingHelper)
DEF_CLR_API(getSharedCCtorHelper)
DEF_CLR_API(getTypeForBox)
DEF_CLR_API(getBoxHelper)
DEF_CLR_API(getUnBoxHelper)
DEF_CLR_API(getRuntimeTypePointer)
DEF_CLR_API(isObjectImmutable)
DEF_CLR_API(getStringChar)
DEF_CLR_API(getObjectType)
DEF_CLR_API(getReadyToRunHelper)
DEF_CLR_API(getReadyToRunDelegateCtorHelper)
DEF_CLR_API(initClass)
DEF_CLR_API(classMustBeLoadedBeforeCodeIsRun)
DEF_CLR_API(getBuiltinClass)
DEF_CLR_API(getTypeForPrimitiveValueClass)
DEF_CLR_API(getTypeForPrimitiveNumericClass)
DEF_CLR_API(canCast)
DEF_CLR_API(compareTypesForCast)
DEF_CLR_API(compareTypesForEquality)
DEF_CLR_API(isMoreSpecificType)
DEF_CLR_API(isEnum)
DEF_CLR_API(getParentType)
DEF_CLR_API(getChildType)
DEF_CLR_API(isSDArray)
DEF_CLR_API(getArrayRank)
DEF_CLR_API(getArrayIntrinsicID)
DEF_CLR_API(getArrayInitializationData)
DEF_CLR_API(canAccessClass)
DEF_CLR_API(printFieldName)
DEF_CLR_API(getFieldClass)
DEF_CLR_API(getFieldType)
DEF_CLR_API(getFieldOffset)
DEF_CLR_API(getFieldInfo)
DEF_CLR_API(getThreadLocalFieldInfo)
DEF_CLR_API(getThreadLocalStaticBlocksInfo)
DEF_CLR_API(isFieldStatic)
DEF_CLR_API(getArrayOrStringLength)
DEF_CLR_API(getBoundaries)
DEF_CLR_API(setBoundaries)
DEF_CLR_API(getVars)
DEF_CLR_API(setVars)
DEF_CLR_API(reportRichMappings)
DEF_CLR_API(allocateArray)
DEF_CLR_API(freeArray)
DEF_CLR_API(getArgNext)
DEF_CLR_API(getArgType)
DEF_CLR_API(getExactClasses)
DEF_CLR_API(getArgClass)
DEF_CLR_API(getHFAType)
DEF_CLR_API(runWithErrorTrap)
DEF_CLR_API(runWithSPMIErrorTrap)
DEF_CLR_API(getEEInfo)
DEF_CLR_API(getJitTimeLogFilename)
DEF_CLR_API(getMethodDefFromMethod)
DEF_CLR_API(printMethodName)
DEF_CLR_API(getMethodNameFromMetadata)
DEF_CLR_API(getMethodHash)
DEF_CLR_API(findNameOfToken)
DEF_CLR_API(getSystemVAmd64PassStructInRegisterDescriptor)
DEF_CLR_API(getLoongArch64PassStructInRegisterFlags)
DEF_CLR_API(getRISCV64PassStructInRegisterFlags)
DEF_CLR_API(getThreadTLSIndex)
DEF_CLR_API(getInlinedCallFrameVptr)
DEF_CLR_API(getAddrOfCaptureThreadGlobal)
DEF_CLR_API(getHelperFtn)
DEF_CLR_API(getFunctionEntryPoint)
DEF_CLR_API(getFunctionFixedEntryPoint)
DEF_CLR_API(getMethodSync)
DEF_CLR_API(getLazyStringLiteralHelper)
DEF_CLR_API(embedModuleHandle)
DEF_CLR_API(embedClassHandle)
DEF_CLR_API(embedMethodHandle)
DEF_CLR_API(embedFieldHandle)
DEF_CLR_API(embedGenericHandle)
DEF_CLR_API(getLocationOfThisType)
DEF_CLR_API(getAddressOfPInvokeTarget)
DEF_CLR_API(GetCookieForPInvokeCalliSig)
DEF_CLR_API(canGetCookieForPInvokeCalliSig)
DEF_CLR_API(getJustMyCodeHandle)
DEF_CLR_API(GetProfilingHandle)
DEF_CLR_API(getCallInfo)
DEF_CLR_API(canAccessFamily)
DEF_CLR_API(isRIDClassDomainID)
DEF_CLR_API(getClassDomainID)
DEF_CLR_API(getStaticFieldContent)
DEF_CLR_API(getObjectContent)
DEF_CLR_API(getStaticFieldCurrentClass)
DEF_CLR_API(getVarArgsHandle)
DEF_CLR_API(canGetVarArgsHandle)
DEF_CLR_API(constructStringLiteral)
DEF_CLR_API(emptyStringLiteral)
DEF_CLR_API(getFieldThreadLocalStoreID)
DEF_CLR_API(GetDelegateCtor)
DEF_CLR_API(MethodCompileComplete)
DEF_CLR_API(getTailCallHelpers)
DEF_CLR_API(convertPInvokeCalliToCall)
DEF_CLR_API(notifyInstructionSetUsage)
DEF_CLR_API(updateEntryPointForTailCall)
DEF_CLR_API(allocMem)
DEF_CLR_API(reserveUnwindInfo)
DEF_CLR_API(allocUnwindInfo)
DEF_CLR_API(allocGCInfo)
DEF_CLR_API(setEHcount)
DEF_CLR_API(setEHinfo)
DEF_CLR_API(logMsg)
DEF_CLR_API(doAssert)
DEF_CLR_API(reportFatalError)
DEF_CLR_API(getPgoInstrumentationResults)
DEF_CLR_API(allocPgoInstrumentationBySchema)
DEF_CLR_API(recordCallSite)
DEF_CLR_API(recordRelocation)
DEF_CLR_API(getRelocTypeHint)
DEF_CLR_API(getExpectedTargetArchitecture)
DEF_CLR_API(getJitFlags)

#undef DEF_CLR_API
