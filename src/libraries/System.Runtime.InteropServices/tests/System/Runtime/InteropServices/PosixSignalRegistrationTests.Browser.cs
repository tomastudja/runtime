// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Linq;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace System.Tests
{
    public partial class PosixSignalRegistrationTests
    {
        public static IEnumerable<object[]> UninstallableSignals() => Enumerable.Empty<object[]>();

        public static IEnumerable<object[]> SupportedSignals() => Enumerable.Empty<object[]>();

        public static IEnumerable<object[]> UnsupportedSignals()
        {
            foreach (PosixSignal signal in Enum.GetValues<PosixSignal>())
            {
                yield return new object[] { signal };
            }

            yield return new object[] { 0 };
            yield return new object[] { 3 };
            yield return new object[] { -1000 };
            yield return new object[] { 1000 };
        }
    }
}
