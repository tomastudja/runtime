// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Net
{
    internal static class ExceptionCheck
    {
        internal static bool IsFatal(Exception exception) => exception is OutOfMemoryException;
    }
}
