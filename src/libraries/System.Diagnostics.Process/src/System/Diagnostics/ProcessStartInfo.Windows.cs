// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Security;
using System.Runtime.Versioning;

namespace System.Diagnostics
{
    public sealed partial class ProcessStartInfo
    {
        private string? _domain;

        private const bool CaseSensitiveEnvironmentVariables = false;

        [SupportedOSPlatform("windows7.0")]
        public string? PasswordInClearText { get; set; }

        [SupportedOSPlatform("windows7.0")]
        public string Domain
        {
            get => _domain ?? string.Empty;
            set => _domain = value;
        }

        [SupportedOSPlatform("windows7.0")]
        public bool LoadUserProfile { get; set; }

        [CLSCompliant(false)]
        [SupportedOSPlatform("windows7.0")]
        public SecureString? Password { get; set; }
    }
}
