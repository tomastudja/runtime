// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;

namespace JIT.HardwareIntrinsics.Arm
{
    public static partial class Program
    {
        static Program()
        {
            TestList = new Dictionary<string, Action>() {
                ["LeadingSignCount.Int32"] = LeadingSignCount_Int32,
                ["LeadingSignCount.Int64"] = LeadingSignCount_Int64,
                ["LeadingZeroCount.Int64"] = LeadingZeroCount_Int64,
                ["LeadingZeroCount.UInt64"] = LeadingZeroCount_UInt64,
                ["ReverseElementBits.Int64"] = ReverseElementBits_Int64,
                ["ReverseElementBits.UInt64"] = ReverseElementBits_UInt64,
            };
        }
    }
}
