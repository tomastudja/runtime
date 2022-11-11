// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics.X86;
using System.Runtime.Intrinsics;
using Xunit;

namespace IntelHardwareIntrinsicTest._Sse3
{
    public static partial class Program
    {
        [Fact]
        public static unsafe void LoadAndDuplicationToVector128()
        {
            int testResult = Pass;

            if (Sse3.IsSupported)
            {
                using (TestTable<double> doubleTable = new TestTable<double>(new double[2] { 1, -5 }, new double[4]))
                {
                    var vf = Sse3.LoadAndDuplicateToVector128((double*)(doubleTable.inArrayPtr));
                    Unsafe.Write(doubleTable.outArrayPtr, vf);

                    if (BitConverter.DoubleToInt64Bits(doubleTable.inArray[0]) != BitConverter.DoubleToInt64Bits(doubleTable.outArray[0]) || 
                        BitConverter.DoubleToInt64Bits(doubleTable.inArray[0]) != BitConverter.DoubleToInt64Bits(doubleTable.outArray[1]))
                    {
                        Console.WriteLine("Sse3 LoadAndDuplicateToVector128 failed on double:");
                        foreach (var item in doubleTable.outArray)
                        {
                            Console.Write(item + ", ");
                        }
                        Console.WriteLine();
                        testResult = Fail;
                    }
                }
            }

            Assert.Equal(Pass, testResult);
        }
    }
}
