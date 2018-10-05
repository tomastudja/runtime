﻿//
// Driver.cs
//
// Author:
//   Jb Evain (jbevain@gmail.com)
//
// (C) 2006 Jb Evain
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.IO;
using System.Collections.Generic;
using System.Xml.XPath;

using Mono.Linker.Steps;

namespace Mono.Linker {

	public class Driver {

#if FEATURE_ILLINK
		static readonly string _linker = "IL Linker";
#else
		static readonly string _linker = "Mono CIL Linker";
#endif

		public static int Main (string [] args)
		{
			return Execute (args);
		}

		public static int Execute (string[] args, ILogger customLogger = null)
		{
			if (args.Length == 0)
				Usage ("No parameters specified");

			try {

				Driver driver = new Driver (args);
				driver.Run (customLogger);

			} catch (Exception e) {
				Console.WriteLine ("Fatal error in {0}", _linker);
				Console.WriteLine (e);
				return 1;
			}

			return 0;
		}

		Queue<string> _queue;
		bool _needAddBypassNGenStep;

		public Driver (string [] args)
		{
			_queue = new Queue<string> (args);
		}

		bool HaveMoreTokens ()
		{
			return _queue.Count > 0;
		}

		public void Run (ILogger customLogger = null)
		{
			Pipeline p = GetStandardPipeline ();
			using (LinkContext context = GetDefaultContext (p)) {
				if (customLogger != null)
					context.Logger = customLogger;

				I18nAssemblies assemblies = I18nAssemblies.All;
				var custom_steps = new List<string> ();
				var excluded_features = new HashSet<string> (StringComparer.Ordinal);
				var disabled_optimizations = new HashSet<string> (StringComparer.Ordinal);
				bool dumpDependencies = false;
				bool ignoreDescriptors = false;

				bool resolver = false;
				while (HaveMoreTokens ()) {
					string token = GetParam ();
					if (token.Length < 2)
						Usage ("Option is too short");

					if (!(token [0] == '-' || token [1] == '/'))
						Usage ("Expecting an option, got instead: " + token);

					if (token [0] == '-' && token [1] == '-') {

						if (token.Length < 3)
							Usage ("Option is too short");

						switch (token) {
						case "--skip-unresolved":
							bool ignoreUnresolved = bool.Parse (GetParam ());
							context.IgnoreUnresolved = ignoreUnresolved;
							context.Resolver.IgnoreUnresolved = ignoreUnresolved;
							continue;
						
						case "--verbose":
							context.LogMessages = true;
							continue;

						case "--dependencies-file":
							context.Tracer.DependenciesFileName = GetParam ();
							continue;

						case "--dump-dependencies":
							dumpDependencies = true;
							continue;

						case "--reduced-tracing":
							context.EnableReducedTracing = bool.Parse (GetParam ());
							continue;

						case "--used-attrs-only":
							context.KeepUsedAttributeTypesOnly = bool.Parse (GetParam ());
							continue;

						case "--strip-security":
							if (bool.Parse (GetParam ()))
								p.AddStepBefore (typeof (MarkStep), new RemoveSecurityStep ());
							continue;

						case "--strip-resources":
							context.StripResources = bool.Parse (GetParam ());
							continue;

						case "--exclude-feature":
							var name = GetParam ();
							if (!excluded_features.Contains (name))
								excluded_features.Add (name);
							continue;

						case "--custom-step":
							custom_steps.Add (GetParam ());
							continue;

						case "--keep-facades":
							context.KeepTypeForwarderOnlyAssemblies = bool.Parse (GetParam ());
							continue;

						case "--ignore-descriptors":
							ignoreDescriptors = bool.Parse (GetParam ());
							continue;

						case "--disable-opt":
							var opt = GetParam ();
							if (!disabled_optimizations.Contains (opt))
								disabled_optimizations.Add (opt);

							continue;
						}

						switch (token [2]) {
						case 'v':
							Version ();
							break;
						case 'a':
							About ();
							break;
						default:
							Usage (null);
							break;
						}
					}

					switch (token [1]) {
					case 'd':
						DirectoryInfo info = new DirectoryInfo (GetParam ());
						context.Resolver.AddSearchDirectory (info.FullName);
							break;
					case 'o':
						context.OutputDirectory = GetParam ();
						break;
					case 'c':
						context.CoreAction = ParseAssemblyAction (GetParam ());
						break;
					case 'u':
						context.UserAction = ParseAssemblyAction (GetParam ());
						break;
					case 'p':
						AssemblyAction action = ParseAssemblyAction (GetParam ());
						context.Actions [GetParam ()] = action;
						break;
					case 't':
						context.KeepTypeForwarderOnlyAssemblies = true;
						break;
					case 'x':
						foreach (string file in GetFiles (GetParam ()))
							p.PrependStep (new ResolveFromXmlStep (new XPathDocument (file)));
						resolver = true;
						break;
					case 'r':
					case 'a':
						var rootVisibility = (token [1] == 'r')
							? ResolveFromAssemblyStep.RootVisibility.PublicAndFamily
							: ResolveFromAssemblyStep.RootVisibility.Any;
						foreach (string file in GetFiles (GetParam ()))
							p.PrependStep (new ResolveFromAssemblyStep (file, rootVisibility));
						resolver = true;
						break;
					case 'i':
						foreach (string file in GetFiles (GetParam ()))
							p.PrependStep (new ResolveFromXApiStep (new XPathDocument (file)));
						resolver = true;
						break;
					case 'l':
						assemblies = ParseI18n (GetParam ());
						break;
					case 'm':
						context.SetParameter (GetParam (), GetParam ());
						break;
					case 'b':
						context.LinkSymbols = bool.Parse (GetParam ());
						break;
					case 'g':
						if (!bool.Parse (GetParam ()))
							p.RemoveStep (typeof (RegenerateGuidStep));
						break;
					case 'z':
						ignoreDescriptors = !bool.Parse (GetParam ());
						break;
					case 'v':
						context.KeepMembersForDebugger = bool.Parse (GetParam ());
						break;
					default:
						Usage ("Unknown option: `" + token [1] + "'");
						break;
					}
				}

				if (!resolver)
					Usage ("No resolver was created (use -x, -a or -i)");

				if (ignoreDescriptors)
					p.RemoveStep (typeof (BlacklistStep));
					
				if (dumpDependencies)
					context.Tracer.Start ();

				foreach (string custom_step in custom_steps)
					AddCustomStep (p, custom_step);

				p.AddStepAfter (typeof (LoadReferencesStep), new LoadI18nAssemblies (assemblies));

				if (_needAddBypassNGenStep) {
					p.AddStepAfter (typeof (SweepStep), new AddBypassNGenStep ());
				}

				if (assemblies != I18nAssemblies.None) {
					p.AddStepAfter (typeof (PreserveDependencyLookupStep), new PreserveCalendarsStep (assemblies));
				}

				if (excluded_features.Count > 0) {
					var excluded = new string [excluded_features.Count];
					excluded_features.CopyTo (excluded);
					context.ExcludedFeatures = excluded;
				}

				if (disabled_optimizations.Count > 0) {
					foreach (var item in disabled_optimizations) {
						switch (item) {
						case "beforefieldinit":
							context.DisabledOptimizations |= CodeOptimizations.BeforeFieldInit;
							break;
						}
					}
				}

				try {
					p.Process (context);
				}
				finally {
					if (dumpDependencies)
						context.Tracer.Finish ();
				}
			}
		}

