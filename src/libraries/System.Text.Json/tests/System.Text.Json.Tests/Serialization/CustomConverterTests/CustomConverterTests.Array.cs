// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace System.Text.Json.Serialization.Tests
{
    public static partial class CustomConverterTests
    {
        // A custom long[] converter as comma-delimited string "1,2,3".
        internal class LongArrayConverter : JsonConverter<long[]>
        {
            public LongArrayConverter() { }

            public override long[] Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                string json = reader.GetString();

                var list = new List<long>();

                foreach (string str in json.Split(','))
                {
                    if (!long.TryParse(str, out long l))
                    {
                        throw new JsonException("Too big for a long");
                    }

                    list.Add(l);
                }

                return list.ToArray();
            }

            public override void Write(Utf8JsonWriter writer, long[] value, JsonSerializerOptions options)
            {
                var builder = new StringBuilder();

                for (int i = 0; i < value.Length; i++)
                {
                    builder.Append(value[i].ToString());

                    if (i != value.Length - 1)
                    {
                        builder.Append(",");
                    }
                }

                writer.WriteStringValue(builder.ToString());
            }
        }

        [Fact]
        public static void CustomArrayConverterAsRoot()
        {
            const string json = @"""1,2,3""";

            var options = new JsonSerializerOptions();
            options.Converters.Add(new LongArrayConverter());

            long[] arr = JsonSerializer.Deserialize<long[]>(json, options);
            Assert.Equal(1, arr[0]);
            Assert.Equal(2, arr[1]);
            Assert.Equal(3, arr[2]);

            string jsonSerialized = JsonSerializer.Serialize(arr, options);
            Assert.Equal(json, jsonSerialized);
        }

        [Fact]
        public static void CustomArrayConverterFail()
        {
            string json = $"\"{long.MaxValue}0\"";

            var options = new JsonSerializerOptions { Converters = { new LongArrayConverter() } };
            JsonException ex = Assert.Throws<JsonException>(() => JsonSerializer.Deserialize<long[]>(json, options));

            Assert.Null(ex.InnerException);
            Assert.Equal("$", ex.Path);
            Assert.Equal("Too big for a long", ex.Message);
        }

        private class ClassWithProperty
        {
            public long[] Array1 { get; set; }
            public long[] Array2 { get; set; }
        }

        [Fact]
        public static void CustomArrayConverterInProperty()
        {
            const string json = @"{""Array1"":""1,2,3"",""Array2"":""4,5""}";

            var options = new JsonSerializerOptions();
            options.Converters.Add(new LongArrayConverter());

            ClassWithProperty obj = JsonSerializer.Deserialize<ClassWithProperty>(json, options);
            Assert.Equal(1, obj.Array1[0]);
            Assert.Equal(2, obj.Array1[1]);
            Assert.Equal(3, obj.Array1[2]);
            Assert.Equal(4, obj.Array2[0]);
            Assert.Equal(5, obj.Array2[1]);

            string jsonSerialized = JsonSerializer.Serialize(obj, options);
            Assert.Equal(json, jsonSerialized);
        }

        [Theory]
        [InlineData(typeof(ProxyClassConverter))]
        [InlineData(typeof(ProxyClassWithConstructorConverter))]
        public static async Task ClassWithProxyConverter_WorksWithAsyncDeserialization(Type converterType)
        {
            // Regression test for https://github.com/dotnet/runtime/issues/74108

            const int Count = 1000;

            var stream = new MemoryStream();
            var data = Enumerable.Range(1, Count).Select(_ => new { OtherValue = "otherValue", Value = "value" });
            await JsonSerializer.SerializeAsync(stream, data);
            stream.Position = 0;

            var options = new JsonSerializerOptions
            {
                Converters = { (JsonConverter)Activator.CreateInstance(converterType) },
            };

            PocoWithValue[] result = await JsonSerializer.DeserializeAsync<PocoWithValue[]>(stream, options);
            Assert.Equal(Count, result.Length);
            Assert.All(result, entry => Assert.Equal("value", entry.Value));
        }

        class PocoWithValue
        {
            public string? Value { get; set; }
        }

        class ProxyClass
        {
            public string? Value { get; set; }
        }

        class ProxyClassConverter : JsonConverter<PocoWithValue>
        {
            private JsonConverter<ProxyClass> GetConverter(JsonSerializerOptions options)
                => (JsonConverter<ProxyClass>)options.GetConverter(typeof(ProxyClass));

            public override PocoWithValue? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                var proxy = GetConverter(options).Read(ref reader, typeof(ProxyClass), options);
                return new PocoWithValue { Value = proxy.Value };
            }

            public override void Write(Utf8JsonWriter writer, PocoWithValue value, JsonSerializerOptions options)
                => throw new NotImplementedException();
        }

        class ProxyClassWithConstructor
        {
            public ProxyClassWithConstructor(string? value) => Value = value;
            public string? Value { get; }
        }

        class ProxyClassWithConstructorConverter : JsonConverter<PocoWithValue>
        {
            private JsonConverter<ProxyClassWithConstructor> GetConverter(JsonSerializerOptions options)
                => (JsonConverter<ProxyClassWithConstructor>)options.GetConverter(typeof(ProxyClassWithConstructor));

            public override PocoWithValue? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                var proxy = GetConverter(options).Read(ref reader, typeof(ProxyClassWithConstructor), options);
                return new PocoWithValue { Value = proxy.Value };
            }

            public override void Write(Utf8JsonWriter writer, PocoWithValue value, JsonSerializerOptions options)
                => throw new NotImplementedException();
        }
    }
}
