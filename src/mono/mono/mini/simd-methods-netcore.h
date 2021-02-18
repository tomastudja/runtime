METHOD2(".ctor", ctor)
METHOD(CopyTo)
METHOD(Equals)
METHOD(GreaterThan)
METHOD(GreaterThanOrEqual)
METHOD(LessThan)
METHOD(LessThanOrEqual)
METHOD(Min)
METHOD(Max)
METHOD(MinScalar)
METHOD(MaxScalar)
METHOD(PopCount)
METHOD(LeadingZeroCount)
METHOD(get_Count)
METHOD(get_IsHardwareAccelerated)
METHOD(get_IsSupported)
METHOD(get_AllBitsSet)
METHOD(get_Item)
METHOD(get_One)
METHOD(get_Zero)
METHOD(op_Addition)
METHOD(op_BitwiseAnd)
METHOD(op_BitwiseOr)
METHOD(op_Division)
METHOD(op_Equality)
METHOD(op_ExclusiveOr)
METHOD(op_Explicit)
METHOD(op_Inequality)
METHOD(op_Multiply)
METHOD(op_Subtraction)
// Vector
METHOD(ConvertToInt32)
METHOD(ConvertToInt32WithTruncation)
METHOD(ConvertToUInt32)
METHOD(ConvertToInt64)
METHOD(ConvertToInt64WithTruncation)
METHOD(ConvertToUInt64)
METHOD(ConvertToSingle)
METHOD(ConvertToDouble)
METHOD(Narrow)
METHOD(Widen)
// Vector128
METHOD(AsByte)
METHOD(AsDouble)
METHOD(AsInt16)
METHOD(AsInt32)
METHOD(AsInt64)
METHOD(AsSByte)
METHOD(AsSingle)
METHOD(AsUInt16)
METHOD(AsUInt32)
METHOD(AsUInt64)
METHOD(Create)
METHOD(CreateScalarUnsafe)
// Bmi1
METHOD(AndNot)
METHOD(BitFieldExtract)
METHOD(ExtractLowestSetBit)
METHOD(GetMaskUpToLowestSetBit)
METHOD(ResetLowestSetBit)
METHOD(TrailingZeroCount)
// Bmi2
METHOD(ZeroHighBits)
METHOD(MultiplyNoFlags)
METHOD(ParallelBitDeposit)
METHOD(ParallelBitExtract)
// Sse
METHOD(Add)
METHOD(CompareGreaterThanOrEqual)
METHOD(CompareLessThanOrEqual)
METHOD(CompareNotEqual)
METHOD(CompareNotGreaterThan)
METHOD(CompareNotGreaterThanOrEqual)
METHOD(CompareNotLessThan)
METHOD(CompareNotLessThanOrEqual)
METHOD(CompareScalarGreaterThan)
METHOD(CompareScalarGreaterThanOrEqual)
METHOD(CompareScalarLessThan)
METHOD(CompareScalarLessThanOrEqual)
METHOD(CompareScalarNotEqual)
METHOD(CompareScalarNotGreaterThan)
METHOD(CompareScalarNotGreaterThanOrEqual)
METHOD(CompareScalarNotLessThan)
METHOD(CompareScalarNotLessThanOrEqual)
METHOD(CompareScalarOrderedEqual)
METHOD(CompareScalarOrderedGreaterThan)
METHOD(CompareScalarOrderedGreaterThanOrEqual)
METHOD(CompareScalarOrderedLessThan)
METHOD(CompareScalarOrderedLessThanOrEqual)
METHOD(CompareScalarOrderedNotEqual)
METHOD(CompareScalarUnorderedEqual)
METHOD(CompareScalarUnorderedGreaterThan)
METHOD(CompareScalarUnorderedGreaterThanOrEqual)
METHOD(CompareScalarUnorderedLessThan)
METHOD(CompareScalarUnorderedLessThanOrEqual)
METHOD(CompareScalarUnorderedNotEqual)
METHOD(CompareOrdered)
METHOD(CompareUnordered)
METHOD(CompareScalarOrdered)
METHOD(CompareScalarUnordered)
METHOD(ConvertScalarToVector128Single)
METHOD(Divide)
METHOD(DivideScalar)
METHOD(Store)
METHOD(StoreFence)
METHOD(StoreHigh)
METHOD(StoreLow)
METHOD(Subtract)
METHOD(SubtractScalar)
METHOD(CompareEqual)
METHOD(Extract)
METHOD(LoadHigh)
METHOD(LoadLow)
METHOD(LoadVector128)
METHOD(LoadScalarVector128)
METHOD(MoveHighToLow)
METHOD(MoveLowToHigh)
METHOD(MoveMask)
METHOD(MoveScalar)
METHOD(Multiply)
METHOD(MultiplyAddAdjacent)
METHOD(MultiplyScalar)
METHOD(Shuffle)
METHOD(UnpackHigh)
METHOD(UnpackLow)
METHOD(Prefetch0)
METHOD(Prefetch1)
METHOD(Prefetch2)
METHOD(PrefetchNonTemporal)
METHOD(Reciprocal)
METHOD(ReciprocalScalar)
METHOD(ReciprocalSqrt)
METHOD(ReciprocalSqrtScalar)
METHOD(Sqrt)
METHOD(SqrtScalar)
// Sse2
METHOD(AddSaturate)
METHOD(AddScalar)
METHOD(And)
METHOD(Average)
METHOD(Or)
METHOD(LoadAlignedVector128)
METHOD(Xor)
METHOD(CompareGreaterThan)
METHOD(CompareScalarEqual)
METHOD(ConvertScalarToVector128Double)
METHOD(ConvertScalarToVector128Int32)
METHOD(ConvertScalarToVector128Int64)
METHOD(ConvertScalarToVector128UInt32)
METHOD(ConvertScalarToVector128UInt64)
METHOD(ConvertToVector128Double)
METHOD(ConvertToVector128Int32)
METHOD(ConvertToVector128Int32WithTruncation)
METHOD(ConvertToVector128Single)
METHOD(MaskMove)
METHOD(MultiplyHigh)
METHOD(MultiplyLow)
METHOD(PackSignedSaturate)
METHOD(PackUnsignedSaturate)
METHOD(ShuffleHigh)
METHOD(ShuffleLow)
METHOD(SubtractSaturate)
METHOD(SumAbsoluteDifferences)
METHOD(StoreScalar)
METHOD(StoreAligned)
METHOD(StoreAlignedNonTemporal)
METHOD(StoreNonTemporal)
METHOD(ShiftLeftLogical)
METHOD(ShiftLeftLogical128BitLane)
METHOD(ShiftRightArithmetic)
METHOD(ShiftRightLogical)
METHOD(ShiftRightLogical128BitLane)
METHOD(CompareLessThan)
METHOD(LoadFence)
METHOD(MemoryFence)
// Sse3
METHOD(HorizontalAdd)
METHOD(HorizontalSubtract)
METHOD(AddSubtract)
METHOD(LoadAndDuplicateToVector128)
METHOD(LoadDquVector128)
METHOD(MoveAndDuplicate)
METHOD(MoveHighAndDuplicate)
METHOD(MoveLowAndDuplicate)
// Ssse3
METHOD(Abs) // Also used by ARM64
METHOD(AlignRight)
METHOD(HorizontalAddSaturate)
METHOD(HorizontalSubtractSaturate)
METHOD(MultiplyHighRoundScale)
METHOD(Sign)
// Sse41
METHOD(Blend)
METHOD(BlendVariable)
METHOD(Ceiling)
METHOD(CeilingScalar)
METHOD(ConvertToVector128Int16)
METHOD(ConvertToVector128Int64)
METHOD(Floor)
METHOD(FloorScalar)
METHOD(Insert)
METHOD(LoadAlignedVector128NonTemporal)
METHOD(RoundCurrentDirectionScalar)
METHOD(RoundToNearestInteger)
METHOD(RoundToNearestIntegerScalar)
METHOD(RoundToNegativeInfinity)
METHOD(RoundToNegativeInfinityScalar)
METHOD(RoundToPositiveInfinity)
METHOD(RoundToPositiveInfinityScalar)
METHOD(RoundToZero)
METHOD(RoundToZeroScalar)
METHOD(RoundCurrentDirection)
METHOD(MinHorizontal)
METHOD(TestC)
METHOD(TestNotZAndNotC)
METHOD(TestZ)
METHOD(DotProduct)
METHOD(MultipleSumAbsoluteDifferences)
// Sse42
METHOD(Crc32)
// Aes
METHOD(Decrypt)
METHOD(DecryptLast)
METHOD(Encrypt)
METHOD(EncryptLast)
METHOD(InverseMixColumns)
METHOD(KeygenAssist)
// Pclmulqdq
METHOD(CarrylessMultiply)
// ArmBase
METHOD(LeadingSignCount)
METHOD(ReverseElementBits)
// Crc32
METHOD(ComputeCrc32)
METHOD(ComputeCrc32C)
// X86Base
METHOD(BitScanForward)
METHOD(BitScanReverse)
// Crypto
METHOD(FixedRotate)
METHOD(HashUpdateChoose)
METHOD(HashUpdateMajority)
METHOD(HashUpdateParity)
METHOD(HashUpdate1)
METHOD(HashUpdate2)
METHOD(ScheduleUpdate0)
METHOD(ScheduleUpdate1)
METHOD(MixColumns)
//AdvSimd
METHOD(AbsSaturate)
METHOD(AbsScalar)
METHOD(AbsoluteCompareGreaterThan)
METHOD(AbsoluteCompareGreaterThanOrEqual)
METHOD(AbsoluteCompareLessThan)
METHOD(AbsoluteCompareLessThanOrEqual)
