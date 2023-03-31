// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

using System;
using Xunit;

public class AA
{
    internal static void Static5(int param1)
    {
        if (param1 != 0)
        {
            if (param1 == 1)
                return;
        }
#pragma warning disable 1717
        param1 = param1;
#pragma warning restore 1717
    }
    [Fact]
    public static int TestEntryPoint() { Static5(0); return 100; }
}
