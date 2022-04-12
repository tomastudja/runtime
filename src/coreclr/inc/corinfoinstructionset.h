// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// DO NOT EDIT THIS FILE! IT IS AUTOGENERATED
// FROM /src/coreclr/tools/Common/JitInterface/ThunkGenerator/InstructionSetDesc.txt
// using /src/coreclr/tools/Common/JitInterface/ThunkGenerator/gen.bat

#ifndef CORINFOINSTRUCTIONSET_H
#define CORINFOINSTRUCTIONSET_H

#include "readytoruninstructionset.h"
#include <stdint.h>

enum CORINFO_InstructionSet
{
    InstructionSet_ILLEGAL = 0,
    InstructionSet_NONE = 63,
#ifdef TARGET_ARM64
    InstructionSet_ArmBase=1,
    InstructionSet_AdvSimd=2,
    InstructionSet_Aes=3,
    InstructionSet_Crc32=4,
    InstructionSet_Dp=5,
    InstructionSet_Rdm=6,
    InstructionSet_Sha1=7,
    InstructionSet_Sha256=8,
    InstructionSet_Atomics=9,
    InstructionSet_Vector64=10,
    InstructionSet_Vector128=11,
    InstructionSet_Dczva=12,
    InstructionSet_ArmBase_Arm64=13,
    InstructionSet_AdvSimd_Arm64=14,
    InstructionSet_Aes_Arm64=15,
    InstructionSet_Crc32_Arm64=16,
    InstructionSet_Dp_Arm64=17,
    InstructionSet_Rdm_Arm64=18,
    InstructionSet_Sha1_Arm64=19,
    InstructionSet_Sha256_Arm64=20,
    InstructionSet_Rcpc=21,
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
    InstructionSet_X86Base=1,
    InstructionSet_SSE=2,
    InstructionSet_SSE2=3,
    InstructionSet_SSE3=4,
    InstructionSet_SSSE3=5,
    InstructionSet_SSE41=6,
    InstructionSet_SSE42=7,
    InstructionSet_AVX=8,
    InstructionSet_AVX2=9,
    InstructionSet_AES=10,
    InstructionSet_BMI1=11,
    InstructionSet_BMI2=12,
    InstructionSet_FMA=13,
    InstructionSet_LZCNT=14,
    InstructionSet_PCLMULQDQ=15,
    InstructionSet_POPCNT=16,
    InstructionSet_Vector128=17,
    InstructionSet_Vector256=18,
    InstructionSet_AVXVNNI=19,
    InstructionSet_X86Base_X64=20,
    InstructionSet_SSE_X64=21,
    InstructionSet_SSE2_X64=22,
    InstructionSet_SSE3_X64=23,
    InstructionSet_SSSE3_X64=24,
    InstructionSet_SSE41_X64=25,
    InstructionSet_SSE42_X64=26,
    InstructionSet_AVX_X64=27,
    InstructionSet_AVX2_X64=28,
    InstructionSet_AES_X64=29,
    InstructionSet_BMI1_X64=30,
    InstructionSet_BMI2_X64=31,
    InstructionSet_FMA_X64=32,
    InstructionSet_LZCNT_X64=33,
    InstructionSet_PCLMULQDQ_X64=34,
    InstructionSet_POPCNT_X64=35,
    InstructionSet_AVXVNNI_X64=36,
#endif // TARGET_AMD64
#ifdef TARGET_X86
    InstructionSet_X86Base=1,
    InstructionSet_SSE=2,
    InstructionSet_SSE2=3,
    InstructionSet_SSE3=4,
    InstructionSet_SSSE3=5,
    InstructionSet_SSE41=6,
    InstructionSet_SSE42=7,
    InstructionSet_AVX=8,
    InstructionSet_AVX2=9,
    InstructionSet_AES=10,
    InstructionSet_BMI1=11,
    InstructionSet_BMI2=12,
    InstructionSet_FMA=13,
    InstructionSet_LZCNT=14,
    InstructionSet_PCLMULQDQ=15,
    InstructionSet_POPCNT=16,
    InstructionSet_Vector128=17,
    InstructionSet_Vector256=18,
    InstructionSet_AVXVNNI=19,
    InstructionSet_X86Base_X64=20,
    InstructionSet_SSE_X64=21,
    InstructionSet_SSE2_X64=22,
    InstructionSet_SSE3_X64=23,
    InstructionSet_SSSE3_X64=24,
    InstructionSet_SSE41_X64=25,
    InstructionSet_SSE42_X64=26,
    InstructionSet_AVX_X64=27,
    InstructionSet_AVX2_X64=28,
    InstructionSet_AES_X64=29,
    InstructionSet_BMI1_X64=30,
    InstructionSet_BMI2_X64=31,
    InstructionSet_FMA_X64=32,
    InstructionSet_LZCNT_X64=33,
    InstructionSet_PCLMULQDQ_X64=34,
    InstructionSet_POPCNT_X64=35,
    InstructionSet_AVXVNNI_X64=36,
#endif // TARGET_X86

};

