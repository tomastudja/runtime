// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Runtime.CompilerServices
{
    // Custom attribute to indicate that strings should be frozen.

    [AttributeUsage(AttributeTargets.Assembly, Inherited = false)]
    public sealed class StringFreezingAttribute : Attribute
    {
        public StringFreezingAttribute() { }
    }
}
