// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices;
using System.Security.Cryptography;

internal static partial class Interop
{
    internal static partial class Advapi32
    {
        [LibraryImport(Libraries.Advapi32, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static partial bool CryptDeriveKey(
            SafeProvHandle hProv,
            int Algid,
            SafeHashHandle hBaseData,
            int dwFlags,
            out SafeCapiKeyHandle phKey);
    }
}
