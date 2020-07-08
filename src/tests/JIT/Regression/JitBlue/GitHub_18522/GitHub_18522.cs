// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;

// Based on a program generated by Fuzzlyn

struct S0
{
    public ushort F0;
}

struct S1
{
    public S0 F0;
    public ushort F1;
}

public class GitHub_18522
{
    static S1 s_36;

    // When generating code for the x64 SysV ABI, the jit was
    // incorrectly typing the return type from M113, and so
    // inadvertently overwriting the F1 field of s_36 on return from
    // the (inlined) call.
    public static int Main()
    {
        s_36.F1 = 0xAA;
        s_36.F0 = M113();
        return (s_36.F1 == 0xAA ? 100 : 0);
    }

    static S0 M113()
    {
        return new S0();
    }
}
