// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.


using System.Runtime.InteropServices;
using System;

internal class NullableTest
{
    private static bool BoxUnboxToNQ(IEmptyGen<int> o)
    {
        return Helper.Compare((ImplementTwoInterfaceGen<int>)o, Helper.Create(default(ImplementTwoInterfaceGen<int>)));
    }

    private static bool BoxUnboxToQ(IEmptyGen<int> o)
    {
        return Helper.Compare((ImplementTwoInterfaceGen<int>?)o, Helper.Create(default(ImplementTwoInterfaceGen<int>)));
    }

    private static int Main()
    {
        ImplementTwoInterfaceGen<int>? s = Helper.Create(default(ImplementTwoInterfaceGen<int>));

        if (BoxUnboxToNQ(s) && BoxUnboxToQ(s))
            return ExitCode.Passed;
        else
            return ExitCode.Failed;
    }
}


