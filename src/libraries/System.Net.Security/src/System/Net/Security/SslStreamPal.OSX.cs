// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Buffers;
using System.ComponentModel;
using System.Diagnostics;
using System.Security.Authentication;
using System.Security.Authentication.ExtendedProtection;
using System.Security.Cryptography.X509Certificates;

using PAL_TlsHandshakeState = Interop.AppleCrypto.PAL_TlsHandshakeState;
using PAL_TlsIo = Interop.AppleCrypto.PAL_TlsIo;

namespace System.Net.Security
{
    internal static class SslStreamPal
    {
        public static Exception GetException(SecurityStatusPal status)
        {
            return status.Exception ?? new Win32Exception((int)status.ErrorCode);
        }

        internal const bool StartMutualAuthAsAnonymous = false;

        // SecureTransport is okay with a 0 byte input, but it produces a 0 byte output.
        // Since ST is not producing the framed empty message just call this false and avoid the
        // special case of an empty array being passed to the `fixed` statement.
        internal const bool CanEncryptEmptyMessage = false;

        public static void VerifyPackageInfo()
        {
        }

        public static SecurityStatusPal AcceptSecurityContext(
            ref SafeFreeCredentials credential,
            ref SafeDeleteSslContext? context,
            ReadOnlySpan<byte> inputBuffer,
            ref byte[]? outputBuffer,
            SslAuthenticationOptions sslAuthenticationOptions)
        {
            return HandshakeInternal(credential, ref context, inputBuffer, ref outputBuffer, sslAuthenticationOptions);
        }

        public static SecurityStatusPal InitializeSecurityContext(
            ref SafeFreeCredentials credential,
            ref SafeDeleteSslContext? context,
            string? targetName,
            ReadOnlySpan<byte> inputBuffer,
            ref byte[]? outputBuffer,
            SslAuthenticationOptions sslAuthenticationOptions)
        {
            return HandshakeInternal(credential, ref context, inputBuffer, ref outputBuffer, sslAuthenticationOptions);
        }

        public static SecurityStatusPal Renegotiate(ref SafeFreeCredentials? credentialsHandle, ref SafeDeleteSslContext? context, SslAuthenticationOptions sslAuthenticationOptions, out byte[]? outputBuffer)
        {
            throw new PlatformNotSupportedException();
        }

        public static SafeFreeCredentials AcquireCredentialsHandle(
            SslStreamCertificateContext? certificateContext,
            SslProtocols protocols,
            EncryptionPolicy policy,
            bool isServer)
        {
            return new SafeFreeSslCredentials(certificateContext, protocols, policy);
        }

        internal static byte[]? GetNegotiatedApplicationProtocol(SafeDeleteSslContext? context)
        {
            if (context == null)
                return null;

            return Interop.AppleCrypto.SslGetAlpnSelected(context.SslContext);
        }

