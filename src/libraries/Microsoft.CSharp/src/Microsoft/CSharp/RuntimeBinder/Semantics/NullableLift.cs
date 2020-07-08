// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace Microsoft.CSharp.RuntimeBinder.Semantics
{
    internal enum NullableCallLiftKind
    {
        NotLifted,
        Operator,
        EqualityOperator,
        InequalityOperator,
        UserDefinedConversion,
        NullableConversion,
        NullableConversionConstructor,
        NullableIntermediateConversion,
        NotLiftedIntermediateConversion
    }
}
