﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Buffers;
using Xunit;

namespace System.Text.Json.Serialization.Tests
{
    public static partial class CustomConverterTests
    {
        [Fact]
        public static void ValueTypeConverter_NoOverride()
        {
            // Baseline
            Assert.Throws<JsonException>(() => JsonSerializer.Deserialize<int>("null"));

            // Per null handling default value for value types (true), converter handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new Int32NullConverter_SpecialCaseNull());

            Assert.Equal(-1, JsonSerializer.Deserialize<int>("null", options));
        }

        private class Int32NullConverter_SpecialCaseNull : JsonConverter<int>
        {
            public override int Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType == JsonTokenType.Null)
                {
                    return -1;
                }

                throw new JsonException();
            }

            public override void Write(Utf8JsonWriter writer, int value, JsonSerializerOptions options)
            {
                throw new NotImplementedException();
            }
        }

        [Fact]
        public static void ValueTypeConverter_OptOut()
        {
            // Per null handling opt-out, serializer handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new Int32NullConverter_OptOut());

            Assert.Equal(0, JsonSerializer.Deserialize<int>("null", options));
        }

        private class Int32NullConverter_OptOut : Int32NullConverter_SpecialCaseNull
        {
            public override bool HandleNull => false;
        }

        [Fact]
        public static void ValueTypeConverter_NullOptIn()
        {
            // Per null handling opt-in, converter handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new Int32NullConverter_NullOptIn());

            Assert.Equal(-1, JsonSerializer.Deserialize<int>("null", options));
        }

        private class Int32NullConverter_NullOptIn : Int32NullConverter_SpecialCaseNull
        {
            public override bool HandleNull => true;
        }

        [Fact]
        public static void ComplexValueTypeConverter_NoOverride()
        {
            // Baseline
            Assert.Throws<JsonException>(() => JsonSerializer.Deserialize<Point_2D_Struct>("null"));

            var options = new JsonSerializerOptions();
            options.Converters.Add(new PointStructConverter_SpecialCaseNull());

            // Per null handling default value for value types (true), converter handles null.
            var obj = JsonSerializer.Deserialize<Point_2D_Struct>("null", options);
            Assert.Equal(-1, obj.X);
            Assert.Equal(-1, obj.Y);
        }

        private class PointStructConverter_SpecialCaseNull : JsonConverter<Point_2D_Struct>
        {
            public override Point_2D_Struct Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType == JsonTokenType.Null)
                {
                    return new Point_2D_Struct(-1, -1);
                }

                throw new JsonException();
            }

            public override void Write(Utf8JsonWriter writer, Point_2D_Struct value, JsonSerializerOptions options)
            {
                throw new NotImplementedException();
            }
        }

        [Fact]
        public static void ComplexValueTypeConverter_OptOut()
        {
            // Per null handling opt-out, serializer handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new PointStructConverter_OptOut());

            var obj = JsonSerializer.Deserialize<Point_2D_Struct>("null", options);
            Assert.Equal(0, obj.X);
            Assert.Equal(0, obj.Y);
        }

        private class PointStructConverter_OptOut : PointStructConverter_SpecialCaseNull
        {
            public override bool HandleNull => false;
        }

        [Fact]
        public static void ComplexValueTypeConverter_NullOptIn()
        {
            // Baseline
            Assert.Throws<JsonException>(() => JsonSerializer.Deserialize<Point_2D_Struct>("null"));

            // Per null handling opt-in, converter handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new PointStructConverter_NullOptIn());

            var obj = JsonSerializer.Deserialize<Point_2D_Struct>("null", options);
            Assert.Equal(-1, obj.X);
            Assert.Equal(-1, obj.Y);
        }

        private class PointStructConverter_NullOptIn : PointStructConverter_SpecialCaseNull
        {
            public override bool HandleNull => true;
        }

        [Fact]
        public static void NullableValueTypeConverter_NoOverride()
        {
            // Baseline
            int? val = JsonSerializer.Deserialize<int?>("null");
            Assert.Null(val);
            Assert.Equal("null", JsonSerializer.Serialize(val));

            // Per null handling default value for value types (true), converter handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new NullableInt32NullConverter_SpecialCaseNull());

            val = JsonSerializer.Deserialize<int?>("null", options);
            Assert.Equal(-1, val);

            val = null;
            Assert.Equal("-1", JsonSerializer.Serialize(val, options));
        }

        private class NullableInt32NullConverter_SpecialCaseNull : JsonConverter<int?>
        {
            public override int? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType == JsonTokenType.Null)
                {
                    return -1;
                }

                throw new JsonException();
            }

            public override void Write(Utf8JsonWriter writer, int? value, JsonSerializerOptions options)
            {
                if (!value.HasValue)
                {
                    writer.WriteNumberValue(-1);
                    return;
                }

                throw new NotSupportedException();
            }
        }

        [Fact]
        public static void NullableValueTypeConverter_OptOut()
        {
            // Baseline
            int? val = JsonSerializer.Deserialize<int?>("null");
            Assert.Null(val);
            Assert.Equal("null", JsonSerializer.Serialize(val));

            // Per null handling opt-out, serializer handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new NullableInt32NullConverter_NullOptOut());

            val = JsonSerializer.Deserialize<int?>("null", options);
            Assert.Null(val);
            Assert.Equal("null", JsonSerializer.Serialize(val, options));
        }

        private class NullableInt32NullConverter_NullOptOut : NullableInt32NullConverter_SpecialCaseNull
        {
            public override bool HandleNull => false;
        }

        [Fact]
        public static void ReferenceTypeConverter_NoOverride()
        {
            // Baseline
            Uri val = JsonSerializer.Deserialize<Uri>("null");
            Assert.Null(val);
            Assert.Equal("null", JsonSerializer.Serialize(val));

            // Per null handling default value for reference types (false), serializer handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new Int32NullConverter_SpecialCaseNull());

            val = JsonSerializer.Deserialize<Uri>("null", options);
            Assert.Null(val);
            Assert.Equal("null", JsonSerializer.Serialize(val, options));
        }

        private class UriNullConverter_SpecialCaseNull : JsonConverter<Uri>
        {
            public override Uri Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType == JsonTokenType.Null)
                {
                    return new Uri("https://default");
                }

                throw new JsonException();
            }

            public override void Write(Utf8JsonWriter writer, Uri value, JsonSerializerOptions options)
            {
                if (value == null)
                {
                    writer.WriteStringValue("https://default");
                    return;
                }

                throw new NotSupportedException();
            }
        }

        [Fact]
        public static void ReferenceTypeConverter_OptOut()
        {
            // Per null handling opt-out, serializer handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new UriNullConverter_OptOut());

            Uri val = JsonSerializer.Deserialize<Uri>("null", options);
            Assert.Null(val);
            Assert.Equal("null", JsonSerializer.Serialize(val, options));
        }

        private class UriNullConverter_OptOut : UriNullConverter_SpecialCaseNull
        {
            public override bool HandleNull => false;
        }

        [Fact]
        public static void ReferenceTypeConverter_NullOptIn()
        {
            // Per null handling opt-in, converter handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new UriNullConverter_NullOptIn());

            Uri val = JsonSerializer.Deserialize<Uri>("null", options);
            Assert.Equal(new Uri("https://default"), val);

            val = null;
            Assert.Equal(@"""https://default""", JsonSerializer.Serialize(val, options));
        }

        private class UriNullConverter_NullOptIn : UriNullConverter_SpecialCaseNull
        {
            public override bool HandleNull => true;
        }

        [Fact]
        public static void ComplexReferenceTypeConverter_NoOverride()
        {
            // Baseline
            Point_2D obj = JsonSerializer.Deserialize<Point_2D>("null");
            Assert.Null(obj);
            Assert.Equal("null", JsonSerializer.Serialize(obj));

            // Per null handling default value for reference types (false), serializer handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new PointClassConverter_SpecialCaseNull());

            obj = JsonSerializer.Deserialize<Point_2D>("null", options);
            Assert.Null(obj);
            Assert.Equal("null", JsonSerializer.Serialize(obj));
        }

        private class PointClassConverter_SpecialCaseNull : JsonConverter<Point_2D>
        {
            public override Point_2D Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType == JsonTokenType.Null)
                {
                    return new Point_2D(-1, -1);
                }

                throw new JsonException();
            }

            public override void Write(Utf8JsonWriter writer, Point_2D value, JsonSerializerOptions options)
            {
                if (value == null)
                {
                    writer.WriteStartObject();
                    writer.WriteNumber("X", -1);
                    writer.WriteNumber("Y", -1);
                    writer.WriteEndObject();
                    return;
                }

                throw new JsonException();
            }
        }

        [Fact]
        public static void ComplexReferenceTypeConverter_NullOptIn()
        {
            // Per null handling opt-in, converter handles null.
            var options = new JsonSerializerOptions();
            options.Converters.Add(new PointClassConverter_NullOptIn());

            Point_2D obj = JsonSerializer.Deserialize<Point_2D>("null", options);
            Assert.Equal(-1, obj.X);
            Assert.Equal(-1, obj.Y);

            obj = null;
            JsonTestHelper.AssertJsonEqual(@"{""X"":-1,""Y"":-1}", JsonSerializer.Serialize(obj, options));
        }

        private class PointClassConverter_NullOptIn : PointClassConverter_SpecialCaseNull
        {
            public override bool HandleNull => true;
        }

        [Fact]
        public static void ConverterNotCalled_IgnoreNullValues()
        {
            var options = new JsonSerializerOptions();
            options.Converters.Add(new UriNullConverter_NullOptIn());

            // Converter is not called.
            ClassWithIgnoredUri obj = JsonSerializer.Deserialize<ClassWithIgnoredUri>(@"{""MyUri"":null}", options);
            Assert.Equal(new Uri("https://microsoft.com"), obj.MyUri);

            obj.MyUri = null;
            Assert.Equal("{}", JsonSerializer.Serialize(obj, options));
        }

        private class ClassWithIgnoredUri
        {
            [JsonIgnore(Condition = JsonIgnoreCondition.WhenNull)]
            public Uri MyUri { get; set; } = new Uri("https://microsoft.com");
        }

        [Fact]
        public static void ConverterWritesBadAmount()
        {
            var options = new JsonSerializerOptions();
            options.Converters.Add(new BadUriConverter());
            options.Converters.Add(new BadObjectConverter());

            var writerOptions = new JsonWriterOptions { SkipValidation = false };
            using Utf8JsonWriter writer = new Utf8JsonWriter(new ArrayBufferWriter<byte>(), writerOptions);

            Assert.Throws<JsonException>(() => JsonSerializer.Serialize(writer, new ClassWithUri(), options));
            Assert.Throws<JsonException>(() => JsonSerializer.Serialize(writer, new StructWithObject(), options));
        }

        private class BadUriConverter : UriNullConverter_NullOptIn
        {
            public override void Write(Utf8JsonWriter writer, Uri value, JsonSerializerOptions options) { }
        }

        private class BadObjectConverter : JsonConverter<object>
        {
            public override object Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                throw new NotImplementedException();
            }

            public override void Write(Utf8JsonWriter writer, object value, JsonSerializerOptions options)
            {
                writer.WriteNullValue();
                writer.WritePropertyName("hello");
            }

            public override bool HandleNull => true;
        }

        private class ClassWithUri
        {
            public Uri MyUri { get; set; }
        }


        private class StructWithObject
        {
            public object MyObj { get; set; }
        }
    }
}