struct CORINFO_InstructionSetFlags
{
private:
    uint64_t _flags = 0;
public:
    void AddInstructionSet(CORINFO_InstructionSet instructionSet)
    {
        _flags = _flags | (((uint64_t)1) << instructionSet);
    }

    void RemoveInstructionSet(CORINFO_InstructionSet instructionSet)
    {
        _flags = _flags & ~(((uint64_t)1) << instructionSet);
    }

    bool HasInstructionSet(CORINFO_InstructionSet instructionSet) const
    {
        return _flags & (((uint64_t)1) << instructionSet);
    }

    bool Equals(CORINFO_InstructionSetFlags other) const
    {
        return _flags == other._flags;
    }

    void Add(CORINFO_InstructionSetFlags other)
    {
        _flags |= other._flags;
    }

    bool IsEmpty() const
    {
        return _flags == 0;
    }

    void Reset()
    {
        _flags = 0;
    }

    void Set64BitInstructionSetVariants()
    {
#ifdef TARGET_ARM64
        if (HasInstructionSet(InstructionSet_ArmBase))
            AddInstructionSet(InstructionSet_ArmBase_Arm64);
        if (HasInstructionSet(InstructionSet_AdvSimd))
            AddInstructionSet(InstructionSet_AdvSimd_Arm64);
        if (HasInstructionSet(InstructionSet_Aes))
            AddInstructionSet(InstructionSet_Aes_Arm64);
        if (HasInstructionSet(InstructionSet_Crc32))
            AddInstructionSet(InstructionSet_Crc32_Arm64);
        if (HasInstructionSet(InstructionSet_Dp))
            AddInstructionSet(InstructionSet_Dp_Arm64);
        if (HasInstructionSet(InstructionSet_Rdm))
            AddInstructionSet(InstructionSet_Rdm_Arm64);
        if (HasInstructionSet(InstructionSet_Sha1))
            AddInstructionSet(InstructionSet_Sha1_Arm64);
        if (HasInstructionSet(InstructionSet_Sha256))
            AddInstructionSet(InstructionSet_Sha256_Arm64);
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
        if (HasInstructionSet(InstructionSet_X86Base))
            AddInstructionSet(InstructionSet_X86Base_X64);
        if (HasInstructionSet(InstructionSet_SSE))
            AddInstructionSet(InstructionSet_SSE_X64);
        if (HasInstructionSet(InstructionSet_SSE2))
            AddInstructionSet(InstructionSet_SSE2_X64);
        if (HasInstructionSet(InstructionSet_SSE3))
            AddInstructionSet(InstructionSet_SSE3_X64);
        if (HasInstructionSet(InstructionSet_SSSE3))
            AddInstructionSet(InstructionSet_SSSE3_X64);
        if (HasInstructionSet(InstructionSet_SSE41))
            AddInstructionSet(InstructionSet_SSE41_X64);
        if (HasInstructionSet(InstructionSet_SSE42))
            AddInstructionSet(InstructionSet_SSE42_X64);
        if (HasInstructionSet(InstructionSet_AVX))
            AddInstructionSet(InstructionSet_AVX_X64);
        if (HasInstructionSet(InstructionSet_AVX2))
            AddInstructionSet(InstructionSet_AVX2_X64);
        if (HasInstructionSet(InstructionSet_AES))
            AddInstructionSet(InstructionSet_AES_X64);
        if (HasInstructionSet(InstructionSet_BMI1))
            AddInstructionSet(InstructionSet_BMI1_X64);
        if (HasInstructionSet(InstructionSet_BMI2))
            AddInstructionSet(InstructionSet_BMI2_X64);
        if (HasInstructionSet(InstructionSet_FMA))
            AddInstructionSet(InstructionSet_FMA_X64);
        if (HasInstructionSet(InstructionSet_LZCNT))
            AddInstructionSet(InstructionSet_LZCNT_X64);
        if (HasInstructionSet(InstructionSet_PCLMULQDQ))
            AddInstructionSet(InstructionSet_PCLMULQDQ_X64);
        if (HasInstructionSet(InstructionSet_POPCNT))
            AddInstructionSet(InstructionSet_POPCNT_X64);
        if (HasInstructionSet(InstructionSet_AVXVNNI))
            AddInstructionSet(InstructionSet_AVXVNNI_X64);
#endif // TARGET_AMD64
#ifdef TARGET_X86
#endif // TARGET_X86

    }

