﻿using System;
using System.Collections.Generic;

namespace Mono.Linker.Tests.TestCasesRunner
{
	public class TestCaseLinkerOptions
	{
		public string CoreAssembliesAction;
		public string UserAssembliesAction;
		public List<KeyValuePair<string, string>> AssembliesAction = new List<KeyValuePair<string, string>> ();

		public string Il8n;
		public bool IgnoreDescriptors;
		public bool IgnoreSubstitutions;
		public bool IgnoreLinkAttributes;
		public string KeepTypeForwarderOnlyAssemblies;
		public string KeepDebugMembers;
		public string LinkSymbols;
		public bool SkipUnresolved;
		public bool StripDescriptors;
		public bool StripSubstitutions;
		public bool StripLinkAttributes;

		public List<KeyValuePair<string, string[]>> AdditionalArguments = new List<KeyValuePair<string, string[]>> ();

		public List<string> Descriptors = new List<string> ();

		public List<string> Substitutions = new List<string> ();

		public List<string> LinkAttributes = new List<string> ();
	}
}