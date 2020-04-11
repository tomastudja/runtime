// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "metadatagetdispenser.h"

#ifdef WIN32
#include <Windows.h>
#else // WIN32
#include <dlfcn.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif // __APPLE__
#endif // WIN32

MetaDataGetDispenser::MetaDataGetDispenser() :
    _dispenser(NULL),
    _failures(0)
{

}

MetaDataGetDispenser::~MetaDataGetDispenser()
{
    if (_dispenser != NULL)
    {
        _dispenser->Release();
        _dispenser = NULL;
    }
}

GUID MetaDataGetDispenser::GetClsid()
{
    // {7198FF3E-50E8-4AD1-9B89-CB15A1D6E740}
    GUID clsid = { 0x7198FF3E, 0x50E8, 0x4AD1, {0x9B, 0x89, 0xCB, 0x15, 0xA1, 0xD6, 0xE7, 0x40 } };
    return clsid;
}

HRESULT MetaDataGetDispenser::Initialize(IUnknown* pICorProfilerInfoUnk)
{
    HRESULT hr = Profiler::Initialize(pICorProfilerInfoUnk);
    if (FAILED(hr))
    {
        _failures++;
        printf("Profiler::Initialize failed with hr=0x%x\n", hr);
        return hr;
    }

    printf("Initialize started\n");

    DWORD eventMaskLow = COR_PRF_MONITOR_MODULE_LOADS |
                         COR_PRF_DISABLE_ALL_NGEN_IMAGES;
    DWORD eventMaskHigh = 0x0;
    if (FAILED(hr = pCorProfilerInfo->SetEventMask2(eventMaskLow, eventMaskHigh)))
    {
        _failures++;
        printf("ICorProfilerInfo::SetEventMask2() failed hr=0x%x\n", hr);
        return hr;
    }



    return S_OK;
}

HRESULT MetaDataGetDispenser::Shutdown()
{
    Profiler::Shutdown();

    if(_failures == 0)
    {
        printf("PROFILER TEST PASSES\n");
    }
    else
    {
        printf("Test failed number of failures=%d\n", _failures.load());
    }
    fflush(stdout);

    return S_OK;
}

HRESULT MetaDataGetDispenser::ModuleLoadStarted(ModuleID moduleId)
{
    COMPtrHolder<IMetaDataDispenserEx> pDispenser;
    HRESULT hr = GetDispenser(&pDispenser);
    if (FAILED(hr))
    {
        _failures++;
        printf("Failed to get IMetaDataDispenserEx\n");
        return E_FAIL;
    }

    WCHAR filePath[STRING_LENGTH];
    ULONG filePathLength;
    hr = pCorProfilerInfo->GetModuleInfo2(moduleId,
                                          NULL,
                                          STRING_LENGTH,
                                          &filePathLength,
                                          filePath,
                                          NULL,
                                          NULL);
    if (FAILED(hr))
    {
        _failures++;
        printf("Failed to get ModuleInfo\n");
        return E_FAIL;
    }

    COMPtrHolder<IMetaDataImport> pImport;
    hr = pDispenser->OpenScope(filePath,
                               ofRead,
                               IID_IMetaDataImport,
                               (IUnknown **)&pImport);
    if (FAILED(hr))
    {
        _failures++;
        printf("failed to get IMetaDataImport from dispenser.\n");
        return E_FAIL;
    }

    printf("ModuleLoadStarted exiting\n");
    return S_OK;
}


// typedef HRESULT (*GetDispenserFunc) (CLSID *pClsid, IID *pIid, void **ppv);
#if WIN32

HRESULT MetaDataGetDispenser::GetDispenser(IMetaDataDispenserEx **disp)
{
    HMODULE coreclr = LoadLibrary("coreclr.dll");
    if (coreclr == NULL)
    {
        _failures++;
        printf("Failed to find coreclr.dll\n");
        return E_FAIL;
    }

    GetDispenserFunc dispenserFunc = (GetDispenserFunc)GetProcAddress(coreclr, "MetaDataGetDispenser");
    if (dispenserFunc == NULL)
    {
        _failures++;
        printf("Failed to find MetaDataGetDispenser.\n");
        return E_FAIL;
    }

    //hr = pRuntime->GetInterface(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenserEx, (void **)&pDisp);
    HRESULT hr = dispenserFunc(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenserEx, (void **)disp);
    if (FAILED(hr))
    {
        _failures++;
        printf("Failed to call MetaDataGetDispenser.\n");
        return hr;
    }

    FreeLibrary(coreclr);
    printf("Got IMetaDataDispenserEx");
    return S_OK;
}

#else // WIN32

#ifdef __APPLE__
bool EndsWith(const char *lhs, const char *rhs)
{
    size_t lhsLen = strlen(lhs);
    size_t rhsLen = strlen(rhs);
    if (lhsLen < rhsLen)
    {
        return false;
    }

    size_t lhsPos = lhsLen - rhsLen;
    size_t rhsPos = 0;

    while (rhsPos < rhsLen)
    {
        if (lhs[lhsPos] != rhs[rhsPos])
        {
            return false;
        }

        ++lhsPos;
        ++rhsPos;
    }

    return true;
}
const char *GetCoreCLRPath()
{
    const char *coreclrName = "libcoreclr.dylib";
    size_t count = _dyld_image_count();
    for (size_t i = 0; i < count; ++i)
    {
        const char *name = _dyld_get_image_name(i);
        if (EndsWith(name, coreclrName))
        {
            return name;
        }
    }

    return coreclrName;
}
#endif // __APPLE__

HRESULT MetaDataGetDispenser::GetDispenser(IMetaDataDispenserEx **disp)
{
#ifdef __APPLE__
    const char *profilerName = GetCoreCLRPath();
#else // __APPLE__
    const char *profilerName = "libcoreclr.so";
#endif // __APPLE__

    void *coreclr = dlopen(profilerName, RTLD_LAZY | RTLD_NOLOAD);
    if (coreclr == NULL)
    {
        _failures++;
        char *reason = dlerror();
        printf("Failed to find %s reason=%s\n", profilerName, reason);
        return E_FAIL;
    }

    GetDispenserFunc dispenserFunc = (GetDispenserFunc)dlsym(coreclr, "MetaDataGetDispenser");
    if (dispenserFunc == NULL)
    {
        _failures++;
        printf("Failed to find MetaDataGetDispenser.\n");
        return E_FAIL;
    }

    HRESULT hr = dispenserFunc(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenserEx, (void **)disp);
    if (FAILED(hr))
    {
        _failures++;
        printf("Failed to call MetaDataGetDispenser.\n");
        return hr;
    }

    dlclose(coreclr);
    printf("Got IMetaDataDispenserEx\n");
    return S_OK;
}
#endif // WIN32
