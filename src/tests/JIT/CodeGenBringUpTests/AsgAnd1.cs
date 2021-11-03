// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//


using System;
using System.Runtime.CompilerServices;
public class BringUpTest_AsgAnd1
{
    const int Pass = 100;
    const int Fail = -1;

    [MethodImplAttribute(MethodImplOptions.NoInlining)]
    public static int AsgAnd1(int x) { x &= 3; return x; }

    public static int Main()
    {
        if (AsgAnd1(0xf) == 3) return Pass;
        else return Fail;
    }
}
