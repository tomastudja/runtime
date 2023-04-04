// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.


using System;
using System.Diagnostics;

using System.Reflection.Runtime.General;

using Internal.Runtime.Augments;
using Internal.Runtime.CompilerServices;

using Internal.Metadata.NativeFormat;
using Internal.NativeFormat;
using Internal.TypeSystem;

namespace Internal.Runtime.TypeLoader
{
    /// <summary>
    /// This structure represents metadata-based information used to construct method invokers.
    /// TypeLoaderEnvironment.TryGetMethodInvokeMetadata fills in this structure based on metadata lookup across
    /// all currently registered binary modules.
    /// </summary>
    public struct MethodInvokeMetadata
    {
        /// <summary>
        /// module containing the relevant metadata, null when not found
        /// </summary>
        public NativeFormatModuleInfo MappingTableModule;

        /// <summary>
        /// Method entrypoint
        /// </summary>
        public IntPtr MethodEntryPoint;

        /// <summary>
        /// Raw method entrypoint
        /// </summary>
        public IntPtr RawMethodEntryPoint;

        /// <summary>
        /// Method dictionary for components
        /// </summary>
        public IntPtr DictionaryComponent;

        /// <summary>
        /// Dynamic invoke cookie
        /// </summary>
        public uint DynamicInvokeCookie;

        /// <summary>
        /// Invoke flags
        /// </summary>
        public InvokeTableFlags InvokeTableFlags;
    }

    public sealed partial class TypeLoaderEnvironment
    {
        /// <summary>
        /// Compare two arrays sequentially.
        /// </summary>
        /// <param name="seq1">First array to compare</param>
        /// <param name="seq2">Second array to compare</param>
        /// <returns>
        /// true = arrays have the same values and Equals holds for all pairs of elements
        /// with the same indices
        /// </returns>
        private static bool SequenceEqual<T>(T[] seq1, T[] seq2)
        {
            if (seq1.Length != seq2.Length)
                return false;
            for (int i = 0; i < seq1.Length; i++)
                if (!seq1[i].Equals(seq2[i]))
                    return false;
            return true;
        }

        /// <summary>
        /// Locate blob with given ID and create native reader on it.
        /// </summary>
        /// <param name="module">Address of module to search for the blob</param>
        /// <param name="blob">Blob ID within blob map for the module</param>
        /// <returns>Native reader for the blob (asserts and returns an empty native reader when not found)</returns>
        internal static unsafe NativeReader GetNativeReaderForBlob(NativeFormatModuleInfo module, ReflectionMapBlob blob)
        {
            NativeReader reader;
            if (TryGetNativeReaderForBlob(module, blob, out reader))
            {
                return reader;
            }

            Debug.Assert(false);
            return default(NativeReader);
        }

        /// <summary>
        /// Return the metadata handle for a TypeDef if the pay-for-policy enabled this type as browsable. This is used to obtain name and other information for types
        /// obtained via typeof() or Object.GetType(). This can include generic types (not to be confused with generic instances).
        ///
        /// Preconditions:
        ///    runtimeTypeHandle is a typedef (not a constructed type such as an array or generic instance.)
        /// </summary>
        /// <param name="runtimeTypeHandle">Runtime handle of the type in question</param>
        /// <param name="qTypeDefinition">TypeDef handle for the type</param>
        public unsafe bool TryGetMetadataForNamedType(RuntimeTypeHandle runtimeTypeHandle, out QTypeDefinition qTypeDefinition)
        {
            int hashCode = runtimeTypeHandle.GetHashCode();

            // Iterate over all modules, starting with the module that defines the MethodTable
            foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules(RuntimeAugments.GetModuleFromTypeHandle(runtimeTypeHandle)))
            {
                NativeReader typeMapReader;
                if (TryGetNativeReaderForBlob(module, ReflectionMapBlob.TypeMap, out typeMapReader))
                {
                    NativeParser typeMapParser = new NativeParser(typeMapReader, 0);
                    NativeHashtable typeHashtable = new NativeHashtable(typeMapParser);

                    ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                    externalReferences.InitializeCommonFixupsTable(module);

                    var lookup = typeHashtable.Lookup(hashCode);
                    NativeParser entryParser;
                    while (!(entryParser = lookup.GetNext()).IsNull)
                    {
                        RuntimeTypeHandle foundType = externalReferences.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                        if (foundType.Equals(runtimeTypeHandle))
                        {
                            Handle entryMetadataHandle = entryParser.GetUnsigned().AsHandle();
                            if (entryMetadataHandle.HandleType == HandleType.TypeDefinition)
                            {
                                MetadataReader metadataReader = module.MetadataReader;
                                qTypeDefinition = new QTypeDefinition(metadataReader, entryMetadataHandle.ToTypeDefinitionHandle(metadataReader));
                                return true;
                            }
                        }
                    }
                }
            }

            qTypeDefinition = default;
            return false;
        }

        /// <summary>
        /// Return the RuntimeTypeHandle for the named type described in metadata. This is used to implement the Create and Invoke
        /// apis for types.
        ///
        /// Preconditions:
        ///    metadataReader + typeDefHandle  - a valid metadata reader + typeDefinitionHandle where "metadataReader" is one
        ///                                      of the metadata readers returned by ExecutionEnvironment.MetadataReaders.
        ///
        /// Note: Although this method has a "bool" return value like the other mapping table accessors, the pay-for-play design
        /// guarantees that any type enabled for metadata also has a RuntimeTypeHandle underneath.
        /// </summary>
        /// <param name="qTypeDefinition">TypeDef handle for the type to look up</param>
        /// <param name="runtimeTypeHandle">Runtime type handle (MethodTable) for the given type</param>
        public unsafe bool TryGetNamedTypeForMetadata(QTypeDefinition qTypeDefinition, out RuntimeTypeHandle runtimeTypeHandle)
        {
            if (qTypeDefinition.IsNativeFormatMetadataBased)
            {
                MetadataReader metadataReader = qTypeDefinition.NativeFormatReader;
                TypeDefinitionHandle typeDefHandle = qTypeDefinition.NativeFormatHandle;
                int hashCode = typeDefHandle.ComputeHashCode(metadataReader);

                NativeFormatModuleInfo module = ModuleList.Instance.GetModuleInfoForMetadataReader(metadataReader);

                NativeReader typeMapReader;
                if (TryGetNativeReaderForBlob(module, ReflectionMapBlob.TypeMap, out typeMapReader))
                {
                    NativeParser typeMapParser = new NativeParser(typeMapReader, 0);
                    NativeHashtable typeHashtable = new NativeHashtable(typeMapParser);

                    ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                    externalReferences.InitializeCommonFixupsTable(module);

                    var lookup = typeHashtable.Lookup(hashCode);
                    NativeParser entryParser;
                    while (!(entryParser = lookup.GetNext()).IsNull)
                    {
                        var foundTypeIndex = entryParser.GetUnsigned();
                        if (entryParser.GetUnsigned().AsHandle().Equals(typeDefHandle))
                        {
                            runtimeTypeHandle = externalReferences.GetRuntimeTypeHandleFromIndex(foundTypeIndex);
                            return true;
                        }
                    }
                }
            }

            runtimeTypeHandle = default;
            return false;
        }

