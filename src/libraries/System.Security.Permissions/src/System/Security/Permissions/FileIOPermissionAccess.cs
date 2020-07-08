// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Security.Permissions
{
    [Flags]
    public enum FileIOPermissionAccess
    {
        AllAccess = 15,
        Append = 4,
        NoAccess = 0,
        PathDiscovery = 8,
        Read = 1,
        Write = 2,
    }
}
