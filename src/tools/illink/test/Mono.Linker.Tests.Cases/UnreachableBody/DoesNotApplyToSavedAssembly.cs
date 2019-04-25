using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;
using Mono.Linker.Tests.Cases.UnreachableBody.Dependencies;

namespace Mono.Linker.Tests.Cases.UnreachableBody {
	[SetupLinkerAction ("save", "other2")]
	[SetupCompileBefore ("other2.dll", new [] {typeof (OtherAssembly)})]
	[KeptMemberInAssembly ("other2.dll", typeof (OtherAssembly.Foo), "Method()")]
	[KeptMemberInAssembly ("other2.dll", typeof (OtherAssembly.Foo), "UsedByMethod()")]
	[KeptMemberInAssembly ("other2.dll", typeof (OtherAssembly), "UnusedSanityCheck()")]
	public class DoesNotApplyToSavedAssembly {
		public static void Main()
		{
			UsedToMarkMethod (null);
		}

		[Kept]
		static void UsedToMarkMethod (OtherAssembly.Foo f)
		{
			f.Method ();
		}
	}
}