// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace System.Timers
{
    public class ElapsedEventArgs : EventArgs
    {
        internal ElapsedEventArgs(long fileTime)
        {
            SignalTime = DateTime.FromFileTime(fileTime);
        }

        public DateTime SignalTime { get; }
    }
}
