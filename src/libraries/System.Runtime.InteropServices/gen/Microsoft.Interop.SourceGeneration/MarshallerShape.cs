// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using Microsoft.CodeAnalysis;

namespace Microsoft.Interop
{
    [Flags]
    public enum MarshallerShape
    {
        None = 0x0,
        ToUnmanaged = 0x1,
        CallerAllocatedBuffer = 0x2,
        StatelessPinnableReference = 0x4,
        StatefulPinnableReference = 0x8,
        ToManaged = 0x10,
        GuaranteedUnmarshal = 0x20,
        Free = 0x40,
        NotifyInvokeSucceeded = 0x80,
    }

    public static class ShapeMemberNames
    {
        public const string GetPinnableReference = nameof(GetPinnableReference);
        public const string BufferSize = nameof(BufferSize);
        public const string Free = nameof(Free);

        public static class Value
        {
            public static class Stateless
            {
                public const string ConvertToManaged = nameof(ConvertToManaged);
                public const string ConvertToManagedGuaranteed = nameof(ConvertToManagedGuaranteed);
                public const string ConvertToUnmanaged = nameof(ConvertToUnmanaged);
            }

            public static class Stateful
            {
                // Managed to Unmanaged
                public const string FromManaged = nameof(FromManaged);
                public const string ToUnmanaged = nameof(ToUnmanaged);
                // Unmanaged to managed
                public const string ToManaged = nameof(ToManaged);
                public const string ToManagedGuaranteed = nameof(ToManagedGuaranteed);
                public const string FromUnmanaged = nameof(FromUnmanaged);
                // Optional features
                public const string Free = nameof(Free);
                public const string NotifyInvokeSucceeded = nameof(NotifyInvokeSucceeded);
            }
        }

        public static class LinearCollection
        {
            public static class Stateless
            {
                // Managed to unmanaged
                public const string AllocateContainerForUnmanagedElements = nameof(AllocateContainerForUnmanagedElements);
                public const string GetManagedValuesSource = nameof(GetManagedValuesSource);
                public const string GetUnmanagedValuesDestination = nameof(GetUnmanagedValuesDestination);

                // Unmanaged to managed
                public const string AllocateContainerForManagedElements = nameof(AllocateContainerForManagedElements);
                public const string AllocateContainerForManagedElementsGuaranteed = nameof(AllocateContainerForManagedElementsGuaranteed);
                public const string GetManagedValuesDestination = nameof(GetManagedValuesDestination);
                public const string GetUnmanagedValuesSource = nameof(GetUnmanagedValuesSource);
            }

            public static class Stateful
            {
                // Managed to Unmanaged
                public const string FromManaged = nameof(FromManaged);
                public const string ToUnmanaged = nameof(ToUnmanaged);
                public const string GetManagedValuesSource = nameof(GetManagedValuesSource);
                public const string GetUnmanagedValuesDestination = nameof(GetUnmanagedValuesDestination);
                // Unmanaged to managed
                public const string GetManagedValuesDestination = nameof(GetManagedValuesDestination);
                public const string GetUnmanagedValuesSource = nameof(GetUnmanagedValuesSource);
                public const string ToManaged = nameof(ToManaged);
                public const string ToManagedGuaranteed = nameof(ToManagedGuaranteed);
                public const string FromUnmanaged = nameof(FromUnmanaged);
                // Optional features
                public const string Free = nameof(Free);
                public const string NotifyInvokeSucceeded = nameof(NotifyInvokeSucceeded);
            }
        }
    }

    public static class StatelessMarshallerShapeHelper
    {
        public record MarshallerMethods
        {
            public IMethodSymbol? ToUnmanaged;
            public IMethodSymbol? ToUnmanagedWithBuffer;
            public IMethodSymbol? ToManaged;
            public IMethodSymbol? ToManagedGuaranteed;

