// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Microsoft.Win32.SafeHandles;
using System;
using System.Runtime.InteropServices;

internal static partial class Interop
{
    internal static partial class Advapi32
    {
#if DLLIMPORTGENERATOR_ENABLED
        [GeneratedDllImport(Interop.Libraries.Advapi32, CharSet = CharSet.Unicode, SetLastError = true)]
        internal static partial bool ImpersonateLoggedOnUser(SafeAccessTokenHandle userToken);
#else
        [DllImport(Interop.Libraries.Advapi32, CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern bool ImpersonateLoggedOnUser(SafeAccessTokenHandle userToken);
#endif
    }
}
