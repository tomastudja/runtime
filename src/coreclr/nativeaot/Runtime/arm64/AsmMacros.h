;; Licensed to the .NET Foundation under one or more agreements.
;; The .NET Foundation licenses this file to you under the MIT license.

;; OS provided macros
#include <ksarm64.h>
;; generated by the build from AsmOffsets.cpp
#include "AsmOffsets.inc"

;;
;; CONSTANTS -- INTEGER
;;
TSF_Attached                    equ 0x01
TSF_SuppressGcStress            equ 0x08
TSF_DoNotTriggerGc              equ 0x10
TSF_SuppressGcStress__OR__TSF_DoNotTriggerGC equ 0x18

;; Bit position for the flags above, to be used with tbz/tbnz instructions
TSF_Attached_Bit                equ 0
TSF_SuppressGcStress_Bit        equ 3
TSF_DoNotTriggerGc_Bit          equ 4

;; GC type flags
GC_ALLOC_FINALIZE               equ 1
GC_ALLOC_ALIGN8_BIAS            equ 4
GC_ALLOC_ALIGN8                 equ 8

;; Note: these must match the defs in PInvokeTransitionFrameFlags defined in rhbinder.h
PTFF_SAVE_X19           equ 0x00000001
PTFF_SAVE_X20           equ 0x00000002
PTFF_SAVE_X21           equ 0x00000004
PTFF_SAVE_X22           equ 0x00000008
PTFF_SAVE_X23           equ 0x00000010
PTFF_SAVE_X24           equ 0x00000020
PTFF_SAVE_X25           equ 0x00000040
PTFF_SAVE_X26           equ 0x00000080
PTFF_SAVE_X27           equ 0x00000100
PTFF_SAVE_X28           equ 0x00000200
PTFF_SAVE_SP            equ 0x00000400
PTFF_SAVE_ALL_PRESERVED equ 0x000003FF  ;; NOTE: x19-x28
PTFF_SAVE_X0            equ 0x00000800
PTFF_SAVE_X1            equ 0x00001000
PTFF_SAVE_X2            equ 0x00002000
PTFF_SAVE_X3            equ 0x00004000
PTFF_SAVE_X4            equ 0x00008000
PTFF_SAVE_X5            equ 0x00010000
PTFF_SAVE_X6            equ 0x00020000
PTFF_SAVE_X7            equ 0x00040000
PTFF_SAVE_X8            equ 0x00080000
PTFF_SAVE_X9            equ 0x00100000
PTFF_SAVE_X10           equ 0x00200000
PTFF_SAVE_X11           equ 0x00400000
PTFF_SAVE_X12           equ 0x00800000
PTFF_SAVE_X13           equ 0x01000000
PTFF_SAVE_X14           equ 0x02000000
PTFF_SAVE_X15           equ 0x04000000
PTFF_SAVE_X16           equ 0x08000000
PTFF_SAVE_X17           equ 0x10000000
PTFF_SAVE_X18           equ 0x20000000
PTFF_SAVE_ALL_SCRATCH   equ 0x3FFFF800  ;; NOTE: X0-X18
PTFF_SAVE_FP            equ 0x40000000
PTFF_SAVE_LR            equ 0x80000000

;; NOTE: The following flags represent the upper 32 bits of the PInvokeTransitionFrameFlags.
;; Since the assembler doesn't support 64 bit constants in any way, we need to define just
;; the upper bits here
PTFF_X0_IS_GCREF_HI     equ 0x00000001 ;; iff PTFF_SAVE_X0 : set->x0 is Object, clear->x0 is scalar
PTFF_X0_IS_BYREF_HI     equ 0x00000002 ;; iff PTFF_SAVE_X0 : set->x0 is ByRef, clear->x0 is Object or scalar
PTFF_X1_IS_GCREF_HI     equ 0x00000004 ;; iff PTFF_SAVE_X1 : set->x1 is Object, clear->x1 is scalar
PTFF_X1_IS_BYREF_HI     equ 0x00000008 ;; iff PTFF_SAVE_X1 : set->x1 is ByRef, clear->x1 is Object or scalar
PTFF_THREAD_ABORT_HI    equ 0x00000010 ;; indicates that ThreadAbortException should be thrown when returning from the transition

;; Bit position for the flags above, to be used with tbz / tbnz instructions
PTFF_THREAD_ABORT_BIT   equ 36

;; These must match the TrapThreadsFlags enum
TrapThreadsFlags_None            equ 0
TrapThreadsFlags_AbortInProgress equ 1
TrapThreadsFlags_TrapThreads     equ 2

;; Bit position for the flags above, to be used with tbz / tbnz instructions
TrapThreadsFlags_AbortInProgress_Bit equ 0
TrapThreadsFlags_TrapThreads_Bit     equ 1

;; This must match HwExceptionCode.STATUS_REDHAWK_THREAD_ABORT
STATUS_REDHAWK_THREAD_ABORT      equ 0x43

