// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.CompilerServices;

namespace System
{
    // The base class for all event classes.
    [Serializable]
    [TypeForwardedFrom("mscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")]
    public class EventArgs
    {
        public static readonly EventArgs Empty = new EventArgs();

        public EventArgs()
        {
        }
    }
}
