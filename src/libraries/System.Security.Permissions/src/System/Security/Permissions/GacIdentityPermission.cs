// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Security.Permissions
{
    public sealed partial class GacIdentityPermission : CodeAccessPermission
    {
        public GacIdentityPermission() { }
        public GacIdentityPermission(PermissionState state) { }
        public override IPermission Copy() { return default(IPermission); }
        public override void FromXml(SecurityElement securityElement) { }
        public override IPermission Intersect(IPermission target) { return default(IPermission); }
        public override bool IsSubsetOf(IPermission target) { return false; }
        public override SecurityElement ToXml() { return default(SecurityElement); }
        public override IPermission Union(IPermission target) { return default(IPermission); }
    }
}
