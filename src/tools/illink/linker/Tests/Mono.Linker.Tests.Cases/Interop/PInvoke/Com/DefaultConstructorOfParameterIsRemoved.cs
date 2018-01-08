﻿using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Mono.Linker.Tests.Cases.Expectations.Assertions;

namespace Mono.Linker.Tests.Cases.Interop.PInvoke.Com {
	class DefaultConstructorOfParameterIsRemoved {
		public static void Main ()
		{
			SomeMethod (null);
		}

		[Kept]
		[KeptAttributeAttribute (typeof (GuidAttribute))]
		[ComImport]
		[Guid ("D7BB1889-3AB7-4681-A115-60CA9158FECA")]
		class A {
		}

		[Kept]
		[MethodImpl (MethodImplOptions.InternalCall)]
		static extern void SomeMethod (A val);
	}
}
