// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Reflection;

namespace Internal.TypeSystem
{
    public interface IModuleResolver
    {
        ModuleDesc ResolveAssembly(AssemblyName name, bool throwIfNotFound = true);
        ModuleDesc ResolveModule(IAssemblyDesc referencingModule, string fileName, bool throwIfNotFound = true);
    }
}
