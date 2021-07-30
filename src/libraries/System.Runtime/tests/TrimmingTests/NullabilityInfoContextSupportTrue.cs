// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#nullable enable

using System.Reflection;

/// <summary>
/// Ensures setting NullabilityInfoContextSupport = true works in a trimmed app
/// </summary>
class Program
{
    static int Main(string[] args)
    {
        MethodInfo testMethod = typeof(TestType).GetMethod("TestMethod")!;
        NullabilityInfoContext nullabilityContext = new NullabilityInfoContext();
        NullabilityInfo nullability = nullabilityContext.Create(testMethod.ReturnParameter);

        if (nullability.ReadState != NullabilityState.Nullable)
        {
            return -1;
        }

        return 100;
    }
}

class TestType
{
    public static string? TestMethod()
    {
        return null;
    }
}
