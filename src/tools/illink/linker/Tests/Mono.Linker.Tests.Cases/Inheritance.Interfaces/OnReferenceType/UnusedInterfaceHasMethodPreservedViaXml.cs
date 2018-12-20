using Mono.Linker.Tests.Cases.Expectations.Assertions;

namespace Mono.Linker.Tests.Cases.Inheritance.Interfaces.OnReferenceType {
	public class UnusedInterfaceHasMethodPreservedViaXml {
		public static void Main ()
		{
			IFoo i = new A ();
			i.Foo ();
		}

		[Kept]
		interface IFoo {
			[Kept]
			void Foo ();
		}

		interface IBar {
			void Bar ();
		}

		[Kept]
		[KeptMember (".ctor()")]
		[KeptInterface (typeof (IFoo))]
		class A : IBar, IFoo {
			[Kept]
			public void Foo ()
			{
			}

			[Kept]
			public void Bar ()
			{
			}
		}
	}
}