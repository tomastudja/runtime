// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Generated by Fuzzlyn v1.5 on 2022-03-30 08:05:26
// Run on Arm64 MacOS
// Seed: 14607776878871751670
// Reduced from 104.4 KiB to 0.5 KiB in 00:00:19
// Debug: Outputs 0
// Release: Outputs 1
public class Runtime_67331
{
    public static short[] s_2 = new short[]{0};
    public static int Main()
    {
        var vr3 = (uint)s_2[0];
        var vr4 = new byte[]{0};
        var vr5 = new byte[, ]{{0}};
        return M3(vr3, false, 0, 0, vr4, true, vr5, 0, 1, 1) == 1 ? 100 : -1;
    }

    public static int M3(uint arg2, bool arg3, ulong arg4, int arg5, byte[] arg6, bool arg7, byte[, ] arg8, int arg9, sbyte arg10, short arg11)
    {
        arg10 = 0;
        return arg11;
    }
}