    uint64_t GetFlagsRaw()
    {
        return _flags;
    }

    void SetFromFlagsRaw(uint64_t flags)
    {
        _flags = flags;
    }
};

inline CORINFO_InstructionSetFlags EnsureInstructionSetFlagsAreValid(CORINFO_InstructionSetFlags input)
{
    CORINFO_InstructionSetFlags oldflags = input;
    CORINFO_InstructionSetFlags resultflags = input;
    do
    {
        oldflags = resultflags;
#ifdef TARGET_ARM64
        if (resultflags.HasInstructionSet(InstructionSet_ArmBase) && !resultflags.HasInstructionSet(InstructionSet_ArmBase_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_ArmBase);
        if (resultflags.HasInstructionSet(InstructionSet_ArmBase_Arm64) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_ArmBase_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_AdvSimd) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_AdvSimd);
        if (resultflags.HasInstructionSet(InstructionSet_AdvSimd_Arm64) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd))
            resultflags.RemoveInstructionSet(InstructionSet_AdvSimd_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_Aes) && !resultflags.HasInstructionSet(InstructionSet_Aes_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Aes);
        if (resultflags.HasInstructionSet(InstructionSet_Aes_Arm64) && !resultflags.HasInstructionSet(InstructionSet_Aes))
            resultflags.RemoveInstructionSet(InstructionSet_Aes_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_Crc32) && !resultflags.HasInstructionSet(InstructionSet_Crc32_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Crc32);
        if (resultflags.HasInstructionSet(InstructionSet_Crc32_Arm64) && !resultflags.HasInstructionSet(InstructionSet_Crc32))
            resultflags.RemoveInstructionSet(InstructionSet_Crc32_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_Dp) && !resultflags.HasInstructionSet(InstructionSet_Dp_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Dp);
        if (resultflags.HasInstructionSet(InstructionSet_Dp_Arm64) && !resultflags.HasInstructionSet(InstructionSet_Dp))
            resultflags.RemoveInstructionSet(InstructionSet_Dp_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_Rdm) && !resultflags.HasInstructionSet(InstructionSet_Rdm_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Rdm);
        if (resultflags.HasInstructionSet(InstructionSet_Rdm_Arm64) && !resultflags.HasInstructionSet(InstructionSet_Rdm))
            resultflags.RemoveInstructionSet(InstructionSet_Rdm_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_Sha1) && !resultflags.HasInstructionSet(InstructionSet_Sha1_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Sha1);
        if (resultflags.HasInstructionSet(InstructionSet_Sha1_Arm64) && !resultflags.HasInstructionSet(InstructionSet_Sha1))
            resultflags.RemoveInstructionSet(InstructionSet_Sha1_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_Sha256) && !resultflags.HasInstructionSet(InstructionSet_Sha256_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Sha256);
        if (resultflags.HasInstructionSet(InstructionSet_Sha256_Arm64) && !resultflags.HasInstructionSet(InstructionSet_Sha256))
            resultflags.RemoveInstructionSet(InstructionSet_Sha256_Arm64);
        if (resultflags.HasInstructionSet(InstructionSet_AdvSimd) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_AdvSimd);
        if (resultflags.HasInstructionSet(InstructionSet_Aes) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Aes);
        if (resultflags.HasInstructionSet(InstructionSet_Crc32) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Crc32);
        if (resultflags.HasInstructionSet(InstructionSet_Dp) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd))
            resultflags.RemoveInstructionSet(InstructionSet_Dp);
        if (resultflags.HasInstructionSet(InstructionSet_Rdm) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd))
            resultflags.RemoveInstructionSet(InstructionSet_Rdm);
        if (resultflags.HasInstructionSet(InstructionSet_Sha1) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Sha1);
        if (resultflags.HasInstructionSet(InstructionSet_Sha256) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Sha256);
        if (resultflags.HasInstructionSet(InstructionSet_Vector64) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd))
            resultflags.RemoveInstructionSet(InstructionSet_Vector64);
        if (resultflags.HasInstructionSet(InstructionSet_Vector128) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd))
            resultflags.RemoveInstructionSet(InstructionSet_Vector128);
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
        if (resultflags.HasInstructionSet(InstructionSet_X86Base) && !resultflags.HasInstructionSet(InstructionSet_X86Base_X64))
            resultflags.RemoveInstructionSet(InstructionSet_X86Base);
        if (resultflags.HasInstructionSet(InstructionSet_X86Base_X64) && !resultflags.HasInstructionSet(InstructionSet_X86Base))
            resultflags.RemoveInstructionSet(InstructionSet_X86Base_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSE) && !resultflags.HasInstructionSet(InstructionSet_SSE_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE);
        if (resultflags.HasInstructionSet(InstructionSet_SSE_X64) && !resultflags.HasInstructionSet(InstructionSet_SSE))
            resultflags.RemoveInstructionSet(InstructionSet_SSE_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSE2) && !resultflags.HasInstructionSet(InstructionSet_SSE2_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE2);
        if (resultflags.HasInstructionSet(InstructionSet_SSE2_X64) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_SSE2_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSE3) && !resultflags.HasInstructionSet(InstructionSet_SSE3_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE3);
        if (resultflags.HasInstructionSet(InstructionSet_SSE3_X64) && !resultflags.HasInstructionSet(InstructionSet_SSE3))
            resultflags.RemoveInstructionSet(InstructionSet_SSE3_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSSE3) && !resultflags.HasInstructionSet(InstructionSet_SSSE3_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSSE3);
        if (resultflags.HasInstructionSet(InstructionSet_SSSE3_X64) && !resultflags.HasInstructionSet(InstructionSet_SSSE3))
            resultflags.RemoveInstructionSet(InstructionSet_SSSE3_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSE41) && !resultflags.HasInstructionSet(InstructionSet_SSE41_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE41);
        if (resultflags.HasInstructionSet(InstructionSet_SSE41_X64) && !resultflags.HasInstructionSet(InstructionSet_SSE41))
            resultflags.RemoveInstructionSet(InstructionSet_SSE41_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSE42) && !resultflags.HasInstructionSet(InstructionSet_SSE42_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE42);
        if (resultflags.HasInstructionSet(InstructionSet_SSE42_X64) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_SSE42_X64);
        if (resultflags.HasInstructionSet(InstructionSet_AVX) && !resultflags.HasInstructionSet(InstructionSet_AVX_X64))
            resultflags.RemoveInstructionSet(InstructionSet_AVX);
        if (resultflags.HasInstructionSet(InstructionSet_AVX_X64) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_AVX_X64);
        if (resultflags.HasInstructionSet(InstructionSet_AVX2) && !resultflags.HasInstructionSet(InstructionSet_AVX2_X64))
            resultflags.RemoveInstructionSet(InstructionSet_AVX2);
        if (resultflags.HasInstructionSet(InstructionSet_AVX2_X64) && !resultflags.HasInstructionSet(InstructionSet_AVX2))
            resultflags.RemoveInstructionSet(InstructionSet_AVX2_X64);
        if (resultflags.HasInstructionSet(InstructionSet_AES) && !resultflags.HasInstructionSet(InstructionSet_AES_X64))
            resultflags.RemoveInstructionSet(InstructionSet_AES);
        if (resultflags.HasInstructionSet(InstructionSet_AES_X64) && !resultflags.HasInstructionSet(InstructionSet_AES))
            resultflags.RemoveInstructionSet(InstructionSet_AES_X64);
        if (resultflags.HasInstructionSet(InstructionSet_BMI1) && !resultflags.HasInstructionSet(InstructionSet_BMI1_X64))
            resultflags.RemoveInstructionSet(InstructionSet_BMI1);
        if (resultflags.HasInstructionSet(InstructionSet_BMI1_X64) && !resultflags.HasInstructionSet(InstructionSet_BMI1))
            resultflags.RemoveInstructionSet(InstructionSet_BMI1_X64);
        if (resultflags.HasInstructionSet(InstructionSet_BMI2) && !resultflags.HasInstructionSet(InstructionSet_BMI2_X64))
            resultflags.RemoveInstructionSet(InstructionSet_BMI2);
        if (resultflags.HasInstructionSet(InstructionSet_BMI2_X64) && !resultflags.HasInstructionSet(InstructionSet_BMI2))
            resultflags.RemoveInstructionSet(InstructionSet_BMI2_X64);
        if (resultflags.HasInstructionSet(InstructionSet_FMA) && !resultflags.HasInstructionSet(InstructionSet_FMA_X64))
            resultflags.RemoveInstructionSet(InstructionSet_FMA);
        if (resultflags.HasInstructionSet(InstructionSet_FMA_X64) && !resultflags.HasInstructionSet(InstructionSet_FMA))
            resultflags.RemoveInstructionSet(InstructionSet_FMA_X64);
        if (resultflags.HasInstructionSet(InstructionSet_LZCNT) && !resultflags.HasInstructionSet(InstructionSet_LZCNT_X64))
            resultflags.RemoveInstructionSet(InstructionSet_LZCNT);
        if (resultflags.HasInstructionSet(InstructionSet_LZCNT_X64) && !resultflags.HasInstructionSet(InstructionSet_LZCNT))
            resultflags.RemoveInstructionSet(InstructionSet_LZCNT_X64);
        if (resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ) && !resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ_X64))
            resultflags.RemoveInstructionSet(InstructionSet_PCLMULQDQ);
        if (resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ_X64) && !resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ))
            resultflags.RemoveInstructionSet(InstructionSet_PCLMULQDQ_X64);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT) && !resultflags.HasInstructionSet(InstructionSet_POPCNT_X64))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT_X64) && !resultflags.HasInstructionSet(InstructionSet_POPCNT))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT_X64);
        if (resultflags.HasInstructionSet(InstructionSet_AVXVNNI) && !resultflags.HasInstructionSet(InstructionSet_AVXVNNI_X64))
            resultflags.RemoveInstructionSet(InstructionSet_AVXVNNI);
        if (resultflags.HasInstructionSet(InstructionSet_AVXVNNI_X64) && !resultflags.HasInstructionSet(InstructionSet_AVXVNNI))
            resultflags.RemoveInstructionSet(InstructionSet_AVXVNNI_X64);
        if (resultflags.HasInstructionSet(InstructionSet_SSE) && !resultflags.HasInstructionSet(InstructionSet_X86Base))
            resultflags.RemoveInstructionSet(InstructionSet_SSE);
        if (resultflags.HasInstructionSet(InstructionSet_SSE2) && !resultflags.HasInstructionSet(InstructionSet_SSE))
            resultflags.RemoveInstructionSet(InstructionSet_SSE2);
        if (resultflags.HasInstructionSet(InstructionSet_SSE3) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_SSE3);
        if (resultflags.HasInstructionSet(InstructionSet_SSSE3) && !resultflags.HasInstructionSet(InstructionSet_SSE3))
            resultflags.RemoveInstructionSet(InstructionSet_SSSE3);
        if (resultflags.HasInstructionSet(InstructionSet_SSE41) && !resultflags.HasInstructionSet(InstructionSet_SSSE3))
            resultflags.RemoveInstructionSet(InstructionSet_SSE41);
        if (resultflags.HasInstructionSet(InstructionSet_SSE42) && !resultflags.HasInstructionSet(InstructionSet_SSE41))
            resultflags.RemoveInstructionSet(InstructionSet_SSE42);
        if (resultflags.HasInstructionSet(InstructionSet_AVX) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_AVX);
        if (resultflags.HasInstructionSet(InstructionSet_AVX2) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_AVX2);
        if (resultflags.HasInstructionSet(InstructionSet_AES) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_AES);
        if (resultflags.HasInstructionSet(InstructionSet_BMI1) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_BMI1);
        if (resultflags.HasInstructionSet(InstructionSet_BMI2) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_BMI2);
        if (resultflags.HasInstructionSet(InstructionSet_FMA) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_FMA);
        if (resultflags.HasInstructionSet(InstructionSet_LZCNT) && !resultflags.HasInstructionSet(InstructionSet_X86Base))
            resultflags.RemoveInstructionSet(InstructionSet_LZCNT);
        if (resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_PCLMULQDQ);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT);
        if (resultflags.HasInstructionSet(InstructionSet_Vector128) && !resultflags.HasInstructionSet(InstructionSet_SSE))
            resultflags.RemoveInstructionSet(InstructionSet_Vector128);
        if (resultflags.HasInstructionSet(InstructionSet_Vector256) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_Vector256);
        if (resultflags.HasInstructionSet(InstructionSet_AVXVNNI) && !resultflags.HasInstructionSet(InstructionSet_AVX2))
            resultflags.RemoveInstructionSet(InstructionSet_AVXVNNI);
