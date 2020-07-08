// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;
using System.Net.Sockets;

internal static partial class Interop
{
    internal static partial class Winsock
    {
        [DllImport(Interop.Libraries.Ws2_32, SetLastError = true)]
        internal static extern unsafe int recv(
            SafeSocketHandle socketHandle,
            [In] byte* pinnedBuffer,
            [In] int len,
            [In] SocketFlags socketFlags);
    }
}
