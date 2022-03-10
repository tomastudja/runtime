// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;

internal static partial class Interop
{
    internal static partial class User32
    {
        [LibraryImport(Libraries.User32, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static unsafe partial bool GetUserObjectInformationW(IntPtr hObj, int nIndex, void* pvBuffer, uint nLength, ref uint lpnLengthNeeded);
    }
}
