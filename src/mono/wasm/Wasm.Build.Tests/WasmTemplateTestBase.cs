// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#nullable enable

using System.IO;
using Xunit.Abstractions;

namespace Wasm.Build.Tests;

public abstract class WasmTemplateTestBase : BuildTestBase
{
    private readonly WasmSdkBasedProjectProvider _provider;
    protected WasmTemplateTestBase(ITestOutputHelper output, SharedBuildPerTestClassFixture buildContext, WasmSdkBasedProjectProvider? projectProvider = null)
                : base(projectProvider ?? new WasmSdkBasedProjectProvider(output), output, buildContext)
    {
        _provider = GetProvider<WasmSdkBasedProjectProvider>();
        // Wasm templates are not using wasm sdk yet
        _provider.BundleDirName = "AppBundle";
    }

    public string CreateWasmTemplateProject(string id, string template = "wasmbrowser", string extraArgs = "", bool runAnalyzers = true)
    {
        InitPaths(id);
        InitProjectDir(_projectDir, addNuGetSourceForLocalPackages: true);

        File.WriteAllText(Path.Combine(_projectDir, "Directory.Build.props"), "<Project />");
        File.WriteAllText(Path.Combine(_projectDir, "Directory.Build.targets"),
            """
            <Project>
              <Target Name="PrintRuntimePackPath" BeforeTargets="Build">
                  <Message Text="** MicrosoftNetCoreAppRuntimePackDir : '@(ResolvedRuntimePack -> '%(PackageDirectory)')'" Importance="High" Condition="@(ResolvedRuntimePack->Count()) > 0" />
              </Target>

              <Import Project="WasmOverridePacks.targets" Condition="'$(WBTOverrideRuntimePack)' == 'true'" />
            </Project>
            """);
        File.Copy(BuildEnvironment.WasmOverridePacksTargetsPath, Path.Combine(_projectDir, Path.GetFileName(BuildEnvironment.WasmOverridePacksTargetsPath)), overwrite: true);

        new DotNetCommand(s_buildEnv, _testOutput, useDefaultArgs: false)
                .WithWorkingDirectory(_projectDir!)
                .ExecuteWithCapturedOutput($"new {template} {extraArgs}")
                .EnsureSuccessful();

        string projectfile = Path.Combine(_projectDir!, $"{id}.csproj");
        string extraProperties = string.Empty;
        extraProperties += "<TreatWarningsAsErrors>true</TreatWarningsAsErrors>";
        if (runAnalyzers)
            extraProperties += "<RunAnalyzers>true</RunAnalyzers>";

        // TODO: Can be removed after updated templates propagate in.
        string extraItems = string.Empty;
        if (template == "wasmbrowser")
            extraItems += "<WasmExtraFilesToDeploy Include=\"main.js\" />";
        else
            extraItems += "<WasmExtraFilesToDeploy Include=\"main.mjs\" />";

        AddItemsPropertiesToProject(projectfile, extraProperties, extraItems);

        return projectfile;
    }

    public (string projectDir, string buildOutput) BuildTemplateProject(BuildArgs buildArgs,
        string id,
        BuildProjectOptions buildProjectOptions)
    {
        (CommandResult res, string logFilePath) = BuildProjectWithoutAssert(id, buildArgs.Config, buildProjectOptions);
        if (buildProjectOptions.UseCache)
            _buildContext.CacheBuild(buildArgs, new BuildProduct(_projectDir!, logFilePath, true, res.Output));

        if (buildProjectOptions.AssertAppBundle)
        {
            if (buildProjectOptions.IsBrowserProject)
                AssertWasmSdkBundle(buildArgs, buildProjectOptions, res.Output);
            else
                AssertTestMainJsBundle(buildArgs, buildProjectOptions, res.Output);
        }
        return (_projectDir!, res.Output);
    }

    public void AssertTestMainJsBundle(BuildArgs buildArgs,
                              BuildProjectOptions buildProjectOptions,
                              string? buildOutput = null,
                              AssertTestMainJsAppBundleOptions? assertAppBundleOptions = null)
    {
        if (buildOutput is not null)
            ProjectProviderBase.AssertRuntimePackPath(buildOutput, buildProjectOptions.TargetFramework ?? DefaultTargetFramework);

        var testMainJsProvider = new TestMainJsProjectProvider(_testOutput, _projectDir!);
        if (assertAppBundleOptions is not null)
            testMainJsProvider.AssertBundle(assertAppBundleOptions);
        else
            testMainJsProvider.AssertBundle(buildArgs, buildProjectOptions);
    }

    public void AssertWasmSdkBundle(BuildArgs buildArgs,
                              BuildProjectOptions buildProjectOptions,
                              string? buildOutput = null,
                              AssertWasmSdkBundleOptions? assertAppBundleOptions = null)
    {
        if (buildOutput is not null)
            ProjectProviderBase.AssertRuntimePackPath(buildOutput, buildProjectOptions.TargetFramework ?? DefaultTargetFramework);

        var projectProvider = new WasmSdkBasedProjectProvider(_testOutput, _projectDir!);
        if (assertAppBundleOptions is not null)
            projectProvider.AssertBundle(assertAppBundleOptions);
        else
            projectProvider.AssertBundle(buildArgs, buildProjectOptions);
    }
}
