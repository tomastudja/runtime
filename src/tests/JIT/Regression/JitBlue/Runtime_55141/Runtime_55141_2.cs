// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Generated by Fuzzlyn v1.2 on 2021-07-22 13:45:33
// Seed: 7718323254176193466
// Reduced from 89.3 KiB to 0.4 KiB in 00:00:46
// Debug: Outputs 1
// Release: Outputs 0

using System.Runtime.CompilerServices;

struct S2
{
    public uint F0;
    public long F1;
    public uint F2;
    public short F3;
    public bool F4;
    public S2(uint f0, uint f2): this()
    {
        F0 = f0;
        F2 = f2;
    }
}

public class Runtime_55141_2
{
    static int s_1;
    public static int Main()
    {
        int vr23 = s_1;
        S2 vr29 = new S2(1, (uint)vr23 / 40319);
        return M(vr29.F0) == 1 ? 100 : -1;
    }
    
    [MethodImpl(MethodImplOptions.NoInlining)]
    private static uint M(uint val) => val;
}