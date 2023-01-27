// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

//
// This file is used by AsmOffsets.h to validate that our
// assembly-code offsets always match their C++ counterparts.
//
// NOTE: the offsets MUST be in hex notation WITHOUT the 0x prefix

PLAT_ASM_SIZEOF(13c, ExInfo)
PLAT_ASM_OFFSET(0, ExInfo, m_pPrevExInfo)
PLAT_ASM_OFFSET(4, ExInfo, m_pExContext)
PLAT_ASM_OFFSET(8, ExInfo, m_exception)
PLAT_ASM_OFFSET(0c, ExInfo, m_kind)
PLAT_ASM_OFFSET(0d, ExInfo, m_passNumber)
PLAT_ASM_OFFSET(10, ExInfo, m_idxCurClause)
PLAT_ASM_OFFSET(18, ExInfo, m_frameIter)
PLAT_ASM_OFFSET(134, ExInfo, m_notifyDebuggerSP)

PLAT_ASM_OFFSET(4, PInvokeTransitionFrame, m_RIP)
PLAT_ASM_OFFSET(8, PInvokeTransitionFrame, m_FramePointer)
PLAT_ASM_OFFSET(0c, PInvokeTransitionFrame, m_pThread)
PLAT_ASM_OFFSET(10, PInvokeTransitionFrame, m_Flags)
PLAT_ASM_OFFSET(14, PInvokeTransitionFrame, m_PreservedRegs)

PLAT_ASM_SIZEOF(11c, StackFrameIterator)
PLAT_ASM_OFFSET(08, StackFrameIterator, m_FramePointer)
PLAT_ASM_OFFSET(0c, StackFrameIterator, m_ControlPC)
PLAT_ASM_OFFSET(10, StackFrameIterator, m_RegDisplay)
PLAT_ASM_OFFSET(114, StackFrameIterator, m_OriginalControlPC)
PLAT_ASM_OFFSET(118, StackFrameIterator, m_pPreviousTransitionFrame)

PLAT_ASM_SIZEOF(70, PAL_LIMITED_CONTEXT)
PLAT_ASM_OFFSET(24, PAL_LIMITED_CONTEXT, IP)

PLAT_ASM_OFFSET(0, PAL_LIMITED_CONTEXT, R0)
PLAT_ASM_OFFSET(4, PAL_LIMITED_CONTEXT, R4)
PLAT_ASM_OFFSET(8, PAL_LIMITED_CONTEXT, R5)
PLAT_ASM_OFFSET(0c, PAL_LIMITED_CONTEXT, R6)
PLAT_ASM_OFFSET(10, PAL_LIMITED_CONTEXT, R7)
PLAT_ASM_OFFSET(14, PAL_LIMITED_CONTEXT, R8)
PLAT_ASM_OFFSET(18, PAL_LIMITED_CONTEXT, R9)
PLAT_ASM_OFFSET(1c, PAL_LIMITED_CONTEXT, R10)
PLAT_ASM_OFFSET(20, PAL_LIMITED_CONTEXT, R11)
PLAT_ASM_OFFSET(28, PAL_LIMITED_CONTEXT, SP)
PLAT_ASM_OFFSET(2c, PAL_LIMITED_CONTEXT, LR)

PLAT_ASM_SIZEOF(88, REGDISPLAY)
PLAT_ASM_OFFSET(38, REGDISPLAY, SP)

PLAT_ASM_OFFSET(10, REGDISPLAY, pR4)
PLAT_ASM_OFFSET(14, REGDISPLAY, pR5)
PLAT_ASM_OFFSET(18, REGDISPLAY, pR6)
PLAT_ASM_OFFSET(1c, REGDISPLAY, pR7)
PLAT_ASM_OFFSET(20, REGDISPLAY, pR8)
PLAT_ASM_OFFSET(24, REGDISPLAY, pR9)
PLAT_ASM_OFFSET(28, REGDISPLAY, pR10)
PLAT_ASM_OFFSET(2c, REGDISPLAY, pR11)
PLAT_ASM_OFFSET(48, REGDISPLAY, D)
