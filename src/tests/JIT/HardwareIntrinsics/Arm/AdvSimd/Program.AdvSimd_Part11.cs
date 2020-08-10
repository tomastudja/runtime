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
                ["RoundToNearestScalar.Vector64.Double"] = RoundToNearestScalar_Vector64_Double,
                ["RoundToNearestScalar.Vector64.Single"] = RoundToNearestScalar_Vector64_Single,
                ["RoundToNegativeInfinity.Vector64.Single"] = RoundToNegativeInfinity_Vector64_Single,
                ["RoundToNegativeInfinity.Vector128.Single"] = RoundToNegativeInfinity_Vector128_Single,
                ["RoundToNegativeInfinityScalar.Vector64.Double"] = RoundToNegativeInfinityScalar_Vector64_Double,
                ["RoundToNegativeInfinityScalar.Vector64.Single"] = RoundToNegativeInfinityScalar_Vector64_Single,
                ["RoundToPositiveInfinity.Vector64.Single"] = RoundToPositiveInfinity_Vector64_Single,
                ["RoundToPositiveInfinity.Vector128.Single"] = RoundToPositiveInfinity_Vector128_Single,
                ["RoundToPositiveInfinityScalar.Vector64.Double"] = RoundToPositiveInfinityScalar_Vector64_Double,
                ["RoundToPositiveInfinityScalar.Vector64.Single"] = RoundToPositiveInfinityScalar_Vector64_Single,
                ["RoundToZero.Vector64.Single"] = RoundToZero_Vector64_Single,
                ["RoundToZero.Vector128.Single"] = RoundToZero_Vector128_Single,
                ["RoundToZeroScalar.Vector64.Double"] = RoundToZeroScalar_Vector64_Double,
                ["RoundToZeroScalar.Vector64.Single"] = RoundToZeroScalar_Vector64_Single,
                ["ShiftArithmetic.Vector64.Int16"] = ShiftArithmetic_Vector64_Int16,
                ["ShiftArithmetic.Vector64.Int32"] = ShiftArithmetic_Vector64_Int32,
                ["ShiftArithmetic.Vector64.SByte"] = ShiftArithmetic_Vector64_SByte,
                ["ShiftArithmetic.Vector128.Int16"] = ShiftArithmetic_Vector128_Int16,
                ["ShiftArithmetic.Vector128.Int32"] = ShiftArithmetic_Vector128_Int32,
                ["ShiftArithmetic.Vector128.Int64"] = ShiftArithmetic_Vector128_Int64,
                ["ShiftArithmetic.Vector128.SByte"] = ShiftArithmetic_Vector128_SByte,
                ["ShiftArithmeticRounded.Vector64.Int16"] = ShiftArithmeticRounded_Vector64_Int16,
                ["ShiftArithmeticRounded.Vector64.Int32"] = ShiftArithmeticRounded_Vector64_Int32,
                ["ShiftArithmeticRounded.Vector64.SByte"] = ShiftArithmeticRounded_Vector64_SByte,
                ["ShiftArithmeticRounded.Vector128.Int16"] = ShiftArithmeticRounded_Vector128_Int16,
                ["ShiftArithmeticRounded.Vector128.Int32"] = ShiftArithmeticRounded_Vector128_Int32,
                ["ShiftArithmeticRounded.Vector128.Int64"] = ShiftArithmeticRounded_Vector128_Int64,
                ["ShiftArithmeticRounded.Vector128.SByte"] = ShiftArithmeticRounded_Vector128_SByte,
                ["ShiftArithmeticRoundedSaturate.Vector64.Int16"] = ShiftArithmeticRoundedSaturate_Vector64_Int16,
                ["ShiftArithmeticRoundedSaturate.Vector64.Int32"] = ShiftArithmeticRoundedSaturate_Vector64_Int32,
                ["ShiftArithmeticRoundedSaturate.Vector64.SByte"] = ShiftArithmeticRoundedSaturate_Vector64_SByte,
                ["ShiftArithmeticRoundedSaturate.Vector128.Int16"] = ShiftArithmeticRoundedSaturate_Vector128_Int16,
                ["ShiftArithmeticRoundedSaturate.Vector128.Int32"] = ShiftArithmeticRoundedSaturate_Vector128_Int32,
                ["ShiftArithmeticRoundedSaturate.Vector128.Int64"] = ShiftArithmeticRoundedSaturate_Vector128_Int64,
                ["ShiftArithmeticRoundedSaturate.Vector128.SByte"] = ShiftArithmeticRoundedSaturate_Vector128_SByte,
                ["ShiftArithmeticRoundedSaturateScalar.Vector64.Int64"] = ShiftArithmeticRoundedSaturateScalar_Vector64_Int64,
                ["ShiftArithmeticRoundedScalar.Vector64.Int64"] = ShiftArithmeticRoundedScalar_Vector64_Int64,
                ["ShiftArithmeticSaturate.Vector64.Int16"] = ShiftArithmeticSaturate_Vector64_Int16,
                ["ShiftArithmeticSaturate.Vector64.Int32"] = ShiftArithmeticSaturate_Vector64_Int32,
                ["ShiftArithmeticSaturate.Vector64.SByte"] = ShiftArithmeticSaturate_Vector64_SByte,
                ["ShiftArithmeticSaturate.Vector128.Int16"] = ShiftArithmeticSaturate_Vector128_Int16,
                ["ShiftArithmeticSaturate.Vector128.Int32"] = ShiftArithmeticSaturate_Vector128_Int32,
                ["ShiftArithmeticSaturate.Vector128.Int64"] = ShiftArithmeticSaturate_Vector128_Int64,
                ["ShiftArithmeticSaturate.Vector128.SByte"] = ShiftArithmeticSaturate_Vector128_SByte,
                ["ShiftArithmeticSaturateScalar.Vector64.Int64"] = ShiftArithmeticSaturateScalar_Vector64_Int64,
                ["ShiftArithmeticScalar.Vector64.Int64"] = ShiftArithmeticScalar_Vector64_Int64,
                ["ShiftLeftAndInsert.Vector64.Byte"] = ShiftLeftAndInsert_Vector64_Byte,
                ["ShiftLeftAndInsert.Vector64.Int16"] = ShiftLeftAndInsert_Vector64_Int16,
                ["ShiftLeftAndInsert.Vector64.Int32"] = ShiftLeftAndInsert_Vector64_Int32,
                ["ShiftLeftAndInsert.Vector64.SByte"] = ShiftLeftAndInsert_Vector64_SByte,
                ["ShiftLeftAndInsert.Vector64.UInt16"] = ShiftLeftAndInsert_Vector64_UInt16,
                ["ShiftLeftAndInsert.Vector64.UInt32"] = ShiftLeftAndInsert_Vector64_UInt32,
                ["ShiftLeftAndInsert.Vector128.Byte"] = ShiftLeftAndInsert_Vector128_Byte,
                ["ShiftLeftAndInsert.Vector128.Int16"] = ShiftLeftAndInsert_Vector128_Int16,
                ["ShiftLeftAndInsert.Vector128.Int32"] = ShiftLeftAndInsert_Vector128_Int32,
                ["ShiftLeftAndInsert.Vector128.Int64"] = ShiftLeftAndInsert_Vector128_Int64,
                ["ShiftLeftAndInsert.Vector128.SByte"] = ShiftLeftAndInsert_Vector128_SByte,
                ["ShiftLeftAndInsert.Vector128.UInt16"] = ShiftLeftAndInsert_Vector128_UInt16,
                ["ShiftLeftAndInsert.Vector128.UInt32"] = ShiftLeftAndInsert_Vector128_UInt32,
                ["ShiftLeftAndInsert.Vector128.UInt64"] = ShiftLeftAndInsert_Vector128_UInt64,
                ["ShiftLeftAndInsertScalar.Vector64.Int64"] = ShiftLeftAndInsertScalar_Vector64_Int64,
                ["ShiftLeftAndInsertScalar.Vector64.UInt64"] = ShiftLeftAndInsertScalar_Vector64_UInt64,
                ["ShiftLeftLogical.Vector64.Byte.1"] = ShiftLeftLogical_Vector64_Byte_1,
                ["ShiftLeftLogical.Vector64.Int16.1"] = ShiftLeftLogical_Vector64_Int16_1,
                ["ShiftLeftLogical.Vector64.Int32.1"] = ShiftLeftLogical_Vector64_Int32_1,
                ["ShiftLeftLogical.Vector64.SByte.1"] = ShiftLeftLogical_Vector64_SByte_1,
                ["ShiftLeftLogical.Vector64.UInt16.1"] = ShiftLeftLogical_Vector64_UInt16_1,
                ["ShiftLeftLogical.Vector64.UInt32.1"] = ShiftLeftLogical_Vector64_UInt32_1,
                ["ShiftLeftLogical.Vector128.Byte.1"] = ShiftLeftLogical_Vector128_Byte_1,
                ["ShiftLeftLogical.Vector128.Int16.1"] = ShiftLeftLogical_Vector128_Int16_1,
                ["ShiftLeftLogical.Vector128.Int64.1"] = ShiftLeftLogical_Vector128_Int64_1,
                ["ShiftLeftLogical.Vector128.SByte.1"] = ShiftLeftLogical_Vector128_SByte_1,
                ["ShiftLeftLogical.Vector128.UInt16.1"] = ShiftLeftLogical_Vector128_UInt16_1,
                ["ShiftLeftLogical.Vector128.UInt32.1"] = ShiftLeftLogical_Vector128_UInt32_1,
                ["ShiftLeftLogical.Vector128.UInt64.1"] = ShiftLeftLogical_Vector128_UInt64_1,
                ["ShiftLeftLogicalSaturate.Vector64.Byte.1"] = ShiftLeftLogicalSaturate_Vector64_Byte_1,
                ["ShiftLeftLogicalSaturate.Vector64.Int16.1"] = ShiftLeftLogicalSaturate_Vector64_Int16_1,
                ["ShiftLeftLogicalSaturate.Vector64.Int32.1"] = ShiftLeftLogicalSaturate_Vector64_Int32_1,
                ["ShiftLeftLogicalSaturate.Vector64.SByte.1"] = ShiftLeftLogicalSaturate_Vector64_SByte_1,
                ["ShiftLeftLogicalSaturate.Vector64.UInt16.1"] = ShiftLeftLogicalSaturate_Vector64_UInt16_1,
                ["ShiftLeftLogicalSaturate.Vector64.UInt32.1"] = ShiftLeftLogicalSaturate_Vector64_UInt32_1,
                ["ShiftLeftLogicalSaturate.Vector128.Byte.1"] = ShiftLeftLogicalSaturate_Vector128_Byte_1,
                ["ShiftLeftLogicalSaturate.Vector128.Int16.1"] = ShiftLeftLogicalSaturate_Vector128_Int16_1,
                ["ShiftLeftLogicalSaturate.Vector128.Int32.1"] = ShiftLeftLogicalSaturate_Vector128_Int32_1,
                ["ShiftLeftLogicalSaturate.Vector128.Int64.1"] = ShiftLeftLogicalSaturate_Vector128_Int64_1,
                ["ShiftLeftLogicalSaturate.Vector128.SByte.1"] = ShiftLeftLogicalSaturate_Vector128_SByte_1,
                ["ShiftLeftLogicalSaturate.Vector128.UInt16.1"] = ShiftLeftLogicalSaturate_Vector128_UInt16_1,
                ["ShiftLeftLogicalSaturate.Vector128.UInt32.1"] = ShiftLeftLogicalSaturate_Vector128_UInt32_1,
                ["ShiftLeftLogicalSaturate.Vector128.UInt64.1"] = ShiftLeftLogicalSaturate_Vector128_UInt64_1,
                ["ShiftLeftLogicalSaturateScalar.Vector64.Int64.1"] = ShiftLeftLogicalSaturateScalar_Vector64_Int64_1,
                ["ShiftLeftLogicalSaturateScalar.Vector64.UInt64.1"] = ShiftLeftLogicalSaturateScalar_Vector64_UInt64_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector64.Int16.1"] = ShiftLeftLogicalSaturateUnsigned_Vector64_Int16_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector64.Int32.1"] = ShiftLeftLogicalSaturateUnsigned_Vector64_Int32_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector64.SByte.1"] = ShiftLeftLogicalSaturateUnsigned_Vector64_SByte_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector128.Int16.1"] = ShiftLeftLogicalSaturateUnsigned_Vector128_Int16_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector128.Int32.1"] = ShiftLeftLogicalSaturateUnsigned_Vector128_Int32_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector128.Int64.1"] = ShiftLeftLogicalSaturateUnsigned_Vector128_Int64_1,
                ["ShiftLeftLogicalSaturateUnsigned.Vector128.SByte.1"] = ShiftLeftLogicalSaturateUnsigned_Vector128_SByte_1,
                ["ShiftLeftLogicalSaturateUnsignedScalar.Vector64.Int64.1"] = ShiftLeftLogicalSaturateUnsignedScalar_Vector64_Int64_1,
                ["ShiftLeftLogicalScalar.Vector64.Int64.1"] = ShiftLeftLogicalScalar_Vector64_Int64_1,
            };
        }
    }
}
