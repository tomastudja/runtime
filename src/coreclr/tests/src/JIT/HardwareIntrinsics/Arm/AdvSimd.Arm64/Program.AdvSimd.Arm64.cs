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
                ["AddAcross.Vector64.SByte"] = AddAcross_Vector64_SByte,
                ["AddAcross.Vector64.UInt16"] = AddAcross_Vector64_UInt16,
                ["AddAcross.Vector64.Int16"] = AddAcross_Vector64_Int16,
                ["AddAcross.Vector128.Byte"] = AddAcross_Vector128_Byte,
                ["AddAcross.Vector128.SByte"] = AddAcross_Vector128_SByte,
                ["AddAcross.Vector128.UInt16"] = AddAcross_Vector128_UInt16,
                ["AddAcross.Vector128.Int16"] = AddAcross_Vector128_Int16,
                ["AddAcross.Vector128.UInt32"] = AddAcross_Vector128_UInt32,
                ["AddAcross.Vector128.Int32"] = AddAcross_Vector128_Int32,
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
                ["Max.Vector128.Double"] = Max_Vector128_Double,
                ["MaxScalar.Vector64.Double"] = MaxScalar_Vector64_Double,
                ["MaxScalar.Vector64.Single"] = MaxScalar_Vector64_Single,
                ["Min.Vector128.Double"] = Min_Vector128_Double,
                ["MinScalar.Vector64.Double"] = MinScalar_Vector64_Double,
                ["MinScalar.Vector64.Single"] = MinScalar_Vector64_Single,
                ["Multiply.Vector128.Double"] = Multiply_Vector128_Double,
                ["Negate.Vector128.Double"] = Negate_Vector128_Double,
                ["Negate.Vector128.Int64"] = Negate_Vector128_Int64,
                ["NegateScalar.Vector64.Int64"] = NegateScalar_Vector64_Int64,
                ["ReverseElementBits.Vector128.Byte"] = ReverseElementBits_Vector128_Byte,
                ["ReverseElementBits.Vector128.SByte"] = ReverseElementBits_Vector128_SByte,
                ["ReverseElementBits.Vector64.Byte"] = ReverseElementBits_Vector64_Byte,
                ["ReverseElementBits.Vector64.SByte"] = ReverseElementBits_Vector64_SByte,
                ["Sqrt.Vector64.Single"] = Sqrt_Vector64_Single,
                ["Sqrt.Vector128.Double"] = Sqrt_Vector128_Double,
                ["Sqrt.Vector128.Single"] = Sqrt_Vector128_Single,
                ["Subtract.Vector128.Double"] = Subtract_Vector128_Double,
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