#endif // TARGET_AMD64
#ifdef TARGET_X86
        if (resultflags.HasInstructionSet(InstructionSet_SSE) && !resultflags.HasInstructionSet(InstructionSet_X86Base))
            resultflags.RemoveInstructionSet(InstructionSet_SSE);
        if (resultflags.HasInstructionSet(InstructionSet_SSE2) && !resultflags.HasInstructionSet(InstructionSet_SSE))
            resultflags.RemoveInstructionSet(InstructionSet_SSE2);
        if (resultflags.HasInstructionSet(InstructionSet_SSE3) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_SSE3);
        if (resultflags.HasInstructionSet(InstructionSet_SSSE3) && !resultflags.HasInstructionSet(InstructionSet_SSE3))
            resultflags.RemoveInstructionSet(InstructionSet_SSSE3);
        if (resultflags.HasInstructionSet(InstructionSet_SSE41) && !resultflags.HasInstructionSet(InstructionSet_SSSE3))
            resultflags.RemoveInstructionSet(InstructionSet_SSE41);
        if (resultflags.HasInstructionSet(InstructionSet_SSE42) && !resultflags.HasInstructionSet(InstructionSet_SSE41))
            resultflags.RemoveInstructionSet(InstructionSet_SSE42);
        if (resultflags.HasInstructionSet(InstructionSet_AVX) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_AVX);
        if (resultflags.HasInstructionSet(InstructionSet_AVX2) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_AVX2);
        if (resultflags.HasInstructionSet(InstructionSet_AES) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_AES);
        if (resultflags.HasInstructionSet(InstructionSet_BMI1) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_BMI1);
        if (resultflags.HasInstructionSet(InstructionSet_BMI2) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_BMI2);
        if (resultflags.HasInstructionSet(InstructionSet_FMA) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_FMA);
        if (resultflags.HasInstructionSet(InstructionSet_LZCNT) && !resultflags.HasInstructionSet(InstructionSet_X86Base))
            resultflags.RemoveInstructionSet(InstructionSet_LZCNT);
        if (resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_PCLMULQDQ);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT);
        if (resultflags.HasInstructionSet(InstructionSet_Vector128) && !resultflags.HasInstructionSet(InstructionSet_SSE))
            resultflags.RemoveInstructionSet(InstructionSet_Vector128);
        if (resultflags.HasInstructionSet(InstructionSet_Vector256) && !resultflags.HasInstructionSet(InstructionSet_AVX))
            resultflags.RemoveInstructionSet(InstructionSet_Vector256);
        if (resultflags.HasInstructionSet(InstructionSet_AVXVNNI) && !resultflags.HasInstructionSet(InstructionSet_AVX2))
            resultflags.RemoveInstructionSet(InstructionSet_AVXVNNI);
