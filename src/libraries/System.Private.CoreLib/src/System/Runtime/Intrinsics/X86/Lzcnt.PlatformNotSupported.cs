// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.CompilerServices;

namespace System.Runtime.Intrinsics.X86
{
    /// <summary>
    /// This class provides access to Intel LZCNT hardware instructions via intrinsics
    /// </summary>
    [CLSCompliant(false)]
    public abstract class Lzcnt : X86Base
    {
        internal Lzcnt() { }

        public static new bool IsSupported { [Intrinsic] get { return false; } }

        public new abstract class X64 : X86Base.X64
        {
            internal X64() { }

            public static new bool IsSupported { [Intrinsic] get { return false; } }

            /// <summary>
            /// unsigned __int64 _lzcnt_u64 (unsigned __int64 a)
            ///   LZCNT reg, reg/m64
            /// This intrinisc is only available on 64-bit processes
            /// </summary>
            public static ulong LeadingZeroCount(ulong value) { throw new PlatformNotSupportedException(); }
        }

        /// <summary>
        /// unsigned int _lzcnt_u32 (unsigned int a)
        ///   LZCNT reg, reg/m32
        /// </summary>
        public static uint LeadingZeroCount(uint value) { throw new PlatformNotSupportedException(); }
    }
}
