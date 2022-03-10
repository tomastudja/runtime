// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices;
using System.Security.Cryptography;
using Microsoft.Win32.SafeHandles;

internal static partial class Interop
{
    internal static partial class Crypto
    {
        [LibraryImport(Libraries.CryptoNative, EntryPoint = "CryptoNative_EvpPkeyGetEcKey")]
        internal static partial SafeEcKeyHandle EvpPkeyGetEcKey(SafeEvpPKeyHandle pkey);

        [LibraryImport(Libraries.CryptoNative, EntryPoint = "CryptoNative_EvpPkeySetEcKey")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static partial bool EvpPkeySetEcKey(SafeEvpPKeyHandle pkey, SafeEcKeyHandle key);
    }
}
