using System.Runtime.CompilerServices;
using System.Diagnostics.CodeAnalysis;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.DynamicDependencies.Dependencies
{
	public class DynamicDependencyInCopyAssembly
	{
		[DynamicDependency ("ExtraMethod1")]
		public DynamicDependencyInCopyAssembly ()
		{
		}

		static void ExtraMethod1 ()
		{
		}
	}
}
