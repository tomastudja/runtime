// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics;
using System.Text;
using System.Globalization;
using System.Collections.Generic;

namespace System.Reflection
{
    internal static class AssemblyNameFormatter
    {
        public static string ComputeDisplayName(string name, Version? version, string? cultureName, byte[]? pkt, AssemblyNameFlags flags = 0, AssemblyContentType contentType = 0)
        {
            const int PUBLIC_KEY_TOKEN_LEN = 8;
            Debug.Assert(name.Length != 0);

            var vsb = new ValueStringBuilder(stackalloc char[256]);
            vsb.AppendQuoted(name);

            if (version != null)
            {
                Version canonicalizedVersion = version.CanonicalizeVersion();
                if (canonicalizedVersion.Major != ushort.MaxValue)
                {
                    vsb.Append(", Version=");
                    vsb.AppendSpanFormattable(canonicalizedVersion.Major);

                    if (canonicalizedVersion.Minor != ushort.MaxValue)
                    {
                        vsb.Append('.');
                        vsb.AppendSpanFormattable(canonicalizedVersion.Minor);

                        if (canonicalizedVersion.Build != ushort.MaxValue)
                        {
                            vsb.Append('.');
                            vsb.AppendSpanFormattable(canonicalizedVersion.Build);

                            if (canonicalizedVersion.Revision != ushort.MaxValue)
                            {
                                vsb.Append('.');
                                vsb.AppendSpanFormattable(canonicalizedVersion.Revision);
                            }
                        }
                    }
                }
            }

            if (cultureName != null)
            {
                if (cultureName.Length == 0)
                    cultureName = "neutral";
                vsb.Append(", Culture=");
                vsb.AppendQuoted(cultureName);
            }

            if (pkt != null)
            {
                if (pkt.Length > PUBLIC_KEY_TOKEN_LEN)
                    throw new ArgumentException();

                vsb.Append(", PublicKeyToken=");
                if (pkt.Length == 0)
                {
                    vsb.Append("null");
                }
                else
                {
                    HexConverter.EncodeToUtf16(pkt, vsb.AppendSpan(pkt.Length * 2), HexConverter.Casing.Lower);
                }
            }

            if (0 != (flags & AssemblyNameFlags.Retargetable))
                vsb.Append(", Retargetable=Yes");

            if (contentType == AssemblyContentType.WindowsRuntime)
                vsb.Append(", ContentType=WindowsRuntime");

            // NOTE: By design (desktop compat) AssemblyName.FullName and ToString() do not include ProcessorArchitecture.

            return vsb.ToString();
        }

        private static void AppendQuoted(this ref ValueStringBuilder vsb, string s)
        {
            bool needsQuoting = false;
            const char quoteChar = '\"';

            // App-compat: You can use double or single quotes to quote a name, and Fusion (or rather the IdentityAuthority) picks one
            // by some algorithm. Rather than guess at it, we use double quotes consistently.
            if (s != s.Trim() || s.Contains('\"') || s.Contains('\''))
                needsQuoting = true;

            if (needsQuoting)
                vsb.Append(quoteChar);

            for (int i = 0; i < s.Length; i++)
            {
                switch (s[i])
                {
                    case '\\':
                    case ',':
                    case '=':
                    case '\'':
                    case '"':
                        vsb.Append('\\');
                        break;
                    case '\t':
                        vsb.Append("\\t");
                        continue;
                    case '\r':
                        vsb.Append("\\r");
                        continue;
                    case '\n':
                        vsb.Append("\\n");
                        continue;
                }

                vsb.Append(s[i]);
            }

            if (needsQuoting)
                vsb.Append(quoteChar);
        }

        private static Version CanonicalizeVersion(this Version version)
        {
            ushort major = (ushort)version.Major;
            ushort minor = (ushort)version.Minor;
            ushort build = (ushort)version.Build;
            ushort revision = (ushort)version.Revision;

            if (major == version.Major && minor == version.Minor && build == version.Build && revision == version.Revision)
                return version;

            return new Version(major, minor, build, revision);
        }
    }
}
