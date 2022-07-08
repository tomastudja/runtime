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
        public override global::System.Text.Json.Serialization.Metadata.JsonTypeInfo GetTypeInfo(global::System.Type type)
        {
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net60.WeatherForecastWithPOCOs))
            {
                return this.WeatherForecastWithPOCOs;
            }
        
            if (type == typeof(global::System.Text.Json.Tests.SourceGenRegressionTests.Net60.ClassWithCustomConverter))
            {
                return this.ClassWithCustomConverter;
            }
        
            return null!;
        }
    }
}
