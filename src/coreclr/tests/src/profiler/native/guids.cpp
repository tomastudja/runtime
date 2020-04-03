// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include <rpc.h>
#include <guiddef.h>

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif//EXTERN_C

#ifdef DEFINE_GUID
#undef DEFINE_GUID
#endif

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID(GUID_NULL, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
DEFINE_GUID(IID_IUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(IID_IClassFactory, 0x00000001, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

DEFINE_GUID(IID_ICorProfilerCallback, 0x176FBED1,0xA55C,0x4796,0x98,0xCA,0xA9,0xDA,0x0E,0xF8,0x83,0xE7);
DEFINE_GUID(IID_ICorProfilerCallback2, 0x8A8CC829,0xCCF2,0x49fe,0xBB,0xAE,0x0F,0x02,0x22,0x28,0x07,0x1A);
DEFINE_GUID(IID_ICorProfilerCallback3, 0x4FD2ED52,0x7731,0x4b8d,0x94,0x69,0x03,0xD2,0xCC,0x30,0x86,0xC5);
DEFINE_GUID(IID_ICorProfilerCallback4, 0x7B63B2E3,0x107D,0x4d48,0xB2,0xF6,0xF6,0x1E,0x22,0x94,0x70,0xD2);
DEFINE_GUID(IID_ICorProfilerCallback5, 0x8DFBA405,0x8C9F,0x45F8,0xBF,0xFA,0x83,0xB1,0x4C,0xEF,0x78,0xB5);
DEFINE_GUID(IID_ICorProfilerCallback6, 0xFC13DF4B,0x4448,0x4F4F,0x95,0x0C,0xBA,0x8D,0x19,0xD0,0x0C,0x36);
DEFINE_GUID(IID_ICorProfilerCallback7, 0xF76A2DBA,0x1D52,0x4539,0x86,0x6C,0x2A,0xA5,0x18,0xF9,0xEF,0xC3);
DEFINE_GUID(IID_ICorProfilerCallback8, 0x5BED9B15,0xC079,0x4D47,0xBF,0xE2,0x21,0x5A,0x14,0x0C,0x07,0xE0);
DEFINE_GUID(IID_ICorProfilerCallback9,0x27583EC3,0xC8F5,0x482F,0x80,0x52,0x19,0x4B,0x8C,0xE4,0x70,0x5A);
DEFINE_GUID(IID_ICorProfilerInfo, 0x28B5557D,0x3F3F,0x48b4,0x90,0xB2,0x5F,0x9E,0xEA,0x2F,0x6C,0x48);
DEFINE_GUID(IID_ICorProfilerInfo2, 0xCC0935CD,0xA518,0x487d,0xB0,0xBB,0xA9,0x32,0x14,0xE6,0x54,0x78);
DEFINE_GUID(IID_ICorProfilerInfo3, 0xB555ED4F,0x452A,0x4E54,0x8B,0x39,0xB5,0x36,0x0B,0xAD,0x32,0xA0);
DEFINE_GUID(IID_ICorProfilerObjectEnum, 0x2C6269BD,0x2D13,0x4321,0xAE,0x12,0x66,0x86,0x36,0x5F,0xD6,0xAF);
DEFINE_GUID(IID_ICorProfilerFunctionEnum, 0xFF71301A,0xB994,0x429D,0xA1,0x0B,0xB3,0x45,0xA6,0x52,0x80,0xEF);
DEFINE_GUID(IID_ICorProfilerModuleEnum, 0xb0266d75,0x2081,0x4493,0xaf,0x7f,0x02,0x8b,0xa3,0x4d,0xb8,0x91);
DEFINE_GUID(IID_IMethodMalloc, 0xA0EFB28B,0x6EE2,0x4d7b,0xB9,0x83,0xA7,0x5E,0xF7,0xBE,0xED,0xB8);
DEFINE_GUID(IID_ICorProfilerFunctionControl, 0xF0963021,0xE1EA,0x4732,0x85,0x81,0xE0,0x1B,0x0B,0xD3,0xC0,0xC6);
DEFINE_GUID(IID_ICorProfilerInfo4, 0x0d8fdcaa,0x6257,0x47bf,0xb1,0xbf,0x94,0xda,0xc8,0x84,0x66,0xee);
DEFINE_GUID(IID_ICorProfilerInfo5, 0x07602928,0xCE38,0x4B83,0x81,0xE7,0x74,0xAD,0xAF,0x78,0x12,0x14);
DEFINE_GUID(IID_ICorProfilerInfo6, 0xF30A070D,0xBFFB,0x46A7,0xB1,0xD8,0x87,0x81,0xEF,0x7B,0x69,0x8A);
DEFINE_GUID(IID_ICorProfilerInfo7, 0x9AEECC0D,0x63E0,0x4187,0x8C,0x00,0xE3,0x12,0xF5,0x03,0xF6,0x63);
DEFINE_GUID(IID_ICorProfilerInfo8, 0xC5AC80A6,0x782E,0x4716,0x80,0x44,0x39,0x59,0x8C,0x60,0xCF,0xBF);
DEFINE_GUID(IID_ICorProfilerInfo9, 0X008170DB,0XF8CC,0X4796,0X9A,0X51,0XDC,0X8A,0XA0,0XB4,0X70,0x12);
DEFINE_GUID(IID_ICorProfilerInfo10,0X2F1B5152,0XC869,0X40C9,0XAA,0X5F,0X3A,0XBE,0X02,0X6B,0XD7,0X20);
DEFINE_GUID(IID_ICorProfilerInfo11,0x06398876,0x8987,0x4154,0xB6,0x21,0x40,0xA0,0x0D,0x6E,0x4D,0x04);
DEFINE_GUID(IID_ICorProfilerInfo12,0x27b24ccd,0x1cb1,0x47c5,0x96,0xee,0x98,0x19,0x0d,0xc3,0x09,0x59);
DEFINE_GUID(IID_ICorProfilerMethodEnum, 0xFCCEE788,0x0088,0x454B,0xA8,0x11,0xC9,0x9F,0x29,0x8D,0x19,0x42);
DEFINE_GUID(IID_ICorProfilerThreadEnum, 0x571194f7,0x25ed,0x419f,0xaa,0x8b,0x70,0x16,0xb3,0x15,0x97,0x01);
DEFINE_GUID(IID_ICorProfilerAssemblyReferenceProvider, 0x66A78C24,0x2EEF,0x4F65,0xB4,0x5F,0xDD,0x1D,0x80,0x38,0xBF,0x3C);