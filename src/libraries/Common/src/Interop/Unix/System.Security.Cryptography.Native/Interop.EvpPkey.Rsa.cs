// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices;
using System.Security.Cryptography;
using Microsoft.Win32.SafeHandles;

internal static partial class Interop
{
    internal static partial class Crypto
    {
        [DllImport(Libraries.CryptoNative)]
        private static extern SafeEvpPKeyHandle CryptoNative_RsaGenerateKey(int keySize);

        internal static SafeEvpPKeyHandle RsaGenerateKey(int keySize)
        {
            SafeEvpPKeyHandle pkey = CryptoNative_RsaGenerateKey(keySize);

            if (pkey.IsInvalid)
            {
                pkey.Dispose();
                throw CreateOpenSslCryptographicException();
            }

            return pkey;
        }

        [DllImport(Libraries.CryptoNative, EntryPoint = "CryptoNative_EvpPkeyGetRsa")]
        internal static extern SafeRsaHandle EvpPkeyGetRsa(SafeEvpPKeyHandle pkey);

        [DllImport(Libraries.CryptoNative, EntryPoint = "CryptoNative_EvpPkeySetRsa")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EvpPkeySetRsa(SafeEvpPKeyHandle pkey, SafeRsaHandle rsa);
    }
}
