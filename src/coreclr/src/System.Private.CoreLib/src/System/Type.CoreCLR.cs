// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.CompilerServices;
using StackCrawlMark = System.Threading.StackCrawlMark;

namespace System
{
    public abstract partial class Type : MemberInfo, IReflect
    {
        public bool IsInterface
        {
            get
            {
                if (this is RuntimeType rt)
                    return RuntimeTypeHandle.IsInterface(rt);
                return (GetAttributeFlagsImpl() & TypeAttributes.ClassSemanticsMask) == TypeAttributes.Interface;
            }
        }

        [RequiresUnreferencedCode("The type might be removed")]
        [System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        public static Type? GetType(string typeName, bool throwOnError, bool ignoreCase)
        {
            StackCrawlMark stackMark = StackCrawlMark.LookForMyCaller;
            return RuntimeType.GetType(typeName, throwOnError, ignoreCase, ref stackMark);
        }

        [RequiresUnreferencedCode("The type might be removed")]
        [System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        public static Type? GetType(string typeName, bool throwOnError)
        {
            StackCrawlMark stackMark = StackCrawlMark.LookForMyCaller;
            return RuntimeType.GetType(typeName, throwOnError, false, ref stackMark);
        }

        [RequiresUnreferencedCode("The type might be removed")]
        [System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        public static Type? GetType(string typeName)
        {
            StackCrawlMark stackMark = StackCrawlMark.LookForMyCaller;
            return RuntimeType.GetType(typeName, false, false, ref stackMark);
        }

        [RequiresUnreferencedCode("The type might be removed")]
        [System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        public static Type? GetType(
            string typeName,
            Func<AssemblyName, Assembly?>? assemblyResolver,
            Func<Assembly?, string, bool, Type?>? typeResolver)
        {
            StackCrawlMark stackMark = StackCrawlMark.LookForMyCaller;
            return TypeNameParser.GetType(typeName, assemblyResolver, typeResolver, false, false, ref stackMark);
        }

        [RequiresUnreferencedCode("The type might be removed")]
        [System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        public static Type? GetType(
            string typeName,
            Func<AssemblyName, Assembly?>? assemblyResolver,
            Func<Assembly?, string, bool, Type?>? typeResolver,
            bool throwOnError)
        {
            StackCrawlMark stackMark = StackCrawlMark.LookForMyCaller;
            return TypeNameParser.GetType(typeName, assemblyResolver, typeResolver, throwOnError, false, ref stackMark);
        }

        [RequiresUnreferencedCode("The type might be removed")]
        [System.Security.DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        public static Type? GetType(
            string typeName,
            Func<AssemblyName, Assembly?>? assemblyResolver,
            Func<Assembly?, string, bool, Type?>? typeResolver,
            bool throwOnError,
            bool ignoreCase)
        {
            StackCrawlMark stackMark = StackCrawlMark.LookForMyCaller;
            return TypeNameParser.GetType(typeName, assemblyResolver, typeResolver, throwOnError, ignoreCase, ref stackMark);
        }

        ////////////////////////////////////////////////////////////////////////////////
        // This will return a class based upon the progID.  This is provided for
        // COM classic support.  Program ID's are not used in COM+ because they
        // have been superceded by namespace.  (This routine is called this instead
        // of getClass() because of the name conflict with the first method above.)
        //
        //   param progID:     the progID of the class to retrieve
        //   returns:          the class object associated to the progID
        ////
        public static Type? GetTypeFromProgID(string progID, string? server, bool throwOnError)
        {
            return RuntimeType.GetTypeFromProgIDImpl(progID, server, throwOnError);
        }

        ////////////////////////////////////////////////////////////////////////////////
        // This will return a class based upon the CLSID.  This is provided for
        // COM classic support.
        //
        //   param CLSID:      the CLSID of the class to retrieve
        //   returns:          the class object associated to the CLSID
        ////
        public static Type? GetTypeFromCLSID(Guid clsid, string? server, bool throwOnError)
        {
            return RuntimeType.GetTypeFromCLSIDImpl(clsid, server, throwOnError);
        }

        internal virtual RuntimeTypeHandle GetTypeHandleInternal()
        {
            return TypeHandle;
        }

        // Given a class handle, this will return the class for that handle.
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern RuntimeType GetTypeFromHandleUnsafe(IntPtr handle);

        [Intrinsic]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern Type GetTypeFromHandle(RuntimeTypeHandle handle);

        [Intrinsic]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool operator ==(Type? left, Type? right);

        [Intrinsic]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool operator !=(Type? left, Type? right);

        // Exists to faciliate code sharing between CoreCLR and CoreRT.
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal bool IsRuntimeImplemented() => this is RuntimeType;
    }
}
