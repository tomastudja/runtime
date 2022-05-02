// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#nullable enable

using System;

namespace Microsoft.WebAssembly.Diagnostics;

public record RunLoopExitState(RunLoopStopReason reason, Exception? exception)
{
}
