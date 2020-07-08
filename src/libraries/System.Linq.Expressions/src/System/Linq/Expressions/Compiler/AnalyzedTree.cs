// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;

namespace System.Linq.Expressions.Compiler
{
    internal sealed class AnalyzedTree
    {
        internal readonly Dictionary<object, CompilerScope> Scopes = new Dictionary<object, CompilerScope>();
        internal readonly Dictionary<LambdaExpression, BoundConstants> Constants = new Dictionary<LambdaExpression, BoundConstants>();
    }
}
