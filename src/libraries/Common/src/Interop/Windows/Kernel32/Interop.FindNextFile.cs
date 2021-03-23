// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;

internal static partial class Interop
{
    internal static partial class Kernel32
    {
        [DllImport(Libraries.Kernel32, EntryPoint = "FindNextFileW", SetLastError = true, CharSet = CharSet.Unicode, BestFitMapping = false)]
        internal static extern bool FindNextFile(SafeFindHandle hndFindFile, ref WIN32_FIND_DATA lpFindFileData);
    }
}
