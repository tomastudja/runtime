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
using Xunit;

namespace JIT.HardwareIntrinsics.General._NotSupported
{
    public static partial class Program
    {
        [Fact]
        public static void Vector128Int64AsGeneric_Boolean()
        {
            bool succeeded = false;

            try
            {
                Vector128<bool> result = default(Vector128<long>).As<long, bool>();
            }
            catch (NotSupportedException)
            {
                succeeded = true;
            }

            if (!succeeded)
            {
                TestLibrary.TestFramework.LogInformation($"Vector128Int64AsGeneric_Boolean: RunNotSupportedScenario failed to throw NotSupportedException.");
                TestLibrary.TestFramework.LogInformation(string.Empty);

                throw new Exception("One or more scenarios did not complete as expected.");
            }
        }
    }
}
