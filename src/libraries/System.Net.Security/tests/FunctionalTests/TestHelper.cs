// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.IO;
using System.Net.Sockets;
using System.Net.Test.Common;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Security.Cryptography.X509Certificates.Tests.Common;
using System.Text;

namespace System.Net.Security.Tests
{
    public static class TestHelper
    {
        private static readonly X509KeyUsageExtension s_eeKeyUsage =
            new X509KeyUsageExtension(
                X509KeyUsageFlags.DigitalSignature | X509KeyUsageFlags.KeyEncipherment | X509KeyUsageFlags.DataEncipherment,
                critical: false);

        private static readonly X509EnhancedKeyUsageExtension s_tlsServerEku =
            new X509EnhancedKeyUsageExtension(
                new OidCollection
                {
                    new Oid("1.3.6.1.5.5.7.3.1", null)
                },
                false);

        private static readonly X509BasicConstraintsExtension s_eeConstraints =
            new X509BasicConstraintsExtension(false, false, 0, false);

        public static (Stream ClientStream, Stream ServerStream) GetConnectedStreams()
        {
            if (Capability.SecurityForceSocketStreams())
            {
                return GetConnectedTcpStreams();
            }

            return GetConnectedVirtualStreams();
        }

        internal static (NetworkStream ClientStream, NetworkStream ServerStream) GetConnectedTcpStreams()
        {
            using (Socket listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                listener.Bind(new IPEndPoint(IPAddress.Loopback, 0));
                listener.Listen(1);

                var clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                clientSocket.Connect(listener.LocalEndPoint);
                Socket serverSocket = listener.Accept();

                serverSocket.NoDelay = true;
                clientSocket.NoDelay = true;

                return (new NetworkStream(clientSocket, ownsSocket: true), new NetworkStream(serverSocket, ownsSocket: true));
            }

        }

        internal static (VirtualNetworkStream ClientStream, VirtualNetworkStream ServerStream) GetConnectedVirtualStreams()
        {
            VirtualNetwork vn = new VirtualNetwork();

            return (new VirtualNetworkStream(vn, isServer: false), new VirtualNetworkStream(vn, isServer: true));
        }

        internal static (X509Certificate2 certificate, X509Certificate2Collection) GenerateCertificates(string name, string? testName = null)
        {
            X509Certificate2Collection chain = new X509Certificate2Collection();

            X509ExtensionCollection extensions = new X509ExtensionCollection();

            SubjectAlternativeNameBuilder builder = new SubjectAlternativeNameBuilder();
            builder.AddDnsName(name);
            extensions.Add(builder.Build());
            extensions.Add(s_eeConstraints);
            extensions.Add(s_eeKeyUsage);
            extensions.Add(s_tlsServerEku);

            CertificateAuthority.BuildPrivatePki(
                PkiOptions.IssuerRevocationViaCrl,
                out RevocationResponder responder,
                out CertificateAuthority root,
                out CertificateAuthority intermediate,
                out X509Certificate2 endEntity,
                subjectName: name,
                testName: testName,
                keySize: 2048,
                extensions: extensions);

            chain.Add(intermediate.CloneIssuerCert());
            chain.Add(root.CloneIssuerCert());

            responder.Dispose();
            root.Dispose();
            intermediate.Dispose();

            return (endEntity, chain);
        }
    }
}
