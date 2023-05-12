// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

using System;
using Xunit;

public interface IFoo
{

}

public class FooClass : IFoo
{

}

public struct FooStruct : IFoo
{

}

public class GenClass<T> where T : IFoo
{
    public IFoo ConvertToConstraint(T t)
    {
        return t;
    }

    public virtual IFoo VirtConvertToConstraint(T t)
    {
        return t;
    }
}

public struct GenStruct<T> where T : IFoo
{
    public IFoo ConvertToConstraint(T t)
    {
        return t;
    }
}
public class Test_Convert_instance01
{
    public static int counter = 0;
    public static bool result = true;
    public static void Eval(bool exp)
    {
        counter++;
        if (!exp)
        {
            result = exp;
            Console.WriteLine("Test Failed at location: " + counter);
        }

    }

    [Fact]
    public static int TestEntryPoint()
    {
        Eval(new GenClass<FooClass>().ConvertToConstraint(new FooClass()).GetType().Equals(typeof(FooClass)));
        Eval(new GenClass<FooStruct>().ConvertToConstraint(new FooStruct()).GetType().Equals(typeof(FooStruct)));

        Eval(new GenClass<FooClass>().VirtConvertToConstraint(new FooClass()).GetType().Equals(typeof(FooClass)));
        Eval(new GenClass<FooStruct>().VirtConvertToConstraint(new FooStruct()).GetType().Equals(typeof(FooStruct)));

        Eval(new GenStruct<FooClass>().ConvertToConstraint(new FooClass()).GetType().Equals(typeof(FooClass)));
        Eval(new GenStruct<FooStruct>().ConvertToConstraint(new FooStruct()).GetType().Equals(typeof(FooStruct)));

        if (result)
        {
            Console.WriteLine("Test Passed");
            return 100;
        }
        else
        {
            Console.WriteLine("Test Failed");
            return 1;
        }
    }

}