#endif // TARGET_X86

    } while (!oldflags.Equals(resultflags));
    return resultflags;
}

inline const char *InstructionSetToString(CORINFO_InstructionSet instructionSet)
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4065) // disable warning for switch statement with only default label.
#endif

    switch (instructionSet)
    {
#ifdef TARGET_ARM64
        case InstructionSet_ArmBase :
            return "ArmBase";
        case InstructionSet_ArmBase_Arm64 :
            return "ArmBase_Arm64";
        case InstructionSet_AdvSimd :
            return "AdvSimd";
        case InstructionSet_AdvSimd_Arm64 :
            return "AdvSimd_Arm64";
        case InstructionSet_Aes :
            return "Aes";
        case InstructionSet_Aes_Arm64 :
            return "Aes_Arm64";
        case InstructionSet_Crc32 :
            return "Crc32";
        case InstructionSet_Crc32_Arm64 :
            return "Crc32_Arm64";
        case InstructionSet_Dp :
            return "Dp";
        case InstructionSet_Dp_Arm64 :
            return "Dp_Arm64";
        case InstructionSet_Rdm :
            return "Rdm";
        case InstructionSet_Rdm_Arm64 :
            return "Rdm_Arm64";
        case InstructionSet_Sha1 :
            return "Sha1";
        case InstructionSet_Sha1_Arm64 :
            return "Sha1_Arm64";
        case InstructionSet_Sha256 :
            return "Sha256";
        case InstructionSet_Sha256_Arm64 :
            return "Sha256_Arm64";
        case InstructionSet_Atomics :
            return "Atomics";
        case InstructionSet_Vector64 :
            return "Vector64";
        case InstructionSet_Vector128 :
            return "Vector128";
        case InstructionSet_Dczva :
            return "Dczva";
        case InstructionSet_Rcpc :
            return "Rcpc";
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
        case InstructionSet_X86Base :
            return "X86Base";
        case InstructionSet_X86Base_X64 :
            return "X86Base_X64";
        case InstructionSet_SSE :
            return "SSE";
        case InstructionSet_SSE_X64 :
            return "SSE_X64";
        case InstructionSet_SSE2 :
            return "SSE2";
        case InstructionSet_SSE2_X64 :
            return "SSE2_X64";
        case InstructionSet_SSE3 :
            return "SSE3";
        case InstructionSet_SSE3_X64 :
            return "SSE3_X64";
        case InstructionSet_SSSE3 :
            return "SSSE3";
        case InstructionSet_SSSE3_X64 :
            return "SSSE3_X64";
        case InstructionSet_SSE41 :
            return "SSE41";
        case InstructionSet_SSE41_X64 :
            return "SSE41_X64";
        case InstructionSet_SSE42 :
            return "SSE42";
        case InstructionSet_SSE42_X64 :
            return "SSE42_X64";
        case InstructionSet_AVX :
            return "AVX";
        case InstructionSet_AVX_X64 :
            return "AVX_X64";
        case InstructionSet_AVX2 :
            return "AVX2";
        case InstructionSet_AVX2_X64 :
            return "AVX2_X64";
        case InstructionSet_AES :
            return "AES";
        case InstructionSet_AES_X64 :
            return "AES_X64";
        case InstructionSet_BMI1 :
            return "BMI1";
        case InstructionSet_BMI1_X64 :
            return "BMI1_X64";
        case InstructionSet_BMI2 :
            return "BMI2";
        case InstructionSet_BMI2_X64 :
            return "BMI2_X64";
        case InstructionSet_FMA :
            return "FMA";
        case InstructionSet_FMA_X64 :
            return "FMA_X64";
        case InstructionSet_LZCNT :
            return "LZCNT";
        case InstructionSet_LZCNT_X64 :
            return "LZCNT_X64";
        case InstructionSet_PCLMULQDQ :
            return "PCLMULQDQ";
        case InstructionSet_PCLMULQDQ_X64 :
            return "PCLMULQDQ_X64";
        case InstructionSet_POPCNT :
            return "POPCNT";
        case InstructionSet_POPCNT_X64 :
            return "POPCNT_X64";
        case InstructionSet_Vector128 :
            return "Vector128";
        case InstructionSet_Vector256 :
            return "Vector256";
        case InstructionSet_AVXVNNI :
            return "AVXVNNI";
        case InstructionSet_AVXVNNI_X64 :
            return "AVXVNNI_X64";
#endif // TARGET_AMD64
#ifdef TARGET_X86
        case InstructionSet_X86Base :
            return "X86Base";
        case InstructionSet_SSE :
            return "SSE";
        case InstructionSet_SSE2 :
            return "SSE2";
        case InstructionSet_SSE3 :
            return "SSE3";
        case InstructionSet_SSSE3 :
            return "SSSE3";
        case InstructionSet_SSE41 :
            return "SSE41";
        case InstructionSet_SSE42 :
            return "SSE42";
        case InstructionSet_AVX :
            return "AVX";
        case InstructionSet_AVX2 :
            return "AVX2";
        case InstructionSet_AES :
            return "AES";
        case InstructionSet_BMI1 :
            return "BMI1";
        case InstructionSet_BMI2 :
            return "BMI2";
        case InstructionSet_FMA :
            return "FMA";
        case InstructionSet_LZCNT :
            return "LZCNT";
        case InstructionSet_PCLMULQDQ :
            return "PCLMULQDQ";
        case InstructionSet_POPCNT :
            return "POPCNT";
        case InstructionSet_Vector128 :
            return "Vector128";
        case InstructionSet_Vector256 :
            return "Vector256";
        case InstructionSet_AVXVNNI :
            return "AVXVNNI";
#endif // TARGET_X86

        default:
            return "UnknownInstructionSet";
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

inline CORINFO_InstructionSet InstructionSetFromR2RInstructionSet(ReadyToRunInstructionSet r2rSet)
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4065) // disable warning for switch statement with only default label.
#endif

    switch (r2rSet)
    {
#ifdef TARGET_ARM64
        case READYTORUN_INSTRUCTION_ArmBase: return InstructionSet_ArmBase;
        case READYTORUN_INSTRUCTION_AdvSimd: return InstructionSet_AdvSimd;
        case READYTORUN_INSTRUCTION_Aes: return InstructionSet_Aes;
        case READYTORUN_INSTRUCTION_Crc32: return InstructionSet_Crc32;
        case READYTORUN_INSTRUCTION_Dp: return InstructionSet_Dp;
        case READYTORUN_INSTRUCTION_Rdm: return InstructionSet_Rdm;
        case READYTORUN_INSTRUCTION_Sha1: return InstructionSet_Sha1;
        case READYTORUN_INSTRUCTION_Sha256: return InstructionSet_Sha256;
        case READYTORUN_INSTRUCTION_Atomics: return InstructionSet_Atomics;
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
        case READYTORUN_INSTRUCTION_X86Base: return InstructionSet_X86Base;
        case READYTORUN_INSTRUCTION_Sse: return InstructionSet_SSE;
        case READYTORUN_INSTRUCTION_Sse2: return InstructionSet_SSE2;
        case READYTORUN_INSTRUCTION_Sse3: return InstructionSet_SSE3;
        case READYTORUN_INSTRUCTION_Ssse3: return InstructionSet_SSSE3;
        case READYTORUN_INSTRUCTION_Sse41: return InstructionSet_SSE41;
        case READYTORUN_INSTRUCTION_Sse42: return InstructionSet_SSE42;
        case READYTORUN_INSTRUCTION_Avx: return InstructionSet_AVX;
        case READYTORUN_INSTRUCTION_Avx2: return InstructionSet_AVX2;
        case READYTORUN_INSTRUCTION_Aes: return InstructionSet_AES;
        case READYTORUN_INSTRUCTION_Bmi1: return InstructionSet_BMI1;
        case READYTORUN_INSTRUCTION_Bmi2: return InstructionSet_BMI2;
        case READYTORUN_INSTRUCTION_Fma: return InstructionSet_FMA;
        case READYTORUN_INSTRUCTION_Lzcnt: return InstructionSet_LZCNT;
        case READYTORUN_INSTRUCTION_Pclmulqdq: return InstructionSet_PCLMULQDQ;
        case READYTORUN_INSTRUCTION_Popcnt: return InstructionSet_POPCNT;
        case READYTORUN_INSTRUCTION_AvxVnni: return InstructionSet_AVXVNNI;
#endif // TARGET_AMD64
#ifdef TARGET_X86
        case READYTORUN_INSTRUCTION_X86Base: return InstructionSet_X86Base;
        case READYTORUN_INSTRUCTION_Sse: return InstructionSet_SSE;
        case READYTORUN_INSTRUCTION_Sse2: return InstructionSet_SSE2;
        case READYTORUN_INSTRUCTION_Sse3: return InstructionSet_SSE3;
        case READYTORUN_INSTRUCTION_Ssse3: return InstructionSet_SSSE3;
        case READYTORUN_INSTRUCTION_Sse41: return InstructionSet_SSE41;
        case READYTORUN_INSTRUCTION_Sse42: return InstructionSet_SSE42;
        case READYTORUN_INSTRUCTION_Avx: return InstructionSet_AVX;
        case READYTORUN_INSTRUCTION_Avx2: return InstructionSet_AVX2;
        case READYTORUN_INSTRUCTION_Aes: return InstructionSet_AES;
        case READYTORUN_INSTRUCTION_Bmi1: return InstructionSet_BMI1;
        case READYTORUN_INSTRUCTION_Bmi2: return InstructionSet_BMI2;
        case READYTORUN_INSTRUCTION_Fma: return InstructionSet_FMA;
        case READYTORUN_INSTRUCTION_Lzcnt: return InstructionSet_LZCNT;
        case READYTORUN_INSTRUCTION_Pclmulqdq: return InstructionSet_PCLMULQDQ;
        case READYTORUN_INSTRUCTION_Popcnt: return InstructionSet_POPCNT;
        case READYTORUN_INSTRUCTION_AvxVnni: return InstructionSet_AVXVNNI;
#endif // TARGET_X86

        default:
            return InstructionSet_ILLEGAL;
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

#endif // CORINFOINSTRUCTIONSET_H
