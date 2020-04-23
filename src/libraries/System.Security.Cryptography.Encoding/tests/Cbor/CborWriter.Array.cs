﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#nullable enable
using System.Buffers.Binary;
using System.Text;

namespace System.Security.Cryptography.Encoding.Tests.Cbor
{
    internal partial class CborWriter
    {
        public void WriteStartArray(int definiteLength)
        {
            if (definiteLength < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(definiteLength), "must be non-negative integer.");
            }

            WriteUnsignedInteger(CborMajorType.Array, (ulong)definiteLength);
            PushDataItem(CborMajorType.Array, (uint)definiteLength);
        }

        public void WriteEndArray()
        {
            bool isDefiniteLengthArray = _remainingDataItems.HasValue;
            PopDataItem(CborMajorType.Array);

            if (!isDefiniteLengthArray)
            {
                // append break byte for indefinite-length arrays
                EnsureWriteCapacity(1);
                _buffer[_offset++] = CborInitialByte.IndefiniteLengthBreakByte;
            }

            AdvanceDataItemCounters();
        }

        public void WriteStartArrayIndefiniteLength()
        {
            if (CborConformanceLevelHelpers.RequiresDefiniteLengthItems(ConformanceLevel))
            {
                throw new InvalidOperationException("Indefinite-length items are not permitted under the current conformance level.");
            }

            EnsureWriteCapacity(1);
            WriteInitialByte(new CborInitialByte(CborMajorType.Array, CborAdditionalInfo.IndefiniteLength));
            PushDataItem(CborMajorType.Array, expectedNestedItems: null);
        }
    }
}