            // Linear collection
            public IMethodSymbol? ManagedValuesSource;
            public IMethodSymbol? UnmanagedValuesDestination;
            public IMethodSymbol? ManagedValuesDestination;
            public IMethodSymbol? UnmanagedValuesSource;
        }

        public static (MarshallerShape, MarshallerMethods) GetShapeForType(ITypeSymbol marshallerType, ITypeSymbol managedType, bool isLinearCollectionMarshaller, Compilation compilation)
        {
            MarshallerShape shape = MarshallerShape.None;
            MarshallerMethods methods = new();

            INamedTypeSymbol spanOfT = compilation.GetTypeByMetadataName(TypeNames.System_Span_Metadata)!;
            if (isLinearCollectionMarshaller)
            {
                // Managed -> Unmanaged
                INamedTypeSymbol readOnlySpanOfT = compilation.GetTypeByMetadataName(TypeNames.System_ReadOnlySpan_Metadata)!;
                IMethodSymbol? allocateUnmanaged = LinearCollection.AllocateContainerForUnmanagedElements(marshallerType, managedType);
                IMethodSymbol? allocateUnmanagedWithBuffer = LinearCollection.AllocateContainerForUnmanagedElementsWithCallerAllocatedBuffer(marshallerType, managedType, spanOfT);
                IMethodSymbol? managedSource = LinearCollection.GetManagedValuesSource(marshallerType, managedType, readOnlySpanOfT);
                IMethodSymbol? unmanagedDestination = LinearCollection.GetUnmanagedValuesDestination(marshallerType, spanOfT);
                if ((allocateUnmanaged is not null || allocateUnmanagedWithBuffer is not null)
                    && managedSource is not null
                    && unmanagedDestination is not null)
                {
                    if (allocateUnmanaged is not null)
                        shape |= MarshallerShape.ToUnmanaged;

                    if (allocateUnmanagedWithBuffer is not null)
                        shape |= MarshallerShape.CallerAllocatedBuffer;

                    methods = methods with
                    {
                        ToUnmanaged = allocateUnmanaged,
                        ToUnmanagedWithBuffer = allocateUnmanagedWithBuffer,
                        ManagedValuesSource = managedSource,
                        UnmanagedValuesDestination = unmanagedDestination
                    };
                }

                // Unmanaged -> Managed
                IMethodSymbol? allocateManaged = LinearCollection.AllocateContainerForManagedElements(marshallerType, managedType);
                IMethodSymbol? allocateManagedGuaranteed = LinearCollection.AllocateContainerForManagedElementsGuaranteed(marshallerType, managedType, spanOfT);
                IMethodSymbol? managedDestination = LinearCollection.GetManagedValuesDestination(marshallerType, managedType, spanOfT);
                IMethodSymbol? unmanagedSource = LinearCollection.GetUnmanagedValuesSource(marshallerType, readOnlySpanOfT);
                if ((allocateManaged is not null || allocateManagedGuaranteed is not null)
                    && managedDestination is not null
                    && unmanagedSource is not null)
                {
                    if (allocateManaged is not null)
                        shape |= MarshallerShape.ToManaged;

                    if (allocateManagedGuaranteed is not null)
                        shape |= MarshallerShape.GuaranteedUnmarshal;

                    methods = methods with
                    {
                        ToManaged = allocateManaged,
                        ToManagedGuaranteed = allocateManagedGuaranteed,
                        ManagedValuesDestination = managedDestination,
                        UnmanagedValuesSource = unmanagedSource
                    };
                }
            }
            else
            {
                IMethodSymbol? toUnmanaged = Value.ConvertToUnmanaged(marshallerType, managedType);
                if (toUnmanaged is not null)
                    shape |= MarshallerShape.ToUnmanaged;

                IMethodSymbol? toUnmanagedWithBuffer = Value.ConvertToUnmanagedWithCallerAllocatedBuffer(marshallerType, managedType, spanOfT);
                if (toUnmanagedWithBuffer is not null)
                    shape |= MarshallerShape.CallerAllocatedBuffer;

                IMethodSymbol? toManaged = Value.ConvertToManaged(marshallerType, managedType);
                if (toManaged is not null)
                    shape |= MarshallerShape.ToManaged;

                IMethodSymbol? toManagedGuaranteed = Value.ConvertToManagedGuaranteed(marshallerType, managedType);
                if (toManagedGuaranteed is not null)
                    shape |= MarshallerShape.GuaranteedUnmarshal;

                methods = methods with
                {
                    ToUnmanaged = toUnmanaged,
                    ToUnmanagedWithBuffer = toUnmanagedWithBuffer,
                    ToManaged = toManaged,
                    ToManagedGuaranteed = toManagedGuaranteed
                };
            }

            if (GetStatelessGetPinnableReference(marshallerType, managedType) is not null)
                shape |= MarshallerShape.StatelessPinnableReference;

            if (GetStatelessFree(marshallerType) is not null)
                shape |= MarshallerShape.Free;

            return (shape, methods);
        }

