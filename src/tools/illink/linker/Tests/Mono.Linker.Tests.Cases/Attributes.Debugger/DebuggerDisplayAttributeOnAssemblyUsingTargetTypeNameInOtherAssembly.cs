﻿using System.Diagnostics;
using Mono.Linker.Tests.Cases.Attributes.Debugger.Dependencies;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

[assembly: KeptAttributeAttribute (typeof (DebuggerDisplayAttribute))]
[assembly: DebuggerDisplay ("{Property}", TargetTypeName = "Mono.Linker.Tests.Cases.Attributes.Debugger.Dependencies.DebuggerDisplayAttributeOnAssemblyUsingTargetTypeNameInOtherAssembly, library")]

namespace Mono.Linker.Tests.Cases.Attributes.Debugger {
	[IgnoreTestCase ("Need to fix.  Currently we only check the assembly the attribute is in")]
	[SetupLinkerCoreAction ("link")]
	[SetupLinkerKeepDebugMembers ("false")]
	[SetupCompileBefore ("library.dll", new [] { "Dependencies/DebuggerDisplayAttributeOnAssemblyUsingTargetTypeNameInOtherAssembly_Lib.cs" })]
	
	// Can be removed once this bug is fixed https://bugzilla.xamarin.com/show_bug.cgi?id=58168
	[SkipPeVerify (SkipPeVerifyForToolchian.Pedump)]
	
	[KeptMemberInAssembly ("mscorlib.dll", typeof (DebuggerDisplayAttribute), ".ctor(System.String)")]
	[KeptMemberInAssembly ("mscorlib.dll", typeof (DebuggerDisplayAttribute), "set_Target(System.Type)")]
	
	[RemovedMemberInAssembly ("library.dll", typeof (DebuggerDisplayAttributeOnAssemblyUsingTargetTypeNameInOtherAssembly_Lib), "get_Property()")]
	[KeptMemberInAssembly ("library.dll", typeof (DebuggerDisplayAttributeOnAssemblyUsingTargetTypeNameInOtherAssembly_Lib), "set_Property(System.Int32)")]
	public class DebuggerDisplayAttributeOnAssemblyUsingTargetTypeNameInOtherAssembly {
		public static void Main ()
		{
			var foo = new DebuggerDisplayAttributeOnAssemblyUsingTargetTypeNameInOtherAssembly_Lib ();
			foo.Property = 1;
		}
	}
}