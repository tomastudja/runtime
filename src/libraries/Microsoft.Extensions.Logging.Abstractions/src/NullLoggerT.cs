﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;

namespace Microsoft.Extensions.Logging.Abstractions
{
    /// <summary>
    /// Minimalistic logger that does nothing.
    /// </summary>
    public class NullLogger<T> : ILogger<T>
    {
        /// <summary>
        /// Returns an instance of <see cref="NullLogger{T}"/>.
        /// </summary>
        /// <returns>An instance of <see cref="NullLogger{T}"/>.</returns>
        public static readonly NullLogger<T> Instance = new NullLogger<T>();

        /// <inheritdoc />
        public IDisposable BeginScope<TState>(TState state)
        {
            return NullDisposable.Instance;
        }

        /// <inheritdoc />
        /// <remarks>
        /// This method ignores the parameters and does nothing.
        /// </remarks>
        public void Log<TState>(
            LogLevel logLevel,
            EventId eventId,
            TState state,
            Exception exception,
            Func<TState, Exception, string> formatter)
        {
        }

        /// <inheritdoc />
        public bool IsEnabled(LogLevel logLevel)
        {
            return false;
        }

        private class NullDisposable : IDisposable
        {
            public static readonly NullDisposable Instance = new NullDisposable();

            public void Dispose()
            {
                // intentionally does nothing
            }
        }
    }
}
