// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;
// Generated by Fuzzlyn v1.5 on 2022-09-04 15:54:25
// Run on X86 Windows
// Seed: 14105179845188319926
// Reduced from 33.3 KiB to 0.8 KiB in 00:01:22
// Debug: Outputs 0
// Release: Outputs 1

public struct S1
{
    public uint F0;
    public S2 M18(ref int arg0, ulong arg1)
    {
        S1 var6;
        try
        {
        }
        finally
        {
            var6.F0 = Runtime_75249.s_13;
            this = var6;
        }

        return new S2(0);
    }
}

public struct S2
{
    public S1 F0;
    public short F1;
    public S2(short f1): this()
    {
        F1 = f1;
    }
}

public class Runtime_75249
{
    public static byte s_4;
    public static int s_10;
    public static uint s_13 = 1;
    public static int s_19;
    [Fact]
    public static int TestEntryPoint()
    {
        S2 vr1 = new S2(-1);
        byte r = M17(vr1, M17(vr1.F0.M18(ref s_19, 0), s_4));
        return r == 0 ? 100 : -1;
    }

    public static byte M17(S2 arg0, int arg4)
    {
        uint r = arg0.F0.F0;
        System.Console.WriteLine(r);
        return (byte) r;
    }
}
