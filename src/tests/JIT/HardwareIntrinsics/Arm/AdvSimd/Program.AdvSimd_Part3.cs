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
                ["MultiplyBySelectedScalarWideningLower.Vector64.Int16.Vector64.Int16.3"] = MultiplyBySelectedScalarWideningLower_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyBySelectedScalarWideningLower.Vector64.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalarWideningLower_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalarWideningLower.Vector64.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalarWideningLower_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalarWideningLower.Vector64.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalarWideningLower_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalarWideningLower.Vector64.UInt16.Vector64.UInt16.3"] = MultiplyBySelectedScalarWideningLower_Vector64_UInt16_Vector64_UInt16_3,
                ["MultiplyBySelectedScalarWideningLower.Vector64.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalarWideningLower_Vector64_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalarWideningLower.Vector64.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalarWideningLower_Vector64_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalarWideningLower.Vector64.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalarWideningLower_Vector64_UInt32_Vector128_UInt32_3,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.Int16.Vector64.Int16.3"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.UInt16.Vector64.UInt16.3"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_UInt16_Vector64_UInt16_3,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalarWideningLowerAndAdd.Vector64.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalarWideningLowerAndAdd_Vector64_UInt32_Vector128_UInt32_3,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.Int16.Vector64.Int16.3"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.UInt16.Vector64.UInt16.3"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_UInt16_Vector64_UInt16_3,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalarWideningLowerAndSubtract.Vector64.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalarWideningLowerAndSubtract_Vector64_UInt32_Vector128_UInt32_3,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.Int16.Vector64.Int16.3"] = MultiplyBySelectedScalarWideningUpper_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalarWideningUpper_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalarWideningUpper_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalarWideningUpper_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.UInt16.Vector64.UInt16.3"] = MultiplyBySelectedScalarWideningUpper_Vector128_UInt16_Vector64_UInt16_3,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalarWideningUpper_Vector128_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalarWideningUpper_Vector128_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalarWideningUpper.Vector128.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalarWideningUpper_Vector128_UInt32_Vector128_UInt32_3,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.Int16.Vector64.Int16.3"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.UInt16.Vector64.UInt16.3"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_UInt16_Vector64_UInt16_3,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalarWideningUpperAndAdd.Vector128.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalarWideningUpperAndAdd_Vector128_UInt32_Vector128_UInt32_3,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.Int16.Vector64.Int16.3"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.Int16.Vector128.Int16.7"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.Int32.Vector64.Int32.1"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.Int32.Vector128.Int32.3"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.UInt16.Vector64.UInt16.3"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_UInt16_Vector64_UInt16_3,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.UInt16.Vector128.UInt16.7"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_UInt16_Vector128_UInt16_7,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.UInt32.Vector64.UInt32.1"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_UInt32_Vector64_UInt32_1,
                ["MultiplyBySelectedScalarWideningUpperAndSubtract.Vector128.UInt32.Vector128.UInt32.3"] = MultiplyBySelectedScalarWideningUpperAndSubtract_Vector128_UInt32_Vector128_UInt32_3,
                ["MultiplyDoublingByScalarSaturateHigh.Vector64.Int16"] = MultiplyDoublingByScalarSaturateHigh_Vector64_Int16,
                ["MultiplyDoublingByScalarSaturateHigh.Vector64.Int32"] = MultiplyDoublingByScalarSaturateHigh_Vector64_Int32,
                ["MultiplyDoublingByScalarSaturateHigh.Vector128.Int16"] = MultiplyDoublingByScalarSaturateHigh_Vector128_Int16,
                ["MultiplyDoublingByScalarSaturateHigh.Vector128.Int32"] = MultiplyDoublingByScalarSaturateHigh_Vector128_Int32,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector64.Int16.Vector64.Int16.3"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector64.Int16.Vector128.Int16.7"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector64.Int32.Vector64.Int32.1"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector64.Int32.Vector128.Int32.3"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector128.Int16.Vector64.Int16.3"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector128.Int16.Vector128.Int16.7"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector128.Int32.Vector64.Int32.1"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyDoublingBySelectedScalarSaturateHigh.Vector128.Int32.Vector128.Int32.3"] = MultiplyDoublingBySelectedScalarSaturateHigh_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyDoublingSaturateHigh.Vector64.Int16"] = MultiplyDoublingSaturateHigh_Vector64_Int16,
                ["MultiplyDoublingSaturateHigh.Vector64.Int32"] = MultiplyDoublingSaturateHigh_Vector64_Int32,
                ["MultiplyDoublingSaturateHigh.Vector128.Int16"] = MultiplyDoublingSaturateHigh_Vector128_Int16,
                ["MultiplyDoublingSaturateHigh.Vector128.Int32"] = MultiplyDoublingSaturateHigh_Vector128_Int32,
                ["MultiplyDoublingWideningLowerAndAddSaturate.Vector64.Int16"] = MultiplyDoublingWideningLowerAndAddSaturate_Vector64_Int16,
                ["MultiplyDoublingWideningLowerAndAddSaturate.Vector64.Int32"] = MultiplyDoublingWideningLowerAndAddSaturate_Vector64_Int32,
                ["MultiplyDoublingWideningLowerAndSubtractSaturate.Vector64.Int16"] = MultiplyDoublingWideningLowerAndSubtractSaturate_Vector64_Int16,
                ["MultiplyDoublingWideningLowerAndSubtractSaturate.Vector64.Int32"] = MultiplyDoublingWideningLowerAndSubtractSaturate_Vector64_Int32,
                ["MultiplyDoublingWideningLowerByScalarAndAddSaturate.Vector64.Int16.Vector64.Int16"] = MultiplyDoublingWideningLowerByScalarAndAddSaturate_Vector64_Int16_Vector64_Int16,
                ["MultiplyDoublingWideningLowerByScalarAndAddSaturate.Vector64.Int32.Vector64.Int32"] = MultiplyDoublingWideningLowerByScalarAndAddSaturate_Vector64_Int32_Vector64_Int32,
                ["MultiplyDoublingWideningLowerByScalarAndSubtractSaturate.Vector64.Int16.Vector64.Int16"] = MultiplyDoublingWideningLowerByScalarAndSubtractSaturate_Vector64_Int16_Vector64_Int16,
                ["MultiplyDoublingWideningLowerByScalarAndSubtractSaturate.Vector64.Int32.Vector64.Int32"] = MultiplyDoublingWideningLowerByScalarAndSubtractSaturate_Vector64_Int32_Vector64_Int32,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate.Vector64.Int16.Vector64.Int16.3"] = MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate.Vector64.Int16.Vector128.Int16.7"] = MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate.Vector64.Int32.Vector64.Int32.1"] = MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate.Vector64.Int32.Vector128.Int32.3"] = MultiplyDoublingWideningLowerBySelectedScalarAndAddSaturate_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate.Vector64.Int16.Vector64.Int16.3"] = MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate.Vector64.Int16.Vector128.Int16.7"] = MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate.Vector64.Int32.Vector64.Int32.1"] = MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate.Vector64.Int32.Vector128.Int32.3"] = MultiplyDoublingWideningLowerBySelectedScalarAndSubtractSaturate_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyDoublingWideningSaturateLower.Vector64.Int16"] = MultiplyDoublingWideningSaturateLower_Vector64_Int16,
                ["MultiplyDoublingWideningSaturateLower.Vector64.Int32"] = MultiplyDoublingWideningSaturateLower_Vector64_Int32,
                ["MultiplyDoublingWideningSaturateLowerByScalar.Vector64.Int16"] = MultiplyDoublingWideningSaturateLowerByScalar_Vector64_Int16,
                ["MultiplyDoublingWideningSaturateLowerByScalar.Vector64.Int32"] = MultiplyDoublingWideningSaturateLowerByScalar_Vector64_Int32,
                ["MultiplyDoublingWideningSaturateLowerBySelectedScalar.Vector64.Int16.Vector64.Int16.3"] = MultiplyDoublingWideningSaturateLowerBySelectedScalar_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyDoublingWideningSaturateLowerBySelectedScalar.Vector64.Int16.Vector128.Int16.7"] = MultiplyDoublingWideningSaturateLowerBySelectedScalar_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyDoublingWideningSaturateLowerBySelectedScalar.Vector64.Int32.Vector64.Int32.1"] = MultiplyDoublingWideningSaturateLowerBySelectedScalar_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyDoublingWideningSaturateLowerBySelectedScalar.Vector64.Int32.Vector128.Int32.3"] = MultiplyDoublingWideningSaturateLowerBySelectedScalar_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyDoublingWideningSaturateUpper.Vector128.Int16"] = MultiplyDoublingWideningSaturateUpper_Vector128_Int16,
                ["MultiplyDoublingWideningSaturateUpper.Vector128.Int32"] = MultiplyDoublingWideningSaturateUpper_Vector128_Int32,
                ["MultiplyDoublingWideningSaturateUpperByScalar.Vector128.Int16"] = MultiplyDoublingWideningSaturateUpperByScalar_Vector128_Int16,
                ["MultiplyDoublingWideningSaturateUpperByScalar.Vector128.Int32"] = MultiplyDoublingWideningSaturateUpperByScalar_Vector128_Int32,
                ["MultiplyDoublingWideningSaturateUpperBySelectedScalar.Vector128.Int16.Vector64.Int16.3"] = MultiplyDoublingWideningSaturateUpperBySelectedScalar_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyDoublingWideningSaturateUpperBySelectedScalar.Vector128.Int16.Vector128.Int16.7"] = MultiplyDoublingWideningSaturateUpperBySelectedScalar_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyDoublingWideningSaturateUpperBySelectedScalar.Vector128.Int32.Vector64.Int32.1"] = MultiplyDoublingWideningSaturateUpperBySelectedScalar_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyDoublingWideningSaturateUpperBySelectedScalar.Vector128.Int32.Vector128.Int32.3"] = MultiplyDoublingWideningSaturateUpperBySelectedScalar_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyDoublingWideningUpperAndAddSaturate.Vector128.Int16"] = MultiplyDoublingWideningUpperAndAddSaturate_Vector128_Int16,
                ["MultiplyDoublingWideningUpperAndAddSaturate.Vector128.Int32"] = MultiplyDoublingWideningUpperAndAddSaturate_Vector128_Int32,
                ["MultiplyDoublingWideningUpperAndSubtractSaturate.Vector128.Int16"] = MultiplyDoublingWideningUpperAndSubtractSaturate_Vector128_Int16,
                ["MultiplyDoublingWideningUpperAndSubtractSaturate.Vector128.Int32"] = MultiplyDoublingWideningUpperAndSubtractSaturate_Vector128_Int32,
                ["MultiplyDoublingWideningUpperByScalarAndAddSaturate.Vector128.Int16.Vector64.Int16"] = MultiplyDoublingWideningUpperByScalarAndAddSaturate_Vector128_Int16_Vector64_Int16,
                ["MultiplyDoublingWideningUpperByScalarAndAddSaturate.Vector128.Int32.Vector64.Int32"] = MultiplyDoublingWideningUpperByScalarAndAddSaturate_Vector128_Int32_Vector64_Int32,
                ["MultiplyDoublingWideningUpperByScalarAndSubtractSaturate.Vector128.Int16.Vector64.Int16"] = MultiplyDoublingWideningUpperByScalarAndSubtractSaturate_Vector128_Int16_Vector64_Int16,
                ["MultiplyDoublingWideningUpperByScalarAndSubtractSaturate.Vector128.Int32.Vector64.Int32"] = MultiplyDoublingWideningUpperByScalarAndSubtractSaturate_Vector128_Int32_Vector64_Int32,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate.Vector128.Int16.Vector64.Int16.3"] = MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate.Vector128.Int16.Vector128.Int16.7"] = MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate.Vector128.Int32.Vector64.Int32.1"] = MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate.Vector128.Int32.Vector128.Int32.3"] = MultiplyDoublingWideningUpperBySelectedScalarAndAddSaturate_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate.Vector128.Int16.Vector64.Int16.3"] = MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate_Vector128_Int16_Vector64_Int16_3,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate.Vector128.Int16.Vector128.Int16.7"] = MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate.Vector128.Int32.Vector64.Int32.1"] = MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate.Vector128.Int32.Vector128.Int32.3"] = MultiplyDoublingWideningUpperBySelectedScalarAndSubtractSaturate_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyRoundedDoublingByScalarSaturateHigh.Vector64.Int16"] = MultiplyRoundedDoublingByScalarSaturateHigh_Vector64_Int16,
                ["MultiplyRoundedDoublingByScalarSaturateHigh.Vector64.Int32"] = MultiplyRoundedDoublingByScalarSaturateHigh_Vector64_Int32,
                ["MultiplyRoundedDoublingByScalarSaturateHigh.Vector128.Int16"] = MultiplyRoundedDoublingByScalarSaturateHigh_Vector128_Int16,
                ["MultiplyRoundedDoublingByScalarSaturateHigh.Vector128.Int32"] = MultiplyRoundedDoublingByScalarSaturateHigh_Vector128_Int32,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector64.Int16.Vector64.Int16.3"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector64_Int16_Vector64_Int16_3,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector64.Int16.Vector128.Int16.7"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector64_Int16_Vector128_Int16_7,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector64.Int32.Vector64.Int32.1"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector64_Int32_Vector64_Int32_1,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector64.Int32.Vector128.Int32.3"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector64_Int32_Vector128_Int32_3,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector128.Int16.Vector64.Int16.2"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector128_Int16_Vector64_Int16_2,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector128.Int16.Vector128.Int16.7"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector128_Int16_Vector128_Int16_7,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector128.Int32.Vector64.Int32.1"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector128_Int32_Vector64_Int32_1,
                ["MultiplyRoundedDoublingBySelectedScalarSaturateHigh.Vector128.Int32.Vector128.Int32.3"] = MultiplyRoundedDoublingBySelectedScalarSaturateHigh_Vector128_Int32_Vector128_Int32_3,
                ["MultiplyRoundedDoublingSaturateHigh.Vector64.Int16"] = MultiplyRoundedDoublingSaturateHigh_Vector64_Int16,
                ["MultiplyRoundedDoublingSaturateHigh.Vector64.Int32"] = MultiplyRoundedDoublingSaturateHigh_Vector64_Int32,
                ["MultiplyRoundedDoublingSaturateHigh.Vector128.Int16"] = MultiplyRoundedDoublingSaturateHigh_Vector128_Int16,
                ["MultiplyRoundedDoublingSaturateHigh.Vector128.Int32"] = MultiplyRoundedDoublingSaturateHigh_Vector128_Int32,
                ["MultiplyScalarBySelectedScalar.Vector64.Single.Vector64.Single.1"] = MultiplyScalarBySelectedScalar_Vector64_Single_Vector64_Single_1,
                ["MultiplyScalarBySelectedScalar.Vector64.Single.Vector128.Single.3"] = MultiplyScalarBySelectedScalar_Vector64_Single_Vector128_Single_3,
                ["MultiplySubtract.Vector64.Byte"] = MultiplySubtract_Vector64_Byte,
                ["MultiplySubtract.Vector64.Int16"] = MultiplySubtract_Vector64_Int16,
                ["MultiplySubtract.Vector64.Int32"] = MultiplySubtract_Vector64_Int32,
                ["MultiplySubtract.Vector64.SByte"] = MultiplySubtract_Vector64_SByte,
                ["MultiplySubtract.Vector64.UInt16"] = MultiplySubtract_Vector64_UInt16,
                ["MultiplySubtract.Vector64.UInt32"] = MultiplySubtract_Vector64_UInt32,
                ["MultiplySubtract.Vector128.Byte"] = MultiplySubtract_Vector128_Byte,
                ["MultiplySubtract.Vector128.Int16"] = MultiplySubtract_Vector128_Int16,
                ["MultiplySubtract.Vector128.Int32"] = MultiplySubtract_Vector128_Int32,
                ["MultiplySubtract.Vector128.SByte"] = MultiplySubtract_Vector128_SByte,
                ["MultiplySubtract.Vector128.UInt16"] = MultiplySubtract_Vector128_UInt16,
                ["MultiplySubtract.Vector128.UInt32"] = MultiplySubtract_Vector128_UInt32,
                ["MultiplySubtractBySelectedScalar.Vector64.Int16.Vector64.Int16.3"] = MultiplySubtractBySelectedScalar_Vector64_Int16_Vector64_Int16_3,
                ["MultiplySubtractBySelectedScalar.Vector64.Int16.Vector128.Int16.7"] = MultiplySubtractBySelectedScalar_Vector64_Int16_Vector128_Int16_7,
                ["MultiplySubtractBySelectedScalar.Vector64.Int32.Vector64.Int32.1"] = MultiplySubtractBySelectedScalar_Vector64_Int32_Vector64_Int32_1,
                ["MultiplySubtractBySelectedScalar.Vector64.Int32.Vector128.Int32.3"] = MultiplySubtractBySelectedScalar_Vector64_Int32_Vector128_Int32_3,
                ["MultiplySubtractBySelectedScalar.Vector64.UInt16.Vector64.UInt16.3"] = MultiplySubtractBySelectedScalar_Vector64_UInt16_Vector64_UInt16_3,
                ["MultiplySubtractBySelectedScalar.Vector64.UInt16.Vector128.UInt16.7"] = MultiplySubtractBySelectedScalar_Vector64_UInt16_Vector128_UInt16_7,
                ["MultiplySubtractBySelectedScalar.Vector64.UInt32.Vector64.UInt32.1"] = MultiplySubtractBySelectedScalar_Vector64_UInt32_Vector64_UInt32_1,
                ["MultiplySubtractBySelectedScalar.Vector64.UInt32.Vector128.UInt32.3"] = MultiplySubtractBySelectedScalar_Vector64_UInt32_Vector128_UInt32_3,
                ["MultiplySubtractBySelectedScalar.Vector128.Int16.Vector64.Int16.3"] = MultiplySubtractBySelectedScalar_Vector128_Int16_Vector64_Int16_3,
                ["MultiplySubtractBySelectedScalar.Vector128.Int16.Vector128.Int16.7"] = MultiplySubtractBySelectedScalar_Vector128_Int16_Vector128_Int16_7,
                ["MultiplySubtractBySelectedScalar.Vector128.Int32.Vector64.Int32.1"] = MultiplySubtractBySelectedScalar_Vector128_Int32_Vector64_Int32_1,
                ["MultiplySubtractBySelectedScalar.Vector128.Int32.Vector128.Int32.3"] = MultiplySubtractBySelectedScalar_Vector128_Int32_Vector128_Int32_3,
                ["MultiplySubtractBySelectedScalar.Vector128.UInt16.Vector64.UInt16.3"] = MultiplySubtractBySelectedScalar_Vector128_UInt16_Vector64_UInt16_3,
                ["MultiplySubtractBySelectedScalar.Vector128.UInt16.Vector128.UInt16.7"] = MultiplySubtractBySelectedScalar_Vector128_UInt16_Vector128_UInt16_7,
                ["MultiplySubtractBySelectedScalar.Vector128.UInt32.Vector64.UInt32.1"] = MultiplySubtractBySelectedScalar_Vector128_UInt32_Vector64_UInt32_1,
                ["MultiplySubtractBySelectedScalar.Vector128.UInt32.Vector128.UInt32.3"] = MultiplySubtractBySelectedScalar_Vector128_UInt32_Vector128_UInt32_3,
                ["MultiplySubtractByScalar.Vector64.Int16"] = MultiplySubtractByScalar_Vector64_Int16,
                ["MultiplySubtractByScalar.Vector64.Int32"] = MultiplySubtractByScalar_Vector64_Int32,
                ["MultiplySubtractByScalar.Vector64.UInt16"] = MultiplySubtractByScalar_Vector64_UInt16,
                ["MultiplySubtractByScalar.Vector64.UInt32"] = MultiplySubtractByScalar_Vector64_UInt32,
                ["MultiplySubtractByScalar.Vector128.Int16"] = MultiplySubtractByScalar_Vector128_Int16,
                ["MultiplySubtractByScalar.Vector128.Int32"] = MultiplySubtractByScalar_Vector128_Int32,
                ["MultiplySubtractByScalar.Vector128.UInt16"] = MultiplySubtractByScalar_Vector128_UInt16,
                ["MultiplySubtractByScalar.Vector128.UInt32"] = MultiplySubtractByScalar_Vector128_UInt32,
                ["MultiplyWideningLower.Vector64.Byte"] = MultiplyWideningLower_Vector64_Byte,
                ["MultiplyWideningLower.Vector64.Int16"] = MultiplyWideningLower_Vector64_Int16,
                ["MultiplyWideningLower.Vector64.Int32"] = MultiplyWideningLower_Vector64_Int32,
                ["MultiplyWideningLower.Vector64.SByte"] = MultiplyWideningLower_Vector64_SByte,
                ["MultiplyWideningLower.Vector64.UInt16"] = MultiplyWideningLower_Vector64_UInt16,
                ["MultiplyWideningLower.Vector64.UInt32"] = MultiplyWideningLower_Vector64_UInt32,
                ["MultiplyWideningLowerAndAdd.Vector64.Byte"] = MultiplyWideningLowerAndAdd_Vector64_Byte,
                ["MultiplyWideningLowerAndAdd.Vector64.Int16"] = MultiplyWideningLowerAndAdd_Vector64_Int16,
                ["MultiplyWideningLowerAndAdd.Vector64.Int32"] = MultiplyWideningLowerAndAdd_Vector64_Int32,
                ["MultiplyWideningLowerAndAdd.Vector64.SByte"] = MultiplyWideningLowerAndAdd_Vector64_SByte,
                ["MultiplyWideningLowerAndAdd.Vector64.UInt16"] = MultiplyWideningLowerAndAdd_Vector64_UInt16,
                ["MultiplyWideningLowerAndAdd.Vector64.UInt32"] = MultiplyWideningLowerAndAdd_Vector64_UInt32,
                ["MultiplyWideningLowerAndSubtract.Vector64.Byte"] = MultiplyWideningLowerAndSubtract_Vector64_Byte,
                ["MultiplyWideningLowerAndSubtract.Vector64.Int16"] = MultiplyWideningLowerAndSubtract_Vector64_Int16,
                ["MultiplyWideningLowerAndSubtract.Vector64.Int32"] = MultiplyWideningLowerAndSubtract_Vector64_Int32,
                ["MultiplyWideningLowerAndSubtract.Vector64.SByte"] = MultiplyWideningLowerAndSubtract_Vector64_SByte,
                ["MultiplyWideningLowerAndSubtract.Vector64.UInt16"] = MultiplyWideningLowerAndSubtract_Vector64_UInt16,
                ["MultiplyWideningLowerAndSubtract.Vector64.UInt32"] = MultiplyWideningLowerAndSubtract_Vector64_UInt32,
                ["MultiplyWideningUpper.Vector128.Byte"] = MultiplyWideningUpper_Vector128_Byte,
                ["MultiplyWideningUpper.Vector128.Int16"] = MultiplyWideningUpper_Vector128_Int16,
                ["MultiplyWideningUpper.Vector128.Int32"] = MultiplyWideningUpper_Vector128_Int32,
                ["MultiplyWideningUpper.Vector128.SByte"] = MultiplyWideningUpper_Vector128_SByte,
                ["MultiplyWideningUpper.Vector128.UInt16"] = MultiplyWideningUpper_Vector128_UInt16,
                ["MultiplyWideningUpper.Vector128.UInt32"] = MultiplyWideningUpper_Vector128_UInt32,
                ["MultiplyWideningUpperAndAdd.Vector128.Byte"] = MultiplyWideningUpperAndAdd_Vector128_Byte,
                ["MultiplyWideningUpperAndAdd.Vector128.Int16"] = MultiplyWideningUpperAndAdd_Vector128_Int16,
                ["MultiplyWideningUpperAndAdd.Vector128.Int32"] = MultiplyWideningUpperAndAdd_Vector128_Int32,
                ["MultiplyWideningUpperAndAdd.Vector128.SByte"] = MultiplyWideningUpperAndAdd_Vector128_SByte,
                ["MultiplyWideningUpperAndAdd.Vector128.UInt16"] = MultiplyWideningUpperAndAdd_Vector128_UInt16,
                ["MultiplyWideningUpperAndAdd.Vector128.UInt32"] = MultiplyWideningUpperAndAdd_Vector128_UInt32,
                ["MultiplyWideningUpperAndSubtract.Vector128.Byte"] = MultiplyWideningUpperAndSubtract_Vector128_Byte,
                ["MultiplyWideningUpperAndSubtract.Vector128.Int16"] = MultiplyWideningUpperAndSubtract_Vector128_Int16,
                ["MultiplyWideningUpperAndSubtract.Vector128.Int32"] = MultiplyWideningUpperAndSubtract_Vector128_Int32,
                ["MultiplyWideningUpperAndSubtract.Vector128.SByte"] = MultiplyWideningUpperAndSubtract_Vector128_SByte,
                ["MultiplyWideningUpperAndSubtract.Vector128.UInt16"] = MultiplyWideningUpperAndSubtract_Vector128_UInt16,
                ["MultiplyWideningUpperAndSubtract.Vector128.UInt32"] = MultiplyWideningUpperAndSubtract_Vector128_UInt32,
                ["Negate.Vector64.Int16"] = Negate_Vector64_Int16,
                ["Negate.Vector64.Int32"] = Negate_Vector64_Int32,
                ["Negate.Vector64.SByte"] = Negate_Vector64_SByte,
                ["Negate.Vector64.Single"] = Negate_Vector64_Single,
                ["Negate.Vector128.Int16"] = Negate_Vector128_Int16,
                ["Negate.Vector128.Int32"] = Negate_Vector128_Int32,
                ["Negate.Vector128.SByte"] = Negate_Vector128_SByte,
                ["Negate.Vector128.Single"] = Negate_Vector128_Single,
                ["NegateSaturate.Vector64.Int16"] = NegateSaturate_Vector64_Int16,
                ["NegateSaturate.Vector64.Int32"] = NegateSaturate_Vector64_Int32,
                ["NegateSaturate.Vector64.SByte"] = NegateSaturate_Vector64_SByte,
                ["NegateSaturate.Vector128.Int16"] = NegateSaturate_Vector128_Int16,
                ["NegateSaturate.Vector128.Int32"] = NegateSaturate_Vector128_Int32,
                ["NegateSaturate.Vector128.SByte"] = NegateSaturate_Vector128_SByte,
                ["NegateScalar.Vector64.Double"] = NegateScalar_Vector64_Double,
                ["NegateScalar.Vector64.Single"] = NegateScalar_Vector64_Single,
                ["Not.Vector64.Byte"] = Not_Vector64_Byte,
                ["Not.Vector64.Double"] = Not_Vector64_Double,
                ["Not.Vector64.Int16"] = Not_Vector64_Int16,
                ["Not.Vector64.Int32"] = Not_Vector64_Int32,
                ["Not.Vector64.Int64"] = Not_Vector64_Int64,
                ["Not.Vector64.SByte"] = Not_Vector64_SByte,
                ["Not.Vector64.Single"] = Not_Vector64_Single,
                ["Not.Vector64.UInt16"] = Not_Vector64_UInt16,
                ["Not.Vector64.UInt32"] = Not_Vector64_UInt32,
                ["Not.Vector64.UInt64"] = Not_Vector64_UInt64,
                ["Not.Vector128.Byte"] = Not_Vector128_Byte,
                ["Not.Vector128.Double"] = Not_Vector128_Double,
                ["Not.Vector128.Int16"] = Not_Vector128_Int16,
                ["Not.Vector128.Int32"] = Not_Vector128_Int32,
                ["Not.Vector128.Int64"] = Not_Vector128_Int64,
                ["Not.Vector128.SByte"] = Not_Vector128_SByte,
                ["Not.Vector128.Single"] = Not_Vector128_Single,
                ["Not.Vector128.UInt16"] = Not_Vector128_UInt16,
                ["Not.Vector128.UInt32"] = Not_Vector128_UInt32,
                ["Not.Vector128.UInt64"] = Not_Vector128_UInt64,
                ["Or.Vector64.Byte"] = Or_Vector64_Byte,
                ["Or.Vector64.Double"] = Or_Vector64_Double,
                ["Or.Vector64.Int16"] = Or_Vector64_Int16,
                ["Or.Vector64.Int32"] = Or_Vector64_Int32,
                ["Or.Vector64.Int64"] = Or_Vector64_Int64,
                ["Or.Vector64.SByte"] = Or_Vector64_SByte,
                ["Or.Vector64.Single"] = Or_Vector64_Single,
                ["Or.Vector64.UInt16"] = Or_Vector64_UInt16,
                ["Or.Vector64.UInt32"] = Or_Vector64_UInt32,
                ["Or.Vector64.UInt64"] = Or_Vector64_UInt64,
                ["Or.Vector128.Byte"] = Or_Vector128_Byte,
                ["Or.Vector128.Double"] = Or_Vector128_Double,
                ["Or.Vector128.Int16"] = Or_Vector128_Int16,
                ["Or.Vector128.Int32"] = Or_Vector128_Int32,
                ["Or.Vector128.Int64"] = Or_Vector128_Int64,
                ["Or.Vector128.SByte"] = Or_Vector128_SByte,
                ["Or.Vector128.Single"] = Or_Vector128_Single,
                ["Or.Vector128.UInt16"] = Or_Vector128_UInt16,
            };
        }
    }
}
