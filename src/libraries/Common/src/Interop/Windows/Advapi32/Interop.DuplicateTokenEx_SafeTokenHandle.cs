// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Microsoft.Win32.SafeHandles;
using System;
using System.Runtime.InteropServices;
using System.Security.Principal;

internal static partial class Interop
{
    internal static partial class Advapi32
    {
        [GeneratedDllImport(Interop.Libraries.Advapi32, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static partial bool DuplicateTokenEx(
            SafeTokenHandle ExistingTokenHandle,
            TokenAccessLevels DesiredAccess,
            IntPtr TokenAttributes,
            SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
            System.Security.Principal.TokenType TokenType,
            ref SafeTokenHandle? DuplicateTokenHandle);
    }
}
