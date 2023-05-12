// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;
// Generated by Fuzzlyn v1.5 on 2023-03-19 17:44:38
// Run on Arm64 Linux
// Seed: 516098027771570682
public class C0
{
    public uint F1;
    public C0(uint f1)
    {
        F1 = f1;
    }
}

public class Program
{
    // This test was testing an ARM64 regression when optimizing 'x < 0' when it didn't take into account an unsigned operation.
    [Fact]
    public static int TestEntryPoint()
    {
        var vr1 = new C0(4294967295U);
        bool vr3 = !(vr1.F1 > -1);
        if (!vr3)
            return 100;
        else
            return 0;
    }
}