        /// <summary>
        /// Return the metadata handle for a TypeRef if this type was referenced indirectly by other type that pay-for-play has denoted as browsable
        /// (for example, as part of a method signature.)
        ///
        /// This is only used in "debug" builds to provide better diagnostics when metadata is missing.
        ///
        /// Preconditions:
        ///    runtimeTypeHandle is a typedef (not a constructed type such as an array or generic instance.)
        /// </summary>
        /// <param name="runtimeTypeHandle">MethodTable of the type in question</param>
        /// <param name="metadataReader">Metadata reader for the type</param>
        /// <param name="typeRefHandle">Located TypeRef handle</param>
        public static unsafe bool TryGetTypeReferenceForNamedType(RuntimeTypeHandle runtimeTypeHandle, out MetadataReader metadataReader, out TypeReferenceHandle typeRefHandle)
        {
            int hashCode = runtimeTypeHandle.GetHashCode();

            // Iterate over all modules, starting with the module that defines the MethodTable
            foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules(RuntimeAugments.GetModuleFromTypeHandle(runtimeTypeHandle)))
            {
                NativeReader typeMapReader;
                if (TryGetNativeReaderForBlob(module, ReflectionMapBlob.TypeMap, out typeMapReader))
                {
                    NativeParser typeMapParser = new NativeParser(typeMapReader, 0);
                    NativeHashtable typeHashtable = new NativeHashtable(typeMapParser);

                    ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                    externalReferences.InitializeCommonFixupsTable(module);

                    var lookup = typeHashtable.Lookup(hashCode);
                    NativeParser entryParser;
                    while (!(entryParser = lookup.GetNext()).IsNull)
                    {
                        RuntimeTypeHandle foundType = externalReferences.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                        if (foundType.Equals(runtimeTypeHandle))
                        {
                            Handle entryMetadataHandle = entryParser.GetUnsigned().AsHandle();
                            if (entryMetadataHandle.HandleType == HandleType.TypeReference)
                            {
                                metadataReader = module.MetadataReader;
                                typeRefHandle = entryMetadataHandle.ToTypeReferenceHandle(metadataReader);
                                return true;
                            }
                        }
                    }
                }
            }

            metadataReader = null;
            typeRefHandle = default(TypeReferenceHandle);

