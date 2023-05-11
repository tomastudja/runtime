﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;

namespace Microsoft.Extensions.Configuration.Binder.SourceGeneration
{
    internal sealed record SourceGenerationSpec(
        Dictionary<MethodSpecifier, HashSet<TypeSpec>> RootConfigTypes,
        MethodSpecifier MethodsToGen,
        HashSet<ParsableFromStringTypeSpec> PrimitivesForHelperGen,
        HashSet<string> Namespaces)
    {
        public bool HasRootMethods() =>
            ShouldEmitMethods(MethodSpecifier.GetMethods | MethodSpecifier.BindMethods | MethodSpecifier.Configure | MethodSpecifier.GetValueMethods);

        public bool ShouldEmitMethods(MethodSpecifier methods) => (MethodsToGen & methods) != 0;
    }
}
