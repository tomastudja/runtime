﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

// Source files represent a source generated JsonSerializerContext as produced by the .NET 6 SDK.
// Used to validate correctness of contexts generated by previous SDKs against the current System.Text.Json runtime components.
// Unless absolutely necessary DO NOT MODIFY any of these files -- it would invalidate the purpose of the regression tests.

// <auto-generated/>
#nullable enable

// Suppress warnings about [Obsolete] member usage in generated code.
#pragma warning disable CS0618

namespace System.Text.Json.Tests.SourceGenRegressionTests.Net60
{
    public partial class Net60GeneratedContext
    {
        private global::System.Text.Json.Serialization.Metadata.JsonTypeInfo<global::System.Int32>? _Int32;
        public global::System.Text.Json.Serialization.Metadata.JsonTypeInfo<global::System.Int32> Int32
        {
            get
            {
                if (_Int32 == null)
                {
                    global::System.Text.Json.Serialization.JsonConverter? customConverter;
                    if (Options.Converters.Count > 0 && (customConverter = GetRuntimeProvidedCustomConverter(typeof(global::System.Int32))) != null)
                    {
                        _Int32 = global::System.Text.Json.Serialization.Metadata.JsonMetadataServices.CreateValueInfo<global::System.Int32>(Options, customConverter);
                    }
                    else
                    {
                        _Int32 = global::System.Text.Json.Serialization.Metadata.JsonMetadataServices.CreateValueInfo<global::System.Int32>(Options, global::System.Text.Json.Serialization.Metadata.JsonMetadataServices.Int32Converter);
                    }
                }
        
                return _Int32;
            }
        }
    }
}
