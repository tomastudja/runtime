// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;

namespace JIT.HardwareIntrinsics.Arm
{
    public static partial class Program
    {
        static Program()
        {
            TestList = new Dictionary<string, Action>() {
                ["Abs.Vector128.Double"] = Abs_Vector128_Double,
                ["Abs.Vector128.Int64"] = Abs_Vector128_Int64,
                ["AbsScalar.Vector64.Int64"] = AbsScalar_Vector64_Int64,
                ["AbsoluteCompareGreaterThan.Vector128.Double"] = AbsoluteCompareGreaterThan_Vector128_Double,
                ["AbsoluteCompareGreaterThanScalar.Vector64.Double"] = AbsoluteCompareGreaterThanScalar_Vector64_Double,
                ["AbsoluteCompareGreaterThanScalar.Vector64.Single"] = AbsoluteCompareGreaterThanScalar_Vector64_Single,
                ["AbsoluteCompareGreaterThanOrEqual.Vector128.Double"] = AbsoluteCompareGreaterThanOrEqual_Vector128_Double,
                ["AbsoluteCompareGreaterThanOrEqualScalar.Vector64.Double"] = AbsoluteCompareGreaterThanOrEqualScalar_Vector64_Double,
                ["AbsoluteCompareGreaterThanOrEqualScalar.Vector64.Single"] = AbsoluteCompareGreaterThanOrEqualScalar_Vector64_Single,
                ["AbsoluteCompareLessThan.Vector128.Double"] = AbsoluteCompareLessThan_Vector128_Double,
                ["AbsoluteCompareLessThanScalar.Vector64.Double"] = AbsoluteCompareLessThanScalar_Vector64_Double,
                ["AbsoluteCompareLessThanScalar.Vector64.Single"] = AbsoluteCompareLessThanScalar_Vector64_Single,
                ["AbsoluteCompareLessThanOrEqual.Vector128.Double"] = AbsoluteCompareLessThanOrEqual_Vector128_Double,
                ["AbsoluteCompareLessThanOrEqualScalar.Vector64.Double"] = AbsoluteCompareLessThanOrEqualScalar_Vector64_Double,
                ["AbsoluteCompareLessThanOrEqualScalar.Vector64.Single"] = AbsoluteCompareLessThanOrEqualScalar_Vector64_Single,
                ["AbsoluteDifference.Vector128.Double"] = AbsoluteDifference_Vector128_Double,
                ["AbsoluteDifferenceScalar.Vector64.Double"] = AbsoluteDifferenceScalar_Vector64_Double,
                ["AbsoluteDifferenceScalar.Vector64.Single"] = AbsoluteDifferenceScalar_Vector64_Single,
                ["Add.Vector128.Double"] = Add_Vector128_Double,
                ["AddAcross.Vector64.Byte"] = AddAcross_Vector64_Byte,
                ["AddAcross.Vector64.Int16"] = AddAcross_Vector64_Int16,
                ["AddAcross.Vector64.SByte"] = AddAcross_Vector64_SByte,
                ["AddAcross.Vector64.UInt16"] = AddAcross_Vector64_UInt16,
                ["AddAcross.Vector128.Byte"] = AddAcross_Vector128_Byte,
                ["AddAcross.Vector128.Int16"] = AddAcross_Vector128_Int16,
                ["AddAcross.Vector128.Int32"] = AddAcross_Vector128_Int32,
                ["AddAcross.Vector128.SByte"] = AddAcross_Vector128_SByte,
                ["AddAcross.Vector128.UInt16"] = AddAcross_Vector128_UInt16,
                ["AddAcross.Vector128.UInt32"] = AddAcross_Vector128_UInt32,
                ["AddPairwise.Vector128.Byte"] = AddPairwise_Vector128_Byte,
                ["AddPairwise.Vector128.Double"] = AddPairwise_Vector128_Double,
                ["AddPairwise.Vector128.Int16"] = AddPairwise_Vector128_Int16,
                ["AddPairwise.Vector128.Int32"] = AddPairwise_Vector128_Int32,
                ["AddPairwise.Vector128.Int64"] = AddPairwise_Vector128_Int64,
                ["AddPairwise.Vector128.SByte"] = AddPairwise_Vector128_SByte,
                ["AddPairwise.Vector128.Single"] = AddPairwise_Vector128_Single,
                ["AddPairwise.Vector128.UInt16"] = AddPairwise_Vector128_UInt16,
                ["AddPairwise.Vector128.UInt32"] = AddPairwise_Vector128_UInt32,
                ["AddPairwise.Vector128.UInt64"] = AddPairwise_Vector128_UInt64,
                ["AddPairwiseScalar.Vector64.Single"] = AddPairwiseScalar_Vector64_Single,
                ["AddPairwiseScalar.Vector128.Double"] = AddPairwiseScalar_Vector128_Double,
                ["AddPairwiseScalar.Vector128.Int64"] = AddPairwiseScalar_Vector128_Int64,
                ["AddPairwiseScalar.Vector128.UInt64"] = AddPairwiseScalar_Vector128_UInt64,
                ["AddSaturateScalar.Vector64.Byte"] = AddSaturateScalar_Vector64_Byte,
                ["AddSaturateScalar.Vector64.Int16"] = AddSaturateScalar_Vector64_Int16,
                ["AddSaturateScalar.Vector64.Int32"] = AddSaturateScalar_Vector64_Int32,
                ["AddSaturateScalar.Vector64.SByte"] = AddSaturateScalar_Vector64_SByte,
                ["AddSaturateScalar.Vector64.UInt16"] = AddSaturateScalar_Vector64_UInt16,
                ["AddSaturateScalar.Vector64.UInt32"] = AddSaturateScalar_Vector64_UInt32,
                ["CompareEqual.Vector128.Double"] = CompareEqual_Vector128_Double,
                ["CompareEqual.Vector128.Int64"] = CompareEqual_Vector128_Int64,
                ["CompareEqual.Vector128.UInt64"] = CompareEqual_Vector128_UInt64,
                ["CompareEqualScalar.Vector64.Double"] = CompareEqualScalar_Vector64_Double,
                ["CompareEqualScalar.Vector64.Int64"] = CompareEqualScalar_Vector64_Int64,
                ["CompareEqualScalar.Vector64.Single"] = CompareEqualScalar_Vector64_Single,
                ["CompareEqualScalar.Vector64.UInt64"] = CompareEqualScalar_Vector64_UInt64,
                ["CompareGreaterThan.Vector128.Double"] = CompareGreaterThan_Vector128_Double,
                ["CompareGreaterThan.Vector128.Int64"] = CompareGreaterThan_Vector128_Int64,
                ["CompareGreaterThan.Vector128.UInt64"] = CompareGreaterThan_Vector128_UInt64,
                ["CompareGreaterThanScalar.Vector64.Double"] = CompareGreaterThanScalar_Vector64_Double,
                ["CompareGreaterThanScalar.Vector64.Int64"] = CompareGreaterThanScalar_Vector64_Int64,
                ["CompareGreaterThanScalar.Vector64.Single"] = CompareGreaterThanScalar_Vector64_Single,
                ["CompareGreaterThanScalar.Vector64.UInt64"] = CompareGreaterThanScalar_Vector64_UInt64,
                ["CompareGreaterThanOrEqual.Vector128.Double"] = CompareGreaterThanOrEqual_Vector128_Double,
                ["CompareGreaterThanOrEqual.Vector128.Int64"] = CompareGreaterThanOrEqual_Vector128_Int64,
                ["CompareGreaterThanOrEqual.Vector128.UInt64"] = CompareGreaterThanOrEqual_Vector128_UInt64,
                ["CompareGreaterThanOrEqualScalar.Vector64.Double"] = CompareGreaterThanOrEqualScalar_Vector64_Double,
                ["CompareGreaterThanOrEqualScalar.Vector64.Int64"] = CompareGreaterThanOrEqualScalar_Vector64_Int64,
                ["CompareGreaterThanOrEqualScalar.Vector64.Single"] = CompareGreaterThanOrEqualScalar_Vector64_Single,
                ["CompareGreaterThanOrEqualScalar.Vector64.UInt64"] = CompareGreaterThanOrEqualScalar_Vector64_UInt64,
                ["CompareLessThan.Vector128.Double"] = CompareLessThan_Vector128_Double,
                ["CompareLessThan.Vector128.Int64"] = CompareLessThan_Vector128_Int64,
                ["CompareLessThan.Vector128.UInt64"] = CompareLessThan_Vector128_UInt64,
                ["CompareLessThanScalar.Vector64.Double"] = CompareLessThanScalar_Vector64_Double,
                ["CompareLessThanScalar.Vector64.Int64"] = CompareLessThanScalar_Vector64_Int64,
                ["CompareLessThanScalar.Vector64.Single"] = CompareLessThanScalar_Vector64_Single,
                ["CompareLessThanScalar.Vector64.UInt64"] = CompareLessThanScalar_Vector64_UInt64,
                ["CompareLessThanOrEqual.Vector128.Double"] = CompareLessThanOrEqual_Vector128_Double,
                ["CompareLessThanOrEqual.Vector128.Int64"] = CompareLessThanOrEqual_Vector128_Int64,
                ["CompareLessThanOrEqual.Vector128.UInt64"] = CompareLessThanOrEqual_Vector128_UInt64,
                ["CompareLessThanOrEqualScalar.Vector64.Double"] = CompareLessThanOrEqualScalar_Vector64_Double,
                ["CompareLessThanOrEqualScalar.Vector64.Int64"] = CompareLessThanOrEqualScalar_Vector64_Int64,
                ["CompareLessThanOrEqualScalar.Vector64.Single"] = CompareLessThanOrEqualScalar_Vector64_Single,
                ["CompareLessThanOrEqualScalar.Vector64.UInt64"] = CompareLessThanOrEqualScalar_Vector64_UInt64,
                ["CompareTest.Vector128.Double"] = CompareTest_Vector128_Double,
                ["CompareTest.Vector128.Int64"] = CompareTest_Vector128_Int64,
                ["CompareTest.Vector128.UInt64"] = CompareTest_Vector128_UInt64,
                ["CompareTestScalar.Vector64.Double"] = CompareTestScalar_Vector64_Double,
                ["CompareTestScalar.Vector64.Int64"] = CompareTestScalar_Vector64_Int64,
                ["CompareTestScalar.Vector64.UInt64"] = CompareTestScalar_Vector64_UInt64,
                ["Divide.Vector64.Single"] = Divide_Vector64_Single,
                ["Divide.Vector128.Double"] = Divide_Vector128_Double,
                ["Divide.Vector128.Single"] = Divide_Vector128_Single,
                ["FusedMultiplyAdd.Vector128.Double"] = FusedMultiplyAdd_Vector128_Double,
                ["FusedMultiplySubtract.Vector128.Double"] = FusedMultiplySubtract_Vector128_Double,
                ["Max.Vector128.Double"] = Max_Vector128_Double,
                ["MaxAcross.Vector64.Byte"] = MaxAcross_Vector64_Byte,
                ["MaxAcross.Vector64.Int16"] = MaxAcross_Vector64_Int16,
                ["MaxAcross.Vector64.SByte"] = MaxAcross_Vector64_SByte,
                ["MaxAcross.Vector64.UInt16"] = MaxAcross_Vector64_UInt16,
                ["MaxAcross.Vector128.Byte"] = MaxAcross_Vector128_Byte,
                ["MaxAcross.Vector128.Int16"] = MaxAcross_Vector128_Int16,
                ["MaxAcross.Vector128.Int32"] = MaxAcross_Vector128_Int32,
                ["MaxAcross.Vector128.SByte"] = MaxAcross_Vector128_SByte,
                ["MaxAcross.Vector128.Single"] = MaxAcross_Vector128_Single,
                ["MaxAcross.Vector128.UInt16"] = MaxAcross_Vector128_UInt16,
                ["MaxAcross.Vector128.UInt32"] = MaxAcross_Vector128_UInt32,
                ["MaxNumber.Vector128.Double"] = MaxNumber_Vector128_Double,
                ["MaxNumberAcross.Vector128.Single"] = MaxNumberAcross_Vector128_Single,
                ["MaxNumberPairwise.Vector64.Single"] = MaxNumberPairwise_Vector64_Single,
                ["MaxNumberPairwise.Vector128.Double"] = MaxNumberPairwise_Vector128_Double,
                ["MaxNumberPairwise.Vector128.Single"] = MaxNumberPairwise_Vector128_Single,
                ["MaxNumberPairwiseScalar.Vector64.Single"] = MaxNumberPairwiseScalar_Vector64_Single,
                ["MaxNumberPairwiseScalar.Vector128.Double"] = MaxNumberPairwiseScalar_Vector128_Double,
                ["MaxPairwise.Vector128.Byte"] = MaxPairwise_Vector128_Byte,
                ["MaxPairwise.Vector128.Double"] = MaxPairwise_Vector128_Double,
                ["MaxPairwise.Vector128.Int16"] = MaxPairwise_Vector128_Int16,
                ["MaxPairwise.Vector128.Int32"] = MaxPairwise_Vector128_Int32,
                ["MaxPairwise.Vector128.SByte"] = MaxPairwise_Vector128_SByte,
                ["MaxPairwise.Vector128.Single"] = MaxPairwise_Vector128_Single,
                ["MaxPairwise.Vector128.UInt16"] = MaxPairwise_Vector128_UInt16,
                ["MaxPairwise.Vector128.UInt32"] = MaxPairwise_Vector128_UInt32,
                ["MaxPairwiseScalar.Vector64.Single"] = MaxPairwiseScalar_Vector64_Single,
                ["MaxPairwiseScalar.Vector128.Double"] = MaxPairwiseScalar_Vector128_Double,
                ["MaxScalar.Vector64.Double"] = MaxScalar_Vector64_Double,
                ["MaxScalar.Vector64.Single"] = MaxScalar_Vector64_Single,
                ["Min.Vector128.Double"] = Min_Vector128_Double,
                ["MinAcross.Vector64.Byte"] = MinAcross_Vector64_Byte,
                ["MinAcross.Vector64.Int16"] = MinAcross_Vector64_Int16,
                ["MinAcross.Vector64.SByte"] = MinAcross_Vector64_SByte,
                ["MinAcross.Vector64.UInt16"] = MinAcross_Vector64_UInt16,
                ["MinAcross.Vector128.Byte"] = MinAcross_Vector128_Byte,
                ["MinAcross.Vector128.Int16"] = MinAcross_Vector128_Int16,
                ["MinAcross.Vector128.Int32"] = MinAcross_Vector128_Int32,
                ["MinAcross.Vector128.SByte"] = MinAcross_Vector128_SByte,
                ["MinAcross.Vector128.Single"] = MinAcross_Vector128_Single,
                ["MinAcross.Vector128.UInt16"] = MinAcross_Vector128_UInt16,
                ["MinAcross.Vector128.UInt32"] = MinAcross_Vector128_UInt32,
                ["MinNumber.Vector128.Double"] = MinNumber_Vector128_Double,
                ["MinNumberAcross.Vector128.Single"] = MinNumberAcross_Vector128_Single,
                ["MinNumberPairwise.Vector64.Single"] = MinNumberPairwise_Vector64_Single,
                ["MinNumberPairwise.Vector128.Double"] = MinNumberPairwise_Vector128_Double,
                ["MinNumberPairwise.Vector128.Single"] = MinNumberPairwise_Vector128_Single,
                ["MinNumberPairwiseScalar.Vector64.Single"] = MinNumberPairwiseScalar_Vector64_Single,
                ["MinNumberPairwiseScalar.Vector128.Double"] = MinNumberPairwiseScalar_Vector128_Double,
                ["MinPairwise.Vector128.Byte"] = MinPairwise_Vector128_Byte,
                ["MinPairwise.Vector128.Double"] = MinPairwise_Vector128_Double,
                ["MinPairwise.Vector128.Int16"] = MinPairwise_Vector128_Int16,
                ["MinPairwise.Vector128.Int32"] = MinPairwise_Vector128_Int32,
                ["MinPairwise.Vector128.SByte"] = MinPairwise_Vector128_SByte,
                ["MinPairwise.Vector128.Single"] = MinPairwise_Vector128_Single,
                ["MinPairwise.Vector128.UInt16"] = MinPairwise_Vector128_UInt16,
                ["MinPairwise.Vector128.UInt32"] = MinPairwise_Vector128_UInt32,
                ["MinPairwiseScalar.Vector64.Single"] = MinPairwiseScalar_Vector64_Single,
                ["MinPairwiseScalar.Vector128.Double"] = MinPairwiseScalar_Vector128_Double,
                ["MinScalar.Vector64.Double"] = MinScalar_Vector64_Double,
                ["MinScalar.Vector64.Single"] = MinScalar_Vector64_Single,
                ["Multiply.Vector128.Double"] = Multiply_Vector128_Double,
                ["MultiplyExtended.Vector64.Single"] = MultiplyExtended_Vector64_Single,
                ["MultiplyExtended.Vector128.Double"] = MultiplyExtended_Vector128_Double,
                ["MultiplyExtended.Vector128.Single"] = MultiplyExtended_Vector128_Single,
                ["MultiplyExtendedScalar.Vector64.Double"] = MultiplyExtendedScalar_Vector64_Double,
                ["MultiplyExtendedScalar.Vector64.Single"] = MultiplyExtendedScalar_Vector64_Single,
                ["Negate.Vector128.Double"] = Negate_Vector128_Double,
                ["Negate.Vector128.Int64"] = Negate_Vector128_Int64,
                ["NegateScalar.Vector64.Int64"] = NegateScalar_Vector64_Int64,
                ["ReciprocalEstimate.Vector128.Double"] = ReciprocalEstimate_Vector128_Double,
                ["ReciprocalEstimateScalar.Vector64.Double"] = ReciprocalEstimateScalar_Vector64_Double,
                ["ReciprocalEstimateScalar.Vector64.Single"] = ReciprocalEstimateScalar_Vector64_Single,
                ["ReciprocalExponentScalar.Vector64.Double"] = ReciprocalExponentScalar_Vector64_Double,
                ["ReciprocalExponentScalar.Vector64.Single"] = ReciprocalExponentScalar_Vector64_Single,
                ["ReciprocalSquareRootEstimate.Vector128.Double"] = ReciprocalSquareRootEstimate_Vector128_Double,
                ["ReciprocalSquareRootEstimateScalar.Vector64.Double"] = ReciprocalSquareRootEstimateScalar_Vector64_Double,
                ["ReciprocalSquareRootEstimateScalar.Vector64.Single"] = ReciprocalSquareRootEstimateScalar_Vector64_Single,
                ["ReciprocalSquareRootStep.Vector128.Double"] = ReciprocalSquareRootStep_Vector128_Double,
                ["ReciprocalSquareRootStepScalar.Vector64.Double"] = ReciprocalSquareRootStepScalar_Vector64_Double,
                ["ReciprocalSquareRootStepScalar.Vector64.Single"] = ReciprocalSquareRootStepScalar_Vector64_Single,
                ["ReciprocalStep.Vector128.Double"] = ReciprocalStep_Vector128_Double,
                ["ReciprocalStepScalar.Vector64.Double"] = ReciprocalStepScalar_Vector64_Double,
                ["ReciprocalStepScalar.Vector64.Single"] = ReciprocalStepScalar_Vector64_Single,
                ["ReverseElementBits.Vector128.Byte"] = ReverseElementBits_Vector128_Byte,
                ["ReverseElementBits.Vector128.SByte"] = ReverseElementBits_Vector128_SByte,
                ["ReverseElementBits.Vector64.Byte"] = ReverseElementBits_Vector64_Byte,
                ["ReverseElementBits.Vector64.SByte"] = ReverseElementBits_Vector64_SByte,
                ["Sqrt.Vector64.Single"] = Sqrt_Vector64_Single,
                ["Sqrt.Vector128.Double"] = Sqrt_Vector128_Double,
                ["Sqrt.Vector128.Single"] = Sqrt_Vector128_Single,
                ["Subtract.Vector128.Double"] = Subtract_Vector128_Double,
                ["SubtractSaturateScalar.Vector64.Byte"] = SubtractSaturateScalar_Vector64_Byte,
                ["SubtractSaturateScalar.Vector64.Int16"] = SubtractSaturateScalar_Vector64_Int16,
                ["SubtractSaturateScalar.Vector64.Int32"] = SubtractSaturateScalar_Vector64_Int32,
                ["SubtractSaturateScalar.Vector64.SByte"] = SubtractSaturateScalar_Vector64_SByte,
                ["SubtractSaturateScalar.Vector64.UInt16"] = SubtractSaturateScalar_Vector64_UInt16,
                ["SubtractSaturateScalar.Vector64.UInt32"] = SubtractSaturateScalar_Vector64_UInt32,
                ["TransposeEven.Vector64.Byte"] = TransposeEven_Vector64_Byte,
                ["TransposeEven.Vector64.Int16"] = TransposeEven_Vector64_Int16,
                ["TransposeEven.Vector64.Int32"] = TransposeEven_Vector64_Int32,
                ["TransposeEven.Vector64.SByte"] = TransposeEven_Vector64_SByte,
                ["TransposeEven.Vector64.Single"] = TransposeEven_Vector64_Single,
                ["TransposeEven.Vector64.UInt16"] = TransposeEven_Vector64_UInt16,
                ["TransposeEven.Vector64.UInt32"] = TransposeEven_Vector64_UInt32,
                ["TransposeEven.Vector128.Byte"] = TransposeEven_Vector128_Byte,
                ["TransposeEven.Vector128.Double"] = TransposeEven_Vector128_Double,
                ["TransposeEven.Vector128.Int16"] = TransposeEven_Vector128_Int16,
                ["TransposeEven.Vector128.Int32"] = TransposeEven_Vector128_Int32,
                ["TransposeEven.Vector128.Int64"] = TransposeEven_Vector128_Int64,
                ["TransposeEven.Vector128.SByte"] = TransposeEven_Vector128_SByte,
                ["TransposeEven.Vector128.Single"] = TransposeEven_Vector128_Single,
                ["TransposeEven.Vector128.UInt16"] = TransposeEven_Vector128_UInt16,
                ["TransposeEven.Vector128.UInt32"] = TransposeEven_Vector128_UInt32,
                ["TransposeEven.Vector128.UInt64"] = TransposeEven_Vector128_UInt64,
                ["TransposeOdd.Vector64.Byte"] = TransposeOdd_Vector64_Byte,
                ["TransposeOdd.Vector64.Int16"] = TransposeOdd_Vector64_Int16,
                ["TransposeOdd.Vector64.Int32"] = TransposeOdd_Vector64_Int32,
                ["TransposeOdd.Vector64.SByte"] = TransposeOdd_Vector64_SByte,
                ["TransposeOdd.Vector64.Single"] = TransposeOdd_Vector64_Single,
                ["TransposeOdd.Vector64.UInt16"] = TransposeOdd_Vector64_UInt16,
                ["TransposeOdd.Vector64.UInt32"] = TransposeOdd_Vector64_UInt32,
                ["TransposeOdd.Vector128.Byte"] = TransposeOdd_Vector128_Byte,
                ["TransposeOdd.Vector128.Double"] = TransposeOdd_Vector128_Double,
                ["TransposeOdd.Vector128.Int16"] = TransposeOdd_Vector128_Int16,
                ["TransposeOdd.Vector128.Int32"] = TransposeOdd_Vector128_Int32,
                ["TransposeOdd.Vector128.Int64"] = TransposeOdd_Vector128_Int64,
                ["TransposeOdd.Vector128.SByte"] = TransposeOdd_Vector128_SByte,
                ["TransposeOdd.Vector128.Single"] = TransposeOdd_Vector128_Single,
                ["TransposeOdd.Vector128.UInt16"] = TransposeOdd_Vector128_UInt16,
                ["TransposeOdd.Vector128.UInt32"] = TransposeOdd_Vector128_UInt32,
                ["TransposeOdd.Vector128.UInt64"] = TransposeOdd_Vector128_UInt64,
                ["VectorTableLookup.Vector128.Byte"] = VectorTableLookup_Vector128_Byte,
                ["VectorTableLookup.Vector128.SByte"] = VectorTableLookup_Vector128_SByte,
                ["VectorTableLookupExtension.Vector128.Byte"] = VectorTableLookupExtension_Vector128_Byte,
                ["VectorTableLookupExtension.Vector128.SByte"] = VectorTableLookupExtension_Vector128_SByte,
                ["UnzipEven.Vector64.Byte"] = UnzipEven_Vector64_Byte,
                ["UnzipEven.Vector64.Int16"] = UnzipEven_Vector64_Int16,
                ["UnzipEven.Vector64.Int32"] = UnzipEven_Vector64_Int32,
                ["UnzipEven.Vector64.SByte"] = UnzipEven_Vector64_SByte,
                ["UnzipEven.Vector64.Single"] = UnzipEven_Vector64_Single,
                ["UnzipEven.Vector64.UInt16"] = UnzipEven_Vector64_UInt16,
                ["UnzipEven.Vector64.UInt32"] = UnzipEven_Vector64_UInt32,
                ["UnzipEven.Vector128.Byte"] = UnzipEven_Vector128_Byte,
                ["UnzipEven.Vector128.Double"] = UnzipEven_Vector128_Double,
                ["UnzipEven.Vector128.Int16"] = UnzipEven_Vector128_Int16,
                ["UnzipEven.Vector128.Int32"] = UnzipEven_Vector128_Int32,
                ["UnzipEven.Vector128.Int64"] = UnzipEven_Vector128_Int64,
                ["UnzipEven.Vector128.SByte"] = UnzipEven_Vector128_SByte,
                ["UnzipEven.Vector128.Single"] = UnzipEven_Vector128_Single,
                ["UnzipEven.Vector128.UInt16"] = UnzipEven_Vector128_UInt16,
                ["UnzipEven.Vector128.UInt32"] = UnzipEven_Vector128_UInt32,
                ["UnzipEven.Vector128.UInt64"] = UnzipEven_Vector128_UInt64,
                ["UnzipOdd.Vector64.Byte"] = UnzipOdd_Vector64_Byte,
                ["UnzipOdd.Vector64.Int16"] = UnzipOdd_Vector64_Int16,
                ["UnzipOdd.Vector64.Int32"] = UnzipOdd_Vector64_Int32,
                ["UnzipOdd.Vector64.SByte"] = UnzipOdd_Vector64_SByte,
                ["UnzipOdd.Vector64.Single"] = UnzipOdd_Vector64_Single,
                ["UnzipOdd.Vector64.UInt16"] = UnzipOdd_Vector64_UInt16,
                ["UnzipOdd.Vector64.UInt32"] = UnzipOdd_Vector64_UInt32,
                ["UnzipOdd.Vector128.Byte"] = UnzipOdd_Vector128_Byte,
                ["UnzipOdd.Vector128.Double"] = UnzipOdd_Vector128_Double,
                ["UnzipOdd.Vector128.Int16"] = UnzipOdd_Vector128_Int16,
                ["UnzipOdd.Vector128.Int32"] = UnzipOdd_Vector128_Int32,
                ["UnzipOdd.Vector128.Int64"] = UnzipOdd_Vector128_Int64,
                ["UnzipOdd.Vector128.SByte"] = UnzipOdd_Vector128_SByte,
                ["UnzipOdd.Vector128.Single"] = UnzipOdd_Vector128_Single,
                ["UnzipOdd.Vector128.UInt16"] = UnzipOdd_Vector128_UInt16,
                ["UnzipOdd.Vector128.UInt32"] = UnzipOdd_Vector128_UInt32,
                ["UnzipOdd.Vector128.UInt64"] = UnzipOdd_Vector128_UInt64,
                ["ZipHigh.Vector64.Byte"] = ZipHigh_Vector64_Byte,
                ["ZipHigh.Vector64.Int16"] = ZipHigh_Vector64_Int16,
                ["ZipHigh.Vector64.Int32"] = ZipHigh_Vector64_Int32,
                ["ZipHigh.Vector64.SByte"] = ZipHigh_Vector64_SByte,
                ["ZipHigh.Vector64.Single"] = ZipHigh_Vector64_Single,
                ["ZipHigh.Vector64.UInt16"] = ZipHigh_Vector64_UInt16,
                ["ZipHigh.Vector64.UInt32"] = ZipHigh_Vector64_UInt32,
                ["ZipHigh.Vector128.Byte"] = ZipHigh_Vector128_Byte,
                ["ZipHigh.Vector128.Double"] = ZipHigh_Vector128_Double,
                ["ZipHigh.Vector128.Int16"] = ZipHigh_Vector128_Int16,
                ["ZipHigh.Vector128.Int32"] = ZipHigh_Vector128_Int32,
                ["ZipHigh.Vector128.Int64"] = ZipHigh_Vector128_Int64,
                ["ZipHigh.Vector128.SByte"] = ZipHigh_Vector128_SByte,
                ["ZipHigh.Vector128.Single"] = ZipHigh_Vector128_Single,
                ["ZipHigh.Vector128.UInt16"] = ZipHigh_Vector128_UInt16,
                ["ZipHigh.Vector128.UInt32"] = ZipHigh_Vector128_UInt32,
                ["ZipHigh.Vector128.UInt64"] = ZipHigh_Vector128_UInt64,
                ["ZipLow.Vector64.Byte"] = ZipLow_Vector64_Byte,
                ["ZipLow.Vector64.Int16"] = ZipLow_Vector64_Int16,
                ["ZipLow.Vector64.Int32"] = ZipLow_Vector64_Int32,
                ["ZipLow.Vector64.SByte"] = ZipLow_Vector64_SByte,
                ["ZipLow.Vector64.Single"] = ZipLow_Vector64_Single,
                ["ZipLow.Vector64.UInt16"] = ZipLow_Vector64_UInt16,
                ["ZipLow.Vector64.UInt32"] = ZipLow_Vector64_UInt32,
                ["ZipLow.Vector128.Byte"] = ZipLow_Vector128_Byte,
                ["ZipLow.Vector128.Double"] = ZipLow_Vector128_Double,
                ["ZipLow.Vector128.Int16"] = ZipLow_Vector128_Int16,
                ["ZipLow.Vector128.Int32"] = ZipLow_Vector128_Int32,
                ["ZipLow.Vector128.Int64"] = ZipLow_Vector128_Int64,
                ["ZipLow.Vector128.SByte"] = ZipLow_Vector128_SByte,
                ["ZipLow.Vector128.Single"] = ZipLow_Vector128_Single,
                ["ZipLow.Vector128.UInt16"] = ZipLow_Vector128_UInt16,
                ["ZipLow.Vector128.UInt32"] = ZipLow_Vector128_UInt32,
                ["ZipLow.Vector128.UInt64"] = ZipLow_Vector128_UInt64,
            };
        }
    }
}
