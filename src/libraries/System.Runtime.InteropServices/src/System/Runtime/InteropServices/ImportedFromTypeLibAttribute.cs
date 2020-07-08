// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Runtime.InteropServices
{
    [AttributeUsage(AttributeTargets.Assembly, Inherited = false)]
    public sealed class ImportedFromTypeLibAttribute : Attribute
    {
        public ImportedFromTypeLibAttribute(string tlbFile) => Value = tlbFile;

        public string Value { get; }
    }
}
