
// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

// DO NOT EDIT THIS FILE! IT IS AUTOGENERATED
// FROM /src/coreclr/src/tools/Common/JitInterface/ThunkGenerator/InstructionSetDesc.txt
// using /src/coreclr/src/tools/Common/JitInterface/ThunkGenerator/gen.bat

#ifndef CORINFOINSTRUCTIONSET_H
#define CORINFOINSTRUCTIONSET_H

enum CORINFO_InstructionSet
{
    InstructionSet_ILLEGAL = 0,
    InstructionSet_NONE = 63,
#ifdef TARGET_ARM64
    InstructionSet_ArmBase=1,
    InstructionSet_ArmBase_Arm64=2,
    InstructionSet_AdvSimd=3,
    InstructionSet_AdvSimd_Arm64=4,
    InstructionSet_Aes=5,
    InstructionSet_Crc32=6,
    InstructionSet_Crc32_Arm64=7,
    InstructionSet_Sha1=8,
    InstructionSet_Sha256=9,
    InstructionSet_Atomics=10,
    InstructionSet_Vector64=11,
    InstructionSet_Vector128=12,
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
    InstructionSet_SSE=1,
    InstructionSet_SSE2=2,
    InstructionSet_SSE3=3,
    InstructionSet_SSSE3=4,
    InstructionSet_SSE41=5,
    InstructionSet_SSE42=6,
    InstructionSet_AVX=7,
    InstructionSet_AVX2=8,
    InstructionSet_AES=9,
    InstructionSet_BMI1=10,
    InstructionSet_BMI2=11,
    InstructionSet_FMA=12,
    InstructionSet_LZCNT=13,
    InstructionSet_PCLMULQDQ=14,
    InstructionSet_POPCNT=15,
    InstructionSet_Vector128=16,
    InstructionSet_Vector256=17,
    InstructionSet_BMI1_X64=18,
    InstructionSet_BMI2_X64=19,
    InstructionSet_LZCNT_X64=20,
    InstructionSet_POPCNT_X64=21,
    InstructionSet_SSE_X64=22,
    InstructionSet_SSE2_X64=23,
    InstructionSet_SSE41_X64=24,
    InstructionSet_SSE42_X64=25,
#endif // TARGET_AMD64
#ifdef TARGET_X86
    InstructionSet_SSE=1,
    InstructionSet_SSE2=2,
    InstructionSet_SSE3=3,
    InstructionSet_SSSE3=4,
    InstructionSet_SSE41=5,
    InstructionSet_SSE42=6,
    InstructionSet_AVX=7,
    InstructionSet_AVX2=8,
    InstructionSet_AES=9,
    InstructionSet_BMI1=10,
    InstructionSet_BMI2=11,
    InstructionSet_FMA=12,
    InstructionSet_LZCNT=13,
    InstructionSet_PCLMULQDQ=14,
    InstructionSet_POPCNT=15,
    InstructionSet_Vector128=16,
    InstructionSet_Vector256=17,
    InstructionSet_BMI1_X64=18,
    InstructionSet_BMI2_X64=19,
    InstructionSet_LZCNT_X64=20,
    InstructionSet_POPCNT_X64=21,
    InstructionSet_SSE_X64=22,
    InstructionSet_SSE2_X64=23,
    InstructionSet_SSE41_X64=24,
    InstructionSet_SSE42_X64=25,
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
        if (HasInstructionSet(InstructionSet_Crc32))
            AddInstructionSet(InstructionSet_Crc32_Arm64);
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
        if (HasInstructionSet(InstructionSet_SSE))
            AddInstructionSet(InstructionSet_SSE_X64);
        if (HasInstructionSet(InstructionSet_SSE2))
            AddInstructionSet(InstructionSet_SSE2_X64);
        if (HasInstructionSet(InstructionSet_SSE41))
            AddInstructionSet(InstructionSet_SSE41_X64);
        if (HasInstructionSet(InstructionSet_SSE42))
            AddInstructionSet(InstructionSet_SSE42_X64);
        if (HasInstructionSet(InstructionSet_BMI1))
            AddInstructionSet(InstructionSet_BMI1_X64);
        if (HasInstructionSet(InstructionSet_BMI2))
            AddInstructionSet(InstructionSet_BMI2_X64);
        if (HasInstructionSet(InstructionSet_LZCNT))
            AddInstructionSet(InstructionSet_LZCNT_X64);
        if (HasInstructionSet(InstructionSet_POPCNT))
            AddInstructionSet(InstructionSet_POPCNT_X64);
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
        if (resultflags.HasInstructionSet(InstructionSet_AdvSimd) && !resultflags.HasInstructionSet(InstructionSet_AdvSimd_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_AdvSimd);
        if (resultflags.HasInstructionSet(InstructionSet_Crc32) && !resultflags.HasInstructionSet(InstructionSet_Crc32_Arm64))
            resultflags.RemoveInstructionSet(InstructionSet_Crc32);
        if (resultflags.HasInstructionSet(InstructionSet_AdvSimd) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_AdvSimd);
        if (resultflags.HasInstructionSet(InstructionSet_Aes) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Aes);
        if (resultflags.HasInstructionSet(InstructionSet_Crc32) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Crc32);
        if (resultflags.HasInstructionSet(InstructionSet_Sha1) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Sha1);
        if (resultflags.HasInstructionSet(InstructionSet_Sha256) && !resultflags.HasInstructionSet(InstructionSet_ArmBase))
            resultflags.RemoveInstructionSet(InstructionSet_Sha256);
#endif // TARGET_ARM64
#ifdef TARGET_AMD64
        if (resultflags.HasInstructionSet(InstructionSet_SSE) && !resultflags.HasInstructionSet(InstructionSet_SSE_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE);
        if (resultflags.HasInstructionSet(InstructionSet_SSE2) && !resultflags.HasInstructionSet(InstructionSet_SSE2_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE2);
        if (resultflags.HasInstructionSet(InstructionSet_SSE41) && !resultflags.HasInstructionSet(InstructionSet_SSE41_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE41);
        if (resultflags.HasInstructionSet(InstructionSet_SSE42) && !resultflags.HasInstructionSet(InstructionSet_SSE42_X64))
            resultflags.RemoveInstructionSet(InstructionSet_SSE42);
        if (resultflags.HasInstructionSet(InstructionSet_BMI1) && !resultflags.HasInstructionSet(InstructionSet_BMI1_X64))
            resultflags.RemoveInstructionSet(InstructionSet_BMI1);
        if (resultflags.HasInstructionSet(InstructionSet_BMI2) && !resultflags.HasInstructionSet(InstructionSet_BMI2_X64))
            resultflags.RemoveInstructionSet(InstructionSet_BMI2);
        if (resultflags.HasInstructionSet(InstructionSet_LZCNT) && !resultflags.HasInstructionSet(InstructionSet_LZCNT_X64))
            resultflags.RemoveInstructionSet(InstructionSet_LZCNT);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT) && !resultflags.HasInstructionSet(InstructionSet_POPCNT_X64))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT);
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
        if (resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_PCLMULQDQ);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT);
#endif // TARGET_AMD64
#ifdef TARGET_X86
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
        if (resultflags.HasInstructionSet(InstructionSet_PCLMULQDQ) && !resultflags.HasInstructionSet(InstructionSet_SSE2))
            resultflags.RemoveInstructionSet(InstructionSet_PCLMULQDQ);
        if (resultflags.HasInstructionSet(InstructionSet_POPCNT) && !resultflags.HasInstructionSet(InstructionSet_SSE42))
            resultflags.RemoveInstructionSet(InstructionSet_POPCNT);
#endif // TARGET_X86

    } while (!oldflags.Equals(resultflags));
    return resultflags;
}



#endif // CORINFOINSTRUCTIONSET_H
