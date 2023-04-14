// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// <Area> Nullable - Box-Unbox </Area>
// <Title> Nullable type with unbox box expr  </Title>
// <Description>  
// checking type of ImplementAllInterface<int> using is operator
// </Description> 
// <RelatedBugs> </RelatedBugs>  
//<Expects Status=success></Expects>
// <Code> 


using System.Runtime.InteropServices;
using System;
using Xunit;

public class NullableTest
{
    private static bool BoxUnboxToNQ(IEmpty o)
    {
        return Helper.Compare((ImplementAllInterface<int>)o, Helper.Create(default(ImplementAllInterface<int>)));
    }

    private static bool BoxUnboxToQ(IEmpty o)
    {
        return Helper.Compare((ImplementAllInterface<int>?)o, Helper.Create(default(ImplementAllInterface<int>)));
    }

    [Fact]
    public static int TestEntryPoint()
    {
        ImplementAllInterface<int>? s = Helper.Create(default(ImplementAllInterface<int>));

        if (BoxUnboxToNQ(s) && BoxUnboxToQ(s))
            return ExitCode.Passed;
        else
            return ExitCode.Failed;
    }
}


