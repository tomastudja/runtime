// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Net.Test.Common
{
    public static class HuffmanEncoder
    {
        // Stolen from product code
        // See https://github.com/dotnet/corefx/blob/ae7b3970bb2c8d76004ea397083ce7ceb1238133/src/System.Net.Http/src/System/Net/Http/SocketsHttpHandler/HPack/Huffman.cs#L12
        private static readonly (uint code, int bitLength)[] s_encodingTable = new (uint code, int bitLength)[]
        {
            (0b11111111_11000000_00000000_00000000, 13),
            (0b11111111_11111111_10110000_00000000, 23),
            (0b11111111_11111111_11111110_00100000, 28),
            (0b11111111_11111111_11111110_00110000, 28),
            (0b11111111_11111111_11111110_01000000, 28),
            (0b11111111_11111111_11111110_01010000, 28),
            (0b11111111_11111111_11111110_01100000, 28),
            (0b11111111_11111111_11111110_01110000, 28),
            (0b11111111_11111111_11111110_10000000, 28),
            (0b11111111_11111111_11101010_00000000, 24),
            (0b11111111_11111111_11111111_11110000, 30),
            (0b11111111_11111111_11111110_10010000, 28),
            (0b11111111_11111111_11111110_10100000, 28),
            (0b11111111_11111111_11111111_11110100, 30),
            (0b11111111_11111111_11111110_10110000, 28),
            (0b11111111_11111111_11111110_11000000, 28),
            (0b11111111_11111111_11111110_11010000, 28),
            (0b11111111_11111111_11111110_11100000, 28),
            (0b11111111_11111111_11111110_11110000, 28),
            (0b11111111_11111111_11111111_00000000, 28),
            (0b11111111_11111111_11111111_00010000, 28),
            (0b11111111_11111111_11111111_00100000, 28),
            (0b11111111_11111111_11111111_11111000, 30),
            (0b11111111_11111111_11111111_00110000, 28),
            (0b11111111_11111111_11111111_01000000, 28),
            (0b11111111_11111111_11111111_01010000, 28),
            (0b11111111_11111111_11111111_01100000, 28),
            (0b11111111_11111111_11111111_01110000, 28),
            (0b11111111_11111111_11111111_10000000, 28),
            (0b11111111_11111111_11111111_10010000, 28),
            (0b11111111_11111111_11111111_10100000, 28),
            (0b11111111_11111111_11111111_10110000, 28),
            (0b01010000_00000000_00000000_00000000, 6),
            (0b11111110_00000000_00000000_00000000, 10),
            (0b11111110_01000000_00000000_00000000, 10),
            (0b11111111_10100000_00000000_00000000, 12),
            (0b11111111_11001000_00000000_00000000, 13),
            (0b01010100_00000000_00000000_00000000, 6),
            (0b11111000_00000000_00000000_00000000, 8),
            (0b11111111_01000000_00000000_00000000, 11),
            (0b11111110_10000000_00000000_00000000, 10),
            (0b11111110_11000000_00000000_00000000, 10),
            (0b11111001_00000000_00000000_00000000, 8),
            (0b11111111_01100000_00000000_00000000, 11),
            (0b11111010_00000000_00000000_00000000, 8),
            (0b01011000_00000000_00000000_00000000, 6),
            (0b01011100_00000000_00000000_00000000, 6),
            (0b01100000_00000000_00000000_00000000, 6),
            (0b00000000_00000000_00000000_00000000, 5),
            (0b00001000_00000000_00000000_00000000, 5),
            (0b00010000_00000000_00000000_00000000, 5),
            (0b01100100_00000000_00000000_00000000, 6),
            (0b01101000_00000000_00000000_00000000, 6),
            (0b01101100_00000000_00000000_00000000, 6),
            (0b01110000_00000000_00000000_00000000, 6),
            (0b01110100_00000000_00000000_00000000, 6),
            (0b01111000_00000000_00000000_00000000, 6),
            (0b01111100_00000000_00000000_00000000, 6),
            (0b10111000_00000000_00000000_00000000, 7),
            (0b11111011_00000000_00000000_00000000, 8),
            (0b11111111_11111000_00000000_00000000, 15),
            (0b10000000_00000000_00000000_00000000, 6),
            (0b11111111_10110000_00000000_00000000, 12),
            (0b11111111_00000000_00000000_00000000, 10),
            (0b11111111_11010000_00000000_00000000, 13),
            (0b10000100_00000000_00000000_00000000, 6),
            (0b10111010_00000000_00000000_00000000, 7),
            (0b10111100_00000000_00000000_00000000, 7),
            (0b10111110_00000000_00000000_00000000, 7),
            (0b11000000_00000000_00000000_00000000, 7),
            (0b11000010_00000000_00000000_00000000, 7),
            (0b11000100_00000000_00000000_00000000, 7),
            (0b11000110_00000000_00000000_00000000, 7),
            (0b11001000_00000000_00000000_00000000, 7),
            (0b11001010_00000000_00000000_00000000, 7),
            (0b11001100_00000000_00000000_00000000, 7),
            (0b11001110_00000000_00000000_00000000, 7),
            (0b11010000_00000000_00000000_00000000, 7),
            (0b11010010_00000000_00000000_00000000, 7),
            (0b11010100_00000000_00000000_00000000, 7),
            (0b11010110_00000000_00000000_00000000, 7),
            (0b11011000_00000000_00000000_00000000, 7),
            (0b11011010_00000000_00000000_00000000, 7),
            (0b11011100_00000000_00000000_00000000, 7),
            (0b11011110_00000000_00000000_00000000, 7),
            (0b11100000_00000000_00000000_00000000, 7),
            (0b11100010_00000000_00000000_00000000, 7),
            (0b11100100_00000000_00000000_00000000, 7),
            (0b11111100_00000000_00000000_00000000, 8),
            (0b11100110_00000000_00000000_00000000, 7),
            (0b11111101_00000000_00000000_00000000, 8),
            (0b11111111_11011000_00000000_00000000, 13),
            (0b11111111_11111110_00000000_00000000, 19),
            (0b11111111_11100000_00000000_00000000, 13),
            (0b11111111_11110000_00000000_00000000, 14),
            (0b10001000_00000000_00000000_00000000, 6),
            (0b11111111_11111010_00000000_00000000, 15),
            (0b00011000_00000000_00000000_00000000, 5),
            (0b10001100_00000000_00000000_00000000, 6),
            (0b00100000_00000000_00000000_00000000, 5),
            (0b10010000_00000000_00000000_00000000, 6),
            (0b00101000_00000000_00000000_00000000, 5),
            (0b10010100_00000000_00000000_00000000, 6),
            (0b10011000_00000000_00000000_00000000, 6),
            (0b10011100_00000000_00000000_00000000, 6),
            (0b00110000_00000000_00000000_00000000, 5),
            (0b11101000_00000000_00000000_00000000, 7),
            (0b11101010_00000000_00000000_00000000, 7),
            (0b10100000_00000000_00000000_00000000, 6),
            (0b10100100_00000000_00000000_00000000, 6),
            (0b10101000_00000000_00000000_00000000, 6),
            (0b00111000_00000000_00000000_00000000, 5),
            (0b10101100_00000000_00000000_00000000, 6),
            (0b11101100_00000000_00000000_00000000, 7),
            (0b10110000_00000000_00000000_00000000, 6),
            (0b01000000_00000000_00000000_00000000, 5),
            (0b01001000_00000000_00000000_00000000, 5),
            (0b10110100_00000000_00000000_00000000, 6),
            (0b11101110_00000000_00000000_00000000, 7),
            (0b11110000_00000000_00000000_00000000, 7),
            (0b11110010_00000000_00000000_00000000, 7),
            (0b11110100_00000000_00000000_00000000, 7),
            (0b11110110_00000000_00000000_00000000, 7),
            (0b11111111_11111100_00000000_00000000, 15),
            (0b11111111_10000000_00000000_00000000, 11),
            (0b11111111_11110100_00000000_00000000, 14),
            (0b11111111_11101000_00000000_00000000, 13),
            (0b11111111_11111111_11111111_11000000, 28),
            (0b11111111_11111110_01100000_00000000, 20),
            (0b11111111_11111111_01001000_00000000, 22),
            (0b11111111_11111110_01110000_00000000, 20),
            (0b11111111_11111110_10000000_00000000, 20),
            (0b11111111_11111111_01001100_00000000, 22),
            (0b11111111_11111111_01010000_00000000, 22),
            (0b11111111_11111111_01010100_00000000, 22),
            (0b11111111_11111111_10110010_00000000, 23),
            (0b11111111_11111111_01011000_00000000, 22),
            (0b11111111_11111111_10110100_00000000, 23),
            (0b11111111_11111111_10110110_00000000, 23),
            (0b11111111_11111111_10111000_00000000, 23),
            (0b11111111_11111111_10111010_00000000, 23),
            (0b11111111_11111111_10111100_00000000, 23),
            (0b11111111_11111111_11101011_00000000, 24),
            (0b11111111_11111111_10111110_00000000, 23),
            (0b11111111_11111111_11101100_00000000, 24),
            (0b11111111_11111111_11101101_00000000, 24),
            (0b11111111_11111111_01011100_00000000, 22),
            (0b11111111_11111111_11000000_00000000, 23),
            (0b11111111_11111111_11101110_00000000, 24),
            (0b11111111_11111111_11000010_00000000, 23),
            (0b11111111_11111111_11000100_00000000, 23),
            (0b11111111_11111111_11000110_00000000, 23),
            (0b11111111_11111111_11001000_00000000, 23),
            (0b11111111_11111110_11100000_00000000, 21),
            (0b11111111_11111111_01100000_00000000, 22),
            (0b11111111_11111111_11001010_00000000, 23),
            (0b11111111_11111111_01100100_00000000, 22),
            (0b11111111_11111111_11001100_00000000, 23),
            (0b11111111_11111111_11001110_00000000, 23),
            (0b11111111_11111111_11101111_00000000, 24),
            (0b11111111_11111111_01101000_00000000, 22),
            (0b11111111_11111110_11101000_00000000, 21),
            (0b11111111_11111110_10010000_00000000, 20),
            (0b11111111_11111111_01101100_00000000, 22),
            (0b11111111_11111111_01110000_00000000, 22),
            (0b11111111_11111111_11010000_00000000, 23),
            (0b11111111_11111111_11010010_00000000, 23),
            (0b11111111_11111110_11110000_00000000, 21),
            (0b11111111_11111111_11010100_00000000, 23),
            (0b11111111_11111111_01110100_00000000, 22),
            (0b11111111_11111111_01111000_00000000, 22),
            (0b11111111_11111111_11110000_00000000, 24),
            (0b11111111_11111110_11111000_00000000, 21),
            (0b11111111_11111111_01111100_00000000, 22),
            (0b11111111_11111111_11010110_00000000, 23),
            (0b11111111_11111111_11011000_00000000, 23),
            (0b11111111_11111111_00000000_00000000, 21),
            (0b11111111_11111111_00001000_00000000, 21),
            (0b11111111_11111111_10000000_00000000, 22),
            (0b11111111_11111111_00010000_00000000, 21),
            (0b11111111_11111111_11011010_00000000, 23),
            (0b11111111_11111111_10000100_00000000, 22),
            (0b11111111_11111111_11011100_00000000, 23),
            (0b11111111_11111111_11011110_00000000, 23),
            (0b11111111_11111110_10100000_00000000, 20),
            (0b11111111_11111111_10001000_00000000, 22),
            (0b11111111_11111111_10001100_00000000, 22),
            (0b11111111_11111111_10010000_00000000, 22),
            (0b11111111_11111111_11100000_00000000, 23),
            (0b11111111_11111111_10010100_00000000, 22),
            (0b11111111_11111111_10011000_00000000, 22),
            (0b11111111_11111111_11100010_00000000, 23),
            (0b11111111_11111111_11111000_00000000, 26),
            (0b11111111_11111111_11111000_01000000, 26),
            (0b11111111_11111110_10110000_00000000, 20),
            (0b11111111_11111110_00100000_00000000, 19),
            (0b11111111_11111111_10011100_00000000, 22),
            (0b11111111_11111111_11100100_00000000, 23),
            (0b11111111_11111111_10100000_00000000, 22),
            (0b11111111_11111111_11110110_00000000, 25),
            (0b11111111_11111111_11111000_10000000, 26),
            (0b11111111_11111111_11111000_11000000, 26),
            (0b11111111_11111111_11111001_00000000, 26),
            (0b11111111_11111111_11111011_11000000, 27),
            (0b11111111_11111111_11111011_11100000, 27),
            (0b11111111_11111111_11111001_01000000, 26),
            (0b11111111_11111111_11110001_00000000, 24),
            (0b11111111_11111111_11110110_10000000, 25),
            (0b11111111_11111110_01000000_00000000, 19),
            (0b11111111_11111111_00011000_00000000, 21),
            (0b11111111_11111111_11111001_10000000, 26),
            (0b11111111_11111111_11111100_00000000, 27),
            (0b11111111_11111111_11111100_00100000, 27),
            (0b11111111_11111111_11111001_11000000, 26),
            (0b11111111_11111111_11111100_01000000, 27),
            (0b11111111_11111111_11110010_00000000, 24),
            (0b11111111_11111111_00100000_00000000, 21),
            (0b11111111_11111111_00101000_00000000, 21),
            (0b11111111_11111111_11111010_00000000, 26),
            (0b11111111_11111111_11111010_01000000, 26),
            (0b11111111_11111111_11111111_11010000, 28),
            (0b11111111_11111111_11111100_01100000, 27),
            (0b11111111_11111111_11111100_10000000, 27),
            (0b11111111_11111111_11111100_10100000, 27),
            (0b11111111_11111110_11000000_00000000, 20),
            (0b11111111_11111111_11110011_00000000, 24),
            (0b11111111_11111110_11010000_00000000, 20),
            (0b11111111_11111111_00110000_00000000, 21),
            (0b11111111_11111111_10100100_00000000, 22),
            (0b11111111_11111111_00111000_00000000, 21),
            (0b11111111_11111111_01000000_00000000, 21),
            (0b11111111_11111111_11100110_00000000, 23),
            (0b11111111_11111111_10101000_00000000, 22),
            (0b11111111_11111111_10101100_00000000, 22),
            (0b11111111_11111111_11110111_00000000, 25),
            (0b11111111_11111111_11110111_10000000, 25),
            (0b11111111_11111111_11110100_00000000, 24),
            (0b11111111_11111111_11110101_00000000, 24),
            (0b11111111_11111111_11111010_10000000, 26),
            (0b11111111_11111111_11101000_00000000, 23),
            (0b11111111_11111111_11111010_11000000, 26),
            (0b11111111_11111111_11111100_11000000, 27),
            (0b11111111_11111111_11111011_00000000, 26),
            (0b11111111_11111111_11111011_01000000, 26),
            (0b11111111_11111111_11111100_11100000, 27),
            (0b11111111_11111111_11111101_00000000, 27),
            (0b11111111_11111111_11111101_00100000, 27),
            (0b11111111_11111111_11111101_01000000, 27),
            (0b11111111_11111111_11111101_01100000, 27),
            (0b11111111_11111111_11111111_11100000, 28),
            (0b11111111_11111111_11111101_10000000, 27),
            (0b11111111_11111111_11111101_10100000, 27),
            (0b11111111_11111111_11111101_11000000, 27),
            (0b11111111_11111111_11111101_11100000, 27),
            (0b11111111_11111111_11111110_00000000, 27),
            (0b11111111_11111111_11111011_10000000, 26),
            (0b11111111_11111111_11111111_11111100, 30)
        };

        public static int GetEncodedLength(ReadOnlySpan<byte> src)
        {
            int bits = 0;

            foreach (byte x in src)
            {
                bits += s_encodingTable[x].bitLength;
            }

            return (bits + 7) / 8;
        }

        public static int Encode(ReadOnlySpan<byte> src, Span<byte> dst)
        {
            ulong buffer = 0;
            int bufferLength = 0;
            int dstIdx = 0;

            foreach (byte x in src)
            {
                (uint code, int codeLength) = s_encodingTable[x];

                buffer = buffer << codeLength | code >> 32 - codeLength;
                bufferLength += codeLength;

                while (bufferLength >= 8)
                {
                    bufferLength -= 8;
                    dst[dstIdx++] = (byte)(buffer >> bufferLength);
                }
            }

            if (bufferLength != 0)
            {
                dst[dstIdx++] = (byte)(buffer << (8 - bufferLength) | 0xFFu >> bufferLength);
            }

            return dstIdx;
        }
    }
}
