// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Generated by Fuzzlyn v1.5 on 2022-07-03 14:40:52
// Run on Arm64 MacOS
// Seed: 5960657379714964908
// Reduced from 328.5 KiB to 0.8 KiB in 00:02:08
// Debug: Outputs 0
// Release: Outputs 1
using System.Runtime.CompilerServices;
using Xunit;

public struct S0
{
    public long F0;
    public long F1;
    public S0(long f1): this()
    {
        F1 = f1;
    }
}

public struct S1
{
    public uint F1;
    public ushort F2;
    public S0 F4;
    public S1(ulong f5): this()
    {
    }

    public long M82(ref short[] arg0)
    {
        Runtime_71600.s_13 = this.F1++;
        S1 var1 = new S1(0);
        return Runtime_71600.s_39[0].F1;
    }
}

public class Runtime_71600
{
    public static uint s_13;
    public static short[] s_28;
    public static S0[] s_39;
    [Fact]
    public static int TestEntryPoint()
    {
        S0[] vr2 = new S0[]{new S0(-1)};
        s_39 = vr2;
        S1 vr3 = default(S1);
        return M80(vr3) == 0 ? 100 : -1;
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static int M80(S1 arg0)
    {
        arg0.F2 += (ushort)(arg0.F1 % (uint)arg0.M82(ref s_28));
        return arg0.F2;
    }
}
