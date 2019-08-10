﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Collections.Generic;
using Xunit;

namespace System.Text.Json.Serialization.Tests
{
    public static partial class NullTests
    {
        [Fact]
        public static void DefaultIgnoreNullValuesOnWrite()
        {
            var obj = new TestClassWithInitializedProperties
            {
                MyString = null,
                MyInt = null,
                MyIntArray = null,
                MyIntList = null,
                MyObjectList = new List<object> { null },
                MyListList = new List<List<object>> { new List<object> { null } },
                MyDictionaryList = new List<Dictionary<string, string>> { new Dictionary<string, string>() { ["key"] = null } },
                MyStringDictionary = new Dictionary<string, string>() { ["key"] = null },
                MyObjectDictionary = new Dictionary<string, object>() { ["key"] = null },
                MyStringDictionaryDictionary = new Dictionary<string, Dictionary<string, string>>() { ["key"] = null },
                MyListDictionary = new Dictionary<string, List<object>>() { ["key"] = null },
                MyObjectDictionaryDictionary = new Dictionary<string, Dictionary<string, object>>() { ["key"] = null }
            };

            string json = JsonSerializer.Serialize(obj);
            Assert.Contains(@"""MyString"":null", json);
            Assert.Contains(@"""MyInt"":null", json);
            Assert.Contains(@"""MyIntArray"":null", json);
            Assert.Contains(@"""MyIntList"":null", json);
            Assert.Contains(@"""MyObjectList"":[null],", json);
            Assert.Contains(@"""MyListList"":[[null]],", json);
            Assert.Contains(@"""MyDictionaryList"":[{""key"":null}],", json);
            Assert.Contains(@"""MyStringDictionary"":{""key"":null},", json);
            Assert.Contains(@"""MyObjectDictionary"":{""key"":null},", json);
            Assert.Contains(@"""MyStringDictionaryDictionary"":{""key"":null},", json);
            Assert.Contains(@"""MyListDictionary"":{""key"":null},", json);
            Assert.Contains(@"""MyObjectDictionaryDictionary"":{""key"":null}", json);
        }

        [Fact]
        public static void EnableIgnoreNullValuesOnWrite()
        {
            JsonSerializerOptions options = new JsonSerializerOptions();
            options.IgnoreNullValues = true;

            var obj = new TestClassWithInitializedProperties
            {
                MyString = null,
                MyInt = null,
                MyIntArray = null,
                MyIntList = null,
                MyObjectList = new List<object> { null },
                MyListList = new List<List<object>> { new List<object> { null } },
                MyDictionaryList = new List<Dictionary<string, string>> { new Dictionary<string, string>() { ["key"] = null } },
                MyStringDictionary = new Dictionary<string, string>() { ["key"] = null },
                MyObjectDictionary = new Dictionary<string, object>() { ["key"] = null },
                MyStringDictionaryDictionary = new Dictionary<string, Dictionary<string, string>>() { ["key"] = null },
                MyListDictionary = new Dictionary<string, List<object>>() { ["key"] = null },
                MyObjectDictionaryDictionary = new Dictionary<string, Dictionary<string, object>>() { ["key"] = null }
            };

            string expectedJson =
                    @"{" +
                        @"""MyObjectList"":[null]," +
                        @"""MyListList"":[[null]]," +
                        @"""MyDictionaryList"":[" +
                            @"{" +
                                @"""key"":null" +
                            @"}" +
                        @"]," +
                        @"""MyStringDictionary"":{" +
                            @"""key"":null" +
                        @"}," +
                        @"""MyObjectDictionary"":{" +
                            @"""key"":null" +
                        @"}," +
                        @"""MyStringDictionaryDictionary"":{" +
                            @"""key"":null" +
                        @"}," +
                        @"""MyListDictionary"":{" +
                            @"""key"":null" +
                        @"}," +
                        @"""MyObjectDictionaryDictionary"":{" +
                            @"""key"":null" +
                        @"}" +
                    @"}";

            Assert.Equal(expectedJson, JsonSerializer.Serialize(obj, options));

            var parentObj = new WrapperForTestClassWithInitializedProperties
            {
                MyClass = obj
            };

            expectedJson = @"{""MyClass"":" + expectedJson + "}";
            Assert.Equal(expectedJson, JsonSerializer.Serialize(parentObj, options));
        }

        [Fact]
        public static void NullReferences()
        {
            var obj = new ObjectWithObjectProperties();
            obj.Address = null;
            obj.Array = null;
            obj.List = null;
            obj.IEnumerableT = null;
            obj.IListT = null;
            obj.ICollectionT = null;
            obj.IReadOnlyCollectionT = null;
            obj.IReadOnlyListT = null;
            obj.StackT = null;
            obj.QueueT = null;
            obj.HashSetT = null;
            obj.LinkedListT = null;
            obj.SortedSetT = null;
            obj.NullableInt = null;
            obj.NullableIntArray = null;
            obj.Object = null;

            string json = JsonSerializer.Serialize(obj);
            Assert.Contains(@"""Address"":null", json);
            Assert.Contains(@"""List"":null", json);
            Assert.Contains(@"""Array"":null", json);
            Assert.Contains(@"""IEnumerableT"":null", json);
            Assert.Contains(@"""IListT"":null", json);
            Assert.Contains(@"""ICollectionT"":null", json);
            Assert.Contains(@"""IReadOnlyCollectionT"":null", json);
            Assert.Contains(@"""IReadOnlyListT"":null", json);
            Assert.Contains(@"""StackT"":null", json);
            Assert.Contains(@"""QueueT"":null", json);
            Assert.Contains(@"""HashSetT"":null", json);
            Assert.Contains(@"""LinkedListT"":null", json);
            Assert.Contains(@"""SortedSetT"":null", json);
            Assert.Contains(@"""NullableInt"":null", json);
            Assert.Contains(@"""Object"":null", json);
            Assert.Contains(@"""NullableIntArray"":null", json);
        }

        [Fact]
        public static void NullArrayElement()
        {
            string json = JsonSerializer.Serialize(new ObjectWithObjectProperties[]{ null });
            Assert.Equal("[null]", json);
        }

        [Fact]
        public static void NullArgumentFail()
        {
            Assert.Throws<ArgumentNullException>(() => JsonSerializer.Serialize("", (Type)null));
        }

        [Fact]
        public static void NullObjectOutput()
        {
            {
                string output = JsonSerializer.Serialize<string>(null);
                Assert.Equal("null", output);
            }

            {
                string output = JsonSerializer.Serialize<string>(null, null);
                Assert.Equal("null", output);
            }
        }

        class WrapperForTestClassWithInitializedProperties
        {
            public TestClassWithInitializedProperties MyClass { get; set; }
        }

        [Fact]
        public static void SerializeDictionaryWithNullValues()
        {
            Dictionary<string, string> StringVals = new Dictionary<string, string>()
            {
                ["key"] = null,
            };
            Assert.Equal(@"{""key"":null}", JsonSerializer.Serialize(StringVals));

            Dictionary<string, object> ObjVals = new Dictionary<string, object>()
            {
                ["key"] = null,
            };
            Assert.Equal(@"{""key"":null}", JsonSerializer.Serialize(ObjVals));

            Dictionary<string, Dictionary<string, string>> StringDictVals = new Dictionary<string, Dictionary<string, string>>()
            {
                ["key"] = null,
            };
            Assert.Equal(@"{""key"":null}", JsonSerializer.Serialize(StringDictVals));

            Dictionary<string, Dictionary<string, object>> ObjectDictVals = new Dictionary<string, Dictionary<string, object>>()
            {
                ["key"] = null,
            };
            Assert.Equal(@"{""key"":null}", JsonSerializer.Serialize(ObjectDictVals));
        }

        [Fact]
        public static void DeserializeDictionaryWithNullValues()
        {
            {
                Dictionary<string, string> dict = JsonSerializer.Deserialize<Dictionary<string, string>>(@"{""key"":null}");
                Assert.Null(dict["key"]);
            }

            {
                Dictionary<string, object> dict = JsonSerializer.Deserialize<Dictionary<string, object>>(@"{""key"":null}");
                Assert.Null(dict["key"]);
            }

            {
                Dictionary<string, Dictionary<string, string>> dict = JsonSerializer.Deserialize<Dictionary<string, Dictionary<string, string>>>(@"{""key"":null}");
                Assert.Null(dict["key"]);
            }

            {
                Dictionary<string, Dictionary<string, object>> dict = JsonSerializer.Deserialize<Dictionary<string, Dictionary<string, object>>>(@"{""key"":null}");
                Assert.Null(dict["key"]);
            }
        }
    }
}
