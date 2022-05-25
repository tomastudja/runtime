// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace Microsoft.WebAssembly.AppHost;

#nullable enable

internal sealed class WasmLogMessage
{
    public string? method { get; set; }
    public string? payload { get; set; }
}
