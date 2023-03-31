// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;
// Generated by Fuzzlyn v1.2 on 2021-08-15 23:15:19
// Run on .NET 6.0.0-dev on Arm Linux
// Seed: 18219619158927602726
// Reduced from 82.6 KiB to 0.3 KiB in 00:02:54
// Debug: Outputs 14270
// Release: Outputs 4294953026
public class Runtime_57640
{
    static long[] s_28 = new long[]{1};
    [Fact]
    public static int TestEntryPoint()
    {
        bool correct = true;
        var vr10 = s_28[0];
        for (int vr13 = 0; vr13 < 2; vr13++)
        {
            uint vr12 = (uint)(0 - (-14270 * vr10));
            correct &= vr12 == 14270;
        }

        return correct ? 100 : -1;
    }
}
