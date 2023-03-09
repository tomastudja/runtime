﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Generated by Fuzzlyn v1.5 on 2023-03-07 17:55:15
// Run on X86 Windows
// Seed: 7526964204781879473
// Reduced from 425.4 KiB to 0.4 KiB in 00:12:20
// Debug: Prints 0 line(s)
// Release: Prints 1 line(s)
public struct S0
{
    public ulong F1;
}

public class C0
{
    public uint F0;
}

public class C1
{
    public S0 F1;
}

public class Runtime_83140
{
    public static C0 s_10 = new C0();
    public static C1 s_64 = new C1();
    public static int Main()
    {
        // We were optimizing GE(relop, 0) by reversing the relop, but the optimization is only valid for EQ/NE(relop, 0).
        if (0 > (int)(s_10.F0 / 2699312582U))
        {
            var vr5 = s_64.F1.F1;
            return -1;
        }

        return 100;
    }
}
