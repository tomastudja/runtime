// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;

internal partial class Interop
{
    internal partial class Kernel32
    {
        [DllImport(Libraries.Kernel32, SetLastError = true)]
        internal static extern unsafe int ReadFile(
            IntPtr handle,
            byte* bytes,
            int numBytesToRead,
            out int numBytesRead,
            IntPtr mustBeZero);
    }
}
