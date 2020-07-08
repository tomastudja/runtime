// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Diagnostics
{
    public partial class Stopwatch
    {
        private static long QueryPerformanceFrequency()
        {
            return (long)Interop.Sys.GetTimestampResolution();
        }

        private static long QueryPerformanceCounter()
        {
            return (long)Interop.Sys.GetTimestamp();
        }
    }
}