;;
;; Rename fields of nested structs
;;
OFFSETOF__Thread__m_alloc_context__alloc_ptr        equ OFFSETOF__Thread__m_rgbAllocContextBuffer + OFFSETOF__gc_alloc_context__alloc_ptr
OFFSETOF__Thread__m_alloc_context__alloc_limit      equ OFFSETOF__Thread__m_rgbAllocContextBuffer + OFFSETOF__gc_alloc_context__alloc_limit

;;
;; IMPORTS
;;
    EXTERN RhpGcAlloc
    EXTERN RhExceptionHandling_FailedAllocation
    EXTERN RhDebugBreak
    EXTERN RhpWaitForGC2
    EXTERN RhpReversePInvokeAttachOrTrapThread2
    EXTERN RhThrowHwEx
    EXTERN RhThrowEx
    EXTERN RhRethrow

    EXTERN RhpTrapThreads
    EXTERN g_lowest_address
    EXTERN g_highest_address
    EXTERN g_ephemeral_low
    EXTERN g_ephemeral_high
    EXTERN g_card_table


;; -----------------------------------------------------------------------------
;; Macro used to assign an alternate name to a symbol containing characters normally disallowed in a symbol
;; name (e.g. C++ decorated names).
    MACRO
      SETALIAS   $name, $symbol
        GBLS    $name
$name   SETS    "|$symbol|"
    MEND

;;-----------------------------------------------------------------------------
;; Macro for loading a 64-bit constant by a minimal number of instructions
;; Since the asssembles doesn't support 64 bit arithmetics in expressions,
;; the value is passed in as lo, hi pair.
    MACRO
        MOVL64 $Reg, $ConstantLo, $ConstantHi

        LCLS MovInstr
MovInstr SETS "movz"

         IF ((($ConstantHi):SHR:16):AND:0xffff) != 0
         $MovInstr $Reg, #((($Constant):SHR:16):AND:0xffff), lsl #48
MovInstr SETS "movk"
         ENDIF

         IF (($ConstantHi):AND:0xffff) != 0
         $MovInstr $Reg, #(($ConstantHi):AND:0xffff), lsl #32
MovInstr SETS "movk"
         ENDIF

        IF ((($ConstantLo):SHR:16):AND:0xffff) != 0
        $MovInstr $Reg, #((($ConstantLo):SHR:16):AND:0xffff), lsl #16
MovInstr SETS "movk"
        ENDIF

        $MovInstr $Reg, #(($ConstantLo):AND:0xffff)
    MEND

;; -----------------------------------------------------------------------------
;;
;; Macro to export a pointer to an address inside a stub as a 64-bit variable
;;
    MACRO
        EXPORT_POINTER_TO_ADDRESS $Name
        LCLS CodeLbl
CodeLbl SETS "$Name":CC:"Lbl"
$CodeLbl
        AREA | .rdata | , ALIGN = 8, DATA, READONLY
$Name
        DCQ         $CodeLbl
        EXPORT      $Name
        TEXTAREA
        ROUT

    MEND

;; -----------------------------------------------------------------------------
;;
;; Macro for indicating an alternate entry point into a function.
;;

    MACRO
        LABELED_RETURN_ADDRESS $ReturnAddressName

        ; export the return address name, but do not perturb the code by forcing alignment
$ReturnAddressName
        EXPORT $ReturnAddressName

        ; flush any pending literal pool stuff
        ROUT

    MEND

;; -----------------------------------------------------------------------------
;;
;; Macro to get a pointer to the Thread* object for the currently executing thread
;;

__tls_array     equ 0x58    ;; offsetof(TEB, ThreadLocalStoragePointer)

    EXTERN _tls_index

    GBLS __SECTIONREL_tls_CurrentThread
__SECTIONREL_tls_CurrentThread SETS "SECTIONREL_tls_CurrentThread"

    MACRO
        INLINE_GETTHREAD $destReg, $trashReg

        ;; The following macro variables are just some assembler magic to get the name of the 32-bit version
        ;; of $trashReg. It does it by string manipulation. Replaces something like x3 with w3.
        LCLS TrashRegister32Bit
