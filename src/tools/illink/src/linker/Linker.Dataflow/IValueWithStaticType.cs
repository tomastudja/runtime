﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Mono.Cecil;

namespace Mono.Linker.Dataflow
{
	interface IValueWithStaticType
	{
		/// <summary>
		/// The IL type of the value, represented as closely as possible, but not always exact.  It can be null, for
		/// example, when the analysis is imprecise or operating on malformed IL.
		/// </summary>
		TypeDefinition? StaticType { get; }
	}
}
