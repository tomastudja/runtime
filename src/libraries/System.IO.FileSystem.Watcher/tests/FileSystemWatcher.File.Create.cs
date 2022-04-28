// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Xunit;
using Xunit.Sdk;

namespace System.IO.Tests
{
    public class File_Create_Tests : FileSystemWatcherTest
    {
        [Fact]
        public void FileSystemWatcher_File_Create()
        {
            using (var watcher = new FileSystemWatcher(TestDirectory))
            {
                string fileName = Path.Combine(TestDirectory, "file");
                watcher.Filter = Path.GetFileName(fileName);

                Action action = () => File.Create(fileName).Dispose();
                Action cleanup = () => File.Delete(fileName);

                ExpectEvent(watcher, WatcherChangeTypes.Created, action, cleanup, fileName);
            }
        }

        [Fact]
        [OuterLoop]
        public void FileSystemWatcher_File_Create_EnablingDisablingNotAffectRaisingEvent()
        {
            FileSystemWatcherTest.Execute(() =>
            {
                using (var watcher = new FileSystemWatcher(TestDirectory))
                {
                    string fileName = Path.Combine(TestDirectory, "file");
                    watcher.Filter = Path.GetFileName(fileName);

                    int numberOfRaisedEvents = 0;
                    AutoResetEvent autoResetEvent = new AutoResetEvent(false);
                    FileSystemEventHandler handler = (o, e) =>
                    {
                        Interlocked.Increment(ref numberOfRaisedEvents);
                        autoResetEvent.Set();
                    };

                    watcher.Created += handler;

                    for (int i = 0; i < 100; i++)
                    {
                        watcher.EnableRaisingEvents = true;
                        watcher.EnableRaisingEvents = false;
                    }

                    watcher.EnableRaisingEvents = true;

                    // this should raise one and only one event
                    File.Create(fileName).Dispose();
                    Assert.True(autoResetEvent.WaitOne(WaitForExpectedEventTimeout_NoRetry));
                    Assert.False(autoResetEvent.WaitOne(SubsequentExpectedWait));
                    Assert.True(numberOfRaisedEvents == 1);
                }
            }, DefaultAttemptsForExpectedEvent, (iteration) => RetryDelayMilliseconds);
        }

        [Fact]
        public void FileSystemWatcher_File_Create_ForcedRestart()
        {
            using (var watcher = new FileSystemWatcher(TestDirectory))
            {
                string fileName = Path.Combine(TestDirectory, "file");
                watcher.Filter = Path.GetFileName(fileName);

                Action action = () =>
                {
                    watcher.NotifyFilter = NotifyFilters.FileName; // change filter to force restart
                    File.Create(fileName).Dispose();
                };
                Action cleanup = () => File.Delete(fileName);

                ExpectEvent(watcher, WatcherChangeTypes.Created, action, cleanup, fileName);
            }
        }

        [Fact]
        public void FileSystemWatcher_File_Create_InNestedDirectory()
        {
            string nestedDir = CreateTestDirectory(TestDirectory, "dir1", "nested");
            using (var watcher = new FileSystemWatcher(TestDirectory, "*"))
            {
                watcher.IncludeSubdirectories = true;
                watcher.NotifyFilter = NotifyFilters.FileName;

                string fileName = Path.Combine(nestedDir, "file");
                Action action = () => File.Create(fileName).Dispose();
                Action cleanup = () => File.Delete(fileName);

                ExpectEvent(watcher, WatcherChangeTypes.Created, action, cleanup, fileName);
            }
        }

        [Fact]
        [OuterLoop("This test has a longer than average timeout and may fail intermittently")]
        public void FileSystemWatcher_File_Create_DeepDirectoryStructure()
        {
            string deepDir = CreateTestDirectory(TestDirectory, "dir", "dir", "dir", "dir", "dir", "dir", "dir");
            using (var watcher = new FileSystemWatcher(TestDirectory, "*"))
            {
                watcher.IncludeSubdirectories = true;
                watcher.NotifyFilter = NotifyFilters.FileName;

                // Put a file at the very bottom and expect it to raise an event
                string fileName = Path.Combine(deepDir, "file");
                Action action = () => File.Create(fileName).Dispose();
                Action cleanup = () => File.Delete(fileName);

                ExpectEvent(watcher, WatcherChangeTypes.Created, action, cleanup, fileName, LongWaitTimeout);
            }
        }

        [ConditionalFact(typeof(MountHelper), nameof(MountHelper.CanCreateSymbolicLinks))]
        public void FileSystemWatcher_File_Create_SymLink()
        {
            string dir = CreateTestDirectory(TestDirectory, "dir");
            string temp = CreateTestFile();
            using (var watcher = new FileSystemWatcher(dir, "*"))
            {
                // Make the symlink in our path (to the temp file) and make sure an event is raised
                string symLinkPath = Path.Combine(dir, GetRandomLinkName());
                Action action = () => Assert.True(MountHelper.CreateSymbolicLink(symLinkPath, temp, false));
                Action cleanup = () => File.Delete(symLinkPath);

                ExpectEvent(watcher, WatcherChangeTypes.Created, action, cleanup, symLinkPath);
            }
        }

        [Fact]
        public void FileSystemWatcher_File_Create_SynchronizingObject()
        {
            FileSystemWatcherTest.Execute(() =>
            {
                using (var watcher = new FileSystemWatcher(TestDirectory))
                {
                    TestISynchronizeInvoke invoker = new TestISynchronizeInvoke();
                    watcher.SynchronizingObject = invoker;

                    string fileName = Path.Combine(TestDirectory, "file");
                    watcher.Filter = Path.GetFileName(fileName);

                    Action action = () => File.Create(fileName).Dispose();
                    Action cleanup = () => File.Delete(fileName);

                    ExpectEvent(watcher, WatcherChangeTypes.Created, action, cleanup, fileName);
                    Assert.True(invoker.BeginInvoke_Called);
                }
            }, maxAttempts: DefaultAttemptsForExpectedEvent, backoffFunc: (iteration) => RetryDelayMilliseconds, retryWhen: e => e is XunitException);
        }
    }
}
