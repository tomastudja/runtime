// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.IO.Tests;
using Xunit;

namespace System.IO.MemoryMappedFiles.Tests
{
    public abstract class MemoryMappedViewStreamConformanceTests : StandaloneStreamConformanceTests
    {
        protected override bool CanSetLength => false;

        protected override Stream CreateReadOnlyStreamCore(byte[] initialData) => CreateStream(initialData, FileAccess.Read);
        protected override Stream CreateReadWriteStreamCore(byte[] initialData) => CreateStream(initialData, FileAccess.ReadWrite);
        protected override Stream CreateWriteOnlyStreamCore(byte[] initialData) => CreateStream(initialData, FileAccess.Write);

        protected abstract MemoryMappedFile CreateFile(int length);

        private MemoryMappedViewStream CreateStream(byte[] initialData, FileAccess access)
        {
            MemoryMappedViewStream stream = null;

            if (initialData is not null)
            {
                using MemoryMappedFile mmf = CreateFile(initialData.Length);
                if (mmf != null)
                {
                    using (MemoryMappedViewStream s = mmf.CreateViewStream())
                    {
                        s.Write(initialData);
                    }

                    stream = mmf.CreateViewStream(0, initialData.Length, access switch
                    {
                        FileAccess.Read => MemoryMappedFileAccess.Read,
                        FileAccess.ReadWrite => MemoryMappedFileAccess.ReadWrite,
                        FileAccess.Write => MemoryMappedFileAccess.Write,
                        _ => throw new Exception($"Unexpected FileAccess: {access}")
                    });
                }
            }

            return stream;
        }
    }

    public class AnonymousMemoryMappedViewStreamConformanceTests : MemoryMappedViewStreamConformanceTests
    {
        protected override MemoryMappedFile CreateFile(int length) =>
            MemoryMappedFile.CreateNew(null, length);
    }

    public class NamedMemoryMappedViewStreamConformanceTests : MemoryMappedViewStreamConformanceTests
    {
        protected override MemoryMappedFile CreateFile(int length) =>
            MemoryMappedFilesTestBase.MapNamesSupported ? MemoryMappedFile.CreateNew(MemoryMappedFilesTestBase.CreateUniqueMapName(), length) :
            null;
    }

    public class FileMemoryMappedViewStreamConformanceTests : MemoryMappedViewStreamConformanceTests
    {
        protected override MemoryMappedFile CreateFile(int length) =>
            MemoryMappedFile.CreateFromFile(Path.Combine(TestDirectory, Guid.NewGuid().ToString("N")), FileMode.CreateNew, null, length, MemoryMappedFileAccess.ReadWrite);
    }

    public class NamedFileMemoryMappedViewStreamConformanceTests : MemoryMappedViewStreamConformanceTests
    {
        protected override MemoryMappedFile CreateFile(int length) =>
            MemoryMappedFilesTestBase.MapNamesSupported ? MemoryMappedFile.CreateFromFile(Path.Combine(TestDirectory, Guid.NewGuid().ToString("N")), FileMode.CreateNew, MemoryMappedFilesTestBase.CreateUniqueMapName(), length, MemoryMappedFileAccess.ReadWrite) :
            null;
    }
}