        public static SecurityStatusPal EncryptMessage(
            SafeDeleteSslContext securityContext,
            ReadOnlyMemory<byte> input,
            int headerSize,
            int trailerSize,
            ref byte[] output,
            out int resultSize)
        {
            resultSize = 0;

            Debug.Assert(input.Length > 0, $"{nameof(input.Length)} > 0 since {nameof(CanEncryptEmptyMessage)} is false");

            try
            {
                SafeSslHandle sslHandle = securityContext.SslContext;

                unsafe
                {
                    MemoryHandle memHandle = input.Pin();
                    try
                    {
                        PAL_TlsIo status = Interop.AppleCrypto.SslWrite(
                                sslHandle,
                                (byte*)memHandle.Pointer,
                                input.Length,
                                out int written);

                        if (status < 0)
                        {
                            return new SecurityStatusPal(
                                SecurityStatusPalErrorCode.InternalError,
                                Interop.AppleCrypto.CreateExceptionForOSStatus((int)status));
                        }

                        if (securityContext.BytesReadyForConnection <= output?.Length)
                        {
                            resultSize = securityContext.ReadPendingWrites(output, 0, output.Length);
                        }
                        else
                        {
                            output = securityContext.ReadPendingWrites()!;
                            resultSize = output.Length;
                        }

                        switch (status)
                        {
                            case PAL_TlsIo.Success:
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.OK);
                            case PAL_TlsIo.WouldBlock:
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.ContinueNeeded);
                            default:
                                Debug.Fail($"Unknown status value: {status}");
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.InternalError);
                        }
                    }
                    finally
                    {
                        memHandle.Dispose();
                    }
                }
            }
            catch (Exception e)
            {
                return new SecurityStatusPal(SecurityStatusPalErrorCode.InternalError, e);
            }
        }

        public static SecurityStatusPal DecryptMessage(
            SafeDeleteSslContext securityContext,
            Span<byte> buffer,
            out int offset,
            out int count)
        {
            offset = 0;
            count = 0;

            try
            {
                SafeSslHandle sslHandle = securityContext.SslContext;
                securityContext.Write(buffer);

                unsafe
                {
                    fixed (byte* ptr = buffer)
                    {
                        PAL_TlsIo status = Interop.AppleCrypto.SslRead(sslHandle, ptr, buffer.Length, out int written);
                        if (status < 0)
                        {
                            return new SecurityStatusPal(
                                SecurityStatusPalErrorCode.InternalError,
                                Interop.AppleCrypto.CreateExceptionForOSStatus((int)status));
                        }

                        count = written;
                        offset = 0;

                        switch (status)
                        {
                            case PAL_TlsIo.Success:
                            case PAL_TlsIo.WouldBlock:
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.OK);
                            case PAL_TlsIo.ClosedGracefully:
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.ContextExpired);
                            case PAL_TlsIo.Renegotiate:
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.Renegotiate);
                            default:
                                Debug.Fail($"Unknown status value: {status}");
                                return new SecurityStatusPal(SecurityStatusPalErrorCode.InternalError);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                return new SecurityStatusPal(SecurityStatusPalErrorCode.InternalError, e);
            }
        }

        public static ChannelBinding? QueryContextChannelBinding(
            SafeDeleteContext securityContext,
            ChannelBindingKind attribute)
        {
            switch (attribute)
            {
                case ChannelBindingKind.Endpoint:
                    return EndpointChannelBindingToken.Build(securityContext);
            }

            // SecureTransport doesn't expose the Finished messages, so a Unique binding token
            // cannot be built.
            //
            // Windows/netfx compat says to return null for not supported kinds (including unmapped enum values).
            return null;
        }

        public static void QueryContextStreamSizes(
            SafeDeleteContext? securityContext,
            out StreamSizes streamSizes)
        {
            streamSizes = StreamSizes.Default;
        }

        public static void QueryContextConnectionInfo(
            SafeDeleteSslContext securityContext,
            out SslConnectionInfo connectionInfo)
        {
            connectionInfo = new SslConnectionInfo(securityContext.SslContext);
        }

        private static SecurityStatusPal HandshakeInternal(
            SafeFreeCredentials credential,
            ref SafeDeleteSslContext? context,
            ReadOnlySpan<byte> inputBuffer,
            ref byte[]? outputBuffer,
            SslAuthenticationOptions sslAuthenticationOptions)
        {
            Debug.Assert(!credential.IsInvalid);

            try
            {
                SafeDeleteSslContext? sslContext = ((SafeDeleteSslContext?)context);

                if ((null == context) || context.IsInvalid)
                {
                    sslContext = new SafeDeleteSslContext((credential as SafeFreeSslCredentials)!, sslAuthenticationOptions);
                    context = sslContext;

                    if (!string.IsNullOrEmpty(sslAuthenticationOptions.TargetHost) && !sslAuthenticationOptions.IsServer)
                    {
                        Interop.AppleCrypto.SslSetTargetName(sslContext.SslContext, sslAuthenticationOptions.TargetHost);
                    }

                    if (sslAuthenticationOptions.CertificateContext == null && sslAuthenticationOptions.CertSelectionDelegate != null)
                    {
                        // certificate was not provided but there is user callback. We can break handshake if server asks for certificate
                        // and we can try to get it based on remote certificate and trusted issuers.
                        Interop.AppleCrypto.SslBreakOnCertRequested(sslContext.SslContext, true);
                    }

                    if (sslAuthenticationOptions.IsServer && sslAuthenticationOptions.RemoteCertRequired)
                    {
                        Interop.AppleCrypto.SslSetAcceptClientCert(sslContext.SslContext);
                    }
                }

                if (inputBuffer.Length > 0)
                {
                    sslContext!.Write(inputBuffer);
                }

                SafeSslHandle sslHandle = sslContext!.SslContext;
                SecurityStatusPal status = PerformHandshake(sslHandle);
                if (status.ErrorCode == SecurityStatusPalErrorCode.CredentialsNeeded)
                {
                    // we should not be here if CertSelectionDelegate is null but better check before dereferencing..
                    if (sslAuthenticationOptions.CertSelectionDelegate != null)
                    {
                        X509Certificate2? remoteCert = null;
                        try
                        {
                            string[] issuers = CertificateValidationPal.GetRequestCertificateAuthorities(context);
                            remoteCert = CertificateValidationPal.GetRemoteCertificate(context);
                            if (sslAuthenticationOptions.ClientCertificates == null)
                            {
                                sslAuthenticationOptions.ClientCertificates = new X509CertificateCollection();
                            }
                            X509Certificate2 clientCertificate = (X509Certificate2)sslAuthenticationOptions.CertSelectionDelegate(sslAuthenticationOptions.TargetHost!, sslAuthenticationOptions.ClientCertificates, remoteCert, issuers);
                            if (clientCertificate != null)
                            {
                                SafeDeleteSslContext.SetCertificate(sslContext.SslContext,  SslStreamCertificateContext.Create(clientCertificate));
                            }
                        }
                        finally
                        {
                            remoteCert?.Dispose();
                        }
                    }

                    // We either got certificate or we can proceed without it. It is up to the server to decide if either is OK.
                    status = PerformHandshake(sslHandle);
                }

                outputBuffer = sslContext.ReadPendingWrites();
                return status;
            }
            catch (Exception exc)
            {
                return new SecurityStatusPal(SecurityStatusPalErrorCode.InternalError, exc);
            }
        }

        private static SecurityStatusPal PerformHandshake(SafeSslHandle sslHandle)
        {
            while (true)
            {
                PAL_TlsHandshakeState handshakeState = Interop.AppleCrypto.SslHandshake(sslHandle);

                switch (handshakeState)
                {
                    case PAL_TlsHandshakeState.Complete:
                        return new SecurityStatusPal(SecurityStatusPalErrorCode.OK);
                    case PAL_TlsHandshakeState.WouldBlock:
                        return new SecurityStatusPal(SecurityStatusPalErrorCode.ContinueNeeded);
                    case PAL_TlsHandshakeState.ServerAuthCompleted:
                    case PAL_TlsHandshakeState.ClientAuthCompleted:
                        // The standard flow would be to call the verification callback now, and
                        // possibly abort.  But the library is set up to call this "success" and
                        // do verification between "handshake complete" and "first send/receive".
                        //
                        // So, call SslHandshake again to indicate to Secure Transport that we've
                        // accepted this handshake and it should go into the ready state.
                        break;
                    case PAL_TlsHandshakeState.ClientCertRequested:
                        return new SecurityStatusPal(SecurityStatusPalErrorCode.CredentialsNeeded);
                    default:
                        return new SecurityStatusPal(
                            SecurityStatusPalErrorCode.InternalError,
                            Interop.AppleCrypto.CreateExceptionForOSStatus((int)handshakeState));
                }
            }
        }

        public static SecurityStatusPal ApplyAlertToken(
            ref SafeFreeCredentials? credentialsHandle,
            SafeDeleteContext? securityContext,
            TlsAlertType alertType,
            TlsAlertMessage alertMessage)
        {
            // There doesn't seem to be an exposed API for writing an alert,
            // the API seems to assume that all alerts are generated internally by
            // SSLHandshake.
            return new SecurityStatusPal(SecurityStatusPalErrorCode.OK);
        }

        public static SecurityStatusPal ApplyShutdownToken(
            ref SafeFreeCredentials? credentialsHandle,
            SafeDeleteSslContext securityContext)
        {
            SafeSslHandle sslHandle = securityContext.SslContext;

            int osStatus = Interop.AppleCrypto.SslShutdown(sslHandle);

            if (osStatus == 0)
            {
                return new SecurityStatusPal(SecurityStatusPalErrorCode.OK);
            }

            return new SecurityStatusPal(
                SecurityStatusPalErrorCode.InternalError,
                Interop.AppleCrypto.CreateExceptionForOSStatus(osStatus));
        }
    }
}
