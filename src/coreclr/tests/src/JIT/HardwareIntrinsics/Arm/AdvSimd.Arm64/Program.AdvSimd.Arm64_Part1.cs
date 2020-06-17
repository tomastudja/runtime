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
                ["ReverseElementBits.Vector64.SByte"] = ReverseElementBits_Vector64_SByte,
                ["ShiftArithmeticRoundedSaturateScalar.Vector64.Int16"] = ShiftArithmeticRoundedSaturateScalar_Vector64_Int16,
                ["ShiftArithmeticRoundedSaturateScalar.Vector64.Int32"] = ShiftArithmeticRoundedSaturateScalar_Vector64_Int32,
                ["ShiftArithmeticRoundedSaturateScalar.Vector64.SByte"] = ShiftArithmeticRoundedSaturateScalar_Vector64_SByte,
                ["ShiftArithmeticSaturateScalar.Vector64.Int16"] = ShiftArithmeticSaturateScalar_Vector64_Int16,
                ["ShiftArithmeticSaturateScalar.Vector64.Int32"] = ShiftArithmeticSaturateScalar_Vector64_Int32,
                ["ShiftArithmeticSaturateScalar.Vector64.SByte"] = ShiftArithmeticSaturateScalar_Vector64_SByte,
                ["ShiftLeftLogicalSaturateScalar.Vector64.Byte.7"] = ShiftLeftLogicalSaturateScalar_Vector64_Byte_7,
                ["ShiftLeftLogicalSaturateScalar.Vector64.Int16.15"] = ShiftLeftLogicalSaturateScalar_Vector64_Int16_15,
                ["ShiftLeftLogicalSaturateScalar.Vector64.Int32.31"] = ShiftLeftLogicalSaturateScalar_Vector64_Int32_31,
                ["ShiftLeftLogicalSaturateScalar.Vector64.SByte.1"] = ShiftLeftLogicalSaturateScalar_Vector64_SByte_1,
                ["ShiftLeftLogicalSaturateScalar.Vector64.UInt16.1"] = ShiftLeftLogicalSaturateScalar_Vector64_UInt16_1,
                ["ShiftLeftLogicalSaturateScalar.Vector64.UInt32.1"] = ShiftLeftLogicalSaturateScalar_Vector64_UInt32_1,
                ["ShiftLeftLogicalSaturateUnsignedScalar.Vector64.Int16.5"] = ShiftLeftLogicalSaturateUnsignedScalar_Vector64_Int16_5,
                ["ShiftLeftLogicalSaturateUnsignedScalar.Vector64.Int32.7"] = ShiftLeftLogicalSaturateUnsignedScalar_Vector64_Int32_7,
                ["ShiftLeftLogicalSaturateUnsignedScalar.Vector64.SByte.3"] = ShiftLeftLogicalSaturateUnsignedScalar_Vector64_SByte_3,
                ["ShiftLogicalRoundedSaturateScalar.Vector64.Byte"] = ShiftLogicalRoundedSaturateScalar_Vector64_Byte,
                ["ShiftLogicalRoundedSaturateScalar.Vector64.Int16"] = ShiftLogicalRoundedSaturateScalar_Vector64_Int16,
                ["ShiftLogicalRoundedSaturateScalar.Vector64.Int32"] = ShiftLogicalRoundedSaturateScalar_Vector64_Int32,
                ["ShiftLogicalRoundedSaturateScalar.Vector64.SByte"] = ShiftLogicalRoundedSaturateScalar_Vector64_SByte,
                ["ShiftLogicalRoundedSaturateScalar.Vector64.UInt16"] = ShiftLogicalRoundedSaturateScalar_Vector64_UInt16,
                ["ShiftLogicalRoundedSaturateScalar.Vector64.UInt32"] = ShiftLogicalRoundedSaturateScalar_Vector64_UInt32,
                ["ShiftLogicalSaturateScalar.Vector64.Byte"] = ShiftLogicalSaturateScalar_Vector64_Byte,
                ["ShiftLogicalSaturateScalar.Vector64.Int16"] = ShiftLogicalSaturateScalar_Vector64_Int16,
                ["ShiftLogicalSaturateScalar.Vector64.Int32"] = ShiftLogicalSaturateScalar_Vector64_Int32,
                ["ShiftLogicalSaturateScalar.Vector64.SByte"] = ShiftLogicalSaturateScalar_Vector64_SByte,
                ["ShiftLogicalSaturateScalar.Vector64.UInt16"] = ShiftLogicalSaturateScalar_Vector64_UInt16,
                ["ShiftLogicalSaturateScalar.Vector64.UInt32"] = ShiftLogicalSaturateScalar_Vector64_UInt32,
                ["ShiftRightArithmeticNarrowingSaturateScalar.Vector64.Int16.16"] = ShiftRightArithmeticNarrowingSaturateScalar_Vector64_Int16_16,
                ["ShiftRightArithmeticNarrowingSaturateScalar.Vector64.Int32.32"] = ShiftRightArithmeticNarrowingSaturateScalar_Vector64_Int32_32,
                ["ShiftRightArithmeticNarrowingSaturateScalar.Vector64.SByte.8"] = ShiftRightArithmeticNarrowingSaturateScalar_Vector64_SByte_8,
                ["ShiftRightArithmeticNarrowingSaturateUnsignedScalar.Vector64.Byte.3"] = ShiftRightArithmeticNarrowingSaturateUnsignedScalar_Vector64_Byte_3,
                ["ShiftRightArithmeticNarrowingSaturateUnsignedScalar.Vector64.UInt16.5"] = ShiftRightArithmeticNarrowingSaturateUnsignedScalar_Vector64_UInt16_5,
                ["ShiftRightArithmeticNarrowingSaturateUnsignedScalar.Vector64.UInt32.7"] = ShiftRightArithmeticNarrowingSaturateUnsignedScalar_Vector64_UInt32_7,
                ["ShiftRightArithmeticRoundedNarrowingSaturateScalar.Vector64.Int16.32"] = ShiftRightArithmeticRoundedNarrowingSaturateScalar_Vector64_Int16_32,
                ["ShiftRightArithmeticRoundedNarrowingSaturateScalar.Vector64.Int32.64"] = ShiftRightArithmeticRoundedNarrowingSaturateScalar_Vector64_Int32_64,
                ["ShiftRightArithmeticRoundedNarrowingSaturateScalar.Vector64.SByte.16"] = ShiftRightArithmeticRoundedNarrowingSaturateScalar_Vector64_SByte_16,
                ["ShiftRightArithmeticRoundedNarrowingSaturateUnsignedScalar.Vector64.Byte.1"] = ShiftRightArithmeticRoundedNarrowingSaturateUnsignedScalar_Vector64_Byte_1,
                ["ShiftRightArithmeticRoundedNarrowingSaturateUnsignedScalar.Vector64.UInt16.1"] = ShiftRightArithmeticRoundedNarrowingSaturateUnsignedScalar_Vector64_UInt16_1,
                ["ShiftRightArithmeticRoundedNarrowingSaturateUnsignedScalar.Vector64.UInt32.1"] = ShiftRightArithmeticRoundedNarrowingSaturateUnsignedScalar_Vector64_UInt32_1,
                ["ShiftRightLogicalNarrowingSaturateScalar.Vector64.Byte.5"] = ShiftRightLogicalNarrowingSaturateScalar_Vector64_Byte_5,
                ["ShiftRightLogicalNarrowingSaturateScalar.Vector64.Int16.7"] = ShiftRightLogicalNarrowingSaturateScalar_Vector64_Int16_7,
                ["ShiftRightLogicalNarrowingSaturateScalar.Vector64.Int32.11"] = ShiftRightLogicalNarrowingSaturateScalar_Vector64_Int32_11,
                ["ShiftRightLogicalNarrowingSaturateScalar.Vector64.SByte.3"] = ShiftRightLogicalNarrowingSaturateScalar_Vector64_SByte_3,
                ["ShiftRightLogicalNarrowingSaturateScalar.Vector64.UInt16.5"] = ShiftRightLogicalNarrowingSaturateScalar_Vector64_UInt16_5,
                ["ShiftRightLogicalNarrowingSaturateScalar.Vector64.UInt32.7"] = ShiftRightLogicalNarrowingSaturateScalar_Vector64_UInt32_7,
                ["ShiftRightLogicalRoundedNarrowingSaturateScalar.Vector64.Byte.1"] = ShiftRightLogicalRoundedNarrowingSaturateScalar_Vector64_Byte_1,
                ["ShiftRightLogicalRoundedNarrowingSaturateScalar.Vector64.Int16.1"] = ShiftRightLogicalRoundedNarrowingSaturateScalar_Vector64_Int16_1,
                ["ShiftRightLogicalRoundedNarrowingSaturateScalar.Vector64.Int32.1"] = ShiftRightLogicalRoundedNarrowingSaturateScalar_Vector64_Int32_1,
                ["ShiftRightLogicalRoundedNarrowingSaturateScalar.Vector64.SByte.1"] = ShiftRightLogicalRoundedNarrowingSaturateScalar_Vector64_SByte_1,
                ["ShiftRightLogicalRoundedNarrowingSaturateScalar.Vector64.UInt16.1"] = ShiftRightLogicalRoundedNarrowingSaturateScalar_Vector64_UInt16_1,
                ["ShiftRightLogicalRoundedNarrowingSaturateScalar.Vector64.UInt32.1"] = ShiftRightLogicalRoundedNarrowingSaturateScalar_Vector64_UInt32_1,
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
