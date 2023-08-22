// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;
// Generated by Fuzzlyn v1.5 on 2022-11-04 01:59:14
// Run on Arm64 Linux
// Seed: 4397362897463137173
// Reduced from 88.3 KiB to 0.6 KiB in 00:01:58
// Hits JIT assert in Release:
// Assertion failed '!"Should not see phi nodes after rationalize"' in 'Program:M37(byte,bool,short)' during 'Lowering nodeinfo' (IL size 42; hash 0x747e58b5; FullOpts)
//
//     File: /__w/1/s/src/coreclr/jit/lower.cpp Line: 6634
//
public class Runtime_77886
{
    public static long[] s_3;
    public static sbyte s_11;
    public static uint s_17;
    public static short s_18;
    [Fact]
    public static void TestEntryPoint()
    {
        bool vr6 = default(bool);
        M37(s_11, vr6, s_18);
    }

    internal static void M37(sbyte arg0, bool arg1, short arg2)
    {
        s_3 = new long[1];

        if (!arg1)
        {
            uint vr8 = s_17;
            arg1 = 0 <= s_3[0];
        }

        if (!arg1)
        {
            arg0 = s_11;
        }

        if (arg1)
        {
            arg2 = 0;
        }
    }
}
