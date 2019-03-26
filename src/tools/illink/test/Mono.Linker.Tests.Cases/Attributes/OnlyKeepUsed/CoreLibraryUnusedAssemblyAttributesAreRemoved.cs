using System.Reflection;
using System.Timers;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.Attributes.OnlyKeepUsed {
#if NETCOREAPP
	[IgnoreTestCase ("Needs investigation")]
#endif
	[Reference ("System.dll")]
	[SetupLinkerCoreAction ("link")]
	[SetupLinkerArgument ("--used-attrs-only", "true")]
	[RemovedAttributeInAssembly (PlatformAssemblies.CoreLib, typeof (AssemblyDescriptionAttribute))]
#if !NETCOREAPP
	[RemovedAttributeInAssembly ("System.dll", typeof (AssemblyDescriptionAttribute))]
#endif
	[SkipPeVerify]
	public class CoreLibraryUnusedAssemblyAttributesAreRemoved {
		public static void Main ()
		{
			// Use something from System so that the entire reference isn't linked away
			var system = new Timer ();
		}
	}
}