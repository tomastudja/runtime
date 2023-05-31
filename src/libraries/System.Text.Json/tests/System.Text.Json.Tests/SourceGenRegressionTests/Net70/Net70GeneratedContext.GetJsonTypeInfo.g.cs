﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Source files represent a source generated JsonSerializerContext as produced by the .NET 7 SDK.
// Used to validate correctness of contexts generated by previous SDKs against the current System.Text.Json runtime components.
// Unless absolutely necessary DO NOT MODIFY any of these files -- it would invalidate the purpose of the regression tests.

// <auto-generated/>

#nullable enable annotations
#nullable disable warnings

// Suppress warnings about [Obsolete] member usage in generated code.
#pragma warning disable CS0618

namespace System.Text.Json.Tests.SourceGenRegressionTests.Net70
{
    public partial class Net70GeneratedContext: global::System.Text.Json.Serialization.Metadata.IJsonTypeInfoResolver
    {
        /// <inheritdoc/>
        public override global::System.Text.Json.Serialization.Metadata.JsonTypeInfo GetTypeInfo(global::System.Type type)
        {
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.WeatherForecastWithPOCOs))
            {
                return this.WeatherForecastWithPOCOs;
            }
        
            if (type == typeof(global::System.DateTimeOffset))
            {
                return this.DateTimeOffset;
            }
        
            if (type == typeof(global::System.Int32))
            {
                return this.Int32;
            }
        
            if (type == typeof(global::System.String))
            {
                return this.String;
            }
        
            if (type == typeof(global::System.Collections.Generic.List<global::System.DateTimeOffset>))
            {
                return this.ListDateTimeOffset;
            }
        
            if (type == typeof(global::System.Collections.Generic.Dictionary<global::System.String, global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.HighLowTemps>))
            {
                return this.DictionaryStringHighLowTemps;
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.HighLowTemps))
            {
                return this.HighLowTemps;
            }
        
            if (type == typeof(global::System.String[]))
            {
                return this.StringArray;
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.ClassWithCustomConverter))
            {
                return this.ClassWithCustomConverter;
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.MyLinkedList))
            {
                return this.MyLinkedList;
            }
        
            return null!;
        }
        
        global::System.Text.Json.Serialization.Metadata.JsonTypeInfo? global::System.Text.Json.Serialization.Metadata.IJsonTypeInfoResolver.GetTypeInfo(global::System.Type type, global::System.Text.Json.JsonSerializerOptions options)
        {
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.WeatherForecastWithPOCOs))
            {
                return Create_WeatherForecastWithPOCOs(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.DateTimeOffset))
            {
                return Create_DateTimeOffset(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.Int32))
            {
                return Create_Int32(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.String))
            {
                return Create_String(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.Collections.Generic.List<global::System.DateTimeOffset>))
            {
                return Create_ListDateTimeOffset(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.Collections.Generic.Dictionary<global::System.String, global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.HighLowTemps>))
            {
                return Create_DictionaryStringHighLowTemps(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.HighLowTemps))
            {
                return Create_HighLowTemps(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.String[]))
            {
                return Create_StringArray(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.ClassWithCustomConverter))
            {
                return Create_ClassWithCustomConverter(options, makeReadOnly: false);
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net70.MyLinkedList))
            {
                return Create_MyLinkedList(options, makeReadOnly: false);
            }
        
            return null;
        }
        
    }
}