		protected static void AddCustomStep (Pipeline pipeline, string arg)
		{
			int pos = arg.IndexOf (":");
			if (pos == -1) {
				pipeline.AppendStep (ResolveStep (arg));
				return;
			}

			string [] parts = arg.Split (':');
			if (parts.Length != 2)
				Usage ("Step is specified as TYPE:STEP");

			if (parts [0].IndexOf (",") > -1)
				pipeline.AddStepBefore (FindStep (pipeline, parts [1]), ResolveStep (parts [0]));
			else if (parts [1].IndexOf (",") > -1)
				pipeline.AddStepAfter (FindStep (pipeline, parts [0]), ResolveStep (parts [1]));
			else
				Usage ("No comma separator in TYPE or STEP");
		}

		static Type FindStep (Pipeline pipeline, string name)
		{
			foreach (IStep step in pipeline.GetSteps ()) {
				Type t = step.GetType ();
				if (t.Name == name)
					return t;
			}

			return null;
		}

		static IStep ResolveStep (string type)
		{
			Type step = Type.GetType (type, false);
			if (step == null)
				Usage (String.Format ("Step type '{0}' not found.", type));
			if (!typeof (IStep).IsAssignableFrom (step))
				Usage (String.Format ("Step type '{0}' does not implement IStep interface.", type));
			return (IStep) Activator.CreateInstance (step);
		}

