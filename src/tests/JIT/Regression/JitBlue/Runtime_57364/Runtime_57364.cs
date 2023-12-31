// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;
// Generated by Fuzzlyn v1.2 on 2021-08-09 10:31:59
// Run on .NET 6.0.0-dev on Arm64 Linux
// Seed: 16685661424470671852
// Reduced from 36.1 KiB to 0.4 KiB in 00:01:03
// Debug: Runs successfully
// Release: Throws 'System.NullReferenceException'
public class Runtime_57364
{
    static short s_2;
    static bool s_7;
    [Fact]
    public static void TestEntryPoint()
    {
        var vr4 = new ulong[][][][]{new ulong[][][]{new ulong[][]{new ulong[]{0}}}};
        int vr9 = 866278096;
        // On ARM64, the following statement ends up with 2 separate CSEs in it.
        // The first CSE is the constant vr9 while the second is the address of s_2.
        // On the first CSE we were swapping the evaluation order and would end up
        // using the CSE temp for s_2 before assigning it, leading it a null ref.
        s_2 |= (short)(0 % (vr9 | 1));
        if (s_7)
        {
            vr4[0] = vr4[0];
        }

        System.Console.WriteLine(vr9);
    }
}
