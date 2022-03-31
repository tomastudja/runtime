// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Mono.Linker.Tests.Cases.UnreachableBlock.Dependencies
{
	public class LibWithConstantSubstitution
	{
		static bool _value;

		public static bool ReturnFalse ()
		{
			return _value;
		}
	}
}
