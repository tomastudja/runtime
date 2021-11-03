// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;

internal static partial class Interop
{
    internal static partial class Crypt32
    {
        [Flags]
        internal enum CertSetPropertyFlags : int
        {
            CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG = 0x40000000,
            None = 0x00000000,
        }
    }
}
