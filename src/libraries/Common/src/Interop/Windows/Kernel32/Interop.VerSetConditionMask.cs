// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.InteropServices;

internal static partial class Interop
{
    internal static partial class Kernel32
    {
        [GeneratedDllImport(Libraries.Kernel32)]
        internal static partial ulong VerSetConditionMask(ulong ConditionMask, uint TypeMask, byte Condition);
    }
}
