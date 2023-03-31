// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Generated by Fuzzlyn v1.5 on 2021-11-04 18:29:11
// Run on Arm64 Linux
// Seed: 1922924939431163374
// Reduced from 261.1 KiB to 0.2 KiB in 00:05:39
// Hits JIT assert in Release:
// Assertion failed 'isValidImmShift(lsb, size)' in 'Program:M4():int' during 'Generate code' (IL size 26)
//
// File: /__w/1/s/src/coreclr/jit/emitarm64.cpp Line: 7052
//

using System;
using System.Runtime.CompilerServices;
using Xunit;

public class Runtime_61045
{
    public static byte[] s_1;
    [Fact]
    public static int TestEntryPoint()
    {
        try
        {
            Test();
        }
        catch (NullReferenceException)
        {
            return 100;
        }
        return 101;
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static uint Test()
    {
        return (uint)((ushort)~s_1[0] << (0 >> s_1[0]));
    }
}