        private static IMethodSymbol? GetStatelessFree(ITypeSymbol type)
        {
            // static void Free(TNative unmanaged)
            return type.GetMembers(ShapeMemberNames.Free)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1, ReturnsVoid: true });
        }

        private static IMethodSymbol? GetStatelessGetPinnableReference(ITypeSymbol type, ITypeSymbol managedType)
        {
            // static ref TOther GetPinnableReference(TManaged managed)
            // or
            // static ref readonly TOther GetPinnableReference(TManaged managed)
            return type.GetMembers(ShapeMemberNames.GetPinnableReference)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1 } and
                    ({ ReturnsByRef: true } or { ReturnsByRefReadonly: true })
                    && SymbolEqualityComparer.Default.Equals(m.Parameters[0].Type, managedType));
        }

        private static bool IsSpanOfUnmanagedType(ITypeSymbol typeToCheck, ITypeSymbol spanOfT)
        {
            if (typeToCheck is INamedTypeSymbol namedType
                && SymbolEqualityComparer.Default.Equals(spanOfT, namedType.ConstructedFrom)
                && namedType.TypeArguments.Length == 1
                && namedType.TypeArguments[0].IsUnmanagedType)
            {
                return true;
            }

            return false;
        }

        private static class Value
        {
            internal static IMethodSymbol? ConvertToUnmanaged(ITypeSymbol type, ITypeSymbol managedType)
            {
                // static TNative ConvertToUnmanaged(TManaged managed)
                return type.GetMembers(ShapeMemberNames.Value.Stateless.ConvertToUnmanaged)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1, ReturnsVoid: false }
                        && SymbolEqualityComparer.Default.Equals(managedType, m.Parameters[0].Type));
            }

            internal static IMethodSymbol? ConvertToUnmanagedWithCallerAllocatedBuffer(
                ITypeSymbol type,
                ITypeSymbol managedType,
                ITypeSymbol spanOfT)
            {
                // static TNative ConvertToUnmanaged(TManaged managed, Span<TUnmanagedElement> buffer)
                IEnumerable<IMethodSymbol> methods = type.GetMembers(ShapeMemberNames.Value.Stateless.ConvertToUnmanaged)
                    .OfType<IMethodSymbol>()
                    .Where(m => m is { IsStatic: true, Parameters.Length: 2, ReturnsVoid: false }
                        && SymbolEqualityComparer.Default.Equals(managedType, m.Parameters[0].Type));

                foreach (IMethodSymbol method in methods)
                {
                    if (IsSpanOfUnmanagedType(method.Parameters[1].Type, spanOfT))
                    {
                        return method;
                    }
                }

                return null;
            }

            internal static IMethodSymbol? ConvertToManaged(ITypeSymbol type, ITypeSymbol managedType)
            {
                // static TManaged ConvertToManaged(TNative unmanaged)
                return type.GetMembers(ShapeMemberNames.Value.Stateless.ConvertToManaged)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1, ReturnsVoid: false }
                        && SymbolEqualityComparer.Default.Equals(managedType, m.ReturnType));
            }

            internal static IMethodSymbol? ConvertToManagedGuaranteed(ITypeSymbol type, ITypeSymbol managedType)
            {
                // static TManaged ConvertToManagedGuaranteed(TNative unmanaged)
                return type.GetMembers(ShapeMemberNames.Value.Stateless.ConvertToManagedGuaranteed)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1, ReturnsVoid: false }
                        && SymbolEqualityComparer.Default.Equals(managedType, m.ReturnType));
            }
        }

        private static class LinearCollection
        {
            internal static IMethodSymbol? AllocateContainerForUnmanagedElements(ITypeSymbol type, ITypeSymbol managedType)
            {
                // static TNative AllocateContainerForUnmanagedElements(TCollection managed, out int numElements)
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.AllocateContainerForUnmanagedElements)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 2, ReturnsVoid: false }
                        && managedType.IsConstructedFromEqualTypes(m.Parameters[0].Type)
                        && m.Parameters[1].Type.SpecialType == SpecialType.System_Int32
                        && m.Parameters[1].RefKind == RefKind.Out);
            }

            internal static IMethodSymbol? AllocateContainerForUnmanagedElementsWithCallerAllocatedBuffer(ITypeSymbol type, ITypeSymbol managedType, ITypeSymbol spanOfT)
            {
                // static TNative AllocateContainerForUnmanagedElements(TCollection managed, Span<TOther> buffer, out int numElements)
                IEnumerable<IMethodSymbol> methods = type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.AllocateContainerForUnmanagedElements)
                    .OfType<IMethodSymbol>()
                    .Where(m => m is { IsStatic: true, Parameters.Length: 3, ReturnsVoid: false }
                        && managedType.IsConstructedFromEqualTypes(m.Parameters[0].Type)
                        && m.Parameters[2].Type.SpecialType == SpecialType.System_Int32
                        && m.Parameters[2].RefKind == RefKind.Out);

                foreach (IMethodSymbol method in methods)
                {
                    if (IsSpanOfUnmanagedType(method.Parameters[1].Type, spanOfT))
                    {
                        return method;
                    }
                }

                return null;
            }

            internal static IMethodSymbol? GetManagedValuesSource(ITypeSymbol type, ITypeSymbol managedType, ITypeSymbol readOnlySpanOfT)
            {
                // static ReadOnlySpan<TManagedElement> GetManagedValuesSource(TCollection managed)
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.GetManagedValuesSource)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1, ReturnsVoid: false, ReturnType: INamedTypeSymbol returnType }
                        && managedType.IsConstructedFromEqualTypes(m.Parameters[0].Type)
                        && SymbolEqualityComparer.Default.Equals(readOnlySpanOfT, returnType.ConstructedFrom));
            }

            internal static IMethodSymbol? GetUnmanagedValuesDestination(ITypeSymbol type, ITypeSymbol spanOfT)
            {
                // static Span<TUnmanagedElement> GetUnmanagedValuesDestination(TNative unmanaged, int numElements)
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.GetUnmanagedValuesDestination)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 2, ReturnsVoid: false, ReturnType: INamedTypeSymbol returnType }
                        && m.Parameters[1].Type.SpecialType == SpecialType.System_Int32
                        && SymbolEqualityComparer.Default.Equals(spanOfT, returnType.ConstructedFrom));
            }

            internal static IMethodSymbol? AllocateContainerForManagedElements(ITypeSymbol type, ITypeSymbol managedType)
            {
                // static TCollection AllocateContainerForManagedElements(TNative unmanaged, int length);
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.AllocateContainerForManagedElements)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 2, ReturnsVoid: false }
                        && m.Parameters[1].Type.SpecialType == SpecialType.System_Int32
                        && managedType.IsConstructedFromEqualTypes(m.ReturnType));
            }

            internal static IMethodSymbol? AllocateContainerForManagedElementsGuaranteed(ITypeSymbol type, ITypeSymbol managedType, ITypeSymbol spanOfT)
            {
                // static TCollection AllocateContainerForManagedElementsGuaranteed(TNative unmanaged, int length);
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.AllocateContainerForManagedElements)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 2, ReturnsVoid: false }
                        && m.Parameters[1].Type.SpecialType == SpecialType.System_Int32
                        && managedType.IsConstructedFromEqualTypes(m.ReturnType));
            }

            internal static IMethodSymbol? GetManagedValuesDestination(ITypeSymbol type, ITypeSymbol managedType, ITypeSymbol spanOfT)
            {
                // static Span<TManagedElement> GetManagedValuesDestination(TCollection managed)
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.GetManagedValuesDestination)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1, ReturnsVoid: false, ReturnType: INamedTypeSymbol returnType }
                        && managedType.IsConstructedFromEqualTypes(m.Parameters[0].Type)
                        && SymbolEqualityComparer.Default.Equals(spanOfT, returnType.ConstructedFrom));
            }

            internal static IMethodSymbol? GetUnmanagedValuesSource(ITypeSymbol type, ITypeSymbol readOnlySpanOfT)
            {
                // static ReadOnlySpan<TUnmanagedElement> GetUnmanagedValuesSource(TNative nativeValue, int numElements)
                return type.GetMembers(ShapeMemberNames.LinearCollection.Stateless.GetUnmanagedValuesSource)
                    .OfType<IMethodSymbol>()
                    .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 2, ReturnsVoid: false, ReturnType: INamedTypeSymbol returnType }
                        && m.Parameters[1].Type.SpecialType == SpecialType.System_Int32
                        && SymbolEqualityComparer.Default.Equals(readOnlySpanOfT, returnType.ConstructedFrom));
            }
        }
    }

    public static class StatefulMarshallerShapeHelper
    {
        public record MarshallerMethods
        {
            public IMethodSymbol? FromManaged { get; init; }
            public IMethodSymbol? FromManagedWithBuffer { get; init; }
            public IMethodSymbol? ToManaged { get; init; }
            public IMethodSymbol? ToManagedGuranteed { get; init; }
            public IMethodSymbol? FromUnmanaged { get; init; }
            public IMethodSymbol? ToUnmanaged { get; init; }
            public IMethodSymbol? Free { get; init; }
            public IMethodSymbol? NotifyInvokeSucceeded { get; init; }
        }

        public static (MarshallerShape shape, MarshallerMethods methods) GetShapeForType(ITypeSymbol marshallerType, ITypeSymbol managedType, Compilation compilation)
        {
            MarshallerShape shape = MarshallerShape.None;
            MarshallerMethods methods = new();

            ITypeSymbol? unmanagedType = null;

            IMethodSymbol? fromManaged = GetFromManagedMethod(marshallerType, managedType);
            INamedTypeSymbol spanOfT = compilation.GetTypeByMetadataName(TypeNames.System_Span_Metadata)!;
            IMethodSymbol? fromManagedWithCallerAllocatedBuffer = GetFromManagedWithCallerAllocatedBufferMethod(marshallerType, managedType, spanOfT, out _);

            IMethodSymbol? toUnmanaged = GetToUnmanagedMethod(marshallerType);

            if ((fromManaged, fromManagedWithCallerAllocatedBuffer) is not (null, null) && toUnmanaged is not null)
            {
                unmanagedType = toUnmanaged.ReturnType;
                if (unmanagedType.IsUnmanagedType)
                {
                    if (fromManagedWithCallerAllocatedBuffer is not null)
                    {
                        shape |= MarshallerShape.CallerAllocatedBuffer;
                    }
                    if (fromManaged is not null)
                    {
                        shape |= MarshallerShape.ToUnmanaged;
                    }
                    methods = methods with
                    {
                        FromManaged = fromManaged,
                        FromManagedWithBuffer = fromManagedWithCallerAllocatedBuffer,
                        ToUnmanaged = toUnmanaged
                    };
                }
            }

            IMethodSymbol toManaged = GetToManagedMethod(marshallerType, managedType);
            IMethodSymbol toManagedGuaranteed = GetToManagedGuaranteedMethod(marshallerType, managedType);
            IMethodSymbol fromUnmanaged = GetFromUnmanagedMethod(marshallerType, unmanagedType);
            if ((toManaged, toManagedGuaranteed) is not (null, null) && fromUnmanaged is not null)
            {
                if (toManagedGuaranteed is not null)
                {
                    shape |= MarshallerShape.GuaranteedUnmarshal;
                }
                if (toManaged is not null)
                {
                    shape |= MarshallerShape.ToManaged;
                }
                methods = methods with
                {
                    FromUnmanaged = fromUnmanaged,
                    ToManaged = toManaged,
                    ToManagedGuranteed = toManagedGuaranteed
                };
            }

            IMethodSymbol free = GetStatefulFreeMethod(marshallerType);
            if (free is not null)
            {
                shape |= MarshallerShape.Free;
                methods = methods with { Free = free };
            }

            IMethodSymbol notifyInvokeSucceeded = GetNotifyInvokeSucceededMethod(marshallerType);
            if (notifyInvokeSucceeded is not null)
            {
                shape |= MarshallerShape.NotifyInvokeSucceeded;
                methods = methods with { NotifyInvokeSucceeded = notifyInvokeSucceeded };
            }

            if (GetStatelessGetPinnableReference(marshallerType, managedType) is not null)
            {
                shape |= MarshallerShape.StatelessPinnableReference;
            }
            if (GetStatefulGetPinnableReference(marshallerType) is not null)
            {
                shape |= MarshallerShape.StatefulPinnableReference;
            }

            return (shape, methods);
        }

        private static IMethodSymbol? GetFromManagedMethod(ITypeSymbol type, ITypeSymbol managedType)
        {
            return type.GetMembers(ShapeMemberNames.Value.Stateful.FromManaged)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 1, ReturnsVoid: true }
                    && SymbolEqualityComparer.Default.Equals(managedType, m.Parameters[0].Type));
        }

        private static IMethodSymbol? GetFromManagedWithCallerAllocatedBufferMethod(
            ITypeSymbol type,
            ITypeSymbol managedType,
            ITypeSymbol spanOfT,
            out ITypeSymbol? spanElementType)
        {
            spanElementType = null;
            IEnumerable<IMethodSymbol> methods = type.GetMembers(ShapeMemberNames.Value.Stateful.FromManaged)
                .OfType<IMethodSymbol>()
                .Where(m => m is { IsStatic: false, Parameters.Length: 2, ReturnsVoid: true }
                    && SymbolEqualityComparer.Default.Equals(managedType, m.Parameters[0].Type));

            foreach (IMethodSymbol method in methods)
            {
                if (IsSpanOfUnmanagedType(method.Parameters[1].Type, spanOfT, out spanElementType))
                {
                    return method;
                }
            }

            return null;

            static bool IsSpanOfUnmanagedType(ITypeSymbol typeToCheck, ITypeSymbol spanOfT, out ITypeSymbol? typeArgument)
            {
                typeArgument = null;
                if (typeToCheck is INamedTypeSymbol namedType
                    && SymbolEqualityComparer.Default.Equals(spanOfT, namedType.ConstructedFrom)
                    && namedType.TypeArguments.Length == 1
                    && namedType.TypeArguments[0].IsUnmanagedType)
                {
                    typeArgument = namedType.TypeArguments[0];
                    return true;
                }

                return false;
            }
        }

        private static IMethodSymbol? GetToManagedMethod(ITypeSymbol type, ITypeSymbol managedType)
        {
            return type.GetMembers(ShapeMemberNames.Value.Stateful.ToManaged)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 0, ReturnsVoid: false, ReturnsByRef: false, ReturnsByRefReadonly: false }
                    && SymbolEqualityComparer.Default.Equals(managedType, m.ReturnType));
        }

        private static IMethodSymbol? GetToManagedGuaranteedMethod(ITypeSymbol type, ITypeSymbol managedType)
        {
            return type.GetMembers(ShapeMemberNames.Value.Stateful.ToManagedGuaranteed)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 0, ReturnsVoid: false, ReturnsByRef: false, ReturnsByRefReadonly: false }
                    && SymbolEqualityComparer.Default.Equals(managedType, m.ReturnType));
        }

        private static IMethodSymbol? GetToUnmanagedMethod(ITypeSymbol type)
        {
            return type.GetMembers(ShapeMemberNames.Value.Stateful.ToUnmanaged)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 0, ReturnsVoid: false, ReturnsByRef: false, ReturnsByRefReadonly: false });
        }

        private static IMethodSymbol? GetFromUnmanagedMethod(ITypeSymbol type, ITypeSymbol? unmanagedType)
        {
            IMethodSymbol[] candidates = type.GetMembers(ShapeMemberNames.Value.Stateful.FromUnmanaged)
                .OfType<IMethodSymbol>()
                .Where(m => m is { IsStatic: false, Parameters.Length: 1, ReturnsVoid: true })
                .ToArray();

            // If there are multiple overloads of FromUnmanaged, we'll treat it as not present.
            // Otherwise we get into a weird state where bidirectional marshallers would support overloads
            // of FromUnmanaged as we'd have an unmanaged type to check, but unmanaged->managed marshallers
            // would not support it as there's no way to know which overload is the correct overload.
            if (candidates.Length != 1)
            {
                return null;
            }

            if (unmanagedType is null)
            {
                // We don't know the unmanaged type to expected for the parameter, so just assume that the only overload of FromUnmanaged
                // is correct.
                return candidates[0];
            }

            if (SymbolEqualityComparer.Default.Equals(candidates[0].Parameters[0].Type, unmanagedType))
            {
                // We know the unmanaged type and it matches.
                // Use the method as we know it will work.
                return candidates[0];
            }

            // The unmanaged type doesn't match the expected type, so we don't have an overload that will work.
            return null;
        }

        private static IMethodSymbol? GetStatefulFreeMethod(ITypeSymbol type)
        {
            return type.GetMembers(ShapeMemberNames.Value.Stateful.Free)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 0, ReturnsVoid: true });
        }

        private static IMethodSymbol? GetNotifyInvokeSucceededMethod(ITypeSymbol type)
        {
            return type.GetMembers(ShapeMemberNames.Value.Stateful.NotifyInvokeSucceeded)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 0, ReturnsVoid: true });
        }

        private static IMethodSymbol? GetStatelessGetPinnableReference(ITypeSymbol type, ITypeSymbol managedType)
        {
            return type.GetMembers(ShapeMemberNames.GetPinnableReference)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: true, Parameters.Length: 1 } and
                    ({ ReturnsByRef: true } or { ReturnsByRefReadonly: true })
                    && SymbolEqualityComparer.Default.Equals(m.Parameters[0].Type, managedType));
        }

        private static IMethodSymbol? GetStatefulGetPinnableReference(ITypeSymbol type)
        {
            return type.GetMembers(ShapeMemberNames.GetPinnableReference)
                .OfType<IMethodSymbol>()
                .FirstOrDefault(m => m is { IsStatic: false, Parameters.Length: 0 } and
                    ({ ReturnsByRef: true } or { ReturnsByRefReadonly: true }));
        }
    }
}
