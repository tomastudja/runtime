// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.CompilerServices;

namespace System.Threading
{
    internal unsafe sealed partial class LowLevelLifoSemaphore : IDisposable
    {
        private IntPtr lifo_semaphore;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern IntPtr InitInternal();

        private void Create(int maximumSignalCount)
        {
            lifo_semaphore = InitInternal();
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void DeleteInternal(IntPtr semaphore);

        public void Dispose()
        {
            DeleteInternal(lifo_semaphore);
            lifo_semaphore = IntPtr.Zero;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern int TimedWaitInternal(IntPtr semaphore, int timeoutMs);

        private bool WaitCore(int timeoutMs)
        {
            return TimedWaitInternal(lifo_semaphore, timeoutMs) != 0;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void ReleaseInternal(IntPtr semaphore, int count);

        private void ReleaseCore(int count)
        {
            ReleaseInternal(lifo_semaphore, count);
        }
    }
}
