// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;

namespace JIT.HardwareIntrinsics.Arm
{
    public static partial class Program
    {
        static Program()
        {
            TestList = new Dictionary<string, Action>() {
                ["FusedAddRoundedHalving.Vector64.Int16"] = FusedAddRoundedHalving_Vector64_Int16,
                ["FusedAddRoundedHalving.Vector64.Int32"] = FusedAddRoundedHalving_Vector64_Int32,
                ["FusedAddRoundedHalving.Vector64.SByte"] = FusedAddRoundedHalving_Vector64_SByte,
                ["FusedAddRoundedHalving.Vector64.UInt16"] = FusedAddRoundedHalving_Vector64_UInt16,
                ["FusedAddRoundedHalving.Vector64.UInt32"] = FusedAddRoundedHalving_Vector64_UInt32,
                ["FusedAddRoundedHalving.Vector128.Byte"] = FusedAddRoundedHalving_Vector128_Byte,
                ["FusedAddRoundedHalving.Vector128.Int16"] = FusedAddRoundedHalving_Vector128_Int16,
                ["FusedAddRoundedHalving.Vector128.Int32"] = FusedAddRoundedHalving_Vector128_Int32,
                ["FusedAddRoundedHalving.Vector128.SByte"] = FusedAddRoundedHalving_Vector128_SByte,
                ["FusedAddRoundedHalving.Vector128.UInt16"] = FusedAddRoundedHalving_Vector128_UInt16,
                ["FusedAddRoundedHalving.Vector128.UInt32"] = FusedAddRoundedHalving_Vector128_UInt32,
                ["FusedMultiplyAdd.Vector64.Single"] = FusedMultiplyAdd_Vector64_Single,
                ["FusedMultiplyAdd.Vector128.Single"] = FusedMultiplyAdd_Vector128_Single,
                ["FusedMultiplyAddScalar.Vector64.Double"] = FusedMultiplyAddScalar_Vector64_Double,
                ["FusedMultiplyAddScalar.Vector64.Single"] = FusedMultiplyAddScalar_Vector64_Single,
                ["FusedMultiplyAddNegatedScalar.Vector64.Double"] = FusedMultiplyAddNegatedScalar_Vector64_Double,
                ["FusedMultiplyAddNegatedScalar.Vector64.Single"] = FusedMultiplyAddNegatedScalar_Vector64_Single,
                ["FusedMultiplySubtract.Vector64.Single"] = FusedMultiplySubtract_Vector64_Single,
                ["FusedMultiplySubtract.Vector128.Single"] = FusedMultiplySubtract_Vector128_Single,
                ["FusedMultiplySubtractScalar.Vector64.Double"] = FusedMultiplySubtractScalar_Vector64_Double,
                ["FusedMultiplySubtractScalar.Vector64.Single"] = FusedMultiplySubtractScalar_Vector64_Single,
                ["FusedMultiplySubtractNegatedScalar.Vector64.Double"] = FusedMultiplySubtractNegatedScalar_Vector64_Double,
                ["FusedMultiplySubtractNegatedScalar.Vector64.Single"] = FusedMultiplySubtractNegatedScalar_Vector64_Single,
                ["FusedSubtractHalving.Vector64.Byte"] = FusedSubtractHalving_Vector64_Byte,
                ["FusedSubtractHalving.Vector64.Int16"] = FusedSubtractHalving_Vector64_Int16,
                ["FusedSubtractHalving.Vector64.Int32"] = FusedSubtractHalving_Vector64_Int32,
                ["FusedSubtractHalving.Vector64.SByte"] = FusedSubtractHalving_Vector64_SByte,
                ["FusedSubtractHalving.Vector64.UInt16"] = FusedSubtractHalving_Vector64_UInt16,
                ["FusedSubtractHalving.Vector64.UInt32"] = FusedSubtractHalving_Vector64_UInt32,
                ["FusedSubtractHalving.Vector128.Byte"] = FusedSubtractHalving_Vector128_Byte,
                ["FusedSubtractHalving.Vector128.Int16"] = FusedSubtractHalving_Vector128_Int16,
                ["FusedSubtractHalving.Vector128.Int32"] = FusedSubtractHalving_Vector128_Int32,
                ["FusedSubtractHalving.Vector128.SByte"] = FusedSubtractHalving_Vector128_SByte,
                ["FusedSubtractHalving.Vector128.UInt16"] = FusedSubtractHalving_Vector128_UInt16,
                ["FusedSubtractHalving.Vector128.UInt32"] = FusedSubtractHalving_Vector128_UInt32,
                ["Insert.Vector64.Byte.1"] = Insert_Vector64_Byte_1,
                ["Insert.Vector64.Int16.1"] = Insert_Vector64_Int16_1,
                ["Insert.Vector64.Int32.1"] = Insert_Vector64_Int32_1,
                ["Insert.Vector64.SByte.1"] = Insert_Vector64_SByte_1,
                ["Insert.Vector64.Single.1"] = Insert_Vector64_Single_1,
                ["Insert.Vector64.UInt16.1"] = Insert_Vector64_UInt16_1,
                ["Insert.Vector64.UInt32.1"] = Insert_Vector64_UInt32_1,
                ["Insert.Vector128.Byte.1"] = Insert_Vector128_Byte_1,
                ["Insert.Vector128.Double.1"] = Insert_Vector128_Double_1,
                ["Insert.Vector128.Int16.1"] = Insert_Vector128_Int16_1,
                ["Insert.Vector128.Int32.1"] = Insert_Vector128_Int32_1,
                ["Insert.Vector128.Int64.1"] = Insert_Vector128_Int64_1,
                ["Insert.Vector128.SByte.1"] = Insert_Vector128_SByte_1,
                ["Insert.Vector128.Single.1"] = Insert_Vector128_Single_1,
                ["Insert.Vector128.UInt16.1"] = Insert_Vector128_UInt16_1,
                ["Insert.Vector128.UInt32.1"] = Insert_Vector128_UInt32_1,
                ["Insert.Vector128.UInt64.1"] = Insert_Vector128_UInt64_1,
                ["InsertScalar.Vector128.Double.1"] = InsertScalar_Vector128_Double_1,
                ["InsertScalar.Vector128.Int64.1"] = InsertScalar_Vector128_Int64_1,
                ["InsertScalar.Vector128.UInt64.1"] = InsertScalar_Vector128_UInt64_1,
                ["LeadingSignCount.Vector64.Int16"] = LeadingSignCount_Vector64_Int16,
                ["LeadingSignCount.Vector64.Int32"] = LeadingSignCount_Vector64_Int32,
                ["LeadingSignCount.Vector64.SByte"] = LeadingSignCount_Vector64_SByte,
                ["LeadingSignCount.Vector128.Int16"] = LeadingSignCount_Vector128_Int16,
                ["LeadingSignCount.Vector128.Int32"] = LeadingSignCount_Vector128_Int32,
                ["LeadingSignCount.Vector128.SByte"] = LeadingSignCount_Vector128_SByte,
                ["LeadingZeroCount.Vector64.Byte"] = LeadingZeroCount_Vector64_Byte,
                ["LeadingZeroCount.Vector64.Int16"] = LeadingZeroCount_Vector64_Int16,
                ["LeadingZeroCount.Vector64.Int32"] = LeadingZeroCount_Vector64_Int32,
                ["LeadingZeroCount.Vector64.SByte"] = LeadingZeroCount_Vector64_SByte,
                ["LeadingZeroCount.Vector64.UInt16"] = LeadingZeroCount_Vector64_UInt16,
                ["LeadingZeroCount.Vector64.UInt32"] = LeadingZeroCount_Vector64_UInt32,
                ["LeadingZeroCount.Vector128.Byte"] = LeadingZeroCount_Vector128_Byte,
                ["LeadingZeroCount.Vector128.Int16"] = LeadingZeroCount_Vector128_Int16,
                ["LeadingZeroCount.Vector128.Int32"] = LeadingZeroCount_Vector128_Int32,
                ["LeadingZeroCount.Vector128.SByte"] = LeadingZeroCount_Vector128_SByte,
                ["LeadingZeroCount.Vector128.UInt16"] = LeadingZeroCount_Vector128_UInt16,
                ["LeadingZeroCount.Vector128.UInt32"] = LeadingZeroCount_Vector128_UInt32,
                ["LoadAndInsertScalar.Vector64.Byte.7"] = LoadAndInsertScalar_Vector64_Byte_7,
                ["LoadAndInsertScalar.Vector64.Int16.3"] = LoadAndInsertScalar_Vector64_Int16_3,
                ["LoadAndInsertScalar.Vector64.Int32.1"] = LoadAndInsertScalar_Vector64_Int32_1,
                ["LoadAndInsertScalar.Vector64.SByte.7"] = LoadAndInsertScalar_Vector64_SByte_7,
                ["LoadAndInsertScalar.Vector64.Single.1"] = LoadAndInsertScalar_Vector64_Single_1,
                ["LoadAndInsertScalar.Vector64.UInt16.3"] = LoadAndInsertScalar_Vector64_UInt16_3,
                ["LoadAndInsertScalar.Vector64.UInt32.1"] = LoadAndInsertScalar_Vector64_UInt32_1,
                ["LoadAndInsertScalar.Vector128.Byte.15"] = LoadAndInsertScalar_Vector128_Byte_15,
                ["LoadAndInsertScalar.Vector128.Double.1"] = LoadAndInsertScalar_Vector128_Double_1,
                ["LoadAndInsertScalar.Vector128.Int16.7"] = LoadAndInsertScalar_Vector128_Int16_7,
                ["LoadAndInsertScalar.Vector128.Int32.3"] = LoadAndInsertScalar_Vector128_Int32_3,
                ["LoadAndInsertScalar.Vector128.Int64.1"] = LoadAndInsertScalar_Vector128_Int64_1,
                ["LoadAndInsertScalar.Vector128.SByte.15"] = LoadAndInsertScalar_Vector128_SByte_15,
                ["LoadAndInsertScalar.Vector128.Single.3"] = LoadAndInsertScalar_Vector128_Single_3,
                ["LoadAndInsertScalar.Vector128.UInt16.7"] = LoadAndInsertScalar_Vector128_UInt16_7,
                ["LoadAndInsertScalar.Vector128.UInt32.3"] = LoadAndInsertScalar_Vector128_UInt32_3,
                ["LoadAndInsertScalar.Vector128.UInt64.1"] = LoadAndInsertScalar_Vector128_UInt64_1,
                ["LoadAndReplicateToVector64.Byte"] = LoadAndReplicateToVector64_Byte,
                ["LoadAndReplicateToVector64.Int16"] = LoadAndReplicateToVector64_Int16,
                ["LoadAndReplicateToVector64.Int32"] = LoadAndReplicateToVector64_Int32,
                ["LoadAndReplicateToVector64.SByte"] = LoadAndReplicateToVector64_SByte,
                ["LoadAndReplicateToVector64.Single"] = LoadAndReplicateToVector64_Single,
                ["LoadAndReplicateToVector64.UInt16"] = LoadAndReplicateToVector64_UInt16,
                ["LoadAndReplicateToVector64.UInt32"] = LoadAndReplicateToVector64_UInt32,
                ["LoadAndReplicateToVector128.Byte"] = LoadAndReplicateToVector128_Byte,
                ["LoadAndReplicateToVector128.Int16"] = LoadAndReplicateToVector128_Int16,
                ["LoadAndReplicateToVector128.Int32"] = LoadAndReplicateToVector128_Int32,
                ["LoadAndReplicateToVector128.SByte"] = LoadAndReplicateToVector128_SByte,
                ["LoadAndReplicateToVector128.Single"] = LoadAndReplicateToVector128_Single,
                ["LoadAndReplicateToVector128.UInt16"] = LoadAndReplicateToVector128_UInt16,
                ["LoadAndReplicateToVector128.UInt32"] = LoadAndReplicateToVector128_UInt32,
                ["LoadVector64.Byte"] = LoadVector64_Byte,
                ["LoadVector64.Double"] = LoadVector64_Double,
                ["LoadVector64.Int16"] = LoadVector64_Int16,
                ["LoadVector64.Int32"] = LoadVector64_Int32,
                ["LoadVector64.Int64"] = LoadVector64_Int64,
                ["LoadVector64.SByte"] = LoadVector64_SByte,
                ["LoadVector64.Single"] = LoadVector64_Single,
                ["LoadVector64.UInt16"] = LoadVector64_UInt16,
                ["LoadVector64.UInt32"] = LoadVector64_UInt32,
                ["LoadVector64.UInt64"] = LoadVector64_UInt64,
                ["LoadVector128.Byte"] = LoadVector128_Byte,
                ["LoadVector128.Double"] = LoadVector128_Double,
                ["LoadVector128.Int16"] = LoadVector128_Int16,
                ["LoadVector128.Int32"] = LoadVector128_Int32,
                ["LoadVector128.Int64"] = LoadVector128_Int64,
                ["LoadVector128.SByte"] = LoadVector128_SByte,
                ["LoadVector128.Single"] = LoadVector128_Single,
                ["LoadVector128.UInt16"] = LoadVector128_UInt16,
                ["LoadVector128.UInt32"] = LoadVector128_UInt32,
                ["LoadVector128.UInt64"] = LoadVector128_UInt64,
                ["Max.Vector64.Byte"] = Max_Vector64_Byte,
                ["Max.Vector64.Int16"] = Max_Vector64_Int16,
                ["Max.Vector64.Int32"] = Max_Vector64_Int32,
                ["Max.Vector64.SByte"] = Max_Vector64_SByte,
                ["Max.Vector64.Single"] = Max_Vector64_Single,
                ["Max.Vector64.UInt16"] = Max_Vector64_UInt16,
                ["Max.Vector64.UInt32"] = Max_Vector64_UInt32,
                ["Max.Vector128.Byte"] = Max_Vector128_Byte,
                ["Max.Vector128.Int16"] = Max_Vector128_Int16,
                ["Max.Vector128.Int32"] = Max_Vector128_Int32,
                ["Max.Vector128.SByte"] = Max_Vector128_SByte,
                ["Max.Vector128.Single"] = Max_Vector128_Single,
                ["Max.Vector128.UInt16"] = Max_Vector128_UInt16,
                ["Max.Vector128.UInt32"] = Max_Vector128_UInt32,
                ["MaxNumber.Vector64.Single"] = MaxNumber_Vector64_Single,
                ["MaxNumber.Vector128.Single"] = MaxNumber_Vector128_Single,
                ["MaxNumberScalar.Vector64.Double"] = MaxNumberScalar_Vector64_Double,
                ["MaxNumberScalar.Vector64.Single"] = MaxNumberScalar_Vector64_Single,
                ["MaxPairwise.Vector64.Byte"] = MaxPairwise_Vector64_Byte,
                ["MaxPairwise.Vector64.Int16"] = MaxPairwise_Vector64_Int16,
                ["MaxPairwise.Vector64.Int32"] = MaxPairwise_Vector64_Int32,
                ["MaxPairwise.Vector64.SByte"] = MaxPairwise_Vector64_SByte,
                ["MaxPairwise.Vector64.Single"] = MaxPairwise_Vector64_Single,
                ["MaxPairwise.Vector64.UInt16"] = MaxPairwise_Vector64_UInt16,
                ["MaxPairwise.Vector64.UInt32"] = MaxPairwise_Vector64_UInt32,
                ["Min.Vector64.Byte"] = Min_Vector64_Byte,
                ["Min.Vector64.Int16"] = Min_Vector64_Int16,
                ["Min.Vector64.Int32"] = Min_Vector64_Int32,
                ["Min.Vector64.SByte"] = Min_Vector64_SByte,
                ["Min.Vector64.Single"] = Min_Vector64_Single,
                ["Min.Vector64.UInt16"] = Min_Vector64_UInt16,
                ["Min.Vector64.UInt32"] = Min_Vector64_UInt32,
                ["Min.Vector128.Byte"] = Min_Vector128_Byte,
                ["Min.Vector128.Int16"] = Min_Vector128_Int16,
                ["Min.Vector128.Int32"] = Min_Vector128_Int32,
                ["Min.Vector128.SByte"] = Min_Vector128_SByte,
                ["Min.Vector128.Single"] = Min_Vector128_Single,
                ["Min.Vector128.UInt16"] = Min_Vector128_UInt16,
                ["Min.Vector128.UInt32"] = Min_Vector128_UInt32,
                ["MinNumber.Vector64.Single"] = MinNumber_Vector64_Single,
                ["MinNumber.Vector128.Single"] = MinNumber_Vector128_Single,
                ["MinNumberScalar.Vector64.Double"] = MinNumberScalar_Vector64_Double,
                ["MinNumberScalar.Vector64.Single"] = MinNumberScalar_Vector64_Single,
                ["MinPairwise.Vector64.Byte"] = MinPairwise_Vector64_Byte,
                ["MinPairwise.Vector64.Int16"] = MinPairwise_Vector64_Int16,
                ["MinPairwise.Vector64.Int32"] = MinPairwise_Vector64_Int32,
                ["MinPairwise.Vector64.SByte"] = MinPairwise_Vector64_SByte,
                ["MinPairwise.Vector64.Single"] = MinPairwise_Vector64_Single,
                ["MinPairwise.Vector64.UInt16"] = MinPairwise_Vector64_UInt16,
                ["MinPairwise.Vector64.UInt32"] = MinPairwise_Vector64_UInt32,
                ["Multiply.Vector64.Byte"] = Multiply_Vector64_Byte,
                ["Multiply.Vector64.Int16"] = Multiply_Vector64_Int16,
                ["Multiply.Vector64.Int32"] = Multiply_Vector64_Int32,
                ["Multiply.Vector64.SByte"] = Multiply_Vector64_SByte,
                ["Multiply.Vector64.Single"] = Multiply_Vector64_Single,
                ["Multiply.Vector64.UInt16"] = Multiply_Vector64_UInt16,
                ["Multiply.Vector64.UInt32"] = Multiply_Vector64_UInt32,
                ["Multiply.Vector128.Byte"] = Multiply_Vector128_Byte,
                ["Multiply.Vector128.Int16"] = Multiply_Vector128_Int16,
                ["Multiply.Vector128.Int32"] = Multiply_Vector128_Int32,
                ["Multiply.Vector128.SByte"] = Multiply_Vector128_SByte,
                ["Multiply.Vector128.Single"] = Multiply_Vector128_Single,
                ["Multiply.Vector128.UInt16"] = Multiply_Vector128_UInt16,
                ["Multiply.Vector128.UInt32"] = Multiply_Vector128_UInt32,
                ["MultiplyScalar.Vector64.Double"] = MultiplyScalar_Vector64_Double,
                ["MultiplyScalar.Vector64.Single"] = MultiplyScalar_Vector64_Single,
                ["MultiplyAdd.Vector64.Byte"] = MultiplyAdd_Vector64_Byte,
                ["MultiplyAdd.Vector64.Int16"] = MultiplyAdd_Vector64_Int16,
                ["MultiplyAdd.Vector64.Int32"] = MultiplyAdd_Vector64_Int32,
                ["MultiplyAdd.Vector64.SByte"] = MultiplyAdd_Vector64_SByte,
                ["MultiplyAdd.Vector64.UInt16"] = MultiplyAdd_Vector64_UInt16,
                ["MultiplyAdd.Vector64.UInt32"] = MultiplyAdd_Vector64_UInt32,
                ["MultiplyAdd.Vector128.Byte"] = MultiplyAdd_Vector128_Byte,
                ["MultiplyAdd.Vector128.Int16"] = MultiplyAdd_Vector128_Int16,
                ["MultiplyAdd.Vector128.Int32"] = MultiplyAdd_Vector128_Int32,
                ["MultiplyAdd.Vector128.SByte"] = MultiplyAdd_Vector128_SByte,
                ["MultiplyAdd.Vector128.UInt16"] = MultiplyAdd_Vector128_UInt16,
                ["MultiplyAdd.Vector128.UInt32"] = MultiplyAdd_Vector128_UInt32,
                ["MultiplyAddByScalar.Vector64.Int16"] = MultiplyAddByScalar_Vector64_Int16,
                ["MultiplyAddByScalar.Vector64.Int32"] = MultiplyAddByScalar_Vector64_Int32,
                ["MultiplyAddByScalar.Vector64.UInt16"] = MultiplyAddByScalar_Vector64_UInt16,
                ["MultiplyAddByScalar.Vector64.UInt32"] = MultiplyAddByScalar_Vector64_UInt32,
                ["MultiplyAddByScalar.Vector128.Int16"] = MultiplyAddByScalar_Vector128_Int16,
                ["MultiplyAddByScalar.Vector128.Int32"] = MultiplyAddByScalar_Vector128_Int32,
                ["MultiplyAddByScalar.Vector128.UInt16"] = MultiplyAddByScalar_Vector128_UInt16,
                ["MultiplyAddByScalar.Vector128.UInt32"] = MultiplyAddByScalar_Vector128_UInt32,
                ["MultiplyAddBySelectedScalar.Vector64.Int16.Vector64.Int16.3"] = MultiplyAddBySelectedScalar_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyAddBySelectedScalar.Vector64.Int16.Vector128.Int16.7"] = MultiplyAddBySelectedScalar_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyAddBySelectedScalar.Vector64.Int32.Vector64.Int32.1"] = MultiplyAddBySelectedScalar_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyAddBySelectedScalar.Vector64.Int32.Vector128.Int32.3"] = MultiplyAddBySelectedScalar_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyAddBySelectedScalar.Vector64.UInt16.Vector64.UInt16.3"] = MultiplyAddBySelectedScalar_Vector64_UInt16_Vector64_UInt16_3,
                ["MultiplyAddBySelectedScalar.Vector64.UInt16.Vector128.UInt16.7"] = MultiplyAddBySelectedScalar_Vector64_UInt16_Vector128_UInt16_7,
                ["MultiplyAddBySelectedScalar.Vector64.UInt32.Vector64.UInt32.1"] = MultiplyAddBySelectedScalar_Vector64_UInt32_Vector64_UInt32_1,
                ["MultiplyAddBySelectedScalar.Vector64.UInt32.Vector128.UInt32.3"] = MultiplyAddBySelectedScalar_Vector64_UInt32_Vector128_UInt32_3,
                ["MultiplyAddBySelectedScalar.Vector128.Int16.Vector64.Int16.3"] = MultiplyAddBySelectedScalar_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyAddBySelectedScalar.Vector128.Int16.Vector128.Int16.7"] = MultiplyAddBySelectedScalar_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyAddBySelectedScalar.Vector128.Int32.Vector64.Int32.1"] = MultiplyAddBySelectedScalar_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyAddBySelectedScalar.Vector128.Int32.Vector128.Int32.3"] = MultiplyAddBySelectedScalar_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyAddBySelectedScalar.Vector128.UInt16.Vector64.UInt16.3"] = MultiplyAddBySelectedScalar_Vector128_UInt16_Vector64_UInt16_3,
                ["MultiplyAddBySelectedScalar.Vector128.UInt16.Vector128.UInt16.7"] = MultiplyAddBySelectedScalar_Vector128_UInt16_Vector128_UInt16_7,
                ["MultiplyAddBySelectedScalar.Vector128.UInt32.Vector64.UInt32.1"] = MultiplyAddBySelectedScalar_Vector128_UInt32_Vector64_UInt32_1,
                ["MultiplyAddBySelectedScalar.Vector128.UInt32.Vector128.UInt32.3"] = MultiplyAddBySelectedScalar_Vector128_UInt32_Vector128_UInt32_3,
                ["MultiplyByScalar.Vector64.Int16"] = MultiplyByScalar_Vector64_Int16,
                ["MultiplyByScalar.Vector64.Int32"] = MultiplyByScalar_Vector64_Int32,
                ["MultiplyByScalar.Vector64.Single"] = MultiplyByScalar_Vector64_Single,
                ["MultiplyByScalar.Vector64.UInt16"] = MultiplyByScalar_Vector64_UInt16,
                ["MultiplyByScalar.Vector64.UInt32"] = MultiplyByScalar_Vector64_UInt32,
                ["MultiplyByScalar.Vector128.Int16"] = MultiplyByScalar_Vector128_Int16,
                ["MultiplyByScalar.Vector128.Int32"] = MultiplyByScalar_Vector128_Int32,
                ["MultiplyByScalar.Vector128.Single"] = MultiplyByScalar_Vector128_Single,
                ["MultiplyByScalar.Vector128.UInt16"] = MultiplyByScalar_Vector128_UInt16,
                ["MultiplyByScalar.Vector128.UInt32"] = MultiplyByScalar_Vector128_UInt32,
                ["MultiplyBySelectedScalar.Vector64.Int16.Vector64.Int16.1"] = MultiplyBySelectedScalar_Vector64_Int16_Vector64_Int16_1,
                ["MultiplyBySelectedScalar.Vector64.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalar_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalar.Vector64.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalar_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalar.Vector64.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalar_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalar.Vector64.Single.Vector64.Single.1"] = MultiplyBySelectedScalar_Vector64_Single_Vector64_Single_1,
                ["MultiplyBySelectedScalar.Vector64.Single.Vector128.Single.3"] = MultiplyBySelectedScalar_Vector64_Single_Vector128_Single_3,
                ["MultiplyBySelectedScalar.Vector64.UInt16.Vector64.UInt16.1"] = MultiplyBySelectedScalar_Vector64_UInt16_Vector64_UInt16_1,
                ["MultiplyBySelectedScalar.Vector64.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalar_Vector64_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalar.Vector64.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalar_Vector64_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalar.Vector64.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalar_Vector64_UInt32_Vector128_UInt32_3,
                ["MultiplyBySelectedScalar.Vector128.Int16.Vector64.Int16.1"] = MultiplyBySelectedScalar_Vector128_Int16_Vector64_Int16_1,
                ["MultiplyBySelectedScalar.Vector128.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalar_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalar.Vector128.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalar_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalar.Vector128.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalar_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalar.Vector128.Single.Vector64.Single.1"] = MultiplyBySelectedScalar_Vector128_Single_Vector64_Single_1,
                ["MultiplyBySelectedScalar.Vector128.Single.Vector128.Single.3"] = MultiplyBySelectedScalar_Vector128_Single_Vector128_Single_3,
                ["MultiplyBySelectedScalar.Vector128.UInt16.Vector64.UInt16.1"] = MultiplyBySelectedScalar_Vector128_UInt16_Vector64_UInt16_1,
                ["MultiplyBySelectedScalar.Vector128.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalar_Vector128_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalar.Vector128.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalar_Vector128_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalar.Vector128.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalar_Vector128_UInt32_Vector128_UInt32_3,
            };
        }
    }
}
