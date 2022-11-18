// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Mono.Linker.Tests.Cases.UnreachableBlock.Dependencies
{
	public static class LibReturningConstant
	{
		public static bool ReturnFalse () => false;
	}
}
