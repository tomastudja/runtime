// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.IO;
using System.Reflection;
using System.Reflection.Metadata;
using System.Reflection.Metadata.Ecma335;
using System.Reflection.PortableExecutable;
using System.Runtime.CompilerServices;

namespace System.Diagnostics
{
    internal class StackTraceSymbols : IDisposable
    {
        private readonly ConditionalWeakTable<Assembly, MetadataReaderProvider?> _metadataCache;

        /// <summary>
        /// Create an instance of this class.
        /// </summary>
        public StackTraceSymbols()
        {
            _metadataCache = new ConditionalWeakTable<Assembly, MetadataReaderProvider?>();
        }

        /// <summary>
        /// Clean up any cached providers.
        /// </summary>
        void IDisposable.Dispose()
        {
            foreach ((Assembly _, MetadataReaderProvider? provider) in _metadataCache)
            {
                provider?.Dispose();
            }

            _metadataCache.Clear();
        }

        /// <summary>
        /// Returns the source file and line number information for the method.
        /// </summary>
        /// <param name="assembly">managed assembly</param>
        /// <param name="assemblyPath">file path of the assembly or null</param>
        /// <param name="loadedPeAddress">loaded PE image address or zero</param>
        /// <param name="loadedPeSize">loaded PE image size</param>
        /// <param name="inMemoryPdbAddress">in memory PDB address or zero</param>
        /// <param name="inMemoryPdbSize">in memory PDB size</param>
        /// <param name="methodToken">method token</param>
        /// <param name="ilOffset">il offset of the stack frame</param>
        /// <param name="sourceFile">source file return</param>
        /// <param name="sourceLine">line number return</param>
        /// <param name="sourceColumn">column return</param>
        internal void GetSourceLineInfo(Assembly assembly, string assemblyPath, IntPtr loadedPeAddress, int loadedPeSize,
            IntPtr inMemoryPdbAddress, int inMemoryPdbSize, int methodToken, int ilOffset,
            out string? sourceFile, out int sourceLine, out int sourceColumn)
        {
            sourceFile = null;
            sourceLine = 0;
            sourceColumn = 0;

            MetadataReader? reader = TryGetReader(assembly, assemblyPath, loadedPeAddress, loadedPeSize, inMemoryPdbAddress, inMemoryPdbSize);
            if (reader != null)
            {
                Handle handle = MetadataTokens.Handle(methodToken);

                if (handle.Kind == HandleKind.MethodDefinition)
                {
                    MethodDebugInformationHandle methodDebugHandle = ((MethodDefinitionHandle)handle).ToDebugInformationHandle();
                    MethodDebugInformation methodInfo = reader.GetMethodDebugInformation(methodDebugHandle);

                    if (!methodInfo.SequencePointsBlob.IsNil)
                    {
                        SequencePointCollection sequencePoints = methodInfo.GetSequencePoints();

                        SequencePoint? bestPointSoFar = null;
                        foreach (SequencePoint point in sequencePoints)
                        {
                            if (point.Offset > ilOffset)
                                break;

                            if (point.StartLine != SequencePoint.HiddenLine)
                                bestPointSoFar = point;
                        }

                        if (bestPointSoFar.HasValue)
                        {
                            sourceLine = bestPointSoFar.Value.StartLine;
                            sourceColumn = bestPointSoFar.Value.StartColumn;
                            sourceFile = reader.GetString(reader.GetDocument(bestPointSoFar.Value.Document).Name);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Returns the portable PDB reader for the assembly path
        /// </summary>
        /// <param name="assembly">Managed Assembly to be used as a cache key</param>
        /// <param name="assemblyPath">
        /// File path of the assembly or null if the module is dynamic (generated by Reflection.Emit).
        /// </param>
        /// <param name="loadedPeAddress">
        /// Loaded PE image address or zero if the module is dynamic (generated by Reflection.Emit).
        /// Dynamic modules have their PDBs (if any) generated to an in-memory stream
        /// (pointed to by <paramref name="inMemoryPdbAddress"/> and <paramref name="inMemoryPdbSize"/>).
        /// </param>
        /// <param name="loadedPeSize">loaded PE image size</param>
        /// <param name="inMemoryPdbAddress">in memory PDB address or zero</param>
        /// <param name="inMemoryPdbSize">in memory PDB size</param>
        /// <returns>reader</returns>
        /// <remarks>
        /// Accounts for unloadable and dynamic types by keying the cache on the managed Assembly. The
        /// underlying ConditionalWeakTable doesn't keep the assembly alive, so cached types will be
        /// correctly invalidated when the Assembly is unloaded by the GC.
        /// </remarks>
        private unsafe MetadataReader? TryGetReader(Assembly assembly, string assemblyPath, IntPtr loadedPeAddress, int loadedPeSize, IntPtr inMemoryPdbAddress, int inMemoryPdbSize)
        {
            if ((loadedPeAddress == IntPtr.Zero || assemblyPath == null) && inMemoryPdbAddress == IntPtr.Zero)
            {
                // Dynamic or in-memory module without symbols (they would be in-memory if they were available).
                return null;
            }

            // The ConditionalWeakTable's GetValue + callback will atomically create the cache entry for us
            // so we are protected from multiple threads racing to get/create the same MetadataReaderProvider
            MetadataReaderProvider? provider = _metadataCache.GetValue(assembly, (assembly) =>
            {
                return (inMemoryPdbAddress != IntPtr.Zero) ?
                            TryOpenReaderForInMemoryPdb(inMemoryPdbAddress, inMemoryPdbSize) :
                            TryOpenReaderFromAssemblyFile(assemblyPath!, loadedPeAddress, loadedPeSize);
            });

            // The reader has already been open, so this doesn't throw.
            return provider?.GetMetadataReader();
        }

        private static unsafe MetadataReaderProvider? TryOpenReaderForInMemoryPdb(IntPtr inMemoryPdbAddress, int inMemoryPdbSize)
        {
            Debug.Assert(inMemoryPdbAddress != IntPtr.Zero);

            // quick check to avoid throwing exceptions below in common cases:
            const uint ManagedMetadataSignature = 0x424A5342;
            if (inMemoryPdbSize < sizeof(uint) || *(uint*)inMemoryPdbAddress != ManagedMetadataSignature)
            {
                // not a Portable PDB
                return null;
            }

            var provider = MetadataReaderProvider.FromMetadataImage((byte*)inMemoryPdbAddress, inMemoryPdbSize);
            try
            {
                // may throw if the metadata is invalid
                provider.GetMetadataReader();
                return provider;
            }
            catch (BadImageFormatException)
            {
                provider.Dispose();
                return null;
            }
        }

        private static unsafe PEReader? TryGetPEReader(string assemblyPath, IntPtr loadedPeAddress, int loadedPeSize)
        {
            // TODO: https://github.com/dotnet/runtime/issues/18423
            //if (loadedPeAddress != IntPtr.Zero && loadedPeSize > 0)
            //{
            //    return new PEReader((byte*)loadedPeAddress, loadedPeSize, isLoadedImage: true);
            //}

            Stream? peStream = TryOpenFile(assemblyPath);
            if (peStream != null)
            {
                return new PEReader(peStream);
            }

            return null;
        }

        private static MetadataReaderProvider? TryOpenReaderFromAssemblyFile(string assemblyPath, IntPtr loadedPeAddress, int loadedPeSize)
        {
            using (var peReader = TryGetPEReader(assemblyPath, loadedPeAddress, loadedPeSize))
            {
                if (peReader == null)
                {
                    return null;
                }

                string? pdbPath;
                MetadataReaderProvider? provider;
                if (peReader.TryOpenAssociatedPortablePdb(assemblyPath, TryOpenFile, out provider, out pdbPath))
                {
                    // TODO:
                    // Consider caching the provider in a global cache (across stack traces) if the PDB is embedded (pdbPath == null),
                    // as decompressing embedded PDB takes some time.
                    return provider;
                }
            }

            return null;
        }

        private static Stream? TryOpenFile(string path)
        {
            if (!File.Exists(path))
            {
                return null;
            }

            try
            {
                // Open the file with read and delete FileShare flags. This matches what dll loading does
                return new FileStream(path!, FileMode.Open, FileAccess.Read, FileShare.Read | FileShare.Delete);
            }
            catch
            {
                return null;
            }
        }
    }
}
