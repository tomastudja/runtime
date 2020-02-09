// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace Internal.TypeSystem
{
    // Implements canonicalization handling for TypeDefs
    partial class DefType
    {
        protected override TypeDesc ConvertToCanonFormImpl(CanonicalFormKind kind)
        {
            return this;
        }
    }
}