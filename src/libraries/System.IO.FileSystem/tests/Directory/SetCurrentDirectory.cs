// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.DotNet.RemoteExecutor;
using Xunit;

namespace System.IO.Tests
{
    public sealed class Directory_SetCurrentDirectory : FileCleanupTestBase
    {
        [Fact]
        public void Null_Path_Throws_ArgumentNullException()
        {
            Assert.Throws<ArgumentNullException>(() => Directory.SetCurrentDirectory(null));
        }

        [Fact]
        public void Empty_Path_Throws_ArgumentException()
        {
            Assert.Throws<ArgumentException>(() => Directory.SetCurrentDirectory(string.Empty));
        }

        [Fact]
        public void SetToNonExistentDirectory_ThrowsDirectoryNotFoundException()
        {
            Assert.Throws<DirectoryNotFoundException>(() => Directory.SetCurrentDirectory(GetTestFilePath()));
        }

        [ConditionalFact(typeof(RemoteExecutor), nameof(RemoteExecutor.IsSupported))]
        public void SetToValidOtherDirectory()
        {
            RemoteExecutor.Invoke(() =>
            {
                Directory.SetCurrentDirectory(TestDirectory);
                // On OSX, the temp directory /tmp/ is a symlink to /private/tmp, so setting the current
                // directory to a symlinked path will result in GetCurrentDirectory returning the absolute
                // path that followed the symlink.
                if (!RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                {
                    Assert.Equal(TestDirectory, Directory.GetCurrentDirectory());
                }
            }).Dispose();
        }

        public sealed class Directory_SetCurrentDirectory_SymLink : FileSystemTest
        {
            private static bool CanCreateSymbolicLinksAndRemoteExecutorSupported => CanCreateSymbolicLinks && RemoteExecutor.IsSupported;

            [ConditionalFact(nameof(CanCreateSymbolicLinksAndRemoteExecutorSupported))]
            public void SetToPathContainingSymLink()
            {
                RemoteExecutor.Invoke(() =>
                {
                    var path = GetTestFilePath();
                    var linkPath = GetTestFilePath();

                    Directory.CreateDirectory(path);
                    Assert.True(MountHelper.CreateSymbolicLink(linkPath, path, isDirectory: true));

                    // Both the symlink and the target exist
                    Assert.True(Directory.Exists(path), "path should exist");
                    Assert.True(Directory.Exists(linkPath), "linkPath should exist");

                    // Set Current Directory to symlink
                    string currentDir = Directory.GetCurrentDirectory();

                    Directory.SetCurrentDirectory(linkPath);
                    if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                    {
                        Assert.Equal(linkPath, Directory.GetCurrentDirectory());
                    }
                    else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                    {
                        Assert.Equal("/private" + path, Directory.GetCurrentDirectory());
                    }
                    else
                    {
                        Assert.Equal(path, Directory.GetCurrentDirectory());
                    }
                }).Dispose();
            }
        }
    }
}
