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
