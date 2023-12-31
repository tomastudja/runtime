// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using Xunit;


public interface ITestInterfaceEx1<TSelf>
    where TSelf : ITestInterfaceEx1<TSelf>
{
}

public interface ITestInterfaceEx2<TSelf>
    where TSelf : ITestInterfaceEx2<TSelf>
{
    static abstract void Invoke();
}

public interface ITestInterface<TSelf> : ITestInterfaceEx2<TSelf>, ITestInterfaceEx1<TSelf>
    where TSelf : ITestInterface<TSelf>
{
    static void ITestInterfaceEx2<TSelf>.Invoke()
    {
    }
}

public struct Test : ITestInterface<Test>
{
    public Test(object? test)
    {
    }
}

public class Program
{
    [Fact]
    public static void TestEntryPoint()
    {
        new Test(null);
    }
}
