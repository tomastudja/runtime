﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace GenerateRegexCasingTable
{
    /// <summary>
    /// Class which holds both the equivalence map and values for all of the UnicodeData.txt Lowercase
    /// mappings
    /// </summary>
    public class DataTable
    {
        private Dictionary<char, int> _map;
        private Dictionary<int, SortedSet<char>> _values;
        private Dictionary<int, int> _mapAndValueMapping;

        public DataTable(Dictionary<char, int> equivalenceMap, Dictionary<int, SortedSet<char>> equivalenceValues)
        {
            _map = equivalenceMap;
            _values = equivalenceValues;
            _mapAndValueMapping = new();
        }

        /// <summary>
        /// Generates the Regex case equivalence table with <paramref name="numberOfPartitions"/> partitions
        /// and saves it at <paramref name="outputFilePath"/>.
        /// </summary>
        /// <param name="numberOfPartitions">Number of partitions for the First-level lookup table.</param>
        /// <param name="outputFilePath">Filename for the output file that will be generated.</param>
        public void GenerateDataTableWithPartitions(int numberOfPartitions, string outputFilePath)
        {
            byte[] unicodesThatParticipateInCasing = CalculateCharactersThatParticipateInCasing();
            int charactersPerRange = 0x1_0000 / numberOfPartitions;
            byte[] rangesWithData = CalculateRangesWithData();

            using (StreamWriter writer = File.CreateText(outputFilePath))
            {
                EmitFileHeadersAndUsings(writer);

                writer.Write("namespace System.Text.RegularExpressions\n");
                writer.Write("{\n");

                writer.Write("    internal static partial class RegexCaseEquivalences\n    {\n");

                writer.Write("        // THE FOLLOWING DATA IS AUTO GENERATED BY GenerateRegexCasingTable program UNDER THE TOOLS FOLDER\n");
                writer.Write("        // PLEASE DON'T MODIFY BY HAND\n");
                writer.Write("        // IF YOU NEED TO UPDATE UNICODE VERSION FOLLOW THE GUIDE AT src/libraries/System.Private.CoreLib/Tools/GenUnicodeProp/Updating-Unicode-Versions.md\n\n");

                EmitValuesArray(writer);

                writer.Write("\n");

                EmitFirstLevelLookupTable(writer);

                writer.Write("\n");

                EmitMapArray(writer);

                writer.Write("    }\n}\n");
            }

            return;

            byte[] CalculateRangesWithData()
            {
                byte[] result = new byte[numberOfPartitions];
                for (int i = 0; i < numberOfPartitions; i++)
                {
                    byte rangeHasData = 0;
                    for (int x = 0; x < charactersPerRange; x++)
                    {
                        if (unicodesThatParticipateInCasing[(i * charactersPerRange) + x] == 1)
                        {
                            rangeHasData = 1;
                            break;
                        }
                    }
                    result[i] = rangeHasData;
                }

                return result;
            }

            byte[] CalculateCharactersThatParticipateInCasing()
            {
                byte[] result = new byte[0x1_0000];
                for (int i = 0; i < 0x1_0000; i++)
                {
                    if (_map.ContainsKey((char)i))
                    {
                        result[i] = 1;
                    }
                    else
                    {
                        result[i] = 0;
                    }
                }

                return result;
            }

            void EmitFileHeadersAndUsings(StreamWriter writer)
            {
                writer.Write("// Licensed to the .NET Foundation under one or more agreements.\n");
                writer.Write("// The .NET Foundation licenses this file to you under the MIT license.\n\n");
            }

            void EmitFirstLevelLookupTable(StreamWriter writer)
            {
                List<ushort> firstLevelLookupTable = FlattenFirstLevelLookupTable();

                writer.Write($"        private static ReadOnlySpan<ushort> EquivalenceFirstLevelLookup => new ushort[{firstLevelLookupTable.Count}]\n        {{\n");

                writer.Write("            0x{0:x4}", firstLevelLookupTable[0]);
                for (var i = 1; i < firstLevelLookupTable.Count; i++)
                {
                    writer.Write(i % 16 == 0 ? ",\n            " : ", ");
                    writer.Write("0x{0:x4}", firstLevelLookupTable[i]);
                }
                writer.Write("\n        };\n");
            }

            List<ushort> FlattenFirstLevelLookupTable()
            {
                List<ushort> result = new();
                int indexesWithRanges = 0;

                for (int i = 0; i < numberOfPartitions; i++)
                {
                    if (rangesWithData[i] == 0)
                    {
                        result.Add(0xFFFF);
                    }
                    else
                    {
                        Debug.Assert((indexesWithRanges * charactersPerRange) < 0xFFFF);
                        result.Add((ushort)(indexesWithRanges * charactersPerRange));
                        indexesWithRanges++;
                    }
                }

                return result;
            }

            void EmitMapArray(StreamWriter writer)
            {
                List<ushort> flattenedMap = FlattenMapDictionary();

                writer.Write($"        private static ReadOnlySpan<ushort> EquivalenceCasingMap => new ushort[{flattenedMap.Count}]\n        {{\n");

                writer.Write("            0x{0:x4}", flattenedMap[0]);
                for (var i = 1; i < flattenedMap.Count; i++)
                {
                    writer.Write(i % 16 == 0 ? $",\n            " : ", ");
                    writer.Write("0x{0:x4}", flattenedMap[i]);
                }
                writer.Write("\n        };\n");
            }

            List<ushort> FlattenMapDictionary()
            {
                List<ushort> result = new();

                for (int i = 0; i <= 0xFFFF; i++)
                {
                    // If this character belongs to a range that has no data, then don't add it to the list.
                    int index = (i / charactersPerRange) < numberOfPartitions ? i / charactersPerRange : numberOfPartitions - 1;
                    if (rangesWithData[index] == 0)
                        continue;

                    if (_map.TryGetValue((char)i, out int valueMap))
                    {
                        int mapping = _mapAndValueMapping[valueMap];
                        Debug.Assert(mapping < 0x1FFF); // Ensure the index will not need the 3 most significant bits.
                        Debug.Assert(_values[valueMap].Count < 0x07); // Ensure the count will not need more than 3 bits.
                        int mappingValue = mapping | (_values[valueMap].Count << 13); // store the count as the 3 highest significant bits.
                        Debug.Assert(mappingValue < 0xFFFF);
                        result.Add((ushort)mappingValue);
                    }
                    else
                    {
                        result.Add(0xFFFF);
                    }
                }

                return result;
            }

            void EmitValuesArray(StreamWriter writer)
            {
                List<ushort> flattenedValues = FlattenValuesDictionary();

                writer.Write("        private static ReadOnlySpan<char> EquivalenceCasingValues => new char[" + flattenedValues.Count + "]\n        {\n");

                writer.Write("            \'\\u{0:X4}\'", flattenedValues[0]);
                for (var i = 1; i < flattenedValues.Count; i++)
                {
                    writer.Write(i % 16 == 0 ? ",\n            " : ", ");
                    writer.Write("\'\\u{0:X4}\'", flattenedValues[i]);
                }
                writer.Write("\n        };\n");
            }

            List<ushort> FlattenValuesDictionary()
            {
                List<ushort> values = new List<ushort>();
                int valuesSize = 0;

                for (int i = 0; i < _values.Count; i++)
                {
                    // Add a mapping that we will later use to match a character to the position of the values list
                    _mapAndValueMapping.Add(i, valuesSize);
                    foreach(char value in _values[i])
                    {
                        Debug.Assert(value < 0xFFFF);
                        values.Add(value);
                        valuesSize++;
                    }
                }

                return values;
            }
        }
    }
}
