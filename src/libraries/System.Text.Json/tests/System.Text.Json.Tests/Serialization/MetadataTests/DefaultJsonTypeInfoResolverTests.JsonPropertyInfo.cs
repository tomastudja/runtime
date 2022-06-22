﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Text.Json.Serialization.Metadata;
using System.Text.Json.Tests;
using System.Threading.Tasks;
using Xunit;

namespace System.Text.Json.Serialization.Tests
{
    public static partial class DefaultJsonTypeInfoResolverTests
    {
        [Fact]
        public static void JsonPropertyInfoOptionsAreSet()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = JsonTypeInfo.CreateJsonTypeInfo(typeof(MyClass), options);
            CreatePropertyAndCheckOptions(options, typeInfo);

            typeInfo = JsonTypeInfo.CreateJsonTypeInfo<MyClass>(options);
            CreatePropertyAndCheckOptions(options, typeInfo);

            typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(MyClass), options);
            CreatePropertyAndCheckOptions(options, typeInfo);

            static void CreatePropertyAndCheckOptions(JsonSerializerOptions expectedOptions, JsonTypeInfo typeInfo)
            {
                JsonPropertyInfo propertyInfo = typeInfo.CreateJsonPropertyInfo(typeof(string), "test");
                Assert.Same(expectedOptions, propertyInfo.Options);
            }
        }

        [Theory]
        [InlineData(typeof(string))]
        [InlineData(typeof(int))]
        [InlineData(typeof(MyClass))]
        public static void JsonPropertyInfoPropertyTypeIsSetWhenUsingCreateJsonPropertyInfo(Type propertyType)
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = JsonTypeInfo.CreateJsonTypeInfo(typeof(MyClass), options);
            JsonPropertyInfo propertyInfo = typeInfo.CreateJsonPropertyInfo(propertyType, "test");

            Assert.Equal(propertyType, propertyInfo.PropertyType);
        }

        [Fact]
        public static void JsonPropertyInfoPropertyTypeIsSet()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(MyClass), options);
            Assert.Equal(2, typeInfo.Properties.Count);
            JsonPropertyInfo propertyInfo = typeInfo.Properties[0];
            Assert.Equal(typeof(string), propertyInfo.PropertyType);
        }

        [Theory]
        [InlineData(typeof(string))]
        [InlineData(typeof(int))]
        [InlineData(typeof(MyClass))]
        public static void JsonPropertyInfoNameIsSetAndIsMutableWhenUsingCreateJsonPropertyInfo(Type propertyType)
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = JsonTypeInfo.CreateJsonTypeInfo(typeof(MyClass), options);
            JsonPropertyInfo propertyInfo = typeInfo.CreateJsonPropertyInfo(propertyType, "test");

            Assert.Equal("test", propertyInfo.Name);

            propertyInfo.Name = "foo";
            Assert.Equal("foo", propertyInfo.Name);

            Assert.Throws<ArgumentNullException>(() => propertyInfo.Name = null);
        }

        [Fact]
        public static void JsonPropertyInfoNameIsSetAndIsMutableForDefaultResolver()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(MyClass), options);
            Assert.Equal(2, typeInfo.Properties.Count);
            JsonPropertyInfo propertyInfo = typeInfo.Properties[0];

            Assert.Equal(nameof(MyClass.Value), propertyInfo.Name);

            propertyInfo.Name = "foo";
            Assert.Equal("foo", propertyInfo.Name);

            Assert.Throws<ArgumentNullException>(() => propertyInfo.Name = null);
        }

        [Fact]
        public static void JsonPropertyInfoForDefaultResolverHasNamingPoliciesRulesApplied()
        {
            JsonSerializerOptions options = new();
            options.PropertyNamingPolicy = JsonNamingPolicy.CamelCase;
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(MyClass), options);
            Assert.Equal(2, typeInfo.Properties.Count);
            JsonPropertyInfo propertyInfo = typeInfo.Properties[0];

            Assert.Equal(nameof(MyClass.Value).ToLowerInvariant(), propertyInfo.Name);

            // explicitly setting does not change casing
            propertyInfo.Name = "Foo";
            Assert.Equal("Foo", propertyInfo.Name);
        }

        [Fact]
        public static void JsonPropertyInfoCustomConverterIsNullWhenUsingCreateJsonPropertyInfo()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(TestClassWithCustomConverterOnProperty), options);
            JsonPropertyInfo propertyInfo = typeInfo.CreateJsonPropertyInfo(typeof(MyClass), "test");

            Assert.Null(propertyInfo.CustomConverter);
        }

        [Fact]
        public static void JsonPropertyInfoCustomConverterIsNotNullForPropertyWithCustomConverter()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(TestClassWithCustomConverterOnProperty), options);
            Assert.Equal(1, typeInfo.Properties.Count);
            JsonPropertyInfo propertyInfo = typeInfo.Properties[0];

            Assert.NotNull(propertyInfo.CustomConverter);
            Assert.IsType<MyClassConverterOriginal>(propertyInfo.CustomConverter);
        }

        [Fact]
        public static void JsonPropertyInfoCustomConverterSetToNullIsRespected()
        {
            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    Assert.NotNull(propertyInfo.CustomConverter);
                    Assert.IsType<MyClassConverterOriginal>(propertyInfo.CustomConverter);
                    propertyInfo.CustomConverter = null;
                }
            });

            options.TypeInfoResolver = r;

            TestClassWithCustomConverterOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            string json = JsonSerializer.Serialize(obj, options);
            Assert.Equal("""{"MyClassProperty":{"Value":"SomeValue","Thing":null}}""", json);

            TestClassWithCustomConverterOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterOnProperty>(json, options);
            Assert.Equal(obj.MyClassProperty.Value, deserialized.MyClassProperty.Value);
        }

        [Fact]
        public static void JsonPropertyInfoCustomConverterIsRespected()
        {
            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterOnProperty))
                {
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    Assert.NotNull(propertyInfo.CustomConverter);
                    Assert.IsType<MyClassConverterOriginal>(propertyInfo.CustomConverter);
                    propertyInfo.CustomConverter = new MyClassCustomConverter("test_");
                }
            });

            options.TypeInfoResolver = r;

            TestClassWithCustomConverterOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            string json = JsonSerializer.Serialize(obj, options);
            Assert.Equal("""{"MyClassProperty":"test_SomeValue"}""", json);

            TestClassWithCustomConverterOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterOnProperty>(json, options);
            Assert.Equal(obj.MyClassProperty.Value, deserialized.MyClassProperty.Value);
        }

        [Fact]
        public static void JsonPropertyInfoCustomConverterFactoryIsNotExpanded()
        {
            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            JsonConverter? expectedConverter = null;
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterFactoryOnProperty))
                {
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    Assert.NotNull(propertyInfo.CustomConverter);
                    Assert.IsType<MyClassCustomConverterFactory>(propertyInfo.CustomConverter);
                    expectedConverter = ((MyClassCustomConverterFactory)propertyInfo.CustomConverter).ConverterInstance;
                }
            });

            options.TypeInfoResolver = r;

            TestClassWithCustomConverterFactoryOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            string json = JsonSerializer.Serialize(obj, options);
            Assert.Equal("""{"MyClassProperty":"test_SomeValue"}""", json);
            Assert.NotNull(expectedConverter);
            Assert.IsType<MyClassCustomConverter>(expectedConverter);

            TestClassWithCustomConverterFactoryOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterFactoryOnProperty>(json, options);
            Assert.Equal(obj.MyClassProperty.Value, deserialized.MyClassProperty.Value);
        }

        [Fact]
        public static void JsonPropertyInfoCustomConverterFactoryIsNotExpandedWhenSetInResolver()
        {
            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            MyClassCustomConverterFactory converterFactory = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithProperty))
                {
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    Assert.Null(propertyInfo.CustomConverter);
                    propertyInfo.CustomConverter = converterFactory;
                    Assert.Same(converterFactory, propertyInfo.CustomConverter);
                }
            });

            options.TypeInfoResolver = r;

            TestClassWithProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            string json = JsonSerializer.Serialize(obj, options);
            Assert.Equal("""{"MyClassProperty":"test_SomeValue"}""", json);

            TestClassWithProperty deserialized = JsonSerializer.Deserialize<TestClassWithProperty>(json, options);
            Assert.Equal(obj.MyClassProperty.Value, deserialized.MyClassProperty.Value);
        }


        [Fact]
        public static void JsonPropertyInfoGetIsNullAndMutableWhenUsingCreateJsonPropertyInfo()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(TestClassWithCustomConverterOnProperty), options);
            JsonPropertyInfo propertyInfo = typeInfo.CreateJsonPropertyInfo(typeof(MyClass), "test");
            Assert.Null(propertyInfo.Get);
            Func<object, object> get = (obj) =>
            {
                throw new NotImplementedException();
            };

            propertyInfo.Get = get;
            Assert.Same(get, propertyInfo.Get);
        }

        [Fact]
        public static void JsonPropertyInfoGetIsNotNullForDefaultResolver()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(TestClassWithCustomConverterOnProperty), options);
            JsonPropertyInfo propertyInfo = typeInfo.Properties[0];

            Assert.NotNull(propertyInfo.Get);

            TestClassWithCustomConverterOnProperty obj = new();

            Assert.Null(propertyInfo.Get(obj));

            obj.MyClassProperty = new MyClass();
            Assert.Same(obj.MyClassProperty, propertyInfo.Get(obj));

            MyClass sentinel = new();
            Func<object, object> get = (obj) => sentinel;
            propertyInfo.Get = get;
            Assert.Same(get, propertyInfo.Get);
            Assert.Same(sentinel, propertyInfo.Get(obj));
        }

        [Fact]
        public static void JsonPropertyInfoGetPropertyNotSerializableButDeserializableWhenNull()
        {
            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    propertyInfo.Get = null;
                }
            });

            options.TypeInfoResolver = r;

            TestClassWithCustomConverterOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            string json = JsonSerializer.Serialize(obj, options);
            Assert.Equal("{}", json);

            json = """{"MyClassProperty":"SomeValue"}""";
            TestClassWithCustomConverterOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterOnProperty>(json, options);
            Assert.Equal(obj.MyClassProperty.Value, deserialized.MyClassProperty.Value);
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        public static void JsonPropertyInfoGetIsRespected(bool useCustomConverter)
        {
            TestClassWithCustomConverterOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            MyClass substitutedValue = new MyClass() { Value = "SomeOtherValue" };

            bool getterCalled = false;

            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    if (!useCustomConverter)
                    {
                        propertyInfo.CustomConverter = null;
                    }

                    propertyInfo.Get = (o) =>
                    {
                        Assert.Same(obj, o);
                        Assert.False(getterCalled);
                        getterCalled = true;
                        return substitutedValue;
                    };
                }
            });

            options.TypeInfoResolver = r;

            string json = JsonSerializer.Serialize(obj, options);
            if (useCustomConverter)
            {
                Assert.Equal("""{"MyClassProperty":"SomeOtherValue"}""", json);
            }
            else
            {
                Assert.Equal("""{"MyClassProperty":{"Value":"SomeOtherValue","Thing":null}}""", json);
            }

            TestClassWithCustomConverterOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterOnProperty>(json, options);
            Assert.Equal(substitutedValue.Value, deserialized.MyClassProperty.Value);

            Assert.True(getterCalled);
        }

        [Fact]
        public static void JsonPropertyInfoSetIsNullAndMutableWhenUsingCreateJsonPropertyInfo()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(TestClassWithCustomConverterOnProperty), options);
            JsonPropertyInfo propertyInfo = typeInfo.CreateJsonPropertyInfo(typeof(MyClass), "test");
            Assert.Null(propertyInfo.Set);
            Action<object, object> set = (obj, val) =>
            {
                throw new NotImplementedException();
            };

            propertyInfo.Set = set;
            Assert.Same(set, propertyInfo.Set);
        }

        [Fact]
        public static void JsonPropertyInfoSetIsNotNullForDefaultResolver()
        {
            JsonSerializerOptions options = new();
            JsonTypeInfo typeInfo = options.TypeInfoResolver.GetTypeInfo(typeof(TestClassWithCustomConverterOnProperty), options);
            Assert.Equal(1, typeInfo.Properties.Count);
            JsonPropertyInfo propertyInfo = typeInfo.Properties[0];

            Assert.NotNull(propertyInfo.Set);

            TestClassWithCustomConverterOnProperty obj = new();

            MyClass value = new MyClass();
            propertyInfo.Set(obj, value);
            Assert.Same(value, obj.MyClassProperty);

            MyClass sentinel = new();
            Action<object, object> set = (o, value) =>
            {
                Assert.Same(obj, o);
                Assert.Same(sentinel, value);
                obj.MyClassProperty = sentinel;
            };

            propertyInfo.Set = set;
            Assert.Same(set, propertyInfo.Set);

            propertyInfo.Set(obj, sentinel);
            Assert.Same(obj.MyClassProperty, sentinel);
        }

        [Fact]
        public static void JsonPropertyInfoSetPropertyDeserializableButNotSerializableWhenNull()
        {
            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    Assert.NotNull(propertyInfo.Set);
                    propertyInfo.Set = null;
                    Assert.Null(propertyInfo.Set);
                }
            });

            options.TypeInfoResolver = r;

            TestClassWithCustomConverterOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            string json = JsonSerializer.Serialize(obj, options);
            Assert.Equal("""{"MyClassProperty":"SomeValue"}""", json);

            TestClassWithCustomConverterOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterOnProperty>(json, options);
            Assert.Null(deserialized.MyClassProperty);
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        public static void JsonPropertyInfoSetIsRespected(bool useCustomConverter)
        {
            TestClassWithCustomConverterOnProperty obj = new()
            {
                MyClassProperty = new MyClass() { Value = "SomeValue" },
            };

            MyClass substitutedValue = new MyClass() { Value = "SomeOtherValue" };
            bool setterCalled = false;

            JsonSerializerOptions options = new();
            DefaultJsonTypeInfoResolver r = new();
            r.Modifiers.Add(ti =>
            {
                if (ti.Type == typeof(TestClassWithCustomConverterOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    JsonPropertyInfo propertyInfo = ti.Properties[0];
                    if (!useCustomConverter)
                    {
                        propertyInfo.CustomConverter = null;
                    }

                    Assert.NotNull(propertyInfo.Set);

                    Action<object, object?> setter = (o, val) =>
                    {
                        var testClass = (TestClassWithCustomConverterOnProperty)o;
                        Assert.IsType<MyClass>(val);
                        MyClass myClass = (MyClass)val;
                        Assert.Equal(obj.MyClassProperty.Value, myClass.Value);

                        testClass.MyClassProperty = substitutedValue;
                        Assert.False(setterCalled);
                        setterCalled = true;
                    };

                    propertyInfo.Set = setter;
                    Assert.Same(setter, propertyInfo.Set);
                }
            });

            options.TypeInfoResolver = r;

            string json = JsonSerializer.Serialize(obj, options);
            if (useCustomConverter)
            {
                Assert.Equal("""{"MyClassProperty":"SomeValue"}""", json);
            }
            else
            {
                Assert.Equal("""{"MyClassProperty":{"Value":"SomeValue","Thing":null}}""", json);
            }

            TestClassWithCustomConverterOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithCustomConverterOnProperty>(json, options);
            Assert.Same(substitutedValue, deserialized.MyClassProperty);
            Assert.True(setterCalled);
        }

        [Fact]
        public static void AddingNumberHandlingToPropertyIsRespected()
        {
            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumber))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.Null(ti.Properties[0].NumberHandling);
                    ti.Properties[0].NumberHandling = JsonNumberHandling.WriteAsString | JsonNumberHandling.AllowReadingFromString;
                    Assert.Equal(JsonNumberHandling.WriteAsString | JsonNumberHandling.AllowReadingFromString, ti.Properties[0].NumberHandling);
                }
            });

            JsonSerializerOptions o = new();
            o.TypeInfoResolver = resolver;

            TestClassWithNumber obj = new()
            {
                IntProperty = 37,
            };

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":"37"}""", json);

            TestClassWithNumber deserialized = JsonSerializer.Deserialize<TestClassWithNumber>(json, o);
            Assert.Equal(obj.IntProperty, deserialized.IntProperty);
        }

        private class TestClassWithNumber
        {
            public int IntProperty { get; set; }
        }

        [Theory]
        [InlineData(null)]
        [InlineData(JsonNumberHandling.Strict)]
        public static void RemovingOrChangingNumberHandlingFromPropertyIsRespected(JsonNumberHandling? numberHandling)
        {
            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumberHandlingOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.Equal(JsonNumberHandling.WriteAsString | JsonNumberHandling.AllowReadingFromString, ti.Properties[0].NumberHandling);
                    ti.Properties[0].NumberHandling = numberHandling;
                    Assert.Equal(numberHandling, ti.Properties[0].NumberHandling);
                }
            });

            JsonSerializerOptions o = new();
            o.TypeInfoResolver = resolver;

            TestClassWithNumberHandlingOnProperty obj = new()
            {
                IntProperty = 37,
            };

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);

            TestClassWithNumberHandlingOnProperty deserialized = JsonSerializer.Deserialize<TestClassWithNumberHandlingOnProperty>(json, o);
            Assert.Equal(obj.IntProperty, deserialized.IntProperty);
        }

        private class TestClassWithNumberHandlingOnProperty
        {
            [JsonNumberHandling(JsonNumberHandling.WriteAsString | JsonNumberHandling.AllowReadingFromString)]
            public int IntProperty { get; set; }
        }

        [Fact]
        public static void NumberHandlingFromTypeDoesntFlowToPropertyAndOverrideIsRespected()
        {
            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumberHandling))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.Null(ti.Properties[0].NumberHandling);
                    ti.Properties[0].NumberHandling = JsonNumberHandling.Strict;
                    Assert.Equal(JsonNumberHandling.Strict, ti.Properties[0].NumberHandling);
                }
            });

            JsonSerializerOptions o = new();
            o.TypeInfoResolver = resolver;

            TestClassWithNumberHandling obj = new()
            {
                IntProperty = 37,
            };

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);

            TestClassWithNumberHandling deserialized = JsonSerializer.Deserialize<TestClassWithNumberHandling>(json, o);
            Assert.Equal(obj.IntProperty, deserialized.IntProperty);
        }

        [JsonNumberHandling(JsonNumberHandling.WriteAsString | JsonNumberHandling.AllowReadingFromString)]
        private class TestClassWithNumberHandling
        {
            public int IntProperty { get; set; }
        }

        [Fact]
        public static void NumberHandlingFromOptionsDoesntFlowToPropertyAndOverrideIsRespected()
        {
            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumber))
                {
                    Assert.Null(ti.Properties[0].NumberHandling);
                    ti.Properties[0].NumberHandling = JsonNumberHandling.Strict;
                    Assert.Equal(JsonNumberHandling.Strict, ti.Properties[0].NumberHandling);
                }
            });

            JsonSerializerOptions o = new();
            o.NumberHandling = JsonNumberHandling.WriteAsString | JsonNumberHandling.AllowReadingFromString;
            o.TypeInfoResolver = resolver;

            TestClassWithNumber obj = new()
            {
                IntProperty = 37,
            };

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);

            TestClassWithNumber deserialized = JsonSerializer.Deserialize<TestClassWithNumber>(json, o);
            Assert.Equal(obj.IntProperty, deserialized.IntProperty);
        }

        [Fact]
        public static void ShouldSerializeShouldReportBackAssignedValue()
        {
            JsonSerializerOptions o = new();

            JsonTypeInfo ti = JsonTypeInfo.CreateJsonTypeInfo(typeof(MyClass), o);
            JsonPropertyInfo pi = ti.CreateJsonPropertyInfo(typeof(string), "test");

            Assert.Null(pi.ShouldSerialize);

            Func<object, object?, bool> value = (o, val) => throw new NotImplementedException();
            pi.ShouldSerialize = value;
            Assert.Same(value, pi.ShouldSerialize);

            pi.ShouldSerialize = null;
            Assert.Null(pi.ShouldSerialize);
        }

        [Fact]
        public static void AddingShouldSerializeToPropertyIsRespected()
        {
            TestClassWithNumber obj = new()
            {
                IntProperty = 3,
            };

            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumber))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.Null(ti.Properties[0].ShouldSerialize);
                    ti.Properties[0].ShouldSerialize = (o, val) =>
                    {
                        Assert.Same(obj, o);
                        int intValue = (int)val;
                        Assert.Equal(obj.IntProperty, intValue);
                        return intValue != 3;
                    };
                }
            });

            JsonSerializerOptions o = new();
            o.TypeInfoResolver = resolver;

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("{}", json);

            obj.IntProperty = 37;
            json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);
        }

        [Theory]
        [InlineData(true)]
        [InlineData(false)]
        public static void RemovingOrChangingShouldSerializeFromPropertyWithIgnoreConditionIsRespected(bool removeShouldSerialize)
        {
            TestClassWithNumberAndIgnoreConditionOnProperty obj = new()
            {
                IntProperty = 37,
            };

            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumberAndIgnoreConditionOnProperty))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.NotNull(ti.Properties[0].ShouldSerialize);
                    Assert.False(ti.Properties[0].ShouldSerialize(null, 0));
                    Assert.True(ti.Properties[0].ShouldSerialize(null, 1));
                    Assert.True(ti.Properties[0].ShouldSerialize(null, -1));
                    Assert.True(ti.Properties[0].ShouldSerialize(null, 3));

                    if (removeShouldSerialize)
                    {
                        ti.Properties[0].ShouldSerialize = null;
                    }
                    else
                    {
                        ti.Properties[0].ShouldSerialize = (o, val) =>
                        {
                            Assert.Same(obj, o);
                            int intValue = (int)val;
                            Assert.Equal(obj.IntProperty, intValue);
                            return intValue != 3;
                        };
                    }
                }
            });

            JsonSerializerOptions o = new();
            o.TypeInfoResolver = resolver;

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);

            obj.IntProperty = default;
            json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":0}""", json);

            obj.IntProperty = 3;
            json = JsonSerializer.Serialize(obj, o);
            if (removeShouldSerialize)
            {
                Assert.Equal("""{"IntProperty":3}""", json);
            }
            else
            {
                Assert.Equal("{}", json);
            }
        }

        private class TestClassWithNumberAndIgnoreConditionOnProperty
        {
            [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingDefault)]
            public int IntProperty { get; set; }
        }

        [Fact]
        public static void DefaultIgnoreConditionFromOptionsDoesntFlowToShouldSerializePropertyAndOverrideIsRespected()
        {
            TestClassWithNumber obj = new()
            {
                IntProperty = 37,
            };

            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumber))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.Null(ti.Properties[0].ShouldSerialize);
                    ti.Properties[0].ShouldSerialize = (o, val) =>
                    {
                        Assert.Same(obj, o);
                        int intValue = (int)val;
                        Assert.Equal(obj.IntProperty, intValue);
                        return intValue != 3;
                    };
                }
            });

            JsonSerializerOptions o = new();
            o.DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingDefault;
            o.TypeInfoResolver = resolver;

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);

            obj.IntProperty = default;
            json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":0}""", json);

            obj.IntProperty = 3;
            json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("{}", json);
        }

        [Fact]
        public static void DefaultIgnoreConditionFromOptionsIsRespectedWhenShouldSerializePropertyIsAssignedAndCleared()
        {
            TestClassWithNumberAndIgnoreConditionOnProperty obj = new()
            {
                IntProperty = 37,
            };

            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add((ti) =>
            {
                if (ti.Type == typeof(TestClassWithNumber))
                {
                    Assert.Equal(1, ti.Properties.Count);
                    Assert.Null(ti.Properties[0].ShouldSerialize);
                    ti.Properties[0].ShouldSerialize = (o, val) =>
                    {
                        Assert.Same(obj, o);
                        int intValue = (int)val;
                        Assert.Equal(obj.IntProperty, intValue);
                        return intValue != 3;
                    };

                    ti.Properties[0].ShouldSerialize = null;
                }
            });

            JsonSerializerOptions o = new();
            o.DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingDefault;
            o.TypeInfoResolver = resolver;

            string json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":37}""", json);

            obj.IntProperty = default;
            json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("{}", json);

            obj.IntProperty = 3;
            json = JsonSerializer.Serialize(obj, o);
            Assert.Equal("""{"IntProperty":3}""", json);
        }

        public enum ModifyJsonIgnore
        {
            DontModify,
            NeverSerialize,
            AlwaysSerialize,
            DontSerializeNumber3OrStringAsd,
        }

        [Theory]
        [InlineData(JsonIgnoreCondition.WhenWritingDefault, ModifyJsonIgnore.DontModify)]
        [InlineData(JsonIgnoreCondition.WhenWritingNull, ModifyJsonIgnore.DontModify)]
        [InlineData(JsonIgnoreCondition.Never, ModifyJsonIgnore.DontModify)]
        [InlineData(JsonIgnoreCondition.WhenWritingDefault, ModifyJsonIgnore.NeverSerialize)]
        [InlineData(JsonIgnoreCondition.WhenWritingNull, ModifyJsonIgnore.NeverSerialize)]
        [InlineData(JsonIgnoreCondition.Never, ModifyJsonIgnore.NeverSerialize)]
        [InlineData(JsonIgnoreCondition.WhenWritingDefault, ModifyJsonIgnore.AlwaysSerialize)]
        [InlineData(JsonIgnoreCondition.WhenWritingNull, ModifyJsonIgnore.AlwaysSerialize)]
        [InlineData(JsonIgnoreCondition.Never, ModifyJsonIgnore.AlwaysSerialize)]
        [InlineData(JsonIgnoreCondition.WhenWritingDefault, ModifyJsonIgnore.DontSerializeNumber3OrStringAsd)]
        [InlineData(JsonIgnoreCondition.WhenWritingNull, ModifyJsonIgnore.DontSerializeNumber3OrStringAsd)]
        [InlineData(JsonIgnoreCondition.Never, ModifyJsonIgnore.DontSerializeNumber3OrStringAsd)]
        public static void JsonIgnoreConditionIsCorrectlyTranslatedToShouldSerializeDelegateAndChangingShouldSerializeIsRespected(JsonIgnoreCondition defaultIgnoreCondition, ModifyJsonIgnore modify)
        {
            TestClassWithEveryPossibleJsonIgnore obj = new()
            {
                AlwaysProperty = "Always",
                WhenWritingDefaultProperty = 37,
                WhenWritingNullProperty = "WhenWritingNull",
                NeverProperty = "Never",
                Property = "None",
            };

            // sanity check
            bool modifierTestRun = false;

            DefaultJsonTypeInfoResolver resolver = new();
            resolver.Modifiers.Add(ti =>
            {
                if (ti.Type != typeof(TestClassWithEveryPossibleJsonIgnore))
                    return;

                Assert.Equal(5, ti.Properties.Count);
                Assert.False(modifierTestRun);
                modifierTestRun = true;
                foreach (var property in ti.Properties)
                {
                    string jsonIgnoreValue = property.Name.Substring(0, property.Name.Length - "Property".Length);
                    JsonIgnoreCondition? ignoreConditionOnProperty = string.IsNullOrEmpty(jsonIgnoreValue) ? null : (JsonIgnoreCondition)Enum.Parse(typeof(JsonIgnoreCondition), jsonIgnoreValue);
                    TestJsonIgnoreConditionDelegate(defaultIgnoreCondition, ignoreConditionOnProperty, property, modify);
                }
            });

            JsonSerializerOptions options = new();
            options.TypeInfoResolver = resolver;
            options.DefaultIgnoreCondition = defaultIgnoreCondition;


            // - delegate correctly returns value
            // - nulling out delegate removes behavior
            // - check every options default
            string json = JsonSerializer.Serialize(obj, options);
            Assert.True(modifierTestRun);

            switch (modify)
            {
                case ModifyJsonIgnore.DontModify:
                    Assert.Equal("""{"WhenWritingDefaultProperty":37,"WhenWritingNullProperty":"WhenWritingNull","NeverProperty":"Never","Property":"None"}""", json);
                    break;
                case ModifyJsonIgnore.NeverSerialize:
                    Assert.Equal("{}", json);
                    break;
                case ModifyJsonIgnore.AlwaysSerialize:
                case ModifyJsonIgnore.DontSerializeNumber3OrStringAsd:
                    Assert.Equal("""{"AlwaysProperty":"Always","WhenWritingDefaultProperty":37,"WhenWritingNullProperty":"WhenWritingNull","NeverProperty":"Never","Property":"None"}""", json);
                    break;
            }

            obj.AlwaysProperty = default;
            obj.WhenWritingDefaultProperty = default;
            obj.WhenWritingNullProperty = default;
            obj.NeverProperty = default;
            obj.Property = default;

            json = JsonSerializer.Serialize(obj, options);

            switch (modify)
            {
                case ModifyJsonIgnore.DontModify:
                    {
                        string noJsonIgnoreProperty = defaultIgnoreCondition == JsonIgnoreCondition.Never ? @",""Property"":null" : null;
                        Assert.Equal($@"{{""NeverProperty"":null{noJsonIgnoreProperty}}}", json);
                        break;
                    }
                case ModifyJsonIgnore.NeverSerialize:
                    Assert.Equal("{}", json);
                    break;
                case ModifyJsonIgnore.AlwaysSerialize:
                case ModifyJsonIgnore.DontSerializeNumber3OrStringAsd:
                    Assert.Equal("""{"AlwaysProperty":null,"WhenWritingDefaultProperty":0,"WhenWritingNullProperty":null,"NeverProperty":null,"Property":null}""", json);
                    break;
            }

            obj.AlwaysProperty = "asd";
            obj.WhenWritingDefaultProperty = 3;
            obj.WhenWritingNullProperty = "asd";
            obj.NeverProperty = "asd";
            obj.Property = "asd";

            json = JsonSerializer.Serialize(obj, options);

            switch (modify)
            {
                case ModifyJsonIgnore.DontModify:
                    Assert.Equal("""{"WhenWritingDefaultProperty":3,"WhenWritingNullProperty":"asd","NeverProperty":"asd","Property":"asd"}""", json);
                    break;
                case ModifyJsonIgnore.AlwaysSerialize:
                    Assert.Equal("""{"AlwaysProperty":"asd","WhenWritingDefaultProperty":3,"WhenWritingNullProperty":"asd","NeverProperty":"asd","Property":"asd"}""", json);
                    break;
                case ModifyJsonIgnore.NeverSerialize:
                case ModifyJsonIgnore.DontSerializeNumber3OrStringAsd:
                    Assert.Equal("{}", json);
                    break;
            }

            static void TestJsonIgnoreConditionDelegate(JsonIgnoreCondition defaultIgnoreCondition, JsonIgnoreCondition? ignoreConditionOnProperty, JsonPropertyInfo property, ModifyJsonIgnore modify)
            {
                // defaultIgnoreCondition is not taken into accound, we might expect null if defaultIgnoreCondition == ignoreConditionOnProperty
                switch (ignoreConditionOnProperty)
                {
                    case null:
                        Assert.Null(property.ShouldSerialize);
                        break;
                    case JsonIgnoreCondition.Always:
                        Assert.NotNull(property.ShouldSerialize);
                        Assert.False(property.ShouldSerialize(null, null));
                        Assert.False(property.ShouldSerialize(null, ""));
                        Assert.False(property.ShouldSerialize(null, "asd"));

                        Assert.Null(property.Get);
                        Assert.Null(property.Set);
                        break;
                    case JsonIgnoreCondition.WhenWritingDefault:
                        Assert.NotNull(property.ShouldSerialize);
                        Assert.False(property.ShouldSerialize(null, 0));
                        Assert.True(property.ShouldSerialize(null, 1));
                        Assert.True(property.ShouldSerialize(null, -1));
                        break;
                    case JsonIgnoreCondition.WhenWritingNull:
                        Assert.NotNull(property.ShouldSerialize);
                        Assert.False(property.ShouldSerialize(null, null));
                        Assert.True(property.ShouldSerialize(null, ""));
                        Assert.True(property.ShouldSerialize(null, "asd"));
                        break;
                    case JsonIgnoreCondition.Never:
                        Assert.NotNull(property.ShouldSerialize);
                        Assert.True(property.ShouldSerialize(null, null));
                        Assert.True(property.ShouldSerialize(null, ""));
                        Assert.True(property.ShouldSerialize(null, "asd"));
                        break;
                }

                if (modify != ModifyJsonIgnore.DontModify && ignoreConditionOnProperty == JsonIgnoreCondition.Always)
                {
                    property.Get = (o) => ((TestClassWithEveryPossibleJsonIgnore)o).AlwaysProperty;
                }

                switch (modify)
                {
                    case ModifyJsonIgnore.AlwaysSerialize:
                        property.ShouldSerialize = (o, v) => true;
                        break;
                    case ModifyJsonIgnore.NeverSerialize:
                        property.ShouldSerialize = (o, v) => false;
                        break;
                    case ModifyJsonIgnore.DontSerializeNumber3OrStringAsd:
                        property.ShouldSerialize = (o, v) =>
                        {
                            if (v is null)
                            {
                                return true;
                            }
                            else if (v is int intVal)
                            {
                                return intVal != 3;
                            }
                            else if (v is string stringVal)
                            {
                                return stringVal != "asd";
                            }

                            Assert.Fail("ShouldSerialize set for value which is not int or string");
                            return false;
                        };
                        break;
                }
            }
        }

        private class TestClassWithEveryPossibleJsonIgnore
        {
            [JsonIgnore(Condition = JsonIgnoreCondition.Always)]
            public string AlwaysProperty { get; set; }

            [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingDefault)]
            public int WhenWritingDefaultProperty { get; set; }

            [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
            public string WhenWritingNullProperty { get; set; }

            [JsonIgnore(Condition = JsonIgnoreCondition.Never)]
            public string NeverProperty { get; set; }

            public string Property { get; set; }
        }

        private class TestClassWithProperty
        {
            public MyClass MyClassProperty { get; set; }
        }

        private class TestClassWithCustomConverterOnProperty
        {
            [JsonConverter(typeof(MyClassConverterOriginal))]
            public MyClass MyClassProperty { get; set; }
        }

        private class TestClassWithCustomConverterFactoryOnProperty
        {
            [JsonConverter(typeof(MyClassCustomConverterFactory))]
            public MyClass MyClassProperty { get; set; }
        }

        private class MyClassConverterOriginal : JsonConverter<MyClass>
        {
            public override MyClass? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType != JsonTokenType.String)
                    throw new InvalidOperationException($"Wrong token type: {reader.TokenType}");

                MyClass myClass = new MyClass();
                myClass.Value = reader.GetString();
                return myClass;
            }

            public override void Write(Utf8JsonWriter writer, MyClass value, JsonSerializerOptions options)
            {
                writer.WriteStringValue(value.Value);
            }
        }

        private class MyClassCustomConverter : JsonConverter<MyClass>
        {
            private string _prefix;

            public MyClassCustomConverter(string prefix)
            {
                _prefix = prefix;
            }

            public override MyClass? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                if (reader.TokenType != JsonTokenType.String)
                    throw new InvalidOperationException($"Wrong token type: {reader.TokenType}");

                MyClass myClass = new MyClass();
                myClass.Value = reader.GetString().Substring(_prefix.Length);
                return myClass;
            }

            public override void Write(Utf8JsonWriter writer, MyClass value, JsonSerializerOptions options)
            {
                writer.WriteStringValue(_prefix + value.Value);
            }
        }

        private class MyClassCustomConverterFactory : JsonConverterFactory
        {
            internal JsonConverter<MyClass> ConverterInstance { get; } = new MyClassCustomConverter("test_");
            public override bool CanConvert(Type typeToConvert) => typeToConvert == typeof(MyClass);

            public override JsonConverter? CreateConverter(Type typeToConvert, JsonSerializerOptions options)
            {
                Assert.Equal(typeof(MyClass), typeToConvert);
                return ConverterInstance;
            }
        }
    }
}
