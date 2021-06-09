// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Net.WebSockets;
using System.Runtime.InteropServices;
using System.Security;

internal static partial class Interop
{
    internal static partial class WebSocket
    {
        [GeneratedDllImport(Libraries.WebSocket)]
        internal static partial int WebSocketCreateClientHandle(
           Property[] properties,
           uint propertyCount,
           out SafeWebSocketHandle webSocketHandle);
    }
}
