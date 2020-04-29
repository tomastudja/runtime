// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;

namespace JIT.HardwareIntrinsics.General
{
    public static partial class Program
    {
        static Program()
        {
            TestList = new Dictionary<string, Action>() {
                ["Zero.Byte"] = ZeroByte,
                ["Zero.Double"] = ZeroDouble,
                ["Zero.Int16"] = ZeroInt16,
                ["Zero.Int32"] = ZeroInt32,
                ["Zero.Int64"] = ZeroInt64,
                ["Zero.SByte"] = ZeroSByte,
                ["Zero.Single"] = ZeroSingle,
                ["Zero.UInt16"] = ZeroUInt16,
                ["Zero.UInt32"] = ZeroUInt32,
                ["Zero.UInt64"] = ZeroUInt64,
                ["AllBitsSet.Byte"] = AllBitsSetByte,
                ["AllBitsSet.Double"] = AllBitsSetDouble,
                ["AllBitsSet.Int16"] = AllBitsSetInt16,
                ["AllBitsSet.Int32"] = AllBitsSetInt32,
                ["AllBitsSet.Int64"] = AllBitsSetInt64,
                ["AllBitsSet.SByte"] = AllBitsSetSByte,
                ["AllBitsSet.Single"] = AllBitsSetSingle,
                ["AllBitsSet.UInt16"] = AllBitsSetUInt16,
                ["AllBitsSet.UInt32"] = AllBitsSetUInt32,
                ["AllBitsSet.UInt64"] = AllBitsSetUInt64,
                ["As.Byte"] = AsByte,
                ["As.Double"] = AsDouble,
                ["As.Int16"] = AsInt16,
                ["As.Int32"] = AsInt32,
                ["As.Int64"] = AsInt64,
                ["As.SByte"] = AsSByte,
                ["As.Single"] = AsSingle,
                ["As.UInt16"] = AsUInt16,
                ["As.UInt32"] = AsUInt32,
                ["As.UInt64"] = AsUInt64,
                ["AsVector.Byte"] = AsVectorByte,
                ["AsVector.Double"] = AsVectorDouble,
                ["AsVector.Int16"] = AsVectorInt16,
                ["AsVector.Int32"] = AsVectorInt32,
                ["AsVector.Int64"] = AsVectorInt64,
                ["AsVector.SByte"] = AsVectorSByte,
                ["AsVector.Single"] = AsVectorSingle,
                ["AsVector.UInt16"] = AsVectorUInt16,
                ["AsVector.UInt32"] = AsVectorUInt32,
                ["AsVector.UInt64"] = AsVectorUInt64,
                ["GetAndWithElement.Byte.0"] = GetAndWithElementByte0,
                ["GetAndWithElement.Byte.7"] = GetAndWithElementByte7,
                ["GetAndWithElement.Byte.15"] = GetAndWithElementByte15,
                ["GetAndWithElement.Byte.31"] = GetAndWithElementByte31,
                ["GetAndWithElement.Double.0"] = GetAndWithElementDouble0,
                ["GetAndWithElement.Double.1"] = GetAndWithElementDouble1,
                ["GetAndWithElement.Double.3"] = GetAndWithElementDouble3,
                ["GetAndWithElement.Int16.0"] = GetAndWithElementInt160,
                ["GetAndWithElement.Int16.3"] = GetAndWithElementInt163,
                ["GetAndWithElement.Int16.7"] = GetAndWithElementInt167,
                ["GetAndWithElement.Int16.15"] = GetAndWithElementInt1615,
                ["GetAndWithElement.Int32.0"] = GetAndWithElementInt320,
                ["GetAndWithElement.Int32.1"] = GetAndWithElementInt321,
                ["GetAndWithElement.Int32.3"] = GetAndWithElementInt323,
                ["GetAndWithElement.Int32.7"] = GetAndWithElementInt327,
                ["GetAndWithElement.Int64.0"] = GetAndWithElementInt640,
                ["GetAndWithElement.Int64.1"] = GetAndWithElementInt641,
                ["GetAndWithElement.Int64.3"] = GetAndWithElementInt643,
                ["GetAndWithElement.SByte.0"] = GetAndWithElementSByte0,
                ["GetAndWithElement.SByte.7"] = GetAndWithElementSByte7,
                ["GetAndWithElement.SByte.15"] = GetAndWithElementSByte15,
                ["GetAndWithElement.SByte.31"] = GetAndWithElementSByte31,
                ["GetAndWithElement.Single.0"] = GetAndWithElementSingle0,
                ["GetAndWithElement.Single.1"] = GetAndWithElementSingle1,
                ["GetAndWithElement.Single.3"] = GetAndWithElementSingle3,
                ["GetAndWithElement.Single.7"] = GetAndWithElementSingle7,
                ["GetAndWithElement.UInt16.0"] = GetAndWithElementUInt160,
                ["GetAndWithElement.UInt16.3"] = GetAndWithElementUInt163,
                ["GetAndWithElement.UInt16.7"] = GetAndWithElementUInt167,
                ["GetAndWithElement.UInt16.15"] = GetAndWithElementUInt1615,
                ["GetAndWithElement.UInt32.0"] = GetAndWithElementUInt320,
                ["GetAndWithElement.UInt32.1"] = GetAndWithElementUInt321,
                ["GetAndWithElement.UInt32.3"] = GetAndWithElementUInt323,
                ["GetAndWithElement.UInt32.7"] = GetAndWithElementUInt327,
                ["GetAndWithElement.UInt64.0"] = GetAndWithElementUInt640,
                ["GetAndWithElement.UInt64.1"] = GetAndWithElementUInt641,
                ["GetAndWithElement.UInt64.3"] = GetAndWithElementUInt643,
                ["GetAndWithLowerAndUpper.Byte"] = GetAndWithLowerAndUpperByte,
                ["GetAndWithLowerAndUpper.Double"] = GetAndWithLowerAndUpperDouble,
                ["GetAndWithLowerAndUpper.Int16"] = GetAndWithLowerAndUpperInt16,
                ["GetAndWithLowerAndUpper.Int32"] = GetAndWithLowerAndUpperInt32,
                ["GetAndWithLowerAndUpper.Int64"] = GetAndWithLowerAndUpperInt64,
                ["GetAndWithLowerAndUpper.SByte"] = GetAndWithLowerAndUpperSByte,
                ["GetAndWithLowerAndUpper.Single"] = GetAndWithLowerAndUpperSingle,
                ["GetAndWithLowerAndUpper.UInt16"] = GetAndWithLowerAndUpperUInt16,
                ["GetAndWithLowerAndUpper.UInt32"] = GetAndWithLowerAndUpperUInt32,
                ["GetAndWithLowerAndUpper.UInt64"] = GetAndWithLowerAndUpperUInt64,
                ["ToScalar.Byte"] = ToScalarByte,
                ["ToScalar.Double"] = ToScalarDouble,
                ["ToScalar.Int16"] = ToScalarInt16,
                ["ToScalar.Int32"] = ToScalarInt32,
                ["ToScalar.Int64"] = ToScalarInt64,
                ["ToScalar.SByte"] = ToScalarSByte,
                ["ToScalar.Single"] = ToScalarSingle,
                ["ToScalar.UInt16"] = ToScalarUInt16,
                ["ToScalar.UInt32"] = ToScalarUInt32,
                ["ToScalar.UInt64"] = ToScalarUInt64,
                ["ToString.Byte"] = ToStringByte,
                ["ToString.SByte"] = ToStringSByte,
                ["ToString.Int16"] = ToStringInt16,
                ["ToString.UInt16"] = ToStringUInt16,
                ["ToString.Int32"] = ToStringInt32,
                ["ToString.UInt32"] = ToStringUInt32,
                ["ToString.Single"] = ToStringSingle,
                ["ToString.Double"] = ToStringDouble,
                ["ToString.Int64"] = ToStringInt64,
                ["ToString.UInt64"] = ToStringUInt64,
            };
        }
    }
}
