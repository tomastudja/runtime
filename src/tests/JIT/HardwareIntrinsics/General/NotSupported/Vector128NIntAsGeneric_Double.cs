// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/******************************************************************************
 * This file is auto-generated from a template file by the GenerateTests.csx  *
 * script in tests\src\JIT\HardwareIntrinsics\General\Shared. In order to make    *
 * changes, please update the corresponding template and run according to the *
 * directions listed in the file.                                             *
 ******************************************************************************/

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;

namespace JIT.HardwareIntrinsics.General
{
    public static partial class Program
    {
        private static void Vector128NIntAsGeneric_Double()
        {
            bool succeeded = false;

            try
            {
                Vector128<double> result = default(Vector128<nint>).As<nint, double>();
            }
            catch (NotSupportedException)
            {
                succeeded = true;
            }

            if (!succeeded)
            {
                TestLibrary.TestFramework.LogInformation($"Vector128NIntAsGeneric_Double: RunNotSupportedScenario failed to throw NotSupportedException.");
                TestLibrary.TestFramework.LogInformation(string.Empty);

                throw new Exception("One or more scenarios did not complete as expected.");
            }
        }
    }
}
