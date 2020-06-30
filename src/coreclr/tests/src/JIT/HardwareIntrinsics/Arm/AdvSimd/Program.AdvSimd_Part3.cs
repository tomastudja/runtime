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
                ["Or.Vector128.UInt32"] = Or_Vector128_UInt32,
                ["Or.Vector128.UInt64"] = Or_Vector128_UInt64,
                ["OrNot.Vector64.Byte"] = OrNot_Vector64_Byte,
                ["OrNot.Vector64.Double"] = OrNot_Vector64_Double,
                ["OrNot.Vector64.Int16"] = OrNot_Vector64_Int16,
                ["OrNot.Vector64.Int32"] = OrNot_Vector64_Int32,
                ["OrNot.Vector64.Int64"] = OrNot_Vector64_Int64,
                ["OrNot.Vector64.SByte"] = OrNot_Vector64_SByte,
                ["OrNot.Vector64.Single"] = OrNot_Vector64_Single,
                ["OrNot.Vector64.UInt16"] = OrNot_Vector64_UInt16,
                ["OrNot.Vector64.UInt32"] = OrNot_Vector64_UInt32,
                ["OrNot.Vector64.UInt64"] = OrNot_Vector64_UInt64,
                ["OrNot.Vector128.Byte"] = OrNot_Vector128_Byte,
                ["OrNot.Vector128.Double"] = OrNot_Vector128_Double,
                ["OrNot.Vector128.Int16"] = OrNot_Vector128_Int16,
                ["OrNot.Vector128.Int32"] = OrNot_Vector128_Int32,
                ["OrNot.Vector128.Int64"] = OrNot_Vector128_Int64,
                ["OrNot.Vector128.SByte"] = OrNot_Vector128_SByte,
                ["OrNot.Vector128.Single"] = OrNot_Vector128_Single,
                ["OrNot.Vector128.UInt16"] = OrNot_Vector128_UInt16,
                ["OrNot.Vector128.UInt32"] = OrNot_Vector128_UInt32,
                ["OrNot.Vector128.UInt64"] = OrNot_Vector128_UInt64,
                ["PolynomialMultiply.Vector64.Byte"] = PolynomialMultiply_Vector64_Byte,
                ["PolynomialMultiply.Vector64.SByte"] = PolynomialMultiply_Vector64_SByte,
                ["PolynomialMultiply.Vector128.Byte"] = PolynomialMultiply_Vector128_Byte,
                ["PolynomialMultiply.Vector128.SByte"] = PolynomialMultiply_Vector128_SByte,
                ["PolynomialMultiplyWideningLower.Vector64.Byte"] = PolynomialMultiplyWideningLower_Vector64_Byte,
                ["PolynomialMultiplyWideningLower.Vector64.SByte"] = PolynomialMultiplyWideningLower_Vector64_SByte,
                ["PolynomialMultiplyWideningUpper.Vector128.Byte"] = PolynomialMultiplyWideningUpper_Vector128_Byte,
                ["PolynomialMultiplyWideningUpper.Vector128.SByte"] = PolynomialMultiplyWideningUpper_Vector128_SByte,
                ["PopCount.Vector64.Byte"] = PopCount_Vector64_Byte,
                ["PopCount.Vector64.SByte"] = PopCount_Vector64_SByte,
                ["PopCount.Vector128.Byte"] = PopCount_Vector128_Byte,
                ["PopCount.Vector128.SByte"] = PopCount_Vector128_SByte,
                ["ReciprocalEstimate.Vector64.Single"] = ReciprocalEstimate_Vector64_Single,
                ["ReciprocalEstimate.Vector64.UInt32"] = ReciprocalEstimate_Vector64_UInt32,
                ["ReciprocalEstimate.Vector128.Single"] = ReciprocalEstimate_Vector128_Single,
                ["ReciprocalEstimate.Vector128.UInt32"] = ReciprocalEstimate_Vector128_UInt32,
                ["ReciprocalSquareRootEstimate.Vector64.Single"] = ReciprocalSquareRootEstimate_Vector64_Single,
                ["ReciprocalSquareRootEstimate.Vector64.UInt32"] = ReciprocalSquareRootEstimate_Vector64_UInt32,
                ["ReciprocalSquareRootEstimate.Vector128.Single"] = ReciprocalSquareRootEstimate_Vector128_Single,
                ["ReciprocalSquareRootEstimate.Vector128.UInt32"] = ReciprocalSquareRootEstimate_Vector128_UInt32,
                ["ReciprocalSquareRootStep.Vector64.Single"] = ReciprocalSquareRootStep_Vector64_Single,
                ["ReciprocalSquareRootStep.Vector128.Single"] = ReciprocalSquareRootStep_Vector128_Single,
                ["ReciprocalStep.Vector64.Single"] = ReciprocalStep_Vector64_Single,
                ["ReciprocalStep.Vector128.Single"] = ReciprocalStep_Vector128_Single,
                ["RoundAwayFromZero.Vector64.Single"] = RoundAwayFromZero_Vector64_Single,
                ["RoundAwayFromZero.Vector128.Single"] = RoundAwayFromZero_Vector128_Single,
                ["RoundAwayFromZeroScalar.Vector64.Double"] = RoundAwayFromZeroScalar_Vector64_Double,
                ["RoundAwayFromZeroScalar.Vector64.Single"] = RoundAwayFromZeroScalar_Vector64_Single,
                ["RoundToNearest.Vector64.Single"] = RoundToNearest_Vector64_Single,
                ["RoundToNearest.Vector128.Single"] = RoundToNearest_Vector128_Single,
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
            };
        }
    }
}
