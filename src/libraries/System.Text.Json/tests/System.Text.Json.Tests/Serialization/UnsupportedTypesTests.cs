﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Text.Json.Serialization.Tests
{
    public sealed partial class UnsupportedTypesTestsDynamic : UnsupportedTypesTests
    {
        public UnsupportedTypesTestsDynamic() : base(
            JsonSerializerWrapper.StringSerializer,
            supportsJsonPathOnSerialize: true)
        {
        }
    }
}
