﻿using System;
using System.IO;
using BundleTests.Helpers;
using Microsoft.DotNet.Cli.Build.Framework;
using Microsoft.DotNet.CoreSetup.Test;
using Microsoft.NET.HostModel.Bundle;
using Xunit;

namespace AppHost.Bundle.Tests
{
    public class SingleFileApiTests : BundleTestBase, IClassFixture<SingleFileSharedState>
    {
        private SingleFileSharedState sharedTestState;

        public SingleFileApiTests(SingleFileSharedState fixture)
        {
            sharedTestState = fixture;
        }

        [Fact]
        public void SelfContained_SingleFile_APITests()
        {
            string singleFile = BundleHelper.GetHostPath(sharedTestState.PublishedSingleFile);
            Command.Create(singleFile, "fullyqualifiedname codebase appcontext cmdlineargs executing_assembly_location basedirectory")
                .CaptureStdErr()
                .CaptureStdOut()
                .Execute()
                .Should()
                .Pass()
                .And.HaveStdOutContaining("FullyQualifiedName: <Unknown>")
                .And.HaveStdOutContaining("Name: <Unknown>")
                .And.HaveStdOutContaining("CodeBase NotSupported")
                .And.NotHaveStdOutContaining("SingleFileApiTests.deps.json")
                .And.NotHaveStdOutContaining("Microsoft.NETCore.App.deps.json")
                // For single-file, Environment.GetCommandLineArgs[0] should return the file path of the host.
                .And.HaveStdOutContaining("Command line args: " + singleFile)
                .And.HaveStdOutContaining("ExecutingAssembly.Location: " + Environment.NewLine)
                .And.HaveStdOutContaining("AppContext.BaseDirectory: " + Path.GetDirectoryName(singleFile));
        }

        [Fact]
        public void SelfContained_NetCoreApp3_CompatMode_SingleFile_APITests()
        {
            var fixture = sharedTestState.TestFixture.Copy();
            var singleFile = BundleSelfContainedApp(fixture, BundleOptions.BundleAllContent);
            var extractionBaseDir = BundleHelper.GetExtractionRootDir(fixture);

            Command.Create(singleFile, "fullyqualifiedname codebase appcontext cmdlineargs executing_assembly_location basedirectory")
                .CaptureStdErr()
                .CaptureStdOut()
                .EnvironmentVariable(Constants.BundleExtractBase.EnvironmentVariable, extractionBaseDir.FullName)
                .Execute()
                .Should()
                .Pass()
                .And.HaveStdOutContaining(Path.DirectorySeparatorChar + "System.Private.CoreLib.dll") // In extraction directory
                .And.HaveStdOutContaining("System.Private.CoreLib.dll") // In extraction directory
                .And.NotHaveStdOutContaining("CodeBase NotSupported") // CodeBase should point to extraction directory
                .And.HaveStdOutContaining("SingleFileApiTests.dll")
                .And.HaveStdOutContaining("SingleFileApiTests.deps.json") // The app's .deps.json should be available
                .And.NotHaveStdOutContaining("Microsoft.NETCore.App.deps.json") // No framework - it's self-contained
                // For single-file, Environment.GetCommandLineArgs[0] should return the file path of the host.
                .And.HaveStdOutContaining("Command line args: " + singleFile)
                .And.HaveStdOutContaining("ExecutingAssembly.Location: " + extractionBaseDir.FullName) // Should point to the app's dll
                .And.HaveStdOutContaining("AppContext.BaseDirectory: " + extractionBaseDir.FullName); // Should point to the extraction directory
        }

        [Fact]
        public void GetCommandLineArgs_0_Non_Bundled_App()
        {
            var fixture = sharedTestState.TestFixture.Copy();
            var dotnet = fixture.BuiltDotnet;
            var appPath = BundleHelper.GetAppPath(fixture);

            // For non single-file apps, Environment.GetCommandLineArgs[0]
            // should return the file path of the managed entrypoint.
            dotnet.Exec(appPath, "cmdlineargs")
                .CaptureStdErr()
                .CaptureStdOut()
                .Execute()
                .Should()
                .Pass()
                .And
                .HaveStdOutContaining(appPath);
        }

        [Fact]
        public void AppContext_Native_Search_Dirs_Contains_Bundle_Dir()
        {
            string singleFile = BundleHelper.GetHostPath(sharedTestState.PublishedSingleFile);
            string extractionRoot = BundleHelper.GetExtractionRootPath(sharedTestState.PublishedSingleFile);
            string bundleDir = BundleHelper.GetPublishPath(sharedTestState.PublishedSingleFile);

            // If we don't extract anything to disk, the extraction dir shouldn't
            // appear in the native search dirs.
            Command.Create(singleFile, "native_search_dirs")
                .CaptureStdErr()
                .CaptureStdOut()
                .Execute()
                .Should().Pass()
                .And.HaveStdOutContaining(bundleDir)
                .And.NotHaveStdOutContaining(extractionRoot);
        }

        [Fact]
        public void AppContext_Native_Search_Dirs_Contains_Bundle_And_Extraction_Dirs()
        {
            var fixture = sharedTestState.TestFixture.Copy();
            Bundler bundler = BundleSelfContainedApp(fixture, out string singleFile, BundleOptions.BundleNativeBinaries);
            string extractionDir = BundleHelper.GetExtractionDir(fixture, bundler).Name;
            string bundleDir = BundleHelper.GetBundleDir(fixture).FullName;

            Command.Create(singleFile, "native_search_dirs")
                .CaptureStdErr()
                .CaptureStdOut()
                .Execute()
                .Should().Pass()
                .And.HaveStdOutContaining(extractionDir)
                .And.HaveStdOutContaining(bundleDir);
        }
    }
}
