// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#nullable enable
using System.Net.Security;
using System.Runtime.InteropServices;

namespace System.Net
{
    // SspiCli SSPI interface.
    internal interface ISSPIInterface
    {
        SecurityPackageInfoClass[]? SecurityPackages { get; set; }
        int EnumerateSecurityPackages(out int pkgnum, out SafeFreeContextBuffer pkgArray);
        int AcquireCredentialsHandle(string moduleName, Interop.SspiCli.CredentialUse usage, ref SafeSspiAuthDataHandle authdata, out SafeFreeCredentials outCredential);
        unsafe int AcquireCredentialsHandle(string moduleName, Interop.SspiCli.CredentialUse usage, Interop.SspiCli.SCHANNEL_CRED* authdata, out SafeFreeCredentials outCredential);
        unsafe int AcquireCredentialsHandle(string moduleName, Interop.SspiCli.CredentialUse usage, Interop.SspiCli.SCH_CREDENTIALS* authdata, out SafeFreeCredentials outCredential);
        int AcquireDefaultCredential(string moduleName, Interop.SspiCli.CredentialUse usage, out SafeFreeCredentials outCredential);
        int AcceptSecurityContext(SafeFreeCredentials? credential, ref SafeDeleteSslContext? context, InputSecurityBuffers inputBuffers, Interop.SspiCli.ContextFlags inFlags, Interop.SspiCli.Endianness endianness, ref SecurityBuffer outputBuffer, ref Interop.SspiCli.ContextFlags outFlags);
        int InitializeSecurityContext(ref SafeFreeCredentials? credential, ref SafeDeleteSslContext? context, string? targetName, Interop.SspiCli.ContextFlags inFlags, Interop.SspiCli.Endianness endianness, InputSecurityBuffers inputBuffers, ref SecurityBuffer outputBuffer, ref Interop.SspiCli.ContextFlags outFlags);
        int EncryptMessage(SafeDeleteContext context, ref Interop.SspiCli.SecBufferDesc inputOutput, uint sequenceNumber);
        int DecryptMessage(SafeDeleteContext context, ref Interop.SspiCli.SecBufferDesc inputOutput, uint sequenceNumber);
        int MakeSignature(SafeDeleteContext context, ref Interop.SspiCli.SecBufferDesc inputOutput, uint sequenceNumber);
        int VerifySignature(SafeDeleteContext context, ref Interop.SspiCli.SecBufferDesc inputOutput, uint sequenceNumber);

        int QueryContextChannelBinding(SafeDeleteContext phContext, Interop.SspiCli.ContextAttribute attribute, out SafeFreeContextBufferChannelBinding refHandle);
        int QueryContextAttributes(SafeDeleteContext phContext, Interop.SspiCli.ContextAttribute attribute, Span<byte> buffer, Type? handleType, out SafeHandle? refHandle);
        int QuerySecurityContextToken(SafeDeleteContext phContext, out SecurityContextTokenHandle phToken);
        int CompleteAuthToken(ref SafeDeleteSslContext? refContext, in SecurityBuffer inputBuffer);
        int ApplyControlToken(ref SafeDeleteContext? refContext, in SecurityBuffer inputBuffer);
    }
}
