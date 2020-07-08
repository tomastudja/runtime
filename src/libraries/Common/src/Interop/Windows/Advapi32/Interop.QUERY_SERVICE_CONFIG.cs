// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices;

internal partial class Interop
{
    internal partial class Advapi32
    {
        [StructLayout(LayoutKind.Sequential)]
        internal unsafe class QUERY_SERVICE_CONFIG
        {
            internal int dwServiceType;
            internal int dwStartType;
            internal int dwErrorControl;
            internal char* lpBinaryPathName;
            internal char* lpLoadOrderGroup;
            internal int dwTagId;
            internal char* lpDependencies;
            internal char* lpServiceStartName;
            internal char* lpDisplayName;
        }
    }
}
