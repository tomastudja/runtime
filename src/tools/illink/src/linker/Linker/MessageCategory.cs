// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

namespace Mono.Linker
{
	public enum MessageCategory
	{
		Error = 0,
		Warning,
		Info,
		Diagnostic,

		WarningAsError = 0xFF
	}
}
