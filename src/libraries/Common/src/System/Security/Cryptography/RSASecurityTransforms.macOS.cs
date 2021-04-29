// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Buffers;
using System.Diagnostics;
using System.Formats.Asn1;
using System.IO;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Security.Cryptography.Apple;
using System.Security.Cryptography.Asn1;
using Internal.Cryptography;

namespace System.Security.Cryptography
{
    internal static partial class RSAImplementation
    {
        public sealed partial class RSASecurityTransforms : RSA
        {
            public override RSAParameters ExportParameters(bool includePrivateParameters)
            {
                // Apple requires all private keys to be exported encrypted, but since we're trying to export
                // as parsed structures we will need to decrypt it for the user.
                const string ExportPassword = "DotnetExportPassphrase";
                SecKeyPair keys = GetKeys();

                if (includePrivateParameters && keys.PrivateKey == null)
                {
                    throw new CryptographicException(SR.Cryptography_OpenInvalidHandle);
                }

                byte[] keyBlob = Interop.AppleCrypto.SecKeyExport(
                    includePrivateParameters ? keys.PrivateKey : keys.PublicKey,
                    exportPrivate: includePrivateParameters,
                    password: ExportPassword);

                try
                {
                    if (!includePrivateParameters)
                    {
                        // When exporting a key handle opened from a certificate, it seems to
                        // export as a PKCS#1 blob instead of an X509 SubjectPublicKeyInfo blob.
                        // So, check for that.
                        // NOTE: It doesn't affect macOS Mojave when SecCertificateCopyKey API
                        // is used.
                        RSAParameters key;

                        AsnReader reader = new AsnReader(keyBlob, AsnEncodingRules.BER);
                        AsnReader sequenceReader = reader.ReadSequence();

                        if (sequenceReader.PeekTag().Equals(Asn1Tag.Integer))
                        {
                            AlgorithmIdentifierAsn ignored = default;
                            RSAKeyFormatHelper.ReadRsaPublicKey(keyBlob, ignored, out key);
                        }
                        else
                        {
                            RSAKeyFormatHelper.ReadSubjectPublicKeyInfo(
                                keyBlob,
                                out int localRead,
                                out key);
                            Debug.Assert(localRead == keyBlob.Length);
                        }
                        return key;
                    }
                    else
                    {
                        RSAKeyFormatHelper.ReadEncryptedPkcs8(
                            keyBlob,
                            ExportPassword,
                            out int localRead,
                            out RSAParameters key);
                        return key;
                    }
                }
                finally
                {
                    CryptographicOperations.ZeroMemory(keyBlob);
                }
            }

            public override void ImportParameters(RSAParameters parameters)
            {
                ValidateParameters(parameters);
                ThrowIfDisposed();

                bool isPrivateKey = parameters.D != null;

                if (isPrivateKey)
                {
                    // Start with the private key, in case some of the private key fields
                    // don't match the public key fields.
                    //
                    // Public import should go off without a hitch.
                    SafeSecKeyRefHandle privateKey = ImportKey(parameters);

                    RSAParameters publicOnly = new RSAParameters
                    {
                        Modulus = parameters.Modulus,
                        Exponent = parameters.Exponent,
                    };

                    SafeSecKeyRefHandle publicKey;
                    try
                    {
                        publicKey = ImportKey(publicOnly);
                    }
                    catch
                    {
                        privateKey.Dispose();
                        throw;
                    }

                    SetKey(SecKeyPair.PublicPrivatePair(publicKey, privateKey));
                }
                else
                {
                    SafeSecKeyRefHandle publicKey = ImportKey(parameters);
                    SetKey(SecKeyPair.PublicOnly(publicKey));
                }
            }

            private static SafeSecKeyRefHandle ImportKey(RSAParameters parameters)
            {
                AsnWriter keyWriter;
                bool hasPrivateKey;

                if (parameters.D != null)
                {
                    keyWriter = RSAKeyFormatHelper.WritePkcs1PrivateKey(parameters);
                    hasPrivateKey = true;
                }
                else
                {
                    keyWriter = RSAKeyFormatHelper.WriteSubjectPublicKeyInfo(parameters);
                    hasPrivateKey = false;
                }

                byte[] rented = CryptoPool.Rent(keyWriter.GetEncodedLength());

                if (!keyWriter.TryEncode(rented, out int written))
                {
                    Debug.Fail("TryEncode failed with a pre-allocated buffer");
                    throw new InvalidOperationException();
                }

                // Explicitly clear the inner buffer
                keyWriter.Reset();

                try
                {
                    return Interop.AppleCrypto.ImportEphemeralKey(rented.AsSpan(0, written), hasPrivateKey);
                }
                finally
                {
                    CryptoPool.Return(rented, written);
                }
            }

            public override unsafe void ImportSubjectPublicKeyInfo(
                ReadOnlySpan<byte> source,
                out int bytesRead)
            {
                ThrowIfDisposed();

                fixed (byte* ptr = &MemoryMarshal.GetReference(source))
                {
                    using (MemoryManager<byte> manager = new PointerMemoryManager<byte>(ptr, source.Length))
                    {
                        // Validate the DER value and get the number of bytes.
                        RSAKeyFormatHelper.ReadSubjectPublicKeyInfo(
                            manager.Memory,
                            out int localRead);

                        SafeSecKeyRefHandle publicKey = Interop.AppleCrypto.ImportEphemeralKey(source.Slice(0, localRead), false);
                        SetKey(SecKeyPair.PublicOnly(publicKey));

                        bytesRead = localRead;
                    }
                }
            }

            public override unsafe void ImportRSAPublicKey(ReadOnlySpan<byte> source, out int bytesRead)
            {
                ThrowIfDisposed();

                fixed (byte* ptr = &MemoryMarshal.GetReference(source))
                {
                    using (MemoryManager<byte> manager = new PointerMemoryManager<byte>(ptr, source.Length))
                    {
                        AsnReader reader = new AsnReader(manager.Memory, AsnEncodingRules.BER);
                        ReadOnlyMemory<byte> firstElement = reader.PeekEncodedValue();

                        SubjectPublicKeyInfoAsn spki = new SubjectPublicKeyInfoAsn
                        {
                            Algorithm = new AlgorithmIdentifierAsn
                            {
                                Algorithm = Oids.Rsa,
                                Parameters = AlgorithmIdentifierAsn.ExplicitDerNull,
                            },
                            SubjectPublicKey = firstElement,
                        };

                        AsnWriter writer = new AsnWriter(AsnEncodingRules.DER);
                        spki.Encode(writer);

                        ImportSubjectPublicKeyInfo(writer.Encode(), out _);
                        bytesRead = firstElement.Length;
                    }
                }
            }
        }
    }
}
