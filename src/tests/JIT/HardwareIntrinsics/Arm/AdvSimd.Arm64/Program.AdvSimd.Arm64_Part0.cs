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
                ["AbsSaturate.Vector128.Int64"] = AbsSaturate_Vector128_Int64,
                ["AbsSaturateScalar.Vector64.Int16"] = AbsSaturateScalar_Vector64_Int16,
                ["AbsSaturateScalar.Vector64.Int32"] = AbsSaturateScalar_Vector64_Int32,
                ["AbsSaturateScalar.Vector64.Int64"] = AbsSaturateScalar_Vector64_Int64,
                ["AbsSaturateScalar.Vector64.SByte"] = AbsSaturateScalar_Vector64_SByte,
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
                ["AddAcrossWidening.Vector64.Byte"] = AddAcrossWidening_Vector64_Byte,
                ["AddAcrossWidening.Vector64.Int16"] = AddAcrossWidening_Vector64_Int16,
                ["AddAcrossWidening.Vector64.SByte"] = AddAcrossWidening_Vector64_SByte,
                ["AddAcrossWidening.Vector64.UInt16"] = AddAcrossWidening_Vector64_UInt16,
                ["AddAcrossWidening.Vector128.Byte"] = AddAcrossWidening_Vector128_Byte,
                ["AddAcrossWidening.Vector128.Int16"] = AddAcrossWidening_Vector128_Int16,
                ["AddAcrossWidening.Vector128.Int32"] = AddAcrossWidening_Vector128_Int32,
                ["AddAcrossWidening.Vector128.SByte"] = AddAcrossWidening_Vector128_SByte,
                ["AddAcrossWidening.Vector128.UInt16"] = AddAcrossWidening_Vector128_UInt16,
                ["AddAcrossWidening.Vector128.UInt32"] = AddAcrossWidening_Vector128_UInt32,
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
                ["AddSaturate.Vector64.Byte.Vector64.SByte"] = AddSaturate_Vector64_Byte_Vector64_SByte,
                ["AddSaturate.Vector64.Int16.Vector64.UInt16"] = AddSaturate_Vector64_Int16_Vector64_UInt16,
                ["AddSaturate.Vector64.Int32.Vector64.UInt32"] = AddSaturate_Vector64_Int32_Vector64_UInt32,
                ["AddSaturate.Vector64.SByte.Vector64.Byte"] = AddSaturate_Vector64_SByte_Vector64_Byte,
                ["AddSaturate.Vector64.UInt16.Vector64.Int16"] = AddSaturate_Vector64_UInt16_Vector64_Int16,
                ["AddSaturate.Vector64.UInt32.Vector64.Int32"] = AddSaturate_Vector64_UInt32_Vector64_Int32,
                ["AddSaturate.Vector128.Byte.Vector128.SByte"] = AddSaturate_Vector128_Byte_Vector128_SByte,
                ["AddSaturate.Vector128.Int16.Vector128.UInt16"] = AddSaturate_Vector128_Int16_Vector128_UInt16,
                ["AddSaturate.Vector128.Int32.Vector128.UInt32"] = AddSaturate_Vector128_Int32_Vector128_UInt32,
                ["AddSaturate.Vector128.Int64.Vector128.UInt64"] = AddSaturate_Vector128_Int64_Vector128_UInt64,
                ["AddSaturate.Vector128.SByte.Vector128.Byte"] = AddSaturate_Vector128_SByte_Vector128_Byte,
                ["AddSaturate.Vector128.UInt16.Vector128.Int16"] = AddSaturate_Vector128_UInt16_Vector128_Int16,
                ["AddSaturate.Vector128.UInt32.Vector128.Int32"] = AddSaturate_Vector128_UInt32_Vector128_Int32,
                ["AddSaturate.Vector128.UInt64.Vector128.Int64"] = AddSaturate_Vector128_UInt64_Vector128_Int64,
                ["AddSaturateScalar.Vector64.Byte.Vector64.Byte"] = AddSaturateScalar_Vector64_Byte_Vector64_Byte,
                ["AddSaturateScalar.Vector64.Byte.Vector64.SByte"] = AddSaturateScalar_Vector64_Byte_Vector64_SByte,
                ["AddSaturateScalar.Vector64.Int16.Vector64.Int16"] = AddSaturateScalar_Vector64_Int16_Vector64_Int16,
                ["AddSaturateScalar.Vector64.Int16.Vector64.UInt16"] = AddSaturateScalar_Vector64_Int16_Vector64_UInt16,
                ["AddSaturateScalar.Vector64.Int32.Vector64.Int32"] = AddSaturateScalar_Vector64_Int32_Vector64_Int32,
                ["AddSaturateScalar.Vector64.Int32.Vector64.UInt32"] = AddSaturateScalar_Vector64_Int32_Vector64_UInt32,
                ["AddSaturateScalar.Vector64.Int64.Vector64.UInt64"] = AddSaturateScalar_Vector64_Int64_Vector64_UInt64,
                ["AddSaturateScalar.Vector64.SByte.Vector64.Byte"] = AddSaturateScalar_Vector64_SByte_Vector64_Byte,
                ["AddSaturateScalar.Vector64.SByte.Vector64.SByte"] = AddSaturateScalar_Vector64_SByte_Vector64_SByte,
                ["AddSaturateScalar.Vector64.UInt16.Vector64.Int16"] = AddSaturateScalar_Vector64_UInt16_Vector64_Int16,
                ["AddSaturateScalar.Vector64.UInt16.Vector64.UInt16"] = AddSaturateScalar_Vector64_UInt16_Vector64_UInt16,
                ["AddSaturateScalar.Vector64.UInt32.Vector64.Int32"] = AddSaturateScalar_Vector64_UInt32_Vector64_Int32,
                ["AddSaturateScalar.Vector64.UInt32.Vector64.UInt32"] = AddSaturateScalar_Vector64_UInt32_Vector64_UInt32,
                ["AddSaturateScalar.Vector64.UInt64.Vector64.Int64"] = AddSaturateScalar_Vector64_UInt64_Vector64_Int64,
                ["Ceiling.Vector128.Double"] = Ceiling_Vector128_Double,
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
                ["ConvertToDouble.Vector64.Single"] = ConvertToDouble_Vector64_Single,
                ["ConvertToDouble.Vector128.Int64"] = ConvertToDouble_Vector128_Int64,
                ["ConvertToDouble.Vector128.UInt64"] = ConvertToDouble_Vector128_UInt64,
                ["ConvertToDoubleScalar.Vector64.Int64"] = ConvertToDoubleScalar_Vector64_Int64,
                ["ConvertToDoubleScalar.Vector64.UInt64"] = ConvertToDoubleScalar_Vector64_UInt64,
                ["ConvertToDoubleUpper.Vector128.Single"] = ConvertToDoubleUpper_Vector128_Single,
                ["ConvertToInt64RoundAwayFromZero.Vector128.Double"] = ConvertToInt64RoundAwayFromZero_Vector128_Double,
                ["ConvertToInt64RoundAwayFromZeroScalar.Vector64.Double"] = ConvertToInt64RoundAwayFromZeroScalar_Vector64_Double,
                ["ConvertToInt64RoundToEven.Vector128.Double"] = ConvertToInt64RoundToEven_Vector128_Double,
                ["ConvertToInt64RoundToEvenScalar.Vector64.Double"] = ConvertToInt64RoundToEvenScalar_Vector64_Double,
                ["ConvertToInt64RoundToNegativeInfinity.Vector128.Double"] = ConvertToInt64RoundToNegativeInfinity_Vector128_Double,
                ["ConvertToInt64RoundToNegativeInfinityScalar.Vector64.Double"] = ConvertToInt64RoundToNegativeInfinityScalar_Vector64_Double,
                ["ConvertToInt64RoundToPositiveInfinity.Vector128.Double"] = ConvertToInt64RoundToPositiveInfinity_Vector128_Double,
                ["ConvertToInt64RoundToPositiveInfinityScalar.Vector64.Double"] = ConvertToInt64RoundToPositiveInfinityScalar_Vector64_Double,
                ["ConvertToInt64RoundToZero.Vector128.Double"] = ConvertToInt64RoundToZero_Vector128_Double,
                ["ConvertToInt64RoundToZeroScalar.Vector64.Double"] = ConvertToInt64RoundToZeroScalar_Vector64_Double,
                ["ConvertToSingleLower.Vector64.Single"] = ConvertToSingleLower_Vector64_Single,
                ["ConvertToSingleRoundToOddLower.Vector64.Single"] = ConvertToSingleRoundToOddLower_Vector64_Single,
                ["ConvertToSingleRoundToOddUpper.Vector128.Single"] = ConvertToSingleRoundToOddUpper_Vector128_Single,
                ["ConvertToSingleUpper.Vector128.Single"] = ConvertToSingleUpper_Vector128_Single,
                ["ConvertToUInt64RoundAwayFromZero.Vector128.Double"] = ConvertToUInt64RoundAwayFromZero_Vector128_Double,
                ["ConvertToUInt64RoundAwayFromZeroScalar.Vector64.Double"] = ConvertToUInt64RoundAwayFromZeroScalar_Vector64_Double,
                ["ConvertToUInt64RoundToEven.Vector128.Double"] = ConvertToUInt64RoundToEven_Vector128_Double,
                ["ConvertToUInt64RoundToEvenScalar.Vector64.Double"] = ConvertToUInt64RoundToEvenScalar_Vector64_Double,
                ["ConvertToUInt64RoundToNegativeInfinity.Vector128.Double"] = ConvertToUInt64RoundToNegativeInfinity_Vector128_Double,
                ["ConvertToUInt64RoundToNegativeInfinityScalar.Vector64.Double"] = ConvertToUInt64RoundToNegativeInfinityScalar_Vector64_Double,
                ["ConvertToUInt64RoundToPositiveInfinity.Vector128.Double"] = ConvertToUInt64RoundToPositiveInfinity_Vector128_Double,
                ["ConvertToUInt64RoundToPositiveInfinityScalar.Vector64.Double"] = ConvertToUInt64RoundToPositiveInfinityScalar_Vector64_Double,
                ["ConvertToUInt64RoundToZero.Vector128.Double"] = ConvertToUInt64RoundToZero_Vector128_Double,
                ["ConvertToUInt64RoundToZeroScalar.Vector64.Double"] = ConvertToUInt64RoundToZeroScalar_Vector64_Double,
                ["Divide.Vector64.Single"] = Divide_Vector64_Single,
                ["Divide.Vector128.Double"] = Divide_Vector128_Double,
                ["Divide.Vector128.Single"] = Divide_Vector128_Single,
                ["DuplicateSelectedScalarToVector128.V128.Double.1"] = DuplicateSelectedScalarToVector128_V128_Double_1,
                ["DuplicateSelectedScalarToVector128.V128.Int64.1"] = DuplicateSelectedScalarToVector128_V128_Int64_1,
                ["DuplicateSelectedScalarToVector128.V128.UInt64.1"] = DuplicateSelectedScalarToVector128_V128_UInt64_1,
                ["DuplicateToVector128.Double"] = DuplicateToVector128_Double,
                ["DuplicateToVector128.Double.31"] = DuplicateToVector128_Double_31,
                ["DuplicateToVector128.Int64"] = DuplicateToVector128_Int64,
                ["DuplicateToVector128.Int64.31"] = DuplicateToVector128_Int64_31,
                ["DuplicateToVector128.UInt64"] = DuplicateToVector128_UInt64,
                ["DuplicateToVector128.UInt64.31"] = DuplicateToVector128_UInt64_31,
                ["ExtractNarrowingSaturateScalar.Vector64.Byte"] = ExtractNarrowingSaturateScalar_Vector64_Byte,
                ["ExtractNarrowingSaturateScalar.Vector64.Int16"] = ExtractNarrowingSaturateScalar_Vector64_Int16,
                ["ExtractNarrowingSaturateScalar.Vector64.Int32"] = ExtractNarrowingSaturateScalar_Vector64_Int32,
                ["ExtractNarrowingSaturateScalar.Vector64.SByte"] = ExtractNarrowingSaturateScalar_Vector64_SByte,
                ["ExtractNarrowingSaturateScalar.Vector64.UInt16"] = ExtractNarrowingSaturateScalar_Vector64_UInt16,
                ["ExtractNarrowingSaturateScalar.Vector64.UInt32"] = ExtractNarrowingSaturateScalar_Vector64_UInt32,
                ["ExtractNarrowingSaturateUnsignedScalar.Vector64.Byte"] = ExtractNarrowingSaturateUnsignedScalar_Vector64_Byte,
                ["ExtractNarrowingSaturateUnsignedScalar.Vector64.UInt16"] = ExtractNarrowingSaturateUnsignedScalar_Vector64_UInt16,
                ["ExtractNarrowingSaturateUnsignedScalar.Vector64.UInt32"] = ExtractNarrowingSaturateUnsignedScalar_Vector64_UInt32,
                ["Floor.Vector128.Double"] = Floor_Vector128_Double,
                ["FusedMultiplyAdd.Vector128.Double"] = FusedMultiplyAdd_Vector128_Double,
                ["FusedMultiplyAddByScalar.Vector64.Single"] = FusedMultiplyAddByScalar_Vector64_Single,
                ["FusedMultiplyAddByScalar.Vector128.Double"] = FusedMultiplyAddByScalar_Vector128_Double,
                ["FusedMultiplyAddByScalar.Vector128.Single"] = FusedMultiplyAddByScalar_Vector128_Single,
                ["FusedMultiplyAddBySelectedScalar.Vector64.Single.Vector64.Single.1"] = FusedMultiplyAddBySelectedScalar_Vector64_Single_Vector64_Single_1,
                ["FusedMultiplyAddBySelectedScalar.Vector64.Single.Vector128.Single.3"] = FusedMultiplyAddBySelectedScalar_Vector64_Single_Vector128_Single_3,
                ["FusedMultiplyAddBySelectedScalar.Vector128.Double.Vector128.Double.1"] = FusedMultiplyAddBySelectedScalar_Vector128_Double_Vector128_Double_1,
                ["FusedMultiplyAddBySelectedScalar.Vector128.Single.Vector64.Single.1"] = FusedMultiplyAddBySelectedScalar_Vector128_Single_Vector64_Single_1,
                ["FusedMultiplyAddBySelectedScalar.Vector128.Single.Vector128.Single.3"] = FusedMultiplyAddBySelectedScalar_Vector128_Single_Vector128_Single_3,
                ["FusedMultiplyAddScalarBySelectedScalar.Vector64.Double.Vector128.Double.1"] = FusedMultiplyAddScalarBySelectedScalar_Vector64_Double_Vector128_Double_1,
                ["FusedMultiplyAddScalarBySelectedScalar.Vector64.Single.Vector64.Single.1"] = FusedMultiplyAddScalarBySelectedScalar_Vector64_Single_Vector64_Single_1,
                ["FusedMultiplyAddScalarBySelectedScalar.Vector64.Single.Vector128.Single.3"] = FusedMultiplyAddScalarBySelectedScalar_Vector64_Single_Vector128_Single_3,
                ["FusedMultiplySubtract.Vector128.Double"] = FusedMultiplySubtract_Vector128_Double,
                ["FusedMultiplySubtractByScalar.Vector64.Single"] = FusedMultiplySubtractByScalar_Vector64_Single,
                ["FusedMultiplySubtractByScalar.Vector128.Double"] = FusedMultiplySubtractByScalar_Vector128_Double,
                ["FusedMultiplySubtractByScalar.Vector128.Single"] = FusedMultiplySubtractByScalar_Vector128_Single,
                ["FusedMultiplySubtractBySelectedScalar.Vector64.Single.Vector64.Single.1"] = FusedMultiplySubtractBySelectedScalar_Vector64_Single_Vector64_Single_1,
                ["FusedMultiplySubtractBySelectedScalar.Vector64.Single.Vector128.Single.3"] = FusedMultiplySubtractBySelectedScalar_Vector64_Single_Vector128_Single_3,
                ["FusedMultiplySubtractBySelectedScalar.Vector128.Double.Vector128.Double.1"] = FusedMultiplySubtractBySelectedScalar_Vector128_Double_Vector128_Double_1,
                ["FusedMultiplySubtractBySelectedScalar.Vector128.Single.Vector64.Single.1"] = FusedMultiplySubtractBySelectedScalar_Vector128_Single_Vector64_Single_1,
                ["FusedMultiplySubtractBySelectedScalar.Vector128.Single.Vector128.Single.3"] = FusedMultiplySubtractBySelectedScalar_Vector128_Single_Vector128_Single_3,
                ["FusedMultiplySubtractScalarBySelectedScalar.Vector64.Double.Vector128.Double.1"] = FusedMultiplySubtractScalarBySelectedScalar_Vector64_Double_Vector128_Double_1,
                ["FusedMultiplySubtractScalarBySelectedScalar.Vector64.Single.Vector64.Single.1"] = FusedMultiplySubtractScalarBySelectedScalar_Vector64_Single_Vector64_Single_1,
                ["FusedMultiplySubtractScalarBySelectedScalar.Vector64.Single.Vector128.Single.3"] = FusedMultiplySubtractScalarBySelectedScalar_Vector64_Single_Vector128_Single_3,
                ["InsertSelectedScalar.Vector64.Byte.7.Vector64.Byte.7"] = InsertSelectedScalar_Vector64_Byte_7_Vector64_Byte_7,
                ["InsertSelectedScalar.Vector64.Byte.7.Vector128.Byte.15"] = InsertSelectedScalar_Vector64_Byte_7_Vector128_Byte_15,
                ["InsertSelectedScalar.Vector64.Int16.3.Vector64.Int16.3"] = InsertSelectedScalar_Vector64_Int16_3_Vector64_Int16_3,
                ["InsertSelectedScalar.Vector64.Int16.3.Vector128.Int16.7"] = InsertSelectedScalar_Vector64_Int16_3_Vector128_Int16_7,
                ["InsertSelectedScalar.Vector64.Int32.1.Vector64.Int32.1"] = InsertSelectedScalar_Vector64_Int32_1_Vector64_Int32_1,
                ["InsertSelectedScalar.Vector64.Int32.1.Vector128.Int32.3"] = InsertSelectedScalar_Vector64_Int32_1_Vector128_Int32_3,
                ["InsertSelectedScalar.Vector64.SByte.7.Vector64.SByte.7"] = InsertSelectedScalar_Vector64_SByte_7_Vector64_SByte_7,
                ["InsertSelectedScalar.Vector64.SByte.7.Vector128.SByte.15"] = InsertSelectedScalar_Vector64_SByte_7_Vector128_SByte_15,
                ["InsertSelectedScalar.Vector64.Single.1.Vector64.Single.1"] = InsertSelectedScalar_Vector64_Single_1_Vector64_Single_1,
                ["InsertSelectedScalar.Vector64.Single.1.Vector128.Single.3"] = InsertSelectedScalar_Vector64_Single_1_Vector128_Single_3,
                ["InsertSelectedScalar.Vector64.UInt16.3.Vector64.UInt16.3"] = InsertSelectedScalar_Vector64_UInt16_3_Vector64_UInt16_3,
                ["InsertSelectedScalar.Vector64.UInt16.3.Vector128.UInt16.7"] = InsertSelectedScalar_Vector64_UInt16_3_Vector128_UInt16_7,
                ["InsertSelectedScalar.Vector64.UInt32.1.Vector64.UInt32.1"] = InsertSelectedScalar_Vector64_UInt32_1_Vector64_UInt32_1,
                ["InsertSelectedScalar.Vector64.UInt32.1.Vector128.UInt32.3"] = InsertSelectedScalar_Vector64_UInt32_1_Vector128_UInt32_3,
                ["InsertSelectedScalar.Vector128.Byte.15.Vector64.Byte.7"] = InsertSelectedScalar_Vector128_Byte_15_Vector64_Byte_7,
                ["InsertSelectedScalar.Vector128.Byte.15.Vector128.Byte.15"] = InsertSelectedScalar_Vector128_Byte_15_Vector128_Byte_15,
                ["InsertSelectedScalar.Vector128.Double.1.Vector128.Double.1"] = InsertSelectedScalar_Vector128_Double_1_Vector128_Double_1,
                ["InsertSelectedScalar.Vector128.Int16.7.Vector64.Int16.3"] = InsertSelectedScalar_Vector128_Int16_7_Vector64_Int16_3,
                ["InsertSelectedScalar.Vector128.Int16.7.Vector128.Int16.7"] = InsertSelectedScalar_Vector128_Int16_7_Vector128_Int16_7,
                ["InsertSelectedScalar.Vector128.Int32.3.Vector64.Int32.1"] = InsertSelectedScalar_Vector128_Int32_3_Vector64_Int32_1,
                ["InsertSelectedScalar.Vector128.Int32.3.Vector128.Int32.3"] = InsertSelectedScalar_Vector128_Int32_3_Vector128_Int32_3,
                ["InsertSelectedScalar.Vector128.Int64.1.Vector128.Int64.1"] = InsertSelectedScalar_Vector128_Int64_1_Vector128_Int64_1,
                ["InsertSelectedScalar.Vector128.SByte.15.Vector64.SByte.7"] = InsertSelectedScalar_Vector128_SByte_15_Vector64_SByte_7,
                ["InsertSelectedScalar.Vector128.SByte.15.Vector128.SByte.15"] = InsertSelectedScalar_Vector128_SByte_15_Vector128_SByte_15,
                ["InsertSelectedScalar.Vector128.Single.3.Vector64.Single.1"] = InsertSelectedScalar_Vector128_Single_3_Vector64_Single_1,
                ["InsertSelectedScalar.Vector128.Single.3.Vector128.Single.3"] = InsertSelectedScalar_Vector128_Single_3_Vector128_Single_3,
                ["InsertSelectedScalar.Vector128.UInt16.7.Vector64.UInt16.3"] = InsertSelectedScalar_Vector128_UInt16_7_Vector64_UInt16_3,
                ["InsertSelectedScalar.Vector128.UInt16.7.Vector128.UInt16.7"] = InsertSelectedScalar_Vector128_UInt16_7_Vector128_UInt16_7,
                ["InsertSelectedScalar.Vector128.UInt32.3.Vector64.UInt32.1"] = InsertSelectedScalar_Vector128_UInt32_3_Vector64_UInt32_1,
                ["InsertSelectedScalar.Vector128.UInt32.3.Vector128.UInt32.3"] = InsertSelectedScalar_Vector128_UInt32_3_Vector128_UInt32_3,
                ["InsertSelectedScalar.Vector128.UInt64.1.Vector128.UInt64.1"] = InsertSelectedScalar_Vector128_UInt64_1_Vector128_UInt64_1,
                ["LoadAndReplicateToVector128.Double"] = LoadAndReplicateToVector128_Double,
                ["LoadAndReplicateToVector128.Int64"] = LoadAndReplicateToVector128_Int64,
                ["LoadAndReplicateToVector128.UInt64"] = LoadAndReplicateToVector128_UInt64,
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
            };
        }
    }
}
