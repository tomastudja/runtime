// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//
// In this issue, we were not removing all the unreachable blocks and that led us to expect that
// there should be an IG label for one of the unreachable block, but we were not creating it leading
// to an assert failure.
using Xunit;
public class _65659_2
{
    public static bool[][,] s_2;
    public static short[,][] s_8;
    public static bool[] s_10;
    public static ushort[][] s_29;
    [Fact]
    public static void TestEntryPoint()
    {
        bool vr1 = M47();
    }

    public static bool M47()
    {
        bool var0 = default(bool);
        try
        {
            if (var0)
            {
                try
                {
                    if (s_10[0])
                    {
                        return s_2[0][0, 0];
                    }
                }
                finally
                {
                    s_8[0, 0][0] = 0;
                }
            }
        }
        finally
        {
            s_29 = s_29;
        }

        return true;
    }
}