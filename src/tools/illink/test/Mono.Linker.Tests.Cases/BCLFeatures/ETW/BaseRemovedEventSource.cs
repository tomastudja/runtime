﻿using System;
using System.Diagnostics.Tracing;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.BCLFeatures.ETW {
#if NETCOREAPP
	[IgnoreTestCase ("--exclude-feature is not supported on .NET Core")]
#endif
	[SetupLinkerArgument ("--exclude-feature", "etw")]
	// Keep framework code that calls EventSource methods like OnEventCommand
	[SetupLinkerCoreAction ("skip")]
	public class BaseRemovedEventSource {
		public static void Main ()
		{
			var b = CustomCtorEventSource.Log.IsEnabled ();
			if (b)
				CustomCtorEventSource.Log.SomeMethod ();
		}
	}

	[Kept]
	[KeptBaseType (typeof (EventSource))]
	[KeptMember (".ctor()")]
	[KeptMember (".cctor()")]
	[EventSource (Name = "MyCompany")]
	class CustomCtorEventSource : EventSource {
		public class Keywords {
			public const EventKeywords Page = (EventKeywords)1;

			public int Unused;
		}

		[Kept]
		public static CustomCtorEventSource Log = new MyEventSourceBasedOnCustomCtorEventSource (1);

		[Kept]
		[ExpectedInstructionSequence (new []
		{
			"ldarg.0",
			"call",
			"ret",
		})]
		public CustomCtorEventSource (int value)
		{
			Removed ();
		}

		[Kept]
		[ExpectedInstructionSequence (new []
		{
			"ldstr",
			"newobj",
			"throw",
		})]
		protected override void OnEventCommand (EventCommandEventArgs command)
		{
			Removed2 ();
		}

		static void Removed2 ()
		{
		}

		[Kept]
		[ExpectedInstructionSequence (new []
		{
			"ldstr",
			"newobj",
			"throw",
		})]
		[Event (8)]
		public void SomeMethod ()
		{
			Removed ();
		}

		public void Removed ()
		{
		}
	}

	[Kept]
	[KeptBaseType (typeof (CustomCtorEventSource))]
	class MyEventSourceBasedOnCustomCtorEventSource : CustomCtorEventSource {
		[Kept]
		public MyEventSourceBasedOnCustomCtorEventSource (int value) : base (value)
		{
		}
	}
}
