// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#if NETCOREAPP
namespace System.Formats.Asn1
{
    internal static class WellKnownOids
    {
        private static ReadOnlySpan<byte> DSA =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x01, };
        private static ReadOnlySpan<byte> DSAWithSha1 =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x03, };
        private static ReadOnlySpan<byte> EC =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, };
        private static ReadOnlySpan<byte> EcPrimeField =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x01, 0x01, };
        private static ReadOnlySpan<byte> EcChar2Field =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x01, 0x02, };
        private static ReadOnlySpan<byte> Secp256r1 =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, };
        private static ReadOnlySpan<byte> ECDSAWithSha1 =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01, };
        private static ReadOnlySpan<byte> ECDSAWithSha256 =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, };
        private static ReadOnlySpan<byte> ECDSAWithSha384 =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x03, };
        private static ReadOnlySpan<byte> ECDSAWithSha512 =>
            new byte[] { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x04, };
        private static ReadOnlySpan<byte> RSA =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, };
        private static ReadOnlySpan<byte> RSAWithSha1 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05, };
        private static ReadOnlySpan<byte> RSAOAEP =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x07, };
        private static ReadOnlySpan<byte> MGF1 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x08, };
        private static ReadOnlySpan<byte> OaepPSpecified =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x09, };
        private static ReadOnlySpan<byte> RSAPSS =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0A, };
        private static ReadOnlySpan<byte> RSAWithSha256 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, };
        private static ReadOnlySpan<byte> RSAWithSha384 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0C, };
        private static ReadOnlySpan<byte> RSAWithSha512 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0D, };
        private static ReadOnlySpan<byte> PbeWithMD5AndDESCBC =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x03, };
        private static ReadOnlySpan<byte> PbeWithSha1AndDESCBC =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0A, };
        private static ReadOnlySpan<byte> PbeWithSha1AndRC2CBC =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0B, };
        private static ReadOnlySpan<byte> Pbkdf2 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0C, };
        private static ReadOnlySpan<byte> PasswordBasedEncryptionScheme2 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0D, };
        private static ReadOnlySpan<byte> Pkcs7Data =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01, };
        private static ReadOnlySpan<byte> Pkcs7SignedData =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02, };
        private static ReadOnlySpan<byte> Pkcs7EnvelopedData =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x03, };
        private static ReadOnlySpan<byte> Pkcs7EncryptedData =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x06, };
        private static ReadOnlySpan<byte> Pkcs9EmailAddress =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x01, };
        private static ReadOnlySpan<byte> Pkcs9ContentType =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x03, };
        private static ReadOnlySpan<byte> Pkcs9MessageDigest =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x04, };
        private static ReadOnlySpan<byte> Pkcs9SigningTime =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x05, };
        private static ReadOnlySpan<byte> Pkcs9CounterSigner =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x06, };
        private static ReadOnlySpan<byte> Pkcs9Challenge =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x07, };
        private static ReadOnlySpan<byte> Pkcs9ExtensionRequest =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x0E, };
        private static ReadOnlySpan<byte> Pkcs9SMimeCapabilities =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x0F, };
        private static ReadOnlySpan<byte> TstInfo =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x01, 0x04, };
        private static ReadOnlySpan<byte> SigningCertificateAttr =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x02, 0x0C, };
        private static ReadOnlySpan<byte> SignatureTimeStampAttr =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x02, 0x0E, };
        private static ReadOnlySpan<byte> SigningCertificateV2Attr =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x02, 0x2F, };
        private static ReadOnlySpan<byte> Pkcs9FriendlyName =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x14, };
        private static ReadOnlySpan<byte> LocalKeyId =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x15, };
        private static ReadOnlySpan<byte> Pkcs12X509CertType =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x16, 0x01, };
        private static ReadOnlySpan<byte> Pkcs12TripleDes =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x01, 0x03, };
        private static ReadOnlySpan<byte> Pkcs12Rc2Cbc128 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x01, 0x05, };
        private static ReadOnlySpan<byte> Pkcs12Rc2Cbc40 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x01, 0x06, };
        private static ReadOnlySpan<byte> Pkcs12KeyBag =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x01, };
        private static ReadOnlySpan<byte> Pkcs12ShroudedKeyBag =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x02, };
        private static ReadOnlySpan<byte> Pkcs12CertBag =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x03, };
        private static ReadOnlySpan<byte> Pkcs12SecretBag =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x05, };
        private static ReadOnlySpan<byte> Pkcs12SafeContentsBag =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x06, };
        private static ReadOnlySpan<byte> MD5 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05, };
        private static ReadOnlySpan<byte> HMACSHA1 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x07, };
        private static ReadOnlySpan<byte> HMACSHA256 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x09, };
        private static ReadOnlySpan<byte> HMACSHA384 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x0A, };
        private static ReadOnlySpan<byte> HMACSHA512 =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x0B, };
        private static ReadOnlySpan<byte> RC2CBC =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x03, 0x02, };
        private static ReadOnlySpan<byte> TripleDESCBC =>
            new byte[] { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x03, 0x07, };
        private static ReadOnlySpan<byte> Pkcs12KeyProviderName =>
            new byte[] { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x11, 0x01, };
        private static ReadOnlySpan<byte> KeyIdentifier =>
            new byte[] { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x11, 0x03, 0x14, };
        private static ReadOnlySpan<byte> UserPrincipalName =>
            new byte[] { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x14, 0x02, 0x03, };
        private static ReadOnlySpan<byte> DocumentNameAttr =>
            new byte[] { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x58, 0x02, 0x01, };
        private static ReadOnlySpan<byte> DocumentDescriptionAttr =>
            new byte[] { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x58, 0x02, 0x02, };
        private static ReadOnlySpan<byte> KeyPurposeTlsServer =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01, };
        private static ReadOnlySpan<byte> KeyPurposeTlsClient =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, };
        private static ReadOnlySpan<byte> KeyPurposeCodeSign =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x03, };
        private static ReadOnlySpan<byte> KeyPurposeEmailProtection =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x04, };
        private static ReadOnlySpan<byte> KeyPurposeTimestamping =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x08, };
        private static ReadOnlySpan<byte> KeyPurposeOcspSigner =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x09, };
        private static ReadOnlySpan<byte> Pkcs7NoSignature =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x06, 0x02, };
        private static ReadOnlySpan<byte> OCSP =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, };
        private static ReadOnlySpan<byte> OcspNonce =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x02, };
        private static ReadOnlySpan<byte> CAIssuers =>
            new byte[] { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x02, };
        private static ReadOnlySpan<byte> SHA1 =>
            new byte[] { 0x2B, 0x0E, 0x03, 0x02, 0x1A, };
        private static ReadOnlySpan<byte> DES =>
            new byte[] { 0x2B, 0x0E, 0x03, 0x02, 0x07, };
        private static ReadOnlySpan<byte> Secp384r1 =>
            new byte[] { 0x2B, 0x81, 0x04, 0x00, 0x22, };
        private static ReadOnlySpan<byte> Secp521r1 =>
            new byte[] { 0x2B, 0x81, 0x04, 0x00, 0x23, };
        private static ReadOnlySpan<byte> CommonName =>
            new byte[] { 0x55, 0x04, 0x03, };
        private static ReadOnlySpan<byte> SerialNumber =>
            new byte[] { 0x55, 0x04, 0x05, };
        private static ReadOnlySpan<byte> CountryOrRegionName =>
            new byte[] { 0x55, 0x04, 0x06, };
        private static ReadOnlySpan<byte> Locality =>
            new byte[] { 0x55, 0x04, 0x07, };
        private static ReadOnlySpan<byte> StateOrProvinceName =>
            new byte[] { 0x55, 0x04, 0x08, };
        private static ReadOnlySpan<byte> OrganizationName =>
            new byte[] { 0x55, 0x04, 0x0A, };
        private static ReadOnlySpan<byte> OrganizationalUnit =>
            new byte[] { 0x55, 0x04, 0x0B, };
        private static ReadOnlySpan<byte> OrganizationIdentifier =>
            new byte[] { 0x55, 0x04, 0x61, };
        private static ReadOnlySpan<byte> SubjectKeyIdentifier =>
            new byte[] { 0x55, 0x1D, 0x0E, };
        private static ReadOnlySpan<byte> KeyUsage =>
            new byte[] { 0x55, 0x1D, 0x0F, };
        private static ReadOnlySpan<byte> SubjectAlternativeName =>
            new byte[] { 0x55, 0x1D, 0x11, };
        private static ReadOnlySpan<byte> BasicConstraints =>
            new byte[] { 0x55, 0x1D, 0x13, };
        private static ReadOnlySpan<byte> CrlNumber =>
            new byte[] { 0x55, 0x1D, 0x14, };
        private static ReadOnlySpan<byte> AuthorityKeyIdentifier =>
            new byte[] { 0x55, 0x1D, 0x23, };
        private static ReadOnlySpan<byte> Aes128Cbc =>
            new byte[] { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x02, };
        private static ReadOnlySpan<byte> Aes192Cbc =>
            new byte[] { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x16, };
        private static ReadOnlySpan<byte> Aes256Cbc =>
            new byte[] { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x2A, };
        private static ReadOnlySpan<byte> Sha256 =>
            new byte[] { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, };
        private static ReadOnlySpan<byte> Sha384 =>
            new byte[] { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, };
        private static ReadOnlySpan<byte> Sha512 =>
            new byte[] { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, };
        private static ReadOnlySpan<byte> CabForumDV =>
            new byte[] { 0x67, 0x81, 0x0C, 0x01, 0x02, 0x01, };
        private static ReadOnlySpan<byte> CabForumOV =>
            new byte[] { 0x67, 0x81, 0x0C, 0x01, 0x02, 0x02, };

        internal static string? GetValue(ReadOnlySpan<byte> contents)
        {
            return contents switch
            {
                [ 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x01, ] => "1.2.840.10040.4.1",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x03, ] => "1.2.840.10040.4.3",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, ] => "1.2.840.10045.2.1",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x01, 0x01, ] => "1.2.840.10045.1.1",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x01, 0x02, ] => "1.2.840.10045.1.2",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, ] => "1.2.840.10045.3.1.7",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01, ] => "1.2.840.10045.4.1",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, ] => "1.2.840.10045.4.3.2",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x03, ] => "1.2.840.10045.4.3.3",
                [ 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x04, ] => "1.2.840.10045.4.3.4",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, ] => "1.2.840.113549.1.1.1",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05, ] => "1.2.840.113549.1.1.5",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x07, ] => "1.2.840.113549.1.1.7",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x08, ] => "1.2.840.113549.1.1.8",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x09, ] => "1.2.840.113549.1.1.9",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0A, ] => "1.2.840.113549.1.1.10",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, ] => "1.2.840.113549.1.1.11",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0C, ] => "1.2.840.113549.1.1.12",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0D, ] => "1.2.840.113549.1.1.13",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x03, ] => "1.2.840.113549.1.5.3",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0A, ] => "1.2.840.113549.1.5.10",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0B, ] => "1.2.840.113549.1.5.11",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0C, ] => "1.2.840.113549.1.5.12",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x05, 0x0D, ] => "1.2.840.113549.1.5.13",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01, ] => "1.2.840.113549.1.7.1",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02, ] => "1.2.840.113549.1.7.2",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x03, ] => "1.2.840.113549.1.7.3",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x06, ] => "1.2.840.113549.1.7.6",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x01, ] => "1.2.840.113549.1.9.1",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x03, ] => "1.2.840.113549.1.9.3",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x04, ] => "1.2.840.113549.1.9.4",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x05, ] => "1.2.840.113549.1.9.5",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x06, ] => "1.2.840.113549.1.9.6",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x07, ] => "1.2.840.113549.1.9.7",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x0E, ] => "1.2.840.113549.1.9.14",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x0F, ] => "1.2.840.113549.1.9.15",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x01, 0x04, ] => "1.2.840.113549.1.9.16.1.4",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x02, 0x0C, ] => "1.2.840.113549.1.9.16.2.12",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x02, 0x0E, ] => "1.2.840.113549.1.9.16.2.14",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x10, 0x02, 0x2F, ] => "1.2.840.113549.1.9.16.2.47",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x14, ] => "1.2.840.113549.1.9.20",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x15, ] => "1.2.840.113549.1.9.21",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x16, 0x01, ] => "1.2.840.113549.1.9.22.1",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x01, 0x03, ] => "1.2.840.113549.1.12.1.3",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x01, 0x05, ] => "1.2.840.113549.1.12.1.5",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x01, 0x06, ] => "1.2.840.113549.1.12.1.6",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x01, ] => "1.2.840.113549.1.12.10.1.1",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x02, ] => "1.2.840.113549.1.12.10.1.2",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x03, ] => "1.2.840.113549.1.12.10.1.3",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x05, ] => "1.2.840.113549.1.12.10.1.5",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x0C, 0x0A, 0x01, 0x06, ] => "1.2.840.113549.1.12.10.1.6",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05, ] => "1.2.840.113549.2.5",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x07, ] => "1.2.840.113549.2.7",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x09, ] => "1.2.840.113549.2.9",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x0A, ] => "1.2.840.113549.2.10",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x0B, ] => "1.2.840.113549.2.11",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x03, 0x02, ] => "1.2.840.113549.3.2",
                [ 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x03, 0x07, ] => "1.2.840.113549.3.7",
                [ 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x11, 0x01, ] => "1.3.6.1.4.1.311.17.1",
                [ 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x11, 0x03, 0x14, ] => "1.3.6.1.4.1.311.17.3.20",
                [ 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x14, 0x02, 0x03, ] => "1.3.6.1.4.1.311.20.2.3",
                [ 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x58, 0x02, 0x01, ] => "1.3.6.1.4.1.311.88.2.1",
                [ 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x58, 0x02, 0x02, ] => "1.3.6.1.4.1.311.88.2.2",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01, ] => "1.3.6.1.5.5.7.3.1",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, ] => "1.3.6.1.5.5.7.3.2",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x03, ] => "1.3.6.1.5.5.7.3.3",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x04, ] => "1.3.6.1.5.5.7.3.4",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x08, ] => "1.3.6.1.5.5.7.3.8",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x09, ] => "1.3.6.1.5.5.7.3.9",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x06, 0x02, ] => "1.3.6.1.5.5.7.6.2",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, ] => "1.3.6.1.5.5.7.48.1",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x02, ] => "1.3.6.1.5.5.7.48.1.2",
                [ 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x02, ] => "1.3.6.1.5.5.7.48.2",
                [ 0x2B, 0x0E, 0x03, 0x02, 0x1A, ] => "1.3.14.3.2.26",
                [ 0x2B, 0x0E, 0x03, 0x02, 0x07, ] => "1.3.14.3.2.7",
                [ 0x2B, 0x81, 0x04, 0x00, 0x22, ] => "1.3.132.0.34",
                [ 0x2B, 0x81, 0x04, 0x00, 0x23, ] => "1.3.132.0.35",
                [ 0x55, 0x04, 0x03, ] => "2.5.4.3",
                [ 0x55, 0x04, 0x05, ] => "2.5.4.5",
                [ 0x55, 0x04, 0x06, ] => "2.5.4.6",
                [ 0x55, 0x04, 0x07, ] => "2.5.4.7",
                [ 0x55, 0x04, 0x08, ] => "2.5.4.8",
                [ 0x55, 0x04, 0x0A, ] => "2.5.4.10",
                [ 0x55, 0x04, 0x0B, ] => "2.5.4.11",
                [ 0x55, 0x04, 0x61, ] => "2.5.4.97",
                [ 0x55, 0x1D, 0x0E, ] => "2.5.29.14",
                [ 0x55, 0x1D, 0x0F, ] => "2.5.29.15",
                [ 0x55, 0x1D, 0x11, ] => "2.5.29.17",
                [ 0x55, 0x1D, 0x13, ] => "2.5.29.19",
                [ 0x55, 0x1D, 0x14, ] => "2.5.29.20",
                [ 0x55, 0x1D, 0x23, ] => "2.5.29.35",
                [ 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x02, ] => "2.16.840.1.101.3.4.1.2",
                [ 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x16, ] => "2.16.840.1.101.3.4.1.22",
                [ 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x2A, ] => "2.16.840.1.101.3.4.1.42",
                [ 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, ] => "2.16.840.1.101.3.4.2.1",
                [ 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, ] => "2.16.840.1.101.3.4.2.2",
                [ 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, ] => "2.16.840.1.101.3.4.2.3",
                [ 0x67, 0x81, 0x0C, 0x01, 0x02, 0x01, ] => "2.23.140.1.2.1",
                [ 0x67, 0x81, 0x0C, 0x01, 0x02, 0x02, ] => "2.23.140.1.2.2",
               _ => null,
            };
        }

        internal static ReadOnlySpan<byte> GetContents(ReadOnlySpan<char> value)
        {
            return value switch
            {
                "1.2.840.10040.4.1" => DSA,
                "1.2.840.10040.4.3" => DSAWithSha1,
                "1.2.840.10045.2.1" => EC,
                "1.2.840.10045.1.1" => EcPrimeField,
                "1.2.840.10045.1.2" => EcChar2Field,
                "1.2.840.10045.3.1.7" => Secp256r1,
                "1.2.840.10045.4.1" => ECDSAWithSha1,
                "1.2.840.10045.4.3.2" => ECDSAWithSha256,
                "1.2.840.10045.4.3.3" => ECDSAWithSha384,
                "1.2.840.10045.4.3.4" => ECDSAWithSha512,
                "1.2.840.113549.1.1.1" => RSA,
                "1.2.840.113549.1.1.5" => RSAWithSha1,
                "1.2.840.113549.1.1.7" => RSAOAEP,
                "1.2.840.113549.1.1.8" => MGF1,
                "1.2.840.113549.1.1.9" => OaepPSpecified,
                "1.2.840.113549.1.1.10" => RSAPSS,
                "1.2.840.113549.1.1.11" => RSAWithSha256,
                "1.2.840.113549.1.1.12" => RSAWithSha384,
                "1.2.840.113549.1.1.13" => RSAWithSha512,
                "1.2.840.113549.1.5.3" => PbeWithMD5AndDESCBC,
                "1.2.840.113549.1.5.10" => PbeWithSha1AndDESCBC,
                "1.2.840.113549.1.5.11" => PbeWithSha1AndRC2CBC,
                "1.2.840.113549.1.5.12" => Pbkdf2,
                "1.2.840.113549.1.5.13" => PasswordBasedEncryptionScheme2,
                "1.2.840.113549.1.7.1" => Pkcs7Data,
                "1.2.840.113549.1.7.2" => Pkcs7SignedData,
                "1.2.840.113549.1.7.3" => Pkcs7EnvelopedData,
                "1.2.840.113549.1.7.6" => Pkcs7EncryptedData,
                "1.2.840.113549.1.9.1" => Pkcs9EmailAddress,
                "1.2.840.113549.1.9.3" => Pkcs9ContentType,
                "1.2.840.113549.1.9.4" => Pkcs9MessageDigest,
                "1.2.840.113549.1.9.5" => Pkcs9SigningTime,
                "1.2.840.113549.1.9.6" => Pkcs9CounterSigner,
                "1.2.840.113549.1.9.7" => Pkcs9Challenge,
                "1.2.840.113549.1.9.14" => Pkcs9ExtensionRequest,
                "1.2.840.113549.1.9.15" => Pkcs9SMimeCapabilities,
                "1.2.840.113549.1.9.16.1.4" => TstInfo,
                "1.2.840.113549.1.9.16.2.12" => SigningCertificateAttr,
                "1.2.840.113549.1.9.16.2.14" => SignatureTimeStampAttr,
                "1.2.840.113549.1.9.16.2.47" => SigningCertificateV2Attr,
                "1.2.840.113549.1.9.20" => Pkcs9FriendlyName,
                "1.2.840.113549.1.9.21" => LocalKeyId,
                "1.2.840.113549.1.9.22.1" => Pkcs12X509CertType,
                "1.2.840.113549.1.12.1.3" => Pkcs12TripleDes,
                "1.2.840.113549.1.12.1.5" => Pkcs12Rc2Cbc128,
                "1.2.840.113549.1.12.1.6" => Pkcs12Rc2Cbc40,
                "1.2.840.113549.1.12.10.1.1" => Pkcs12KeyBag,
                "1.2.840.113549.1.12.10.1.2" => Pkcs12ShroudedKeyBag,
                "1.2.840.113549.1.12.10.1.3" => Pkcs12CertBag,
                "1.2.840.113549.1.12.10.1.5" => Pkcs12SecretBag,
                "1.2.840.113549.1.12.10.1.6" => Pkcs12SafeContentsBag,
                "1.2.840.113549.2.5" => MD5,
                "1.2.840.113549.2.7" => HMACSHA1,
                "1.2.840.113549.2.9" => HMACSHA256,
                "1.2.840.113549.2.10" => HMACSHA384,
                "1.2.840.113549.2.11" => HMACSHA512,
                "1.2.840.113549.3.2" => RC2CBC,
                "1.2.840.113549.3.7" => TripleDESCBC,
                "1.3.6.1.4.1.311.17.1" => Pkcs12KeyProviderName,
                "1.3.6.1.4.1.311.17.3.20" => KeyIdentifier,
                "1.3.6.1.4.1.311.20.2.3" => UserPrincipalName,
                "1.3.6.1.4.1.311.88.2.1" => DocumentNameAttr,
                "1.3.6.1.4.1.311.88.2.2" => DocumentDescriptionAttr,
                "1.3.6.1.5.5.7.3.1" => KeyPurposeTlsServer,
                "1.3.6.1.5.5.7.3.2" => KeyPurposeTlsClient,
                "1.3.6.1.5.5.7.3.3" => KeyPurposeCodeSign,
                "1.3.6.1.5.5.7.3.4" => KeyPurposeEmailProtection,
                "1.3.6.1.5.5.7.3.8" => KeyPurposeTimestamping,
                "1.3.6.1.5.5.7.3.9" => KeyPurposeOcspSigner,
                "1.3.6.1.5.5.7.6.2" => Pkcs7NoSignature,
                "1.3.6.1.5.5.7.48.1" => OCSP,
                "1.3.6.1.5.5.7.48.1.2" => OcspNonce,
                "1.3.6.1.5.5.7.48.2" => CAIssuers,
                "1.3.14.3.2.26" => SHA1,
                "1.3.14.3.2.7" => DES,
                "1.3.132.0.34" => Secp384r1,
                "1.3.132.0.35" => Secp521r1,
                "2.5.4.3" => CommonName,
                "2.5.4.5" => SerialNumber,
                "2.5.4.6" => CountryOrRegionName,
                "2.5.4.7" => Locality,
                "2.5.4.8" => StateOrProvinceName,
                "2.5.4.10" => OrganizationName,
                "2.5.4.11" => OrganizationalUnit,
                "2.5.4.97" => OrganizationIdentifier,
                "2.5.29.14" => SubjectKeyIdentifier,
                "2.5.29.15" => KeyUsage,
                "2.5.29.17" => SubjectAlternativeName,
                "2.5.29.19" => BasicConstraints,
                "2.5.29.20" => CrlNumber,
                "2.5.29.35" => AuthorityKeyIdentifier,
                "2.16.840.1.101.3.4.1.2" => Aes128Cbc,
                "2.16.840.1.101.3.4.1.22" => Aes192Cbc,
                "2.16.840.1.101.3.4.1.42" => Aes256Cbc,
                "2.16.840.1.101.3.4.2.1" => Sha256,
                "2.16.840.1.101.3.4.2.2" => Sha384,
                "2.16.840.1.101.3.4.2.3" => Sha512,
                "2.23.140.1.2.1" => CabForumDV,
                "2.23.140.1.2.2" => CabForumOV,
                _ => ReadOnlySpan<byte>.Empty,
            };
        }
    }
}
#endif
