﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using SharedTypes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;

using Xunit;

namespace LibraryImportGenerator.IntegrationTests
{
    partial class NativeExportsNE
    {
        public partial class Collections
        {
            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "sum_int_array")]
            public static partial int Sum([MarshalUsing(typeof(ListMarshaller<,>))] List<int> values, int numValues);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "double_values")]
            public static partial int DoubleValues([MarshalUsing(typeof(ListMarshallerWithPinning<,>))] List<BlittableIntWrapper> values, int length);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "sum_int_array_ref")]
            public static partial int SumInArray([MarshalUsing(typeof(ListMarshaller<,>))] in List<int> values, int numValues);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "duplicate_int_array")]
            public static partial void Duplicate([MarshalUsing(typeof(ListMarshaller<,>), CountElementName = "numValues")] ref List<int> values, int numValues);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "create_range_array")]
            [return: MarshalUsing(typeof(ListMarshaller<,>), CountElementName = "numValues")]
            public static partial List<int> CreateRange(int start, int end, out int numValues);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "create_range_array_out")]
            public static partial void CreateRange_Out(int start, int end, out int numValues, [MarshalUsing(typeof(ListMarshaller<,>), CountElementName = "numValues")] out List<int> res);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "get_long_bytes")]
            [return: MarshalUsing(typeof(ListMarshaller<,>), ConstantElementCount = sizeof(long))]
            public static partial List<byte> GetLongBytes(long l);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "and_bool_struct_array")]
            [return: MarshalAs(UnmanagedType.U1)]
            public static partial bool AndAllMembers([MarshalUsing(typeof(ListMarshaller<,>))] List<BoolStruct> pArray, int length);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "and_bool_struct_array_in")]
            [return: MarshalAs(UnmanagedType.U1)]
            public static partial bool AndAllMembersIn([MarshalUsing(typeof(ListMarshaller<,>))] in List<BoolStruct> pArray, int length);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "negate_bool_struct_array_ref")]
            public static partial void NegateBools(
                [MarshalUsing(typeof(ListMarshaller<,>), CountElementName = "numValues")] ref List<BoolStruct> boolStruct,
                int numValues);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "negate_bool_struct_array_out")]
            public static partial void NegateBools(
                [MarshalUsing(typeof(ListMarshaller<,>))] List<BoolStruct> boolStruct,
                int numValues,
                [MarshalUsing(typeof(ListMarshaller<,>), CountElementName = "numValues")] out List<BoolStruct> pBoolStructOut);

            [LibraryImport(NativeExportsNE_Binary, EntryPoint = "negate_bool_struct_array_return")]
            [return: MarshalUsing(typeof(ListMarshaller<,>), CountElementName = "numValues")]
            public static partial List<BoolStruct> NegateBools(
                [MarshalUsing(typeof(ListMarshaller<,>))] List<BoolStruct> boolStruct,
                int numValues);
        }
    }

    public class CollectionTests
    {
        [Fact]
        public void BlittableElementColllection_ByValue()
        {
            var list = new List<int> { 1, 5, 79, 165, 32, 3 };
            Assert.Equal(list.Sum(), NativeExportsNE.Collections.Sum(list, list.Count));
        }

        [Fact]
        public void BlittableElementColllection_WithPinning()
        {
            var data = new List<int> { 1, 5, 79, 165, 32, 3 };
            var list = data.Select(i => new BlittableIntWrapper { i = i }).ToList();
            NativeExportsNE.Collections.DoubleValues(list, list.Count);
            Assert.Equal(data.Select(i => i * 2), list.Select(wrapper => wrapper.i));
        }

        [Fact]
        public void BlittableElementColllection_ByValue_Null()
        {
            Assert.Equal(-1, NativeExportsNE.Collections.Sum(null, 0));
        }

        [Fact]
        public void BlittableElementColllection_In()
        {
            var list = new List<int> { 1, 5, 79, 165, 32, 3 };
            Assert.Equal(list.Sum(), NativeExportsNE.Collections.SumInArray(list, list.Count));
        }

        [Fact]
        public void BlittableElementCollection_Ref()
        {
            var list = new List<int> { 1, 5, 79, 165, 32, 3 };
            var newList = list;
            NativeExportsNE.Collections.Duplicate(ref newList, list.Count);
            Assert.Equal((IEnumerable<int>)list, newList);
        }

        [Fact]
        public void BlittableElementCollection_OutReturn()
        {
            int start = 5;
            int end = 20;

            IEnumerable<int> expected = Enumerable.Range(start, end - start);
            Assert.Equal(expected, NativeExportsNE.Collections.CreateRange(start, end, out _));

            List<int> res;
            NativeExportsNE.Collections.CreateRange_Out(start, end, out _, out res);
            Assert.Equal(expected, res);
        }

        [Fact]
        public void BlittableElementCollection_OutReturn_Null()
        {
            Assert.Null(NativeExportsNE.Collections.CreateRange(1, 0, out _));

            List<int> res;
            NativeExportsNE.Collections.CreateRange_Out(1, 0, out _, out res);
            Assert.Null(res);
        }

        private static List<string> GetStringList()
        {
            return new()
            {
                "ABCdef 123$%^",
                "🍜 !! 🍜 !!",
                "🌲 木 🔥 火 🌾 土 🛡 金 🌊 水" ,
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed vitae posuere mauris, sed ultrices leo. Suspendisse potenti. Mauris enim enim, blandit tincidunt consequat in, varius sit amet neque. Morbi eget porttitor ex. Duis mattis aliquet ante quis imperdiet. Duis sit.",
                string.Empty,
                null
            };
        }

        [Fact]
        public void ConstantSizeCollection()
        {
            var longVal = 0x12345678ABCDEF10L;

            Assert.Equal(longVal, MemoryMarshal.Read<long>(CollectionsMarshal.AsSpan(NativeExportsNE.Collections.GetLongBytes(longVal))));
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        public void NonBlittableElementCollection_ByValue(bool result)
        {
            List<BoolStruct> list = GetBoolStructsToAnd(result);
            Assert.Equal(result, NativeExportsNE.Collections.AndAllMembers(list, list.Count));
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        public void NonBlittableElementCollection_In(bool result)
        {
            List<BoolStruct> list = GetBoolStructsToAnd(result);
            Assert.Equal(result, NativeExportsNE.Collections.AndAllMembersIn(list, list.Count));
        }

        [Fact]
        public void NonBlittableElementCollection_Ref()
        {
            List<BoolStruct> list = GetBoolStructsToNegate();
            List<BoolStruct> expected = GetNegatedBoolStructs(list);

            NativeExportsNE.Collections.NegateBools(ref list, list.Count);
            Assert.Equal(expected, list);
        }

        [Fact]
        public void NonBlittableElementCollection_Out()
        {
            List<BoolStruct> list = GetBoolStructsToNegate();
            List<BoolStruct> expected = GetNegatedBoolStructs(list);

            List<BoolStruct> result;
            NativeExportsNE.Collections.NegateBools(list, list.Count, out result);
            Assert.Equal(expected, result);
        }

        [Fact]
        public void NonBlittableElementCollection_Return()
        {
            List<BoolStruct> list = GetBoolStructsToNegate();
            List<BoolStruct> expected = GetNegatedBoolStructs(list);

            List<BoolStruct> result = NativeExportsNE.Collections.NegateBools(list, list.Count);
            Assert.Equal(expected, result);
        }

        private static List<BoolStruct> GetBoolStructsToAnd(bool result) => new List<BoolStruct>
            {
                new BoolStruct
                {
                    b1 = true,
                    b2 = true,
                    b3 = true,
                },
                new BoolStruct
                {
                    b1 = true,
                    b2 = true,
                    b3 = true,
                },
                new BoolStruct
                {
                    b1 = true,
                    b2 = true,
                    b3 = result,
                },
            };

        private static List<BoolStruct> GetBoolStructsToNegate() => new List<BoolStruct>()
            {
                new BoolStruct
                {
                    b1 = true,
                    b2 = false,
                    b3 = true
                },
                new BoolStruct
                {
                    b1 = false,
                    b2 = true,
                    b3 = false
                },
                new BoolStruct
                {
                    b1 = true,
                    b2 = true,
                    b3 = true
                },
                new BoolStruct
                {
                    b1 = false,
                    b2 = false,
                    b3 = false
                }
            };

        private static List<BoolStruct> GetNegatedBoolStructs(List<BoolStruct> toNegate)
            => toNegate.Select(b => new BoolStruct() { b1 = !b.b1, b2 = !b.b2, b3 = !b.b3 }).ToList();
    }
}