            return false;
        }

        /// <summary>
        /// Return the RuntimeTypeHandle for the named type referenced by another type that pay-for-play denotes as browsable (for example,
        /// in a member signature.) This will only find the typehandle if it is not defined in the current module, and is primarily used
        /// to find non-browsable types.
        ///
        /// This is used to ensure that we can produce a Type object if requested and that it match up with the analogous
        /// Type obtained via typeof().
        ///
        ///
        /// Preconditions:
        ///    metadataReader + typeRefHandle  - a valid metadata reader + typeReferenceHandle where "metadataReader" is one
        ///                                      of the metadata readers returned by ExecutionEnvironment.MetadataReaders.
        ///
        /// Note: Although this method has a "bool" return value like the other mapping table accessors, the pay-for-play design
        /// guarantees that any type that has a metadata TypeReference to it also has a RuntimeTypeHandle underneath.
        /// </summary>
        /// <param name="metadataReader">Metadata reader for module containing the type reference</param>
        /// <param name="typeRefHandle">TypeRef handle to look up</param>
        /// <param name="runtimeTypeHandle">Resolved MethodTable for the type reference</param>
        /// <param name="searchAllModules">Search all modules</param>
        public static unsafe bool TryGetNamedTypeForTypeReference(MetadataReader metadataReader, TypeReferenceHandle typeRefHandle, out RuntimeTypeHandle runtimeTypeHandle, bool searchAllModules = false)
        {
            int hashCode = typeRefHandle.ComputeHashCode(metadataReader);
            NativeFormatModuleInfo typeRefModule = ModuleList.Instance.GetModuleInfoForMetadataReader(metadataReader);
            return TryGetNamedTypeForTypeReference_Inner(metadataReader, typeRefModule, typeRefHandle, hashCode, typeRefModule, out runtimeTypeHandle);
        }

        /// <summary>
        /// Return the RuntimeTypeHandle for the named type referenced by another type that pay-for-play denotes as browsable (for example,
        /// in a member signature.) This lookup will attempt to resolve to an MethodTable in any module to cover situations where the type
        /// does not have a TypeDefinition (non-browsable type) as well as cases where it does.
        ///
        /// Preconditions:
        ///    metadataReader + typeRefHandle  - a valid metadata reader + typeReferenceHandle where "metadataReader" is one
        ///                                      of the metadata readers returned by ExecutionEnvironment.MetadataReaders.
        ///
        /// Note: Although this method has a "bool" return value like the other mapping table accessors, the pay-for-play design
        /// guarantees that any type that has a metadata TypeReference to it also has a RuntimeTypeHandle underneath.
        /// </summary>
        /// <param name="metadataReader">Metadata reader for module containing the type reference</param>
        /// <param name="typeRefHandle">TypeRef handle to look up</param>
        /// <param name="runtimeTypeHandle">Resolved MethodTable for the type reference</param>
        public static unsafe bool TryResolveNamedTypeForTypeReference(MetadataReader metadataReader, TypeReferenceHandle typeRefHandle, out RuntimeTypeHandle runtimeTypeHandle)
        {
            int hashCode = typeRefHandle.ComputeHashCode(metadataReader);
            NativeFormatModuleInfo typeRefModule = ModuleList.Instance.GetModuleInfoForMetadataReader(metadataReader);
            runtimeTypeHandle = default(RuntimeTypeHandle);

            foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules(typeRefModule.Handle))
            {
                if (TryGetNamedTypeForTypeReference_Inner(metadataReader, typeRefModule, typeRefHandle, hashCode, module, out runtimeTypeHandle))
                    return true;
            }

            return false;
        }

        private static unsafe bool TryGetNamedTypeForTypeReference_Inner(MetadataReader metadataReader,
            NativeFormatModuleInfo typeRefModule,
            TypeReferenceHandle typeRefHandle,
            int hashCode,
            NativeFormatModuleInfo module,
            out RuntimeTypeHandle runtimeTypeHandle)
        {
            Debug.Assert(typeRefModule == ModuleList.Instance.GetModuleInfoForMetadataReader(metadataReader));

            NativeReader typeMapReader;
            if (TryGetNativeReaderForBlob(module, ReflectionMapBlob.TypeMap, out typeMapReader))
            {
                NativeParser typeMapParser = new NativeParser(typeMapReader, 0);
                NativeHashtable typeHashtable = new NativeHashtable(typeMapParser);

                ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                externalReferences.InitializeCommonFixupsTable(module);

                var lookup = typeHashtable.Lookup(hashCode);
                NativeParser entryParser;
                while (!(entryParser = lookup.GetNext()).IsNull)
                {
                    var foundTypeIndex = entryParser.GetUnsigned();
                    var handle = entryParser.GetUnsigned().AsHandle();

                    if (module == typeRefModule)
                    {
                        if (handle.Equals(typeRefHandle))
                        {
                            runtimeTypeHandle = externalReferences.GetRuntimeTypeHandleFromIndex(foundTypeIndex);
                            return true;
                        }
                    }
                    else if (handle.HandleType == HandleType.TypeReference)
                    {
                        MetadataReader mrFoundHandle = module.MetadataReader;
                        // We found a type reference handle in another module.. see if it matches
                        if (MetadataReaderHelpers.CompareTypeReferenceAcrossModules(typeRefHandle, metadataReader, handle.ToTypeReferenceHandle(mrFoundHandle), mrFoundHandle))
                        {
                            runtimeTypeHandle = externalReferences.GetRuntimeTypeHandleFromIndex(foundTypeIndex);
                            return true;
                        }
                    }
                    else if (handle.HandleType == HandleType.TypeDefinition)
                    {
                        // We found a type definition handle in another module. See if it matches
                        MetadataReader mrFoundHandle = module.MetadataReader;
                        // We found a type definition handle in another module.. see if it matches
                        if (MetadataReaderHelpers.CompareTypeReferenceToDefinition(typeRefHandle, metadataReader, handle.ToTypeDefinitionHandle(mrFoundHandle), mrFoundHandle))
                        {
                            runtimeTypeHandle = externalReferences.GetRuntimeTypeHandleFromIndex(foundTypeIndex);
                            return true;
                        }
                    }
                }
            }

            runtimeTypeHandle = default(RuntimeTypeHandle);
            return false;
        }

        /// <summary>
        /// Given a RuntimeTypeHandle for any non-dynamic type E, return a RuntimeTypeHandle for type E[]
        /// if the pay for play policy denotes E[] as browsable. This is used to implement Array.CreateInstance().
        /// This is not equivalent to calling TryGetMultiDimTypeForElementType() with a rank of 1!
        ///
        /// Preconditions:
        ///     elementTypeHandle is a valid RuntimeTypeHandle.
        /// </summary>
        /// <param name="elementTypeHandle">MethodTable of the array element type</param>
        /// <param name="arrayTypeHandle">Resolved MethodTable of the array type</param>
        public static unsafe bool TryGetArrayTypeForNonDynamicElementType(RuntimeTypeHandle elementTypeHandle, out RuntimeTypeHandle arrayTypeHandle)
        {
            arrayTypeHandle = new RuntimeTypeHandle();

            int arrayHashcode = TypeHashingAlgorithms.ComputeArrayTypeHashCode(elementTypeHandle.GetHashCode(), -1);

            // Note: ReflectionMapBlob.ArrayMap may not exist in the module that contains the element type.
            // So we must enumerate all loaded modules in order to find ArrayMap and the array type for
            // the given element.
            foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules())
            {
                NativeReader arrayMapReader;
                if (TryGetNativeReaderForBlob(module, ReflectionMapBlob.ArrayMap, out arrayMapReader))
                {
                    NativeParser arrayMapParser = new NativeParser(arrayMapReader, 0);
                    NativeHashtable arrayHashtable = new NativeHashtable(arrayMapParser);

                    ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                    externalReferences.InitializeCommonFixupsTable(module);

                    var lookup = arrayHashtable.Lookup(arrayHashcode);
                    NativeParser entryParser;
                    while (!(entryParser = lookup.GetNext()).IsNull)
                    {
                        RuntimeTypeHandle foundArrayType = externalReferences.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                        RuntimeTypeHandle foundArrayElementType = RuntimeAugments.GetRelatedParameterTypeHandle(foundArrayType);
                        if (foundArrayElementType.Equals(elementTypeHandle))
                        {
                            arrayTypeHandle = foundArrayType;
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        /// <summary>
        /// Locate the static constructor context given the runtime type handle (MethodTable) for the type in question.
        /// </summary>
        /// <param name="typeHandle">MethodTable of the type to look up</param>
        public static unsafe IntPtr TryGetStaticClassConstructionContext(RuntimeTypeHandle typeHandle)
        {
            if (RuntimeAugments.HasCctor(typeHandle))
            {
                if (RuntimeAugments.IsDynamicType(typeHandle))
                {
                    // For dynamic types, its always possible to get the non-gc static data section directly.
                    byte* ptr = (byte*)Instance.TryGetNonGcStaticFieldData(typeHandle);

                    // what we have now is the base address of the non-gc statics of the type
                    // what we need is the cctor context, which is just before that
                    ptr -= sizeof(System.Runtime.CompilerServices.StaticClassConstructionContext);

                    return (IntPtr)ptr;
                }
                else
                {
                    // Non-dynamic types do not provide a way to directly get at the non-gc static region.
                    // Use the CctorContextMap instead.

                    var moduleHandle = RuntimeAugments.GetModuleFromTypeHandle(typeHandle);
                    NativeFormatModuleInfo module = ModuleList.Instance.GetModuleInfoByHandle(moduleHandle);
                    Debug.Assert(!moduleHandle.IsNull);

                    NativeReader typeMapReader;
                    if (TryGetNativeReaderForBlob(module, ReflectionMapBlob.CCtorContextMap, out typeMapReader))
                    {
                        NativeParser typeMapParser = new NativeParser(typeMapReader, 0);
                        NativeHashtable typeHashtable = new NativeHashtable(typeMapParser);

                        ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                        externalReferences.InitializeCommonFixupsTable(module);

                        var lookup = typeHashtable.Lookup(typeHandle.GetHashCode());
                        NativeParser entryParser;
                        while (!(entryParser = lookup.GetNext()).IsNull)
                        {
                            RuntimeTypeHandle foundType = externalReferences.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                            if (foundType.Equals(typeHandle))
                            {
                                byte* pNonGcStaticBase = (byte*)externalReferences.GetIntPtrFromIndex(entryParser.GetUnsigned());

                                // cctor context is located before the non-GC static base
                                return (IntPtr)(pNonGcStaticBase - sizeof(System.Runtime.CompilerServices.StaticClassConstructionContext));
                            }
                        }
                    }
                }

                // If the type has a lazy/deferred Cctor, the compiler must have missed emitting
                // a data structure if we reach this.
                Debug.Assert(false);
            }

            return IntPtr.Zero;
        }

        /// <summary>
        /// Construct the native reader for a given blob in a specified module.
        /// </summary>
        /// <param name="module">Containing binary module for the blob</param>
        /// <param name="blob">Blob ID to fetch from the module</param>
        /// <param name="reader">Native reader created for the module blob</param>
        /// <returns>true when the blob was found in the module, false when not</returns>
        private static unsafe bool TryGetNativeReaderForBlob(NativeFormatModuleInfo module, ReflectionMapBlob blob, out NativeReader reader)
        {
            byte* pBlob;
            uint cbBlob;

            if (module.TryFindBlob(blob, out pBlob, out cbBlob))
            {
                reader = new NativeReader(pBlob, cbBlob);
                return true;
            }

            reader = default(NativeReader);
            return false;
        }

        /// <summary>
        /// Look up the default constructor for a given type. Should not be called by code which has already initialized
        /// the type system.
        /// </summary>
        /// <param name="type">TypeDesc for the type in question</param>
        /// <returns>Function pointer representing the constructor, IntPtr.Zero when not found</returns>
        internal static IntPtr TryGetDefaultConstructorForType(TypeDesc type)
        {
            if (type is DefType defType)
            {
                CanonicallyEquivalentEntryLocator canonHelperSpecific = new CanonicallyEquivalentEntryLocator(defType, CanonicalFormKind.Specific);

                foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules())
                {
                    IntPtr result = TryGetDefaultConstructorForType_Inner(module, ref canonHelperSpecific);

                    if (result != IntPtr.Zero)
                        return result;
                }
            }

            return IntPtr.Zero;
        }

        /// <summary>
        /// Look up the default constructor for a given type. Should not be called by code which has already initialized
        /// the type system.
        /// </summary>
        /// <param name="runtimeTypeHandle">Type handle (MethodTable) for the type in question</param>
        /// <returns>Function pointer representing the constructor, IntPtr.Zero when not found</returns>
        public IntPtr TryGetDefaultConstructorForType(RuntimeTypeHandle runtimeTypeHandle)
        {
            CanonicallyEquivalentEntryLocator canonHelperSpecific = new CanonicallyEquivalentEntryLocator(runtimeTypeHandle, CanonicalFormKind.Specific);

            foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules(RuntimeAugments.GetModuleFromTypeHandle(runtimeTypeHandle)))
            {
                IntPtr result = TryGetDefaultConstructorForType_Inner(module, ref canonHelperSpecific);

                if (result != IntPtr.Zero)
                    return result;
            }

            return IntPtr.Zero;
        }

        /// <summary>
        /// Attempt to locate the default type constructor in a given module.
        /// </summary>
        /// <param name="mappingTableModule">Module to search for the constructor</param>
        /// <param name="canonHelper">Canonically equivalent entry locator representing the type</param>
        /// <returns>Function pointer representing the constructor, IntPtr.Zero when not found</returns>
        internal static unsafe IntPtr TryGetDefaultConstructorForType_Inner(NativeFormatModuleInfo mappingTableModule, ref CanonicallyEquivalentEntryLocator canonHelper)
        {
            NativeReader invokeMapReader;
            if (TryGetNativeReaderForBlob(mappingTableModule, ReflectionMapBlob.InvokeMap, out invokeMapReader))
            {
                NativeParser invokeMapParser = new NativeParser(invokeMapReader, 0);
                NativeHashtable invokeHashtable = new NativeHashtable(invokeMapParser);

                ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                externalReferences.InitializeCommonFixupsTable(mappingTableModule);

                var lookup = invokeHashtable.Lookup(canonHelper.LookupHashCode);
                NativeParser entryParser;
                while (!(entryParser = lookup.GetNext()).IsNull)
                {
                    InvokeTableFlags entryFlags = (InvokeTableFlags)entryParser.GetUnsigned();
                    if ((entryFlags & InvokeTableFlags.IsDefaultConstructor) == 0)
                        continue;

                    entryParser.GetUnsigned(); // Skip method handle or the NameAndSig cookie

                    RuntimeTypeHandle entryType = externalReferences.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                    if (!canonHelper.IsCanonicallyEquivalent(entryType))
                        continue;

                    return externalReferences.GetFunctionPointerFromIndex(entryParser.GetUnsigned());
                }
            }

            // If not found in the invoke map, try the default constructor map
            NativeReader defaultCtorMapReader;
            if (TryGetNativeReaderForBlob(mappingTableModule, ReflectionMapBlob.DefaultConstructorMap, out defaultCtorMapReader))
            {
                NativeParser defaultCtorMapParser = new NativeParser(defaultCtorMapReader, 0);
                NativeHashtable defaultCtorHashtable = new NativeHashtable(defaultCtorMapParser);

                ExternalReferencesTable externalReferencesForDefaultCtorMap = default(ExternalReferencesTable);
                externalReferencesForDefaultCtorMap.InitializeCommonFixupsTable(mappingTableModule);
                var lookup = defaultCtorHashtable.Lookup(canonHelper.LookupHashCode);
                NativeParser defaultCtorParser;
                while (!(defaultCtorParser = lookup.GetNext()).IsNull)
                {
                    RuntimeTypeHandle entryType = externalReferencesForDefaultCtorMap.GetRuntimeTypeHandleFromIndex(defaultCtorParser.GetUnsigned());
                    if (!canonHelper.IsCanonicallyEquivalent(entryType))
                        continue;

                    return externalReferencesForDefaultCtorMap.GetFunctionPointerFromIndex(defaultCtorParser.GetUnsigned());
                }
            }
            return IntPtr.Zero;
        }

        public struct VirtualResolveDataResult
        {
            public RuntimeTypeHandle DeclaringInvokeType;
            public ushort SlotIndex;
            public RuntimeMethodHandle GVMHandle;
            public bool IsGVM;
        }

        public static bool TryGetVirtualResolveData(NativeFormatModuleInfo module,
            RuntimeTypeHandle methodHandleDeclaringType, RuntimeTypeHandle[] genericArgs,
            ref MethodSignatureComparer methodSignatureComparer,
            out VirtualResolveDataResult lookupResult)
        {
            lookupResult = default(VirtualResolveDataResult);
            NativeReader invokeMapReader = GetNativeReaderForBlob(module, ReflectionMapBlob.VirtualInvokeMap);
            NativeParser invokeMapParser = new NativeParser(invokeMapReader, 0);
            NativeHashtable invokeHashtable = new NativeHashtable(invokeMapParser);
            ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
            externalReferences.InitializeCommonFixupsTable(module);

            //
            // The vtable entries for each instantiated type might not necessarily exist.
            // Example 1:
            //      If there's a call to Foo<string>.Method1 and a call to Foo<int>.Method2, Foo<string> will
            //      not have Method2 in its vtable and Foo<int> will not have Method1.
            // Example 2:
            //      If there's a call to Foo<string>.Method1 and a call to Foo<object>.Method2, given that both
            //      of these instantiations share the same canonical form, Foo<__Canon> will have both method
            //      entries, and therefore Foo<string> and Foo<object> will have both entries too.
            // For this reason, the entries that we write to the map will be based on the canonical form
            // of the method's containing type instead of the open type definition.
            //

            CanonicallyEquivalentEntryLocator canonHelper = new CanonicallyEquivalentEntryLocator(
                methodHandleDeclaringType,
                CanonicalFormKind.Specific);

            var lookup = invokeHashtable.Lookup(canonHelper.LookupHashCode);

            NativeParser entryParser;
            while (!(entryParser = lookup.GetNext()).IsNull)
            {
                // Grammar of an entry in the hash table:
                // Virtual Method uses a normal slot
                // TypeKey + NameAndSig metadata offset into the native layout metadata + (NumberOfStepsUpParentHierarchyToType << 1) + slot
                // OR
                // Generic Virtual Method
                // TypeKey + NameAndSig metadata offset into the native layout metadata + (NumberOfStepsUpParentHierarchyToType << 1 + 1)

                RuntimeTypeHandle entryType = externalReferences.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                if (!canonHelper.IsCanonicallyEquivalent(entryType))
                    continue;

                uint nameAndSigPointerToken = entryParser.GetUnsigned();

                MethodNameAndSignature nameAndSig = TypeLoaderEnvironment.Instance.GetMethodNameAndSignatureFromNativeLayoutOffset(module.Handle, nameAndSigPointerToken);
                if (!methodSignatureComparer.IsMatchingNativeLayoutMethodNameAndSignature(nameAndSig.Name, nameAndSig.Signature))
                {
                    continue;
                }

                uint parentHierarchyAndFlag = entryParser.GetUnsigned();
                uint parentHierarchy = parentHierarchyAndFlag >> 1;
                RuntimeTypeHandle declaringTypeOfVirtualInvoke = methodHandleDeclaringType;
                for (uint iType = 0; iType < parentHierarchy; iType++)
                {
                    if (!RuntimeAugments.TryGetBaseType(declaringTypeOfVirtualInvoke, out declaringTypeOfVirtualInvoke))
                    {
                        Debug.Assert(false); // This will only fail if the virtual invoke data is malformed as specifies that a type
                        // has a deeper inheritance hierarchy than it actually does.
                        return false;
                    }
                }

                bool isGenericVirtualMethod = ((parentHierarchyAndFlag & VirtualInvokeTableEntry.FlagsMask) == VirtualInvokeTableEntry.GenericVirtualMethod);

                Debug.Assert(isGenericVirtualMethod == ((genericArgs != null) && genericArgs.Length > 0));

                if (isGenericVirtualMethod)
                {
                    RuntimeSignature methodName;
                    RuntimeSignature methodSignature;

                    if (!TypeLoaderEnvironment.Instance.TryGetMethodNameAndSignaturePointersFromNativeLayoutSignature(module.Handle, nameAndSigPointerToken, out methodName, out methodSignature))
                    {
                        Debug.Assert(false);
                        return false;
                    }

                    RuntimeMethodHandle gvmSlot = TypeLoaderEnvironment.Instance.GetRuntimeMethodHandleForComponents(declaringTypeOfVirtualInvoke, methodName.NativeLayoutSignature(), methodSignature, genericArgs);

                    lookupResult = new VirtualResolveDataResult
                    {
                        DeclaringInvokeType = declaringTypeOfVirtualInvoke,
                        SlotIndex = 0,
                        GVMHandle = gvmSlot,
                        IsGVM = true
                    };
                    return true;
                }
                else
                {
                    uint slot = entryParser.GetUnsigned();

                    lookupResult = new VirtualResolveDataResult
                    {
                        DeclaringInvokeType = declaringTypeOfVirtualInvoke,
                        SlotIndex = checked((ushort)slot),
                        GVMHandle = default(RuntimeMethodHandle),
                        IsGVM = false
                    };
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Try to look up method invoke info for given canon.
        /// </summary>
        /// <param name="declaringTypeHandle">Declaring type for the method</param>
        /// <param name="methodHandle">Method handle</param>
        /// <param name="genericMethodTypeArgumentHandles">Handles of generic argument types</param>
        /// <param name="methodSignatureComparer">Helper class used to compare method signatures</param>
        /// <param name="canonFormKind">Canonical form to use</param>
        /// <param name="methodInvokeMetadata">Output - metadata information for method invoker construction</param>
        /// <returns>true when found, false otherwise</returns>
        public static bool TryGetMethodInvokeMetadata(
            RuntimeTypeHandle declaringTypeHandle,
            QMethodDefinition methodHandle,
            RuntimeTypeHandle[] genericMethodTypeArgumentHandles,
            ref MethodSignatureComparer methodSignatureComparer,
            CanonicalFormKind canonFormKind,
            out MethodInvokeMetadata methodInvokeMetadata)
        {
            if (methodHandle.IsNativeFormatMetadataBased)
            {
                if (TryGetMethodInvokeMetadataFromInvokeMap(
                    methodHandle.NativeFormatReader,
                    declaringTypeHandle,
                    methodHandle.NativeFormatHandle,
                    genericMethodTypeArgumentHandles,
                    ref methodSignatureComparer,
                    canonFormKind,
                    out methodInvokeMetadata))
                {
                    return true;
                }
            }

            methodInvokeMetadata = default;
            return false;
        }

        /// <summary>
        /// Try to look up method invoke info for given canon in InvokeMap blobs for all available modules.
        /// </summary>
        /// <param name="metadataReader">Metadata reader for the declaring type</param>
        /// <param name="declaringTypeHandle">Declaring type for the method</param>
        /// <param name="methodHandle">Method handle</param>
        /// <param name="genericMethodTypeArgumentHandles">Handles of generic argument types</param>
        /// <param name="methodSignatureComparer">Helper class used to compare method signatures</param>
        /// <param name="canonFormKind">Canonical form to use</param>
        /// <param name="methodInvokeMetadata">Output - metadata information for method invoker construction</param>
        /// <returns>true when found, false otherwise</returns>
        private static bool TryGetMethodInvokeMetadataFromInvokeMap(
            MetadataReader metadataReader,
            RuntimeTypeHandle declaringTypeHandle,
            MethodHandle methodHandle,
            RuntimeTypeHandle[] genericMethodTypeArgumentHandles,
            ref MethodSignatureComparer methodSignatureComparer,
            CanonicalFormKind canonFormKind,
            out MethodInvokeMetadata methodInvokeMetadata)
        {
            CanonicallyEquivalentEntryLocator canonHelper = new CanonicallyEquivalentEntryLocator(declaringTypeHandle, canonFormKind);
            TypeManagerHandle methodHandleModule = ModuleList.Instance.GetModuleForMetadataReader(metadataReader);

            foreach (NativeFormatModuleInfo module in ModuleList.EnumerateModules(RuntimeAugments.GetModuleFromTypeHandle(declaringTypeHandle)))
            {
                NativeReader invokeMapReader;
                if (!TryGetNativeReaderForBlob(module, ReflectionMapBlob.InvokeMap, out invokeMapReader))
                {
                    continue;
                }

                NativeParser invokeMapParser = new NativeParser(invokeMapReader, 0);
                NativeHashtable invokeHashtable = new NativeHashtable(invokeMapParser);

                ExternalReferencesTable externalReferences = default(ExternalReferencesTable);
                externalReferences.InitializeCommonFixupsTable(module);

                var lookup = invokeHashtable.Lookup(canonHelper.LookupHashCode);
                var entryData = new InvokeMapEntryDataEnumerator<PreloadedTypeComparator, IntPtr>(
                    new PreloadedTypeComparator(declaringTypeHandle, genericMethodTypeArgumentHandles),
                    canonFormKind,
                    module.Handle,
                    methodHandle,
                    methodHandleModule);

                NativeParser entryParser;
                while (!(entryParser = lookup.GetNext()).IsNull)
                {
                    entryData.GetNext(ref entryParser, ref externalReferences, ref methodSignatureComparer, canonHelper);

                    if (!entryData.IsMatchingOrCompatibleEntry())
                        continue;

                    if (entryData.GetMethodEntryPoint(
                        out methodInvokeMetadata.MethodEntryPoint,
                        out methodInvokeMetadata.DictionaryComponent,
                        out methodInvokeMetadata.RawMethodEntryPoint))
                    {
                        methodInvokeMetadata.MappingTableModule = module;
                        methodInvokeMetadata.DynamicInvokeCookie = entryData._dynamicInvokeCookie;
                        methodInvokeMetadata.InvokeTableFlags = entryData._flags;

                        return true;
                    }
                }
            }

            methodInvokeMetadata = default(MethodInvokeMetadata);
            return false;
        }

        // Api surface for controlling invoke map enumeration.
        private interface IInvokeMapEntryDataDeclaringTypeAndGenericMethodParameterHandling<TDictionaryComponentType>
        {
            bool GetTypeDictionary(out TDictionaryComponentType dictionary);
            bool GetMethodDictionary(MethodNameAndSignature nameAndSignature, out TDictionaryComponentType dictionary);
            bool IsUninterestingDictionaryComponent(TDictionaryComponentType dictionary);
            bool CompareMethodInstantiation(RuntimeTypeHandle[] methodInstantiation);
            bool CanInstantiationsShareCode(RuntimeTypeHandle[] methodInstantiation, CanonicalFormKind canonFormKind);
            IntPtr ProduceFatFunctionPointerMethodEntryPoint(IntPtr methodEntrypoint, TDictionaryComponentType dictionary);
        }

        // Comparator for invoke map when used to find an invoke map entry and the search data is a set of
        // pre-loaded types, and metadata handles.
        private struct PreloadedTypeComparator : IInvokeMapEntryDataDeclaringTypeAndGenericMethodParameterHandling<IntPtr>
        {
            private readonly RuntimeTypeHandle _declaringTypeHandle;
            private readonly RuntimeTypeHandle[] _genericMethodTypeArgumentHandles;

            public PreloadedTypeComparator(RuntimeTypeHandle declaringTypeHandle, RuntimeTypeHandle[] genericMethodTypeArgumentHandles)
            {
                _declaringTypeHandle = declaringTypeHandle;
                _genericMethodTypeArgumentHandles = genericMethodTypeArgumentHandles;
            }

            public bool GetTypeDictionary(out IntPtr dictionary)
            {
                dictionary = RuntimeAugments.GetPointerFromTypeHandle(_declaringTypeHandle);
                Debug.Assert(dictionary != IntPtr.Zero);
                return true;
            }

            public bool GetMethodDictionary(MethodNameAndSignature nameAndSignature, out IntPtr dictionary)
            {
                return TypeLoaderEnvironment.Instance.TryGetGenericMethodDictionaryForComponents(_declaringTypeHandle,
                    _genericMethodTypeArgumentHandles,
                    nameAndSignature,
                    out dictionary);
            }

            public bool IsUninterestingDictionaryComponent(IntPtr dictionary)
            {
                return dictionary == IntPtr.Zero;
            }

            public IntPtr ProduceFatFunctionPointerMethodEntryPoint(IntPtr methodEntrypoint, IntPtr dictionary)
            {
                return FunctionPointerOps.GetGenericMethodFunctionPointer(methodEntrypoint, dictionary);
            }

            public bool CompareMethodInstantiation(RuntimeTypeHandle[] methodInstantiation)
            {
                return SequenceEqual(_genericMethodTypeArgumentHandles, methodInstantiation);
            }

            public bool CanInstantiationsShareCode(RuntimeTypeHandle[] methodInstantiation, CanonicalFormKind canonFormKind)
            {
                return TypeLoaderEnvironment.Instance.CanInstantiationsShareCode(methodInstantiation, _genericMethodTypeArgumentHandles, canonFormKind);
            }
        }

        // Enumerator for discovering methods in the InvokeMap. This is generic to allow highly efficient
        // searching of this table with multiple different input data formats.
        private struct InvokeMapEntryDataEnumerator<TLookupMethodInfo, TDictionaryComponentType> where TLookupMethodInfo : IInvokeMapEntryDataDeclaringTypeAndGenericMethodParameterHandling<TDictionaryComponentType>
        {
            // Read-only inputs
            private TLookupMethodInfo _lookupMethodInfo;
            private readonly CanonicalFormKind _canonFormKind;
            private readonly TypeManagerHandle _moduleHandle;
            private readonly TypeManagerHandle _moduleForMethodHandle;
            private readonly MethodHandle _methodHandle;

            // Parsed data from entry in the hashtable
            public InvokeTableFlags _flags;
            public RuntimeTypeHandle _entryType;
            public IntPtr _methodEntrypoint;
            public uint _dynamicInvokeCookie;
            public RuntimeTypeHandle[] _methodInstantiation;

            // Computed data
            private bool _hasEntryPoint;
            private bool _isMatchingMethodHandleAndDeclaringType;
            private MethodNameAndSignature _nameAndSignature;

            public InvokeMapEntryDataEnumerator(
                TLookupMethodInfo lookupMethodInfo,
                CanonicalFormKind canonFormKind,
                TypeManagerHandle moduleHandle,
                MethodHandle methodHandle,
                TypeManagerHandle moduleForMethodHandle)
            {
                _lookupMethodInfo = lookupMethodInfo;
                _canonFormKind = canonFormKind;
                _moduleHandle = moduleHandle;
                _methodHandle = methodHandle;
                _moduleForMethodHandle = moduleForMethodHandle;

                _flags = 0;
                _entryType = default(RuntimeTypeHandle);
                _methodEntrypoint = IntPtr.Zero;
                _dynamicInvokeCookie = 0xffffffff;
                _hasEntryPoint = false;
                _isMatchingMethodHandleAndDeclaringType = false;
                _methodInstantiation = null;
                _nameAndSignature = null;
            }

            public void GetNext(
                ref NativeParser entryParser,
                ref ExternalReferencesTable extRefTable,
                ref MethodSignatureComparer methodSignatureComparer,
                CanonicallyEquivalentEntryLocator canonHelper)
            {
                // Read flags and reset members data
                _flags = (InvokeTableFlags)entryParser.GetUnsigned();
                _hasEntryPoint = ((_flags & InvokeTableFlags.HasEntrypoint) != 0);
                _isMatchingMethodHandleAndDeclaringType = false;
                _entryType = default(RuntimeTypeHandle);
                _methodEntrypoint = IntPtr.Zero;
                _dynamicInvokeCookie = 0xffffffff;
                _methodInstantiation = null;
                _nameAndSignature = null;

                // If the current entry is not a canonical entry of the same canonical form kind we are looking for, then this cannot be a match
                if (((_flags & InvokeTableFlags.IsUniversalCanonicalEntry) != 0) != (_canonFormKind == CanonicalFormKind.Universal))
                    return;

                if ((_flags & InvokeTableFlags.HasMetadataHandle) != 0)
                {
                    // Metadata handles are not known cross module, and cannot be compared across modules.
                    if (_moduleHandle != _moduleForMethodHandle)
                        return;

                    Handle entryMethodHandle = (((uint)HandleType.Method << 24) | entryParser.GetUnsigned()).AsHandle();
                    if (!_methodHandle.Equals(entryMethodHandle))
                        return;
                }
                else
                {
                    uint nameAndSigToken = entryParser.GetUnsigned();
                    MethodNameAndSignature nameAndSig = TypeLoaderEnvironment.Instance.GetMethodNameAndSignatureFromNativeLayoutOffset(_moduleHandle, nameAndSigToken);
                    Debug.Assert(nameAndSig.Signature.IsNativeLayoutSignature);
                    if (!methodSignatureComparer.IsMatchingNativeLayoutMethodNameAndSignature(nameAndSig.Name, nameAndSig.Signature))
                        return;
                }

                _entryType = extRefTable.GetRuntimeTypeHandleFromIndex(entryParser.GetUnsigned());
                if (!canonHelper.IsCanonicallyEquivalent(_entryType))
                    return;

                // Method handle and entry type match at this point. Continue reading data from the entry...
                _isMatchingMethodHandleAndDeclaringType = true;

                if (_hasEntryPoint)
                    _methodEntrypoint = extRefTable.GetFunctionPointerFromIndex(entryParser.GetUnsigned());

                if ((_flags & InvokeTableFlags.NeedsParameterInterpretation) == 0)
                    _dynamicInvokeCookie = entryParser.GetUnsigned();

                if ((_flags & InvokeTableFlags.IsGenericMethod) == 0)
                    return;

                if ((_flags & InvokeTableFlags.RequiresInstArg) != 0)
                {
                    Debug.Assert(_hasEntryPoint || ((_flags & InvokeTableFlags.HasVirtualInvoke) != 0));

                    uint nameAndSigPointerToken = entryParser.GetUnsigned();
                    _nameAndSignature = TypeLoaderEnvironment.Instance.GetMethodNameAndSignatureFromNativeLayoutOffset(_moduleHandle, nameAndSigPointerToken);
                }

                if ((_flags & InvokeTableFlags.IsUniversalCanonicalEntry) == 0)
                {
                    _methodInstantiation = GetTypeSequence(ref extRefTable, ref entryParser);
                }
            }

            public bool IsMatchingOrCompatibleEntry()
            {
                // Check if method handle and entry type were matching or compatible
                if (!_isMatchingMethodHandleAndDeclaringType)
                    return false;

                // Nothing special about non-generic methods.
                if ((_flags & InvokeTableFlags.IsGenericMethod) == 0)
                    return true;

                // A universal canonical method entry can share code with any method instantiation (no need to call CanInstantiationsShareCode())
                if ((_flags & InvokeTableFlags.IsUniversalCanonicalEntry) != 0)
                {
                    Debug.Assert(_canonFormKind == CanonicalFormKind.Universal);
                    return true;
                }

                return _lookupMethodInfo.CanInstantiationsShareCode(_methodInstantiation, _canonFormKind);
            }

            public bool GetMethodEntryPoint(out IntPtr methodEntrypoint, out TDictionaryComponentType dictionaryComponent, out IntPtr rawMethodEntrypoint)
            {
                // Debug-only sanity check before proceeding (IsMatchingOrCompatibleEntry is called from TryGetDynamicMethodInvokeInfo)
                Debug.Assert(IsMatchingOrCompatibleEntry());

                rawMethodEntrypoint = _methodEntrypoint;
                methodEntrypoint = IntPtr.Zero;

                if (!GetDictionaryComponent(out dictionaryComponent) || !GetMethodEntryPointComponent(dictionaryComponent, out methodEntrypoint))
                    return false;

                return true;
            }

            private bool GetDictionaryComponent(out TDictionaryComponentType dictionaryComponent)
            {
                dictionaryComponent = default(TDictionaryComponentType);

                if (((_flags & InvokeTableFlags.RequiresInstArg) == 0) || !_hasEntryPoint)
                    return true;

                // Dictionary for non-generic method is the type handle of the declaring type
                if ((_flags & InvokeTableFlags.IsGenericMethod) == 0)
                {
                    return _lookupMethodInfo.GetTypeDictionary(out dictionaryComponent);
                }

                // Dictionary for generic method (either found statically or constructed dynamically)
                return _lookupMethodInfo.GetMethodDictionary(_nameAndSignature, out dictionaryComponent);
            }

            private bool GetMethodEntryPointComponent(TDictionaryComponentType dictionaryComponent, out IntPtr methodEntrypoint)
            {
                methodEntrypoint = _methodEntrypoint;

                if (_lookupMethodInfo.IsUninterestingDictionaryComponent(dictionaryComponent))
                    return true;

                // Do not use a fat function-pointer for universal canonical methods because the converter data block already holds the
                // dictionary pointer so it serves as its own instantiating stub
                if ((_flags & InvokeTableFlags.IsUniversalCanonicalEntry) == 0)
                    methodEntrypoint = _lookupMethodInfo.ProduceFatFunctionPointerMethodEntryPoint(_methodEntrypoint, dictionaryComponent);

                return true;
            }
        }

        public bool TryGetMetadataForTypeMethodNameAndSignature(RuntimeTypeHandle declaringTypeHandle, MethodNameAndSignature nameAndSignature, out QMethodDefinition methodHandle)
        {
            if (!nameAndSignature.Signature.IsNativeLayoutSignature)
            {
                ModuleInfo moduleInfo = nameAndSignature.Signature.GetModuleInfo();
                methodHandle = new QMethodDefinition(((NativeFormatModuleInfo)moduleInfo).MetadataReader, nameAndSignature.Signature.Token.AsHandle().ToMethodHandle(null));

                // When working with method signature that draw directly from metadata, just return the metadata token
                return true;
            }

            QTypeDefinition qTypeDefinition;
            RuntimeTypeHandle metadataLookupTypeHandle = GetTypeDefinition(declaringTypeHandle);
            methodHandle = default(QMethodDefinition);

            if (!TryGetMetadataForNamedType(metadataLookupTypeHandle, out qTypeDefinition))
                return false;

            MetadataReader reader = qTypeDefinition.NativeFormatReader;
            TypeDefinitionHandle typeDefinitionHandle = qTypeDefinition.NativeFormatHandle;

            TypeDefinition typeDefinition = typeDefinitionHandle.GetTypeDefinition(reader);

            Debug.Assert(nameAndSignature.Signature.IsNativeLayoutSignature);

            foreach (MethodHandle mh in typeDefinition.Methods)
            {
                Method method = mh.GetMethod(reader);
                if (method.Name.StringEquals(nameAndSignature.Name, reader))
                {
                    MethodSignatureComparer methodSignatureComparer = new MethodSignatureComparer(reader, mh);
                    if (methodSignatureComparer.IsMatchingNativeLayoutMethodSignature(nameAndSignature.Signature))
                    {
                        methodHandle = new QMethodDefinition(reader, mh);
                        return true;
                    }
                }
            }

            return false;
        }
    }
}
