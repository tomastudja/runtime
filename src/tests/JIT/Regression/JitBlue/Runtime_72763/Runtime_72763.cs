// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Adapted from:
// Generated by Fuzzlyn v1.5 on 2022-07-24 16:33:22
// Run on Arm64 MacOS
// Seed: 17376504353311511128
// Reduced from 274.3 KiB to 0.7 KiB in 00:03:55
// Debug: Outputs 1286
// Release: Outputs 1464
using System.Runtime.CompilerServices;

public class Program
{
    public static ushort s_41 = 1646;
    public static int Main()
    {
        int result = Foo(0, 0);
        return result == 38784 ? 100 : -1;
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static int Foo(ushort arg0, ushort arg1)
    {
        arg1 = (ushort)(18350688693879804538UL ^ (ulong)Zero(0));
        arg0 = arg1;
        arg1 %= s_41;
        return arg0 + arg1;
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static short Zero(byte arg2)
    {
        return 0;
    }
}
