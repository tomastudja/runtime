﻿using System;
using Mono.Linker.Tests.Cases.Attributes.Dependencies;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.Attributes.Mcs {
	/// <summary>
	/// This explicit mcs test exists because mcs did not add a reference prior to https://github.com/mono/mono/commit/f71b208ca7b41a2a97ca70b955df0c4c411ce8e5
	/// </summary>
	[IgnoreTestCase ("Can be enabled once the mcs contains https://github.com/mono/mono/commit/f71b208ca7b41a2a97ca70b955df0c4c411ce8e5")]
	[SetupCSharpCompilerToUse ("mcs")]
	[SetupCompileBefore ("LibraryWithType.dll", new [] { typeof(TypeDefinedInReference) })]
	[SetupCompileBefore ("LibraryWithAttribute.dll", new [] { typeof(AttributeDefinedInReference) })]
	[KeptTypeInAssembly ("LibraryWithType.dll", typeof (TypeDefinedInReference))]
	[RemovedMemberInAssembly ("LibraryWithType.dll", typeof (TypeDefinedInReference), "Unused()")]
	[KeptMemberInAssembly ("LibraryWithAttribute.dll", typeof (AttributeDefinedInReference), ".ctor()")]
	[KeptMemberInAssembly ("LibraryWithAttribute.dll", typeof (AttributeDefinedInReference), "FieldType")]
	[KeptDelegateCacheField ("0")]
	public class OnlyTypeUsedInAssemblyIsTypeOnAttributeFieldOnEvent {
		public static void Main ()
		{
			var foo = new Foo ();
			foo.MyEvent += FooOnMyEvent; 
		}

		[Kept]
		private static void FooOnMyEvent (object sender, EventArgs e)
		{
		}

		[Kept]
		[KeptMember (".ctor()")]
		class Foo {
			[Kept]
			[KeptBackingField]
			[KeptEventAddMethod]
			[KeptEventRemoveMethod]
			[KeptAttributeAttribute (typeof (AttributeDefinedInReference))]
			[AttributeDefinedInReference (FieldType = typeof (TypeDefinedInReference))]
			public event EventHandler MyEvent;
		}
	}
}