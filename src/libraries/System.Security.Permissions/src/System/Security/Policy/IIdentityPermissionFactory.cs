// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Security.Policy
{
    public partial interface IIdentityPermissionFactory
    {
        IPermission CreateIdentityPermission(Evidence evidence);
    }
}
