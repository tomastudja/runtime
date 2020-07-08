// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Microsoft.Win32.SafeHandles;
using System;
using System.Runtime.InteropServices;

internal static partial class Interop
{
    internal static partial class Sys
    {
        [DllImport(Libraries.SystemNative, EntryPoint = "SystemNative_CopyFile", SetLastError = true)]
        internal static extern int CopyFile(SafeFileHandle source, string srcPath, string destPath, int overwrite);
    }
}
