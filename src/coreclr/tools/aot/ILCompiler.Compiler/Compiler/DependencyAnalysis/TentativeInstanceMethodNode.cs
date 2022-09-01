﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using Internal.TypeSystem;

using Debug = System.Diagnostics.Debug;

namespace ILCompiler.DependencyAnalysis
{
    /// <summary>
    /// Represents a stand-in for a real method body that can turn into the real method
    /// body at object emission phase if the real method body was marked.
    /// It the real method body wasn't marked, this stub will tail-call into a throw helper.
    /// This node conditionally depends on the real method body - the real method body
    /// will be brough into compilation if the owning type was marked.
    /// This operates under the assumption that instance methods don't get called with
    /// a null `this`. While it's possible to call instance methods with a null `this`,
    /// there are many reasons why it's a bad idea (C# language assumes `this` can never be null;
    /// generically shared native code generated by this compiler assumes `this` is not null and
    /// we can get the generic dictionary out of it, etc.).
    public class TentativeInstanceMethodNode : TentativeMethodNode
    {
        public TentativeInstanceMethodNode(IMethodBodyNode methodNode)
            : base(methodNode)
        {
            Debug.Assert(!methodNode.Method.Signature.IsStatic);
            Debug.Assert(!methodNode.Method.OwningType.IsValueType);
        }

        public override bool HasConditionalStaticDependencies => true;

        protected override ISymbolNode GetTarget(NodeFactory factory)
        {
            // If the class library doesn't provide this helper, the optimization is disabled.
            MethodDesc helper = factory.InstanceMethodRemovedHelper;
            return helper == null ? RealBody: factory.MethodEntrypoint(helper);
        }

        public override IEnumerable<CombinedDependencyListEntry> GetConditionalStaticDependencies(NodeFactory factory)
        {
            // Convert methods on Array<T> into T[]
            TypeDesc owningType = Method.OwningType;
            if (owningType.HasSameTypeDefinition(factory.ArrayOfTClass))
            {
                owningType = owningType.Instantiation[0].MakeArrayType();
            }

            // If a constructed symbol for the owning type was included in the compilation,
            // include the real method body.
            return new CombinedDependencyListEntry[]
            {
                new CombinedDependencyListEntry(
                    RealBody,
                    factory.ConstructedTypeSymbol(owningType),
                    "Instance method on a constructed type"),
            };
        }

        protected override string GetName(NodeFactory factory)
        {
            return "Tentative instance method: " + RealBody.GetMangledName(factory.NameMangler);
        }

        public override int ClassCode => 0x8905207;

        public override string ToString() => Method.ToString();
    }
}
