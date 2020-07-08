// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

internal static partial class Interop
{
    internal static partial class Ole32
    {
        [DllImport(Libraries.Ole32, PreserveSig = false)]
        internal static extern IntPtr CoUnmarshalInterface(
            IStream pStm,                                  // Pointer to the stream
            [MarshalAs(UnmanagedType.LPStruct)] Guid riid  // Reference to the identifier of the interface
            );
    }
}
