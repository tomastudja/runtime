// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using Xunit;

#pragma warning disable CS0649

// This test is to validate that SKIP_ALLOC_FRAME() procedure used by UnwindEspFrameProlog() in eetwain.cpp
// functions correctly on win-x86. The platform doesn't have unwind information as other platforms and hence unwinder either uses ebp chain of the stack frames
// or, in a case of an esp-based frame, unwinds the frame "manually" by decoding instructions in a function prolog.
//
// In order to validate that the unwinding procedure works correctly with such frames:
//
// 1) We need to construct a method with esp-based frame.
// That means that such method cannot call other methods (i.e. it must be a leaf method).
// The method also should not be inlined into its caller.
// However, we cannot simply decorate the method with NoInlining attribute, since the method in that case would have JIT_FLAG_FRAMED set by VM and will have ebp-based frame.
// Therefore, in order to prevent such method from inlining it can be declared as virtual.
//
// 2) There are a number of different options for the JIT to allocate a frame of a method. Such frame may or may not require stack probing.
// All of these options need to be tested.
// This include testing that runtime is back compatible and can run (and unwind) R2R code generated by earlier versions of the JIT.

namespace Runtime_45090
{
    struct _4
    {
        public byte _0;
        public byte _1;
        public byte _2;
        public byte _3;
    }

    struct _10
    {
        public long _0;
        public long _8;
    }

    struct _100
    {
        public _10 _0;
        public _10 _1;
        public _10 _2;
        public _10 _3;
        public _10 _4;
        public _10 _5;
        public _10 _6;
        public _10 _7;
        public _10 _8;
        public _10 _9;
        public _10 _a;
        public _10 _b;
        public _10 _c;
        public _10 _d;
        public _10 _e;
        public _10 _f;
    }

    struct _e00
    {
        public _100 _0;
        public _100 _1;
        public _100 _2;
        public _100 _3;
        public _100 _4;
        public _100 _5;
        public _100 _6;
        public _100 _7;
        public _100 _8;
        public _100 _9;
        public _100 _a;
        public _100 _b;
        public _100 _c;
        public _100 _d;
    }

    struct _1000
    {
        public _100 _0;
        public _100 _1;
        public _100 _2;
        public _100 _3;
        public _100 _4;
        public _100 _5;
        public _100 _6;
        public _100 _7;
        public _100 _8;
        public _100 _9;
        public _100 _a;
        public _100 _b;
        public _100 _c;
        public _100 _d;
        public _100 _e;
        public _100 _f;
    }

    struct _1e00
    {
        public _1000 _0;
        public _e00  _1;
    }

    struct _2000
    {
        public _1000 _0;
        public _1000 _1;
    }

    struct _2e00
    {
        public _1000 _0;
        public _1000 _1;
        public _e00  _2;
    }

    struct _3000
    {
        public _1000 _0;
        public _1000 _1;
        public _1000 _2;
    }

    abstract class AllocFrame
    {
        public abstract int VirtMethodEspBasedFrame();
    }

    class PushReg : AllocFrame
    {
        // Frame size is 4 bytes
        //   push eax
        public unsafe override int VirtMethodEspBasedFrame()
        {
            _4 tmp = new _4();
            *(int*)(&tmp) = 45090;
            return (int)tmp._1;
        }
    }

    class SubSp : AllocFrame
    {
        // Frame size is such that it doesn't require probing
        //     sub esp, #frameSize
        public override int VirtMethodEspBasedFrame()
        {
            _100 tmp = new _100();
            return (int)tmp._0._0;
        }
    }

    class ProbeAfterSubSp : AllocFrame
    {
        // Frame size is smaller than 0x1000 bytes, but needs probing **after** allocation
        //     sub esp, #frameSize
        //     test eax, [esp]
        public override int VirtMethodEspBasedFrame()
        {
            _e00 tmp = new _e00();
            return (int)tmp._0._0._0;
        }
    }

    class ProbeBeforeSubSp1 : AllocFrame
    {
        // Frame size is in range [0x1000, 0x2000) bytes
        // Since 6.0
        //     lea eax, [esp-#frameSize]
        //     call JIT_StackProbe
        //     mov esp, eax
        // Before 6.0
        //     test [esp-0x1000], eax
        //     sub esp, #frameSize
        public override int VirtMethodEspBasedFrame()
        {
            _1000 tmp = new _1000();
            return (int)tmp._0._0._0;
        }
    }

    class ProbeBeforeSubSp2 : AllocFrame
    {
        // Frame size is in range [0x1000, 0x2000) bytes and needs additional probing **after** allocation
        // Since 6.0
        //     lea eax, [esp-#frameSize]
        //     call JIT_StackProbe
        //     mov esp, eax
        // Before 6.0
        //     test [esp-0x1000], eax
        //     sub esp, #frameSize
        //     test [esp], eax
        public override int VirtMethodEspBasedFrame()
        {
            _1e00 tmp = new _1e00();
            return (int)tmp._0._0._0._0;
        }
    }

    class ProbeBeforeSubSp3 : AllocFrame
    {
        // Frame size is in range [0x2000, 0x3000) bytes
        // Since 6.0
        //     lea eax, [esp-#frameSize]
        //     call JIT_StackProbe
        //     mov esp, eax
        // Before 6.0
        //     test [esp-0x1000], eax
        //     test [esp-0x2000], eax
        //     sub esp, #frameSize
        public override int VirtMethodEspBasedFrame()
        {
            _2000 tmp = new _2000();
            return (int)tmp._0._0._0._0;
        }
    }

    class ProbeBeforeSubSp4 : AllocFrame
    {
        // Frame size is in range [0x2000, 0x3000) bytes and needs additional probing **after** allocation
        //
        // Since 6.0
        //     lea eax, [esp-#frameSize]
        //     call JIT_StackProbe
        //     mov esp, eax
        // Before 6.0
        //     test [esp-0x1000], eax
        //     test [esp-0x2000], eax
        //     sub esp, #frameSize
        //     test [esp], eax
        public override int VirtMethodEspBasedFrame()
        {
            _2e00 tmp = new _2e00();
            return (int)tmp._0._0._0._0;
        }
    }

    class ProbeBeforeSubSp5 : AllocFrame
    {
        // Frame size is in range [0x3000, 0x3000) bytes
        //
        // Since 5.0
        //     lea eax, [esp-#frameSize]
        //     call JIT_StackProbe
        //     mov esp, eax
        // Before 5.0
        //     xor eax, eax
        //     [nop]
        //   loop:
        //     test [esp + eax], eax
        //     sub eax, 0x1000
        //     cmp eax, -#frameSize
        //     jge loop
        //     sub esp, #frameSize
        public override int VirtMethodEspBasedFrame()
        {
            _3000 tmp = new _3000();
            return (int)tmp._0._0._0._0;
        }
    }

    public class Program
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        static void TestSkipAllocFrame(AllocFrame scenario)
        {
            scenario.VirtMethodEspBasedFrame();
        }

        [Fact]
        public static void TestEntryPoint()
        {
            TestSkipAllocFrame(new PushReg());
            TestSkipAllocFrame(new SubSp());
            TestSkipAllocFrame(new ProbeAfterSubSp());
            TestSkipAllocFrame(new ProbeBeforeSubSp1());
            TestSkipAllocFrame(new ProbeBeforeSubSp2());
            TestSkipAllocFrame(new ProbeBeforeSubSp3());
            TestSkipAllocFrame(new ProbeBeforeSubSp4());
            TestSkipAllocFrame(new ProbeBeforeSubSp5());
        }
    }
}
