// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

namespace System.Linq.Expressions.Tests
{
    public static class BinaryNullableOrTests
    {
        #region Test methods

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableByteOrTest(bool useInterpreter)
        {
            byte?[] array = new byte?[] { null, 0, 1, byte.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableByteOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableSByteOrTest(bool useInterpreter)
        {
            sbyte?[] array = new sbyte?[] { null, 0, 1, -1, sbyte.MinValue, sbyte.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableSByteOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableUShortOrTest(bool useInterpreter)
        {
            ushort?[] array = new ushort?[] { null, 0, 1, ushort.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableUShortOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableShortOrTest(bool useInterpreter)
        {
            short?[] array = new short?[] { null, 0, 1, -1, short.MinValue, short.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableShortOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableUIntOrTest(bool useInterpreter)
        {
            uint?[] array = new uint?[] { null, 0, 1, uint.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableUIntOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableIntOrTest(bool useInterpreter)
        {
            int?[] array = new int?[] { null, 0, 1, -1, int.MinValue, int.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableIntOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableULongOrTest(bool useInterpreter)
        {
            ulong?[] array = new ulong?[] { null, 0, 1, ulong.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableULongOr(array[i], array[j], useInterpreter);
                }
            }
        }

        [Theory, ClassData(typeof(CompilationTypes))]
        public static void CheckNullableLongOrTest(bool useInterpreter)
        {
            long?[] array = new long?[] { null, 0, 1, -1, long.MinValue, long.MaxValue };
            for (int i = 0; i < array.Length; i++)
            {
                for (int j = 0; j < array.Length; j++)
                {
                    VerifyNullableLongOr(array[i], array[j], useInterpreter);
                }
            }
        }

        #endregion

        #region Test verifiers

        private static void VerifyNullableByteOr(byte? a, byte? b, bool useInterpreter)
        {
            Expression<Func<byte?>> e =
                Expression.Lambda<Func<byte?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(byte?)),
                        Expression.Constant(b, typeof(byte?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<byte?> f = e.Compile(useInterpreter);

            Assert.Equal((byte?)(a | b), f());
        }

        private static void VerifyNullableSByteOr(sbyte? a, sbyte? b, bool useInterpreter)
        {
            Expression<Func<sbyte?>> e =
                Expression.Lambda<Func<sbyte?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(sbyte?)),
                        Expression.Constant(b, typeof(sbyte?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<sbyte?> f = e.Compile(useInterpreter);

            Assert.Equal((sbyte?)(a | b), f());
        }

        private static void VerifyNullableUShortOr(ushort? a, ushort? b, bool useInterpreter)
        {
            Expression<Func<ushort?>> e =
                Expression.Lambda<Func<ushort?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(ushort?)),
                        Expression.Constant(b, typeof(ushort?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<ushort?> f = e.Compile(useInterpreter);

            Assert.Equal((ushort?)(a | b), f());
        }

        private static void VerifyNullableShortOr(short? a, short? b, bool useInterpreter)
        {
            Expression<Func<short?>> e =
                Expression.Lambda<Func<short?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(short?)),
                        Expression.Constant(b, typeof(short?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<short?> f = e.Compile(useInterpreter);

            Assert.Equal((short?)(a | b), f());
        }

        private static void VerifyNullableUIntOr(uint? a, uint? b, bool useInterpreter)
        {
            Expression<Func<uint?>> e =
                Expression.Lambda<Func<uint?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(uint?)),
                        Expression.Constant(b, typeof(uint?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<uint?> f = e.Compile(useInterpreter);

            Assert.Equal(a | b, f());
        }

        private static void VerifyNullableIntOr(int? a, int? b, bool useInterpreter)
        {
            Expression<Func<int?>> e =
                Expression.Lambda<Func<int?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(int?)),
                        Expression.Constant(b, typeof(int?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<int?> f = e.Compile(useInterpreter);

            Assert.Equal(a | b, f());
        }

        private static void VerifyNullableULongOr(ulong? a, ulong? b, bool useInterpreter)
        {
            Expression<Func<ulong?>> e =
                Expression.Lambda<Func<ulong?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(ulong?)),
                        Expression.Constant(b, typeof(ulong?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<ulong?> f = e.Compile(useInterpreter);

            Assert.Equal(a | b, f());
        }

        private static void VerifyNullableLongOr(long? a, long? b, bool useInterpreter)
        {
            Expression<Func<long?>> e =
                Expression.Lambda<Func<long?>>(
                    Expression.Or(
                        Expression.Constant(a, typeof(long?)),
                        Expression.Constant(b, typeof(long?))),
                    Enumerable.Empty<ParameterExpression>());
            Func<long?> f = e.Compile(useInterpreter);

            Assert.Equal(a | b, f());
        }

        #endregion
    }
}
