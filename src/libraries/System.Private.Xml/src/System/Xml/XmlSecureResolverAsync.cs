// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#nullable enable
using System.Threading.Tasks;

namespace System.Xml
{
    public partial class XmlSecureResolver : XmlResolver
    {
        public override Task<object> GetEntityAsync(Uri absoluteUri, string? role, Type ofObjectToReturn)
        {
            return _resolver.GetEntityAsync(absoluteUri, role, ofObjectToReturn);
        }
    }
}
