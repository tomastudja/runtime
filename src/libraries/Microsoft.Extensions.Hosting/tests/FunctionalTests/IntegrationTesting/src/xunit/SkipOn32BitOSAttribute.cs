// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using Microsoft.AspNetCore.Testing;

namespace Microsoft.Extensions.Hosting.IntegrationTesting
{
    /// <summary>
    /// Skips a 64 bit test if the current OS is 32-bit.
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public class SkipOn32BitOSAttribute : Attribute, ITestCondition
    {
        public bool IsMet =>
            RuntimeInformation.OSArchitecture == Architecture.Arm64
            || RuntimeInformation.OSArchitecture == Architecture.X64;

        public string SkipReason => "Skipping the x64 test since Windows is 32-bit";
    }
}