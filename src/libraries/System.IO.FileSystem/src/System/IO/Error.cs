// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.IO
{
    /// <summary>
    /// Provides centralized methods for creating exceptions for System.IO.FileSystem.
    /// </summary>
    internal static class Error
    {
        internal static Exception GetEndOfFile()
        {
            return new EndOfStreamException(SR.IO_EOF_ReadBeyondEOF);
        }
    }
}
