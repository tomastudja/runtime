// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

internal static partial class Interop
{
    internal static partial class Crypt32
    {
        [LibraryImport(Libraries.Crypt32, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static partial bool CryptMsgUpdate(SafeCryptMsgHandle hCryptMsg, byte[] pbData, int cbData, [MarshalAs(UnmanagedType.Bool)] bool fFinal);

        [LibraryImport(Libraries.Crypt32, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static partial bool CryptMsgUpdate(SafeCryptMsgHandle hCryptMsg, IntPtr pbData, int cbData, [MarshalAs(UnmanagedType.Bool)] bool fFinal);

        [LibraryImport(Libraries.Crypt32, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static partial bool CryptMsgUpdate(SafeCryptMsgHandle hCryptMsg, ref byte pbData, int cbData, [MarshalAs(UnmanagedType.Bool)] bool fFinal);
    }
}
