// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.CompilerServices;

namespace System.Reflection
{
    public abstract partial class MethodInfo : MethodBase
    {
        protected MethodInfo() { }

        public override MemberTypes MemberType => MemberTypes.Method;

        public virtual ParameterInfo ReturnParameter => throw NotImplemented.ByDesign;
        public virtual Type ReturnType => throw NotImplemented.ByDesign;

        public override Type[] GetGenericArguments() { throw new NotSupportedException(SR.NotSupported_SubclassOverride); }
        public virtual MethodInfo GetGenericMethodDefinition() { throw new NotSupportedException(SR.NotSupported_SubclassOverride); }
        public virtual MethodInfo MakeGenericMethod(params Type[] typeArguments) { throw new NotSupportedException(SR.NotSupported_SubclassOverride); }

        public abstract MethodInfo GetBaseDefinition();

        public abstract ICustomAttributeProvider ReturnTypeCustomAttributes { get; }

        public virtual Delegate CreateDelegate(Type delegateType) { throw new NotSupportedException(SR.NotSupported_SubclassOverride); }
        public virtual Delegate CreateDelegate(Type delegateType, object? target) { throw new NotSupportedException(SR.NotSupported_SubclassOverride); }

        /// <summary>Creates a delegate of the given type 'T' from this method.</summary>
        public T CreateDelegate<T>() where T : Delegate => (T)CreateDelegate(typeof(T));

        /// <summary>Creates a delegate of the given type 'T' with the specified target from this method.</summary>
        public T CreateDelegate<T>(object? target) where T : Delegate => (T)CreateDelegate(typeof(T), target);

        public override bool Equals(object? obj) => base.Equals(obj);
        public override int GetHashCode() => base.GetHashCode();

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(MethodInfo? left, MethodInfo? right)
        {
            // Test "right" first to allow branch elimination when inlined for null checks (== null)
            // so it can become a simple test
            if (right is null)
            {
                // return true/false not the test result https://github.com/dotnet/runtime/issues/4207
                return (left is null) ? true : false;
            }

            // Try fast reference equality and opposite null check prior to calling the slower virtual Equals
            if ((object?)left == (object)right)
            {
                return true;
            }

            return (left is null) ? false : left.Equals(right);
        }

        public static bool operator !=(MethodInfo? left, MethodInfo? right) => !(left == right);
    }
}
