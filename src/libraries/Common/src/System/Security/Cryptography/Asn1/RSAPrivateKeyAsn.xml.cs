﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#pragma warning disable SA1028 // ignore whitespace warnings for generated code
using System;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Security.Cryptography.Asn1;

namespace System.Security.Cryptography.Asn1
{
    [StructLayout(LayoutKind.Sequential)]
    internal partial struct RSAPrivateKeyAsn
    {
        internal byte Version;
        internal System.Numerics.BigInteger Modulus;
        internal System.Numerics.BigInteger PublicExponent;
        internal System.Numerics.BigInteger PrivateExponent;
        internal System.Numerics.BigInteger Prime1;
        internal System.Numerics.BigInteger Prime2;
        internal System.Numerics.BigInteger Exponent1;
        internal System.Numerics.BigInteger Exponent2;
        internal System.Numerics.BigInteger Coefficient;

        internal void Encode(AsnWriter writer)
        {
            Encode(writer, Asn1Tag.Sequence);
        }

        internal void Encode(AsnWriter writer, Asn1Tag tag)
        {
            writer.PushSequence(tag);

            writer.WriteInteger(Version);
            writer.WriteInteger(Modulus);
            writer.WriteInteger(PublicExponent);
            writer.WriteInteger(PrivateExponent);
            writer.WriteInteger(Prime1);
            writer.WriteInteger(Prime2);
            writer.WriteInteger(Exponent1);
            writer.WriteInteger(Exponent2);
            writer.WriteInteger(Coefficient);
            writer.PopSequence(tag);
        }

        internal static RSAPrivateKeyAsn Decode(ReadOnlyMemory<byte> encoded, AsnEncodingRules ruleSet)
        {
            return Decode(Asn1Tag.Sequence, encoded, ruleSet);
        }

        internal static RSAPrivateKeyAsn Decode(Asn1Tag expectedTag, ReadOnlyMemory<byte> encoded, AsnEncodingRules ruleSet)
        {
            AsnValueReader reader = new AsnValueReader(encoded.Span, ruleSet);

            Decode(ref reader, expectedTag, encoded, out RSAPrivateKeyAsn decoded);
            reader.ThrowIfNotEmpty();
            return decoded;
        }

        internal static void Decode(ref AsnValueReader reader, ReadOnlyMemory<byte> rebind, out RSAPrivateKeyAsn decoded)
        {
            Decode(ref reader, Asn1Tag.Sequence, rebind, out decoded);
        }

        internal static void Decode(ref AsnValueReader reader, Asn1Tag expectedTag, ReadOnlyMemory<byte> rebind, out RSAPrivateKeyAsn decoded)
        {
            decoded = default;
            AsnValueReader sequenceReader = reader.ReadSequence(expectedTag);


            if (!sequenceReader.TryReadUInt8(out decoded.Version))
            {
                sequenceReader.ThrowIfNotEmpty();
            }

            decoded.Modulus = sequenceReader.ReadInteger();
            decoded.PublicExponent = sequenceReader.ReadInteger();
            decoded.PrivateExponent = sequenceReader.ReadInteger();
            decoded.Prime1 = sequenceReader.ReadInteger();
            decoded.Prime2 = sequenceReader.ReadInteger();
            decoded.Exponent1 = sequenceReader.ReadInteger();
            decoded.Exponent2 = sequenceReader.ReadInteger();
            decoded.Coefficient = sequenceReader.ReadInteger();

            sequenceReader.ThrowIfNotEmpty();
        }
    }
}
