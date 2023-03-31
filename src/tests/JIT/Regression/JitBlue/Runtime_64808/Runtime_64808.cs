// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.CompilerServices;
using Xunit;

// Generated by Fuzzlyn v1.5 on 2022-02-03 20:05:52
// Run on X64 Linux
// Seed: 17645686525285880576
// Reduced from 66.2 KiB to 1.0 KiB in 00:04:27
// Hits JIT assert in Release:
// Assertion failed 'compiler->opts.IsOSR() || ((genInitStkLclCnt > 0) == hasUntrLcl)' in 'Program:M8():S0' during 'Generate code' (IL size 110)
// 
//     File: /__w/1/s/src/coreclr/jit/codegencommon.cpp Line: 7256
// 

public class C0
{
    public int F0;
    public ulong F1;
    public sbyte F2;
}

public struct S0
{
    public int F0;
    public S0(int f0): this()
    {
    }
}

public class Runtime_64808
{
    private static IRT s_rt;
    public static long[] s_16;
    public static bool s_23;
    public static int s_result;
    [Fact]
    public static int TestEntryPoint()
    {
        s_16 = new long[1];
        s_rt = new C();
        s_result = -1;
        M8();
        return s_result;
    }

    public static S0 M8()
    {
        try
        {
            var vr0 = new ushort[]{0};
            C0[] vr3 = default(C0[]);
            return M15(vr0, ref s_23, vr3);
        }
        finally
        {
            var vr2 = new S0[]{new S0(0), new S0(0), new S0(0), new S0(0), new S0(0), new S0(0)};
        }
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static S0 M15(ushort[] arg0, ref bool arg1, C0[] arg2)
    {
        s_result = 100;
        S0 vr5 = default(S0);
        return vr5;
    }
}

public interface IRT
{
    void WriteLine<T>(string a, T value);
}

public class C : IRT
{
    public void WriteLine<T>(string a, T value)
    {
        System.Console.WriteLine(value);
    }
}