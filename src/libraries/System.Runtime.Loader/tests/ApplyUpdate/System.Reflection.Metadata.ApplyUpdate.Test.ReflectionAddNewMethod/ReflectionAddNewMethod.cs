// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
using System;


namespace System.Reflection.Metadata.ApplyUpdate.Test
{
    public class ReflectionAddNewMethod
    {
        public string ExistingMethod(string u, double f)
	{
            return u + f.ToString();;
        }

    }
}
