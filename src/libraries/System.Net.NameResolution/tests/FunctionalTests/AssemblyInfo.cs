// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Xunit;

[assembly: SkipOnPlatform(TestPlatforms.Browser, "System.Net.NameResolution is not supported on Browser.")]
