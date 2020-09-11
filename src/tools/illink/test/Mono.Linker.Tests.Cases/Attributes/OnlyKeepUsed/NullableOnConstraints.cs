#nullable enable

using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.Attributes.OnlyKeepUsed
{
#if ILLINK
	[IgnoreTestCase ("Ignored for illink for some reason")]
#endif
	[SetupCSharpCompilerToUse ("csc")]
	[SetupCompileArgument ("/langversion:8.0")]
	[SetupLinkerArgument ("--used-attrs-only", "true")]
	public class NullableOnConstraints
	{
		public static void Main ()
		{
			Test.Run ();
		}

		[Kept]
		[KeptInterface (typeof (I))]
		class Test : I
		{
			[Kept]
			public static void Run ()
			{
				new C<Test> ();
				Method<Test> ();
			}

			[Kept]
			static T? Method<T> () where T : class, I?
			{
				return default;
			}
		}
	}

	[Kept]
	interface I
	{
	}

	[Kept]
	[KeptMember (".ctor()")]
	class C<T> where T : I?
	{
	}
}