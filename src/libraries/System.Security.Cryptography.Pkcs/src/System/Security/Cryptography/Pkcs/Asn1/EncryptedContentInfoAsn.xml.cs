// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Security.Cryptography.Asn1;

namespace System.Security.Cryptography.Pkcs.Asn1
{
    [StructLayout(LayoutKind.Sequential)]
    internal partial struct EncryptedContentInfoAsn
    {
        internal string ContentType;
        internal System.Security.Cryptography.Asn1.AlgorithmIdentifierAsn ContentEncryptionAlgorithm;
        internal ReadOnlyMemory<byte>? EncryptedContent;

        internal void Encode(AsnWriter writer)
        {
            Encode(writer, Asn1Tag.Sequence);
        }

        internal void Encode(AsnWriter writer, Asn1Tag tag)
        {
            writer.PushSequence(tag);

            writer.WriteObjectIdentifier(ContentType);
            ContentEncryptionAlgorithm.Encode(writer);

            if (EncryptedContent.HasValue)
            {
                writer.WriteOctetString(new Asn1Tag(TagClass.ContextSpecific, 0), EncryptedContent.Value.Span);
            }

            writer.PopSequence(tag);
        }

        internal static EncryptedContentInfoAsn Decode(ReadOnlyMemory<byte> encoded, AsnEncodingRules ruleSet)
        {
            return Decode(Asn1Tag.Sequence, encoded, ruleSet);
        }

        internal static EncryptedContentInfoAsn Decode(Asn1Tag expectedTag, ReadOnlyMemory<byte> encoded, AsnEncodingRules ruleSet)
        {
            AsnReader reader = new AsnReader(encoded, ruleSet);

            Decode(reader, expectedTag, out EncryptedContentInfoAsn decoded);
            reader.ThrowIfNotEmpty();
            return decoded;
        }

        internal static void Decode(AsnReader reader, out EncryptedContentInfoAsn decoded)
        {
            if (reader == null)
                throw new ArgumentNullException(nameof(reader));

            Decode(reader, Asn1Tag.Sequence, out decoded);
        }

        internal static void Decode(AsnReader reader, Asn1Tag expectedTag, out EncryptedContentInfoAsn decoded)
        {
            if (reader == null)
                throw new ArgumentNullException(nameof(reader));

            decoded = default;
            AsnReader sequenceReader = reader.ReadSequence(expectedTag);

            decoded.ContentType = sequenceReader.ReadObjectIdentifierAsString();
            System.Security.Cryptography.Asn1.AlgorithmIdentifierAsn.Decode(sequenceReader, out decoded.ContentEncryptionAlgorithm);

            if (sequenceReader.HasData && sequenceReader.PeekTag().HasSameClassAndValue(new Asn1Tag(TagClass.ContextSpecific, 0)))
            {

                if (sequenceReader.TryReadPrimitiveOctetStringBytes(new Asn1Tag(TagClass.ContextSpecific, 0), out ReadOnlyMemory<byte> tmpEncryptedContent))
                {
                    decoded.EncryptedContent = tmpEncryptedContent;
                }
                else
                {
                    decoded.EncryptedContent = sequenceReader.ReadOctetString(new Asn1Tag(TagClass.ContextSpecific, 0));
                }

            }


            sequenceReader.ThrowIfNotEmpty();
        }
    }
}
