﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

namespace Mono.Linker.Tests.Cases.UnreachableBlock.Dependencies
{
	public static class LibReturningConstant
	{
		public static bool ReturnFalse () => false;
	}
}
