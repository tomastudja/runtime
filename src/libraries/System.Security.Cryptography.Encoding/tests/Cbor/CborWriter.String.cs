﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#nullable enable
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;

namespace System.Security.Cryptography.Encoding.Tests.Cbor
{
    internal partial class CborWriter
    {
        private static readonly System.Text.Encoding s_utf8Encoding = new System.Text.UTF8Encoding(encoderShouldEmitUTF8Identifier: false, throwOnInvalidBytes: true);

        // keeps track of chunk offsets for written indefinite-length string ranges
        private List<(int Offset, int Length)>? _currentIndefiniteLengthStringRanges = null;

        // Implements major type 2 encoding per https://tools.ietf.org/html/rfc7049#section-2.1
        public void WriteByteString(ReadOnlySpan<byte> value)
        {
            WriteUnsignedInteger(CborMajorType.ByteString, (ulong)value.Length);
            EnsureWriteCapacity(value.Length);

            if (PatchIndefiniteLengthItems && IsMajorTypeContext(CborMajorType.ByteString))
            {
                // operation is writing chunk of an indefinite-length string
                Debug.Assert(_currentIndefiniteLengthStringRanges != null);
                _currentIndefiniteLengthStringRanges.Add((_offset, value.Length));
            }

            value.CopyTo(_buffer.AsSpan(_offset));
            _offset += value.Length;
            AdvanceDataItemCounters();
        }

        // Implements major type 3 encoding per https://tools.ietf.org/html/rfc7049#section-2.1
        public void WriteTextString(ReadOnlySpan<char> value)
        {
            int length;
            try
            {
                length = s_utf8Encoding.GetByteCount(value);
            }
            catch (EncoderFallbackException e)
            {
                throw new ArgumentException("Provided text string is not valid UTF8.", e);
            }

            WriteUnsignedInteger(CborMajorType.TextString, (ulong)length);
            EnsureWriteCapacity(length);

            if (PatchIndefiniteLengthItems && IsMajorTypeContext(CborMajorType.TextString))
            {
                // operation is writing chunk of an indefinite-length string
                Debug.Assert(_currentIndefiniteLengthStringRanges != null);
                _currentIndefiniteLengthStringRanges.Add((_offset, value.Length));
            }

            s_utf8Encoding.GetBytes(value, _buffer.AsSpan(_offset, length));
            _offset += length;
            AdvanceDataItemCounters();
        }

        public void WriteStartByteStringIndefiniteLength()
        {
            if (PatchIndefiniteLengthItems)
            {
                _currentIndefiniteLengthStringRanges ??= new List<(int, int)>();
            }
            else if (CborConformanceLevelHelpers.RequiresDefiniteLengthItems(ConformanceLevel))
            {
                throw new InvalidOperationException("Indefinite-length items are not permitted under the current conformance level.");
            }

            EnsureWriteCapacity(1);
            WriteInitialByte(new CborInitialByte(CborMajorType.ByteString, CborAdditionalInfo.IndefiniteLength));
            PushDataItem(CborMajorType.ByteString, definiteLength: null);
        }

        public void WriteEndByteStringIndefiniteLength()
        {
            PopDataItem(CborMajorType.ByteString);
            AdvanceDataItemCounters();
        }

        public void WriteStartTextStringIndefiniteLength()
        {
            if (PatchIndefiniteLengthItems)
            {
                _currentIndefiniteLengthStringRanges ??= new List<(int, int)>();
            }
            else if (CborConformanceLevelHelpers.RequiresDefiniteLengthItems(ConformanceLevel))
            {
                throw new InvalidOperationException("Indefinite-length items are not permitted under the current conformance level.");
            }

            EnsureWriteCapacity(1);
            WriteInitialByte(new CborInitialByte(CborMajorType.TextString, CborAdditionalInfo.IndefiniteLength));
            PushDataItem(CborMajorType.TextString, definiteLength: null);
        }

        public void WriteEndTextStringIndefiniteLength()
        {
            PopDataItem(CborMajorType.TextString);
            AdvanceDataItemCounters();
        }

        private void PatchIndefiniteLengthString(CborMajorType type)
        {
            Debug.Assert(type == CborMajorType.ByteString || type == CborMajorType.TextString);
            Debug.Assert(_currentIndefiniteLengthStringRanges != null);

            int currentOffset = _offset;

            // calculate the definite length of the concatenated string
            int definiteLength = 0;
            foreach ((int _, int length) in _currentIndefiniteLengthStringRanges)
            {
                definiteLength += length;
            }

            // copy chunks to a temporary buffer
            byte[] tempBuffer = s_bufferPool.Rent(definiteLength);
            Span<byte> tempSpan = tempBuffer.AsSpan(0, definiteLength);

            Span<byte> s = tempSpan;
            foreach ((int offset, int length) in _currentIndefiniteLengthStringRanges)
            {
                _buffer.AsSpan(offset, length).CopyTo(s);
                s = s.Slice(length);
            }
            Debug.Assert(s.IsEmpty);

            // write back to the original buffer
            _offset = _frameOffset - 1;
            WriteUnsignedInteger(type, (ulong)definiteLength);
            tempSpan.CopyTo(_buffer.AsSpan(_offset, definiteLength));
            _offset += definiteLength;

            // zero out excess bytes & other cleanups
            _buffer.AsSpan(_offset, currentOffset - _offset).Fill(0);
            s_bufferPool.Return(tempBuffer, clearArray: true);
            _currentIndefiniteLengthStringRanges.Clear();
        }
    }
}
