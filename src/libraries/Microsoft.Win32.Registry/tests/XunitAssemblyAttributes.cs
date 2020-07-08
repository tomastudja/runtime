// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

// Registry tests can conflict with each other due to accessing the same keys/values in the registry
[assembly: CollectionBehavior(CollectionBehavior.CollectionPerAssembly)]
