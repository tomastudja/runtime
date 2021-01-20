// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Security;
using Microsoft.Win32.SafeHandles;

namespace System.DirectoryServices.ActiveDirectory
{
    internal sealed class PolicySafeHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public PolicySafeHandle() : base(true) { }

        internal PolicySafeHandle(IntPtr value) : base(true)
        {
            SetHandle(value);
        }

        protected override bool ReleaseHandle() => UnsafeNativeMethods.LsaClose(handle) == 0;
    }

    internal sealed class LsaLogonProcessSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public LsaLogonProcessSafeHandle() : base(true) { }

        internal LsaLogonProcessSafeHandle(IntPtr value) : base(true)
        {
            SetHandle(value);
        }

        protected override bool ReleaseHandle() => NativeMethods.LsaDeregisterLogonProcess(handle) == 0;
    }

    internal sealed class LoadLibrarySafeHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        public LoadLibrarySafeHandle() : base(true) { }

        internal LoadLibrarySafeHandle(IntPtr value) : base(true)
        {
            SetHandle(value);
        }

        protected override bool ReleaseHandle() => UnsafeNativeMethods.FreeLibrary(handle) != 0;
    }
}
