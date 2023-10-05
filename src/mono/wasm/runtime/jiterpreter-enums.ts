export const enum OpcodeInfoType {
    Name = 0,
    Length,
    Sregs,
    Dregs,
    OpArgType
}

// keep in sync with jiterpreter.c, see mono_jiterp_get_counter_address
export const enum JiterpCounter {
    TraceCandidates = 0,
    TracesCompiled,
    EntryWrappersCompiled,
    JitCallsCompiled,
    DirectJitCallsCompiled,
    Failures,
    BytesGenerated,
    NullChecksEliminated,
    NullChecksFused,
    BackBranchesEmitted,
    BackBranchesNotEmitted,
    ElapsedGenerationMs,
    ElapsedCompilationMs,
}

// keep in sync with jiterpreter.c, see mono_jiterp_get_member_offset
export const enum JiterpMember {
    VtableInitialized = 0,
    ArrayData,
    StringLength,
    StringData,
    Imethod,
    DataItems,
    Rmethod,
    SpanLength,
    SpanData,
    ArrayLength,
    BackwardBranchOffsets,
    BackwardBranchOffsetsCount,
    ClauseDataOffsets,
    ParamsCount,
    VTable,
    VTableKlass,
    ClassRank,
    ClassElementClass,
    BoxedValueData,
}

// keep in sync with jiterpreter.c, see mono_jiterp_write_number_unaligned
export const enum JiterpNumberMode {
    U32 = 0,
    I32,
    F32,
    F64
}

// keep in sync with jiterpreter.h
export const enum JiterpreterTable {
    Trace = 0,
    JitCall = 1,
    InterpEntryStatic0 = 2,
    InterpEntryStatic1,
    InterpEntryStatic2,
    InterpEntryStatic3,
    InterpEntryStatic4,
    InterpEntryStatic5,
    InterpEntryStatic6,
    InterpEntryStatic7,
    InterpEntryStatic8,
    InterpEntryStaticRet0,
    InterpEntryStaticRet1,
    InterpEntryStaticRet2,
    InterpEntryStaticRet3,
    InterpEntryStaticRet4,
    InterpEntryStaticRet5,
    InterpEntryStaticRet6,
    InterpEntryStaticRet7,
    InterpEntryStaticRet8,
    InterpEntryInstance0,
    InterpEntryInstance1,
    InterpEntryInstance2,
    InterpEntryInstance3,
    InterpEntryInstance4,
    InterpEntryInstance5,
    InterpEntryInstance6,
    InterpEntryInstance7,
    InterpEntryInstance8,
    InterpEntryInstanceRet0,
    InterpEntryInstanceRet1,
    InterpEntryInstanceRet2,
    InterpEntryInstanceRet3,
    InterpEntryInstanceRet4,
    InterpEntryInstanceRet5,
    InterpEntryInstanceRet6,
    InterpEntryInstanceRet7,
    InterpEntryInstanceRet8,
    LAST = InterpEntryInstanceRet8,
}

export const enum BailoutReason {
    Unknown,
    InterpreterTiering,
    NullCheck,
    VtableNotInitialized,
    Branch,
    BackwardBranch,
    ConditionalBranch,
    ConditionalBackwardBranch,
    ComplexBranch,
    ArrayLoadFailed,
    ArrayStoreFailed,
    StringOperationFailed,
    DivideByZero,
    Overflow,
    Return,
    Call,
    Throw,
    AllocFailed,
    SpanOperationFailed,
    CastFailed,
    SafepointBranchTaken,
    UnboxFailed,
    CallDelegate,
    Debugging,
    Icall,
    UnexpectedRetIp,
    LeaveCheck,
}

export const BailoutReasonNames = [
    "Unknown",
    "InterpreterTiering",
    "NullCheck",
    "VtableNotInitialized",
    "Branch",
    "BackwardBranch",
    "ConditionalBranch",
    "ConditionalBackwardBranch",
    "ComplexBranch",
    "ArrayLoadFailed",
    "ArrayStoreFailed",
    "StringOperationFailed",
    "DivideByZero",
    "Overflow",
    "Return",
    "Call",
    "Throw",
    "AllocFailed",
    "SpanOperationFailed",
    "CastFailed",
    "SafepointBranchTaken",
    "UnboxFailed",
    "CallDelegate",
    "Debugging",
    "Icall",
    "UnexpectedRetIp",
    "LeaveCheck",
];

export const enum JitQueue {
    JitCall = 0,
    InterpEntry = 1
}
