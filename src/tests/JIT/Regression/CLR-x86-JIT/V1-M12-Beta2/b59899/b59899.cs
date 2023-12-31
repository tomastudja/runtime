// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using Xunit;
public class SamplesArray
{
    [Fact]
    public static void TestEntryPoint()
    {
        int[] myLens = new int[1] { 5 };
        int[] myLows = new int[1] { -2 };

        Array myArr = Array.CreateInstance(typeof(String), myLens, myLows);
        for (int i = myArr.GetLowerBound(0); i <= myArr.GetUpperBound(0); i++)
            myArr.SetValue(i.ToString(), i);

        Object[] objSZArray = myArr as Object[];
        if (objSZArray != null)
            Console.Error.WriteLine("Ack!  JIT casting bug!  This is not an SZArray!");

        try
        {
            Array.Reverse(myArr, -1, 3);
        }
        catch (Exception myException)
        {
            Console.WriteLine("Exception: " + myException.ToString());
        }
    }
}
