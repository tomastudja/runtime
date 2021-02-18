// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

//----------------------------------------------------------
// CompileResult.h - CompileResult contains the stuff generated by a compilation
//----------------------------------------------------------
#ifndef _CompileResult
#define _CompileResult

#include "runtimedetails.h"
#include "lightweightmap.h"
#include "agnostic.h"

// MemoryTracker: a very simple allocator and tracker of allocated memory, so it can be deleted when needed.
class MemoryTracker
{
public:
    MemoryTracker() : m_pHead(nullptr) {}
    ~MemoryTracker() { freeAll(); }

    void* allocate(size_t sizeInBytes)
    {
        BYTE* pNew = new BYTE[sizeInBytes];
        m_pHead = new MemoryNode(pNew, m_pHead);    // Prepend this new one to the tracked memory list.
        return pNew;
    }

private:

    MemoryTracker(const MemoryTracker&) = delete; // no copy ctor

    void freeAll()
    {
        for (MemoryNode* p = m_pHead; p != nullptr; )
        {
            MemoryNode* pNext = p->m_pNext;
            delete p;
            p = pNext;
        }
        m_pHead = nullptr;
    }

    struct MemoryNode
    {
        MemoryNode(BYTE* pMem, MemoryNode* pNext) : m_pMem(pMem), m_pNext(pNext) {}
        ~MemoryNode() { delete[] m_pMem; }

        BYTE*       m_pMem;
        MemoryNode* m_pNext;
    };

    MemoryNode* m_pHead;
};

class CompileResult
{
public:

    CompileResult();
    ~CompileResult();

    bool IsEmpty();

    void AddCall(const char* name);
    unsigned int CallLog_GetCount();
    bool CallLog_Contains(const char* str);
    void dmpCallLog(DWORD key, DWORD value);

    void dumpToConsole();

    void* allocateMemory(size_t sizeInBytes);

    void recAssert(const char* buff);
    void dmpAssertLog(DWORD key, DWORD value);
    const char* repAssert();

    void recAllocMem(ULONG              hotCodeSize,
                     ULONG              coldCodeSize,
                     ULONG              roDataSize,
                     ULONG              xcptnsCount,
                     CorJitAllocMemFlag flag,
                     void**             hotCodeBlock,
                     void**             coldCodeBlock,
                     void**             roDataBlock);
    void recAllocMemCapture();
    void dmpAllocMem(DWORD key, const Agnostic_AllocMemDetails& value);
    void repAllocMem(ULONG*              hotCodeSize,
                     ULONG*              coldCodeSize,
                     ULONG*              roDataSize,
                     ULONG*              xcptnsCount,
                     CorJitAllocMemFlag* flag,
                     unsigned char**     hotCodeBlock,
                     unsigned char**     coldCodeBlock,
                     unsigned char**     roDataBlock,
                     void**              orig_hotCodeBlock,
                     void**              orig_coldCodeBlock,
                     void**              orig_roDataBlock);

    void recSetBoundaries(CORINFO_METHOD_HANDLE ftn, ULONG32 cMap, ICorDebugInfo::OffsetMapping* pMap);
    void dmpSetBoundaries(DWORD key, const Agnostic_SetBoundaries& value);
    bool repSetBoundaries(CORINFO_METHOD_HANDLE* ftn, ULONG32* cMap, ICorDebugInfo::OffsetMapping** pMap);

    void recSetVars(CORINFO_METHOD_HANDLE ftn, ULONG32 cVars, ICorDebugInfo::NativeVarInfo* vars);
    void dmpSetVars(DWORD key, const Agnostic_SetVars& value);
    bool repSetVars(CORINFO_METHOD_HANDLE* ftn, ULONG32* cVars, ICorDebugInfo::NativeVarInfo** vars);

    void recSetPatchpointInfo(PatchpointInfo* patchpointInfo);
    void dmpSetPatchpointInfo(DWORD key, const Agnostic_SetPatchpointInfo& value);
    bool repSetPatchpointInfo(PatchpointInfo** patchpointInfo);

    void recAllocGCInfo(size_t size, void* retval);
    void recAllocGCInfoCapture();
    void dmpAllocGCInfo(DWORD key, const Agnostic_AllocGCInfo& value);
    void repAllocGCInfo(size_t* size, void** retval);

    void recCompileMethod(BYTE** nativeEntry, ULONG* nativeSizeOfCode, CorJitResult result);
    void dmpCompileMethod(DWORD key, const Agnostic_CompileMethodResults& value);
    void repCompileMethod(BYTE** nativeEntry, ULONG* nativeSizeOfCode, CorJitResult* result);

    void recMessageLog(const char* fmt, ...);
    void dmpMessageLog(DWORD key, DWORD value);

    void recClassMustBeLoadedBeforeCodeIsRun(CORINFO_CLASS_HANDLE cls);
    void dmpClassMustBeLoadedBeforeCodeIsRun(DWORD key, DWORDLONG value);

