// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.InteropServices;
#if NET7_0_OR_GREATER
using System.Runtime.InteropServices.Marshalling;
#endif

internal static partial class Interop
{
    internal static partial class User32
    {
        [LibraryImport(Libraries.User32, SetLastError = true)]
        internal static partial IntPtr CopyImage(
#if NET7_0_OR_GREATER
            [MarshalUsing(typeof(HandleRefMarshaller))]
#endif
            HandleRef hImage, int uType, int cxDesired, int cyDesired, int fuFlags);
    }
}
