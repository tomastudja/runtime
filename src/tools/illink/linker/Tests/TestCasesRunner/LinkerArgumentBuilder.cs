﻿using System.Collections.Generic;
using Mono.Linker.Tests.Extensions;

namespace Mono.Linker.Tests.TestCasesRunner {
	public class LinkerArgumentBuilder {
		private readonly List<string> _arguments = new List<string> ();

		public virtual void AddSearchDirectory (NPath directory)
		{
			Append ("-d");
			Append (directory.ToString ());
		}

		public virtual void AddOutputDirectory (NPath directory)
		{
			Append ("-o");
			Append (directory.ToString ());
		}

		public virtual void AddLinkXmlFile (NPath path)
		{
			Append ("-x");
			Append (path.ToString ());
		}

		public virtual void AddCoreLink (string value)
		{
			Append ("-c");
			Append (value);
		}

		public virtual void LinkFromAssembly (string fileName)
		{
			Append ("-a");
			Append (fileName);
		}

		public virtual void IncludeBlacklist (bool value)
		{
			Append ("-z");
			Append (value ? "true" : "false");
		}

		public virtual void AddIl8n (string value)
		{
			Append ("-l");
			Append (value);
		}

		public virtual void AddKeepTypeForwarderOnlyAssemblies (string value)
		{
			if (bool.Parse (value))
				Append ("-t");
		}

		public virtual void AddAssemblyAction (string action, string assembly)
		{
			Append ("-p");
			Append (action);
			Append (assembly);
		}

		public string [] ToArgs ()
		{
			return _arguments.ToArray ();
		}

		protected void Append (string arg)
		{
			_arguments.Add (arg);
		}

		public virtual void AddAdditionalArgument (string flag, string [] values)
		{
			Append (flag);
			if (values != null) {
				foreach (var val in values)
					Append (val);
			}
		}

		public virtual void ProcessOptions (TestCaseLinkerOptions options)
		{
			if (options.CoreAssembliesAction != null)
				AddCoreLink (options.CoreAssembliesAction);

			if (options.AssembliesAction != null) {
				foreach (var entry in options.AssembliesAction)
					AddAssemblyAction (entry.Key, entry.Value);
			}

			// Running the blacklist step causes a ton of stuff to be preserved.  That's good for normal use cases, but for
			// our test cases that pollutes the results
			IncludeBlacklist (options.IncludeBlacklistStep);

			if (!string.IsNullOrEmpty (options.Il8n))
				AddIl8n (options.Il8n);

			if (!string.IsNullOrEmpty (options.KeepTypeForwarderOnlyAssemblies))
				AddKeepTypeForwarderOnlyAssemblies (options.KeepTypeForwarderOnlyAssemblies);

			// Unity uses different argument format and needs to be able to translate to their format.  In order to make that easier
			// we keep the information in flag + values format for as long as we can so that this information doesn't have to be parsed out of a single string
			foreach (var additionalArgument in options.AdditionalArguments)
				AddAdditionalArgument (additionalArgument.Key, additionalArgument.Value);
		}
	}
}