		static string [] GetFiles (string param)
		{
			if (param.Length < 1 || param [0] != '@')
				return new string [] {param};

			string file = param.Substring (1);
			return ReadLines (file);
		}

		static string [] ReadLines (string file)
		{
			var lines = new List<string> ();
			using (StreamReader reader = new StreamReader (file)) {
				string line;
				while ((line = reader.ReadLine ()) != null)
					lines.Add (line);
			}
			return lines.ToArray ();
		}

		protected static I18nAssemblies ParseI18n (string str)
		{
			I18nAssemblies assemblies = I18nAssemblies.None;
			string [] parts = str.Split (',');
			foreach (string part in parts)
				assemblies |= (I18nAssemblies) Enum.Parse (typeof (I18nAssemblies), part.Trim (), true);

			return assemblies;
		}


		AssemblyAction ParseAssemblyAction (string s)
		{
			var assemblyAction = (AssemblyAction)Enum.Parse(typeof(AssemblyAction), s, true);
			if ((assemblyAction == AssemblyAction.AddBypassNGen) || (assemblyAction == AssemblyAction.AddBypassNGenUsed)) {
				_needAddBypassNGenStep = true;
			}
			return assemblyAction;
		}

		string GetParam ()
		{
			if (_queue.Count == 0)
				Usage ("Expecting a parameter");

			return _queue.Dequeue ();
		}

		static LinkContext GetDefaultContext (Pipeline pipeline)
		{
			LinkContext context = new LinkContext (pipeline);
			context.CoreAction = AssemblyAction.Skip;
			context.UserAction = AssemblyAction.Link;
			context.OutputDirectory = "output";
			context.StripResources = true;
			return context;
		}

