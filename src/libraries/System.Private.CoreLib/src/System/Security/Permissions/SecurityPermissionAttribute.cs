// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Security.Permissions
{
    [Obsolete(Obsoletions.CodeAccessSecurityMessage, DiagnosticId = Obsoletions.CodeAccessSecurityDiagId, UrlFormat = Obsoletions.SharedUrlFormat)]
    [AttributeUsage((AttributeTargets)(109), AllowMultiple = true, Inherited = false)]
    public sealed partial class SecurityPermissionAttribute : CodeAccessSecurityAttribute
    {
        public SecurityPermissionAttribute(SecurityAction action) : base(default) { }
        public bool Assertion { get; set; }
        public bool BindingRedirects { get; set; }
        public bool ControlAppDomain { get; set; }
        public bool ControlDomainPolicy { get; set; }
        public bool ControlEvidence { get; set; }
        public bool ControlPolicy { get; set; }
        public bool ControlPrincipal { get; set; }
        public bool ControlThread { get; set; }
        public bool Execution { get; set; }
        public SecurityPermissionFlag Flags { get; set; }
        public bool Infrastructure { get; set; }
        public bool RemotingConfiguration { get; set; }
        public bool SerializationFormatter { get; set; }
        public bool SkipVerification { get; set; }
        public bool UnmanagedCode { get; set; }
        public override IPermission? CreatePermission() { return null; }
    }
}
