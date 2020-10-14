﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Mono.Linker.Tests.Cases.DynamicDependencies.Dependencies;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.DynamicDependencies
{
	[SetupCompileBefore ("method_library.dll", new[] { "Dependencies/DynamicDependencyFromAttributeXmlOnNonReferencedAssemblyLibrary.cs" }, defines: new[] { "METHOD" })]
	[SetupCompileBefore ("field_library.dll", new[] { "Dependencies/DynamicDependencyFromAttributeXmlOnNonReferencedAssemblyLibrary.cs" }, defines: new[] { "FIELD" })]
	[KeptAssembly ("method_library.dll")]
	[KeptAssembly ("field_library.dll")]
	[KeptMemberInAssembly ("method_library.dll", "Mono.Linker.Tests.Cases.DynamicDependencies.Dependencies.DynamicDependencyFromAttributeXmlOnNonReferencedAssemblyLibrary_Method", "Method()")]
	[KeptMemberInAssembly ("field_library.dll", "Mono.Linker.Tests.Cases.DynamicDependencies.Dependencies.DynamicDependencyFromAttributeXmlOnNonReferencedAssemblyLibrary_Field", "Method()")]
#if NETCOREAPP
	[SetupLinkAttributesFile ("DynamicDependencyFromAttributeXmlOnNonReferencedAssembly.netcore.Attributes.xml")]
#else
	[SetupLinkAttributesFile ("DynamicDependencyFromAttributeXmlOnNonReferencedAssembly.mono.Attributes.xml")]
#endif
	public class DynamicDependencyFromAttributeXmlOnNonReferencedAssembly
	{
		public static void Main ()
		{
			MethodWithDependencyInXml ();
			_fieldWithDependencyInXml = 0;
		}

		[Kept]
		static void MethodWithDependencyInXml ()
		{
		}

		[Kept]
		static int _fieldWithDependencyInXml;
	}
}