		static void Usage (string msg)
		{
			Console.WriteLine (_linker);
			if (msg != null)
				Console.WriteLine ("Error: " + msg);
#if FEATURE_ILLINK
			Console.WriteLine ("illink [options] -a|-i|-r|-x file");
#else
			Console.WriteLine ("monolinker [options] -a|-i|-r|-x file");
#endif

			Console.WriteLine ("  -a                  Link from a list of assemblies");
			Console.WriteLine ("  -i                  Link from an mono-api-info descriptor");
			Console.WriteLine ("  -r                  Link from a list of assemblies using roots visible outside of the assembly");
			Console.WriteLine ("  -x                  Link from XML descriptor");
			Console.WriteLine ("  -d <path>           Specify additional directories to search in for references");
			Console.WriteLine ("  -b                  Update debug symbols for each linked module. Defaults to false");
			Console.WriteLine ("  -v                  Keep members and types used by debugger. Defaults to false");
			Console.WriteLine ("  -l <name>,<name>    List of i18n assemblies to copy to the output directory. Defaults to 'all'");
			Console.WriteLine ("                        Valid names are 'none', 'all', 'cjk', 'mideast', 'other', 'rare', 'west'");
			Console.WriteLine ("  --about             About the {0}", _linker);
			Console.WriteLine ("  --verbose           Log messages indicating progress and warnings");
			Console.WriteLine ("  --version           Print the version number of the {0}", _linker);
			Console.WriteLine ("  --skip-unresolved   Ignore unresolved types, methods, and assemblies. Defaults to false");
			Console.WriteLine ("  -out <path>         Specify the output directory. Defaults to 'output'");

			Console.WriteLine ();
			Console.WriteLine ("Actions");
			Console.WriteLine ("  -c <action>         Action on the core assemblies. Defaults to 'skip'");
			Console.WriteLine ("                        copy: Copy the files into the output directory");
			Console.WriteLine ("                        copyused: Copy the files only when anything from the assembly is used");
			Console.WriteLine ("                        link: Link the assembly");
			Console.WriteLine ("                        skip: Do not process the assembly");
			Console.WriteLine ("                        addbypassngen: Add BypassNGenAttribute to unused methods");
			Console.WriteLine ("                        addbypassngenused: Same as addbypassngen but unused assemblies are removed");
			Console.WriteLine ("  -u <action>         Action on the user assemblies. Defaults to 'link'");
			Console.WriteLine ("  -p <action> <name>  Overrides the default action for an assembly");

			Console.WriteLine ();
			Console.WriteLine ("Advanced");
			Console.WriteLine ("  --custom-step <name>      Add a custom step to the pipeline");
			Console.WriteLine ("  --disable-opt <name>      Disable one of the default optimizations");
			Console.WriteLine ("                              beforefieldinit: Unused static fields are removed if there is no static ctor");
			Console.WriteLine ("  --exclude-feature <name>  Any code which has a feature <name> in linked assemblies will be removed");
			Console.WriteLine ("                              com: Support for COM Interop");
			Console.WriteLine ("                              remoting: .NET Remoting dependencies");
			Console.WriteLine ("                              sre: System.Reflection.Emit namespace");
			Console.WriteLine ("  --ignore-descriptors      Skips reading embedded descriptors (short -z). Defaults to false");
			Console.WriteLine ("  --keep-facades            Keep assemblies with type-forwarders (short -t). Defaults to false");
			Console.WriteLine ("  --new-mvid                Generate a new guid for each linked assembly (short -g). Defaults to true");
			Console.WriteLine ("  --strip-resources         Remove XML descriptor resources for linked assemblies. Defaults to true");
			Console.WriteLine ("  --strip-security          Remove metadata and code related to Code Access Security. Defaults to true");
			Console.WriteLine ("  --used-attrs-only         Any attribute is removed if the attribute type is not used. Defaults to false");

			Console.WriteLine ();
			Console.WriteLine ("Analyzer");
			Console.WriteLine ("  --dependencies-file <path> Specify the dependencies output. Defaults to 'output/linker-dependencies.xml.gz'");
			Console.WriteLine ("  --dump-dependencies        Dump dependencies for the linker analyzer tool");
			Console.WriteLine ("  --reduced-tracing          Reduces dependency output related to assemblies that will not be modified");
			Console.WriteLine ("");

			Environment.Exit (1);
		}

		static void Version ()
		{
			Console.WriteLine ("{0} Version {1}",
				_linker,
				System.Reflection.Assembly.GetExecutingAssembly ().GetName ().Version);

			Environment.Exit(1);
		}

		static void About ()
		{
			Console.WriteLine ("For more information, visit the project Web site");
			Console.WriteLine ("   http://www.mono-project.com/");

			Environment.Exit(1);
		}

		static Pipeline GetStandardPipeline ()
		{
			Pipeline p = new Pipeline ();
			p.AppendStep (new LoadReferencesStep ());
			p.AppendStep (new BlacklistStep ());
			p.AppendStep (new PreserveDependencyLookupStep ());
			p.AppendStep (new TypeMapStep ());
			p.AppendStep (new MarkStep ());
			p.AppendStep (new SweepStep ());
			p.AppendStep (new CleanStep ());
			p.AppendStep (new RegenerateGuidStep ());
			p.AppendStep (new OutputStep ());
			return p;
		}
	}

	[Flags]
	public enum CodeOptimizations
	{
		BeforeFieldInit = 1 << 0,
	}
}