    void recReportInliningDecision(CORINFO_METHOD_HANDLE inlinerHnd,
                                   CORINFO_METHOD_HANDLE inlineeHnd,
                                   CorInfoInline         inlineResult,
                                   const char*           reason);
    void dmpReportInliningDecision(DWORD key, const Agnostic_ReportInliningDecision& value);
    CorInfoInline repReportInliningDecision(CORINFO_METHOD_HANDLE inlinerHnd, CORINFO_METHOD_HANDLE inlineeHnd);

    void recSetEHcount(unsigned cEH);
    void dmpSetEHcount(DWORD key, DWORD value);
    ULONG repSetEHcount();

    void recSetEHinfo(unsigned EHnumber, const CORINFO_EH_CLAUSE* clause);
    void dmpSetEHinfo(DWORD key, const Agnostic_CORINFO_EH_CLAUSE& value);
    void repSetEHinfo(unsigned EHnumber,
                      ULONG*   flags,
                      ULONG*   tryOffset,
                      ULONG*   tryLength,
                      ULONG*   handlerOffset,
                      ULONG*   handlerLength,
                      ULONG*   classToken);

    void recSetMethodAttribs(CORINFO_METHOD_HANDLE ftn, CorInfoMethodRuntimeFlags attribs);
    void dmpSetMethodAttribs(DWORDLONG key, DWORD value);
    CorInfoMethodRuntimeFlags repSetMethodAttribs(CORINFO_METHOD_HANDLE ftn);

    void recMethodMustBeLoadedBeforeCodeIsRun(CORINFO_METHOD_HANDLE method);
    void dmpMethodMustBeLoadedBeforeCodeIsRun(DWORD key, DWORDLONG value);

    void recReportTailCallDecision(CORINFO_METHOD_HANDLE callerHnd,
                                   CORINFO_METHOD_HANDLE calleeHnd,
                                   bool                  fIsTailPrefix,
                                   CorInfoTailCall       tailCallResult,
                                   const char*           reason);
    void dmpReportTailCallDecision(DWORD key, const Agnostic_ReportTailCallDecision& value);

    void recReportFatalError(CorJitResult result);
    void dmpReportFatalError(DWORD key, DWORD value);

    void recRecordRelocation(void* location, void* target, WORD fRelocType, WORD slotNum, INT32 addlDelta);
    void dmpRecordRelocation(DWORD key, const Agnostic_RecordRelocation& value);
    void repRecordRelocation(void* location, void* target, WORD fRelocType, WORD slotNum, INT32 addlDelta);
    void applyRelocs(unsigned char* block1, ULONG blocksize1, void* originalAddr);

    void recProcessName(const char* name);
    void dmpProcessName(DWORD key, DWORD value);
    const char* repProcessName();

    void recAddressMap(void* original_address, void* replay_address, unsigned int size);
    void dmpAddressMap(DWORDLONG key, const Agnostic_AddressMap& value);
    void* repAddressMap(void* replay_address);
    void* searchAddressMap(void* replay_address);

    void recReserveUnwindInfo(BOOL isFunclet, BOOL isColdCode, ULONG unwindSize);
    void dmpReserveUnwindInfo(DWORD key, const Agnostic_ReserveUnwindInfo& value);

    void recAllocUnwindInfo(BYTE*          pHotCode,
                            BYTE*          pColdCode,
                            ULONG          startOffset,
                            ULONG          endOffset,
                            ULONG          unwindSize,
                            BYTE*          pUnwindBlock,
                            CorJitFuncKind funcKind);
    void dmpAllocUnwindInfo(DWORD key, const Agnostic_AllocUnwindInfo& value);

    void recRecordCallSite(ULONG instrOffset, CORINFO_SIG_INFO* callSig, CORINFO_METHOD_HANDLE methodHandle);
    void dmpRecordCallSiteWithSignature(DWORD key, const Agnostic_RecordCallSite& value) const;
    void dmpRecordCallSiteWithoutSignature(DWORD key, DWORDLONG methodHandle) const;
    void repRecordCallSite(ULONG instrOffset, CORINFO_SIG_INFO* callSig, CORINFO_METHOD_HANDLE methodHandle);
    bool fndRecordCallSiteSigInfo(ULONG instrOffset, CORINFO_SIG_INFO* pCallSig);
    bool fndRecordCallSiteMethodHandle(ULONG instrOffset, CORINFO_METHOD_HANDLE* pMethodHandle);

    void dmpCrSigInstHandleMap(DWORD key, DWORDLONG value);

    DOUBLE    secondsToCompile;
    ULONGLONG clockCyclesToCompile;

#define LWM(map, key, value) LightWeightMap<key, value>* map;
#define DENSELWM(map, value) DenseLightWeightMap<value>* map;
#include "crlwmlist.h"

    // not persisted to disk.
public:
    LightWeightMap<DWORDLONG, DWORD>* CallTargetTypes;

private:
    MemoryTracker*          memoryTracker;
    Capture_AllocMemDetails allocMemDets;
    allocGCInfoDetails      allocGCInfoDets;
};
#endif
