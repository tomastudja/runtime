// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Diagnostics;
using System.Collections.ObjectModel;

namespace System.Security.Cryptography
{
    public sealed class CngPropertyCollection : Collection<CngProperty>
    {
        public CngPropertyCollection()
            : base()
        {
        }
    }
}