TrashRegister32Bit SETS "$trashReg"
TrashRegister32Bit SETS "w":CC:("$TrashRegister32Bit":RIGHT:((:LEN:TrashRegister32Bit) - 1))

        ldr         $trashReg, =_tls_index
        ldr         $TrashRegister32Bit, [$trashReg]
        ldr         $destReg, [xpr, #__tls_array]
        ldr         $destReg, [$destReg, $trashReg lsl #3]
        ldr         $trashReg, =$__SECTIONREL_tls_CurrentThread
        ldr         $trashReg, [$trashReg]
        add         $destReg, $destReg, $trashReg
    MEND

    ;; INLINE_GETTHREAD_CONSTANT_POOL macro has to be used after the last function in the .asm file that used
    ;; INLINE_GETTHREAD. Optionally, it can be also used after any function that used INLINE_GETTHREAD
    ;; to improve density, or to reduce distance betweeen the constant pool and its use.
    MACRO
        INLINE_GETTHREAD_CONSTANT_POOL
        EXTERN tls_CurrentThread

    ;; Section relocs are 32 bits. Using an extra DCD initialized to zero for 8-byte alignment.
$__SECTIONREL_tls_CurrentThread
        DCD tls_CurrentThread
        RELOC 8, tls_CurrentThread      ;; SECREL
        DCD 0

__SECTIONREL_tls_CurrentThread SETS "$__SECTIONREL_tls_CurrentThread":CC:"_"

    MEND

    MACRO
        INLINE_THREAD_UNHIJACK $threadReg, $trashReg1, $trashReg2
        ;;
        ;; Thread::Unhijack()
        ;;
        ldr         $trashReg1, [$threadReg, #OFFSETOF__Thread__m_pvHijackedReturnAddress]
        cbz         $trashReg1, %ft0

        ldr         $trashReg2, [$threadReg, #OFFSETOF__Thread__m_ppvHijackedReturnAddressLocation]
        str         $trashReg1, [$trashReg2]
        str         xzr, [$threadReg, #OFFSETOF__Thread__m_ppvHijackedReturnAddressLocation]
        str         xzr, [$threadReg, #OFFSETOF__Thread__m_pvHijackedReturnAddress]
0
    MEND

;; ---------------------------------------------------------------------------- -
;;
;; Macro to add a memory barrier. Equal to __sync_synchronize().
;;

    MACRO
        ArmInterlockedOperationBarrier

        dmb         ish
    MEND

;; -----------------------------------------------------------------------------
;;
;; Macro used from unmanaged helpers called from managed code where the helper does not transition immediately
;; into pre-emptive mode but may cause a GC and thus requires the stack is crawlable. This is typically the
;; case for helpers that meddle in GC state (e.g. allocation helpers) where the code must remain in
;; cooperative mode since it handles object references and internal GC state directly but a garbage collection
;; may be inevitable. In these cases we need to be able to transition to pre-meptive mode deep within the
;; unmanaged code but still be able to initialize the stack iterator at the first stack frame which may hold
;; interesting GC references. In all our helper cases this corresponds to the most recent managed frame (e.g.
;; the helper's caller).
;;
;; This macro builds a frame describing the current state of managed code.
;;
;; INVARIANTS
;; - The macro assumes it defines the method prolog, it should typically be the first code in a method and
;;   certainly appear before any attempt to alter the stack pointer.
;; - This macro uses trashReg (after its initial value has been saved in the frame) and upon exit trashReg
;;   will contain the address of transition frame.
;;

DEFAULT_FRAME_SAVE_FLAGS equ PTFF_SAVE_ALL_PRESERVED + PTFF_SAVE_SP

    MACRO
        PUSH_COOP_PINVOKE_FRAME $trashReg

        PROLOG_SAVE_REG_PAIR   fp, lr, #-0x80!      ;; Push down stack pointer and store FP and LR

        ;; 0x10 bytes reserved for Thread* and flags

        ;; Save callee saved registers
        PROLOG_SAVE_REG_PAIR   x19, x20, #0x20
        PROLOG_SAVE_REG_PAIR   x21, x22, #0x30
        PROLOG_SAVE_REG_PAIR   x23, x24, #0x40
        PROLOG_SAVE_REG_PAIR   x25, x26, #0x50
        PROLOG_SAVE_REG_PAIR   x27, x28, #0x60

        ;; Save the value of SP before stack allocation to the last slot in the frame (slot #15)
        add                    $trashReg, sp, #0x80
        str                    $trashReg, [sp, #0x70]

        ;; Record the bitmask of saved registers in the frame (slot #3)
        mov                    $trashReg, #DEFAULT_FRAME_SAVE_FLAGS
        str                    $trashReg, [sp, #0x18]

        mov $trashReg, sp
    MEND

;; Pop the frame and restore register state preserved by PUSH_COOP_PINVOKE_FRAME
    MACRO
        POP_COOP_PINVOKE_FRAME

        EPILOG_RESTORE_REG_PAIR   x19, x20, #0x20
        EPILOG_RESTORE_REG_PAIR   x21, x22, #0x30
        EPILOG_RESTORE_REG_PAIR   x23, x24, #0x40
        EPILOG_RESTORE_REG_PAIR   x25, x26, #0x50
        EPILOG_RESTORE_REG_PAIR   x27, x28, #0x60
        EPILOG_RESTORE_REG_PAIR   fp, lr, #0x80!
    MEND


#ifdef FEATURE_GC_STRESS
    SETALIAS THREAD__HIJACKFORGCSTRESS, ?HijackForGcStress@Thread@@SAXPEAUPAL_LIMITED_CONTEXT@@@Z
    SETALIAS REDHAWKGCINTERFACE__STRESSGC, ?StressGc@RedhawkGCInterface@@SAXXZ

    EXTERN $REDHAWKGCINTERFACE__STRESSGC
    EXTERN $THREAD__HIJACKFORGCSTRESS
#endif ;; FEATURE_GC_STRESS
