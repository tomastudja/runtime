using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Reflection;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace ILLink.Tasks
{
	public class ILLink : ToolTask
	{
		/// <summary>
		///   Paths to the assembly files that should be considered as
		///   input to the linker.
		///   Optional metadata:
		///       TrimMode ("copy", "link", etc...): sets the illink action to take for this assembly.
		///   There is an optional metadata for each optimization that can be set to "True" or "False" to
		///   enable or disable it per-assembly:
		///       BeforeFieldInit
		///       OverrideRemoval
		///       UnreachableBodies
		///       UnusedInterfaces
		///       IPConstProp
		///       Sealer
		///   Maps to '-reference', and possibly '-p', '--enable-opt', '--disable-opt'
		/// </summary>
		[Required]
		public ITaskItem[] AssemblyPaths { get; set; }

		/// <summary>
		///    Paths to assembly files that are reference assemblies,
		///    representing the surface area for compilation.
		///    Maps to '-reference', with action set to 'skip' via '-p'.
		/// </summary>
		public ITaskItem[] ReferenceAssemblyPaths { get; set; }

		/// <summary>
		///   The names of the assemblies to root. This should contain
		///   assembly names without an extension, not file names or
		///   paths. Exactly which parts of the assemblies get rooted
		///   is subject to change. Currently these get passed to
		///   illink with "-a", which roots the entry point for
		///   executables, and everything for libraries. To control
		///   the linker more explicitly, either pass descriptor
		///   files, or pass extra arguments for illink.
		/// </summary>
		[Required]
		public ITaskItem[] RootAssemblyNames { get; set; }

		/// <summary>
		///   The directory in which to place linked assemblies.
		///    Maps to '-out'.
		/// </summary>
		[Required]
		public ITaskItem OutputDirectory { get; set; }

		/// <summary>
		/// The subset of warnings that have to be turned off. 
		/// Maps to '--nowarn'.
		/// </summary>
		public string NoWarn { get; set; }

		/// <summary>
		/// The warning version to use.
		/// Maps to '--warn'.
		/// </summary>
		public string Warn { get; set; }

		/// <summary>
		/// Treat all warnings as errors.
		/// Maps to '--warnaserror' if true, '--warnaserror-' if false.
		/// </summary>
		public bool TreatWarningsAsErrors { set => _treatWarningsAsErrors = value; }
		bool? _treatWarningsAsErrors;

		/// <summary>
		/// The list of warnings to report as errors.
		/// Maps to '--warnaserror LIST-OF-WARNINGS'.
		/// </summary>
		public string WarningsAsErrors { get; set; }

		/// <summary>
		/// The list of warnings to report as usual.
		/// Maps to '--warnaserror- LIST-OF-WARNINGS'.
		/// </summary>
		public string WarningsNotAsErrors { get; set; }

		/// <summary>
		///   A list of XML root descriptor files specifying linker
		///   roots at a granular level. See the mono/linker
		///   documentation for details about the format.
		///   Maps to '-x'.
		/// </summary>
		public ITaskItem[] RootDescriptorFiles { get; set; }

		/// <summary>
		///   Boolean specifying whether to enable beforefieldinit optimization globally.
		///   Maps to '--enable-opt beforefieldinit' or '--disable-opt beforefieldinit'.
		/// </summary>
		public bool BeforeFieldInit { set => _beforeFieldInit = value; }
		bool? _beforeFieldInit;

		/// <summary>
		///   Boolean specifying whether to enable overrideremoval optimization globally.
		///   Maps to '--enable-opt overrideremoval' or '--disable-opt overrideremoval'.
		/// </summary>
		public bool OverrideRemoval { set => _overrideRemoval = value; }
		bool? _overrideRemoval;

		/// <summary>
		///   Boolean specifying whether to enable unreachablebodies optimization globally.
		///   Maps to '--enable-opt unreachablebodies' or '--disable-opt unreachablebodies'.
		/// </summary>
		public bool UnreachableBodies { set => _unreachableBodies = value; }
		bool? _unreachableBodies;

		/// <summary>
		///   Boolean specifying whether to enable unusedinterfaces optimization globally.
		///   Maps to '--enable-opt unusedinterfaces' or '--disable-opt unusedinterfaces'.
		/// </summary>
		public bool UnusedInterfaces { set => _unusedInterfaces = value; }
		bool? _unusedInterfaces;

		/// <summary>
		///   Boolean specifying whether to enable ipconstprop optimization globally.
		///   Maps to '--enable-opt ipconstprop' or '--disable-opt ipconstprop'.
		/// </summary>
		public bool IPConstProp { set => _iPConstProp = value; }
		bool? _iPConstProp;

		/// <summary>
		///   A list of feature names used by the body substitution logic.
		///   Each Item requires "Value" boolean metadata with the value of
		///   the feature setting.
		///   Maps to '--feature'.
		/// </summary>
		public ITaskItem[] FeatureSettings { get; set; }

		/// <summary>
		///   Boolean specifying whether to enable sealer optimization globally.
		///   Maps to '--enable-opt sealer' or '--disable-opt sealer'.
		/// </summary>
		public bool Sealer { set => _sealer = value; }
		bool? _sealer;

		static readonly string[] _optimizationNames = new string[] {
			"BeforeFieldInit",
			"OverrideRemoval",
			"UnreachableBodies",
			"UnusedInterfaces",
			"IPConstProp",
			"Sealer"
		};

		/// <summary>
		///   Custom data key-value pairs to pass to the linker.
		///   The name of the item is the key, and the required "Value"
		///   metadata is the value. Maps to '--custom-data key=value'.
		/// </summary>
		public ITaskItem[] CustomData { get; set; }

		/// <summary>
		///   Extra arguments to pass to illink, delimited by spaces.
		/// </summary>
		public string ExtraArgs { get; set; }

		/// <summary>
		///   Make illink dump dependencies file for linker analyzer tool.
		///   Maps to '--dump-dependencies'.
		/// </summary>
		public bool DumpDependencies { get; set; }

		/// <summary>
		///   Remove debug symbols from linked assemblies.
		///   Maps to '-b' if false.
		///   Default if not specified is to remove symbols, like
		///   the command-line. (Target files will likely set their own defaults to keep symbols.)
		/// </summary>
		public bool RemoveSymbols { set => _removeSymbols = value; }
		bool? _removeSymbols;

		/// <summary>
		///   Sets the default action for assemblies.
		///   Maps to '-c' and '-u'.
		/// </summary>
		public string TrimMode { get; set; }

		/// <summary>
		///   A list of custom steps to insert into the linker pipeline.
		///   Each ItemSpec should be the path to the assembly containing the custom step.
		///   Each Item requires "Type" metadata with the name of the custom step type.
		///   Optional metadata:
		///   BeforeStep: The name of a linker step. The custom step will be inserted before it.
		///   AfterStep: The name of a linker step. The custom step will be inserted after it.
		///   The default (if neither BeforeStep or AfterStep is specified) is to insert the
		///   custom step at the end of the pipeline.
		///   It is an error to specify both BeforeStep and AfterStep.
		///   Maps to '--custom-step'.
		/// </summary>
		public ITaskItem[] CustomSteps { get; set; }

		private readonly static string DotNetHostPathEnvironmentName = "DOTNET_HOST_PATH";

		private string _dotnetPath;

		private string DotNetPath {
			get {
				if (!String.IsNullOrEmpty (_dotnetPath))
					return _dotnetPath;

				_dotnetPath = Environment.GetEnvironmentVariable (DotNetHostPathEnvironmentName);
				if (String.IsNullOrEmpty (_dotnetPath))
					throw new InvalidOperationException ($"{DotNetHostPathEnvironmentName} is not set");

				return _dotnetPath;
			}
		}


		/// ToolTask implementation

		protected override MessageImportance StandardErrorLoggingImportance => MessageImportance.High;

		protected override string ToolName => Path.GetFileName (DotNetPath);

		protected override string GenerateFullPathToTool () => DotNetPath;

		private string _illinkPath = "";

		public string ILLinkPath {
			get {
				if (!String.IsNullOrEmpty (_illinkPath))
					return _illinkPath;

				var taskDirectory = Path.GetDirectoryName (Assembly.GetExecutingAssembly ().Location);
				// The linker always runs on .NET Core, even when using desktop MSBuild to host ILLink.Tasks.
				_illinkPath = Path.Combine (Path.GetDirectoryName (taskDirectory), "netcoreapp3.0", "illink.dll");
				return _illinkPath;
			}
			set => _illinkPath = value;
		}

		private static string Quote (string path)
		{
			return $"\"{path.TrimEnd ('\\')}\"";
		}

		protected override string GenerateCommandLineCommands ()
		{
			var args = new StringBuilder ();
			args.Append (Quote (ILLinkPath));
			return args.ToString ();
		}

		private void SetOpt (StringBuilder args, string opt, bool enabled)
		{
			args.Append (enabled ? "--enable-opt " : "--disable-opt ").AppendLine (opt);
		}

		private void SetOpt (StringBuilder args, string opt, string assembly, bool enabled)
		{
			args.Append (enabled ? "--enable-opt " : "--disable-opt ").Append (opt).Append (" ").AppendLine (assembly);
		}

		protected override string GenerateResponseFileCommands ()
		{
			var args = new StringBuilder ();

			if (RootDescriptorFiles != null) {
				foreach (var rootFile in RootDescriptorFiles)
					args.Append ("-x ").AppendLine (Quote (rootFile.ItemSpec));
			}

			foreach (var assemblyItem in RootAssemblyNames)
				args.Append ("-a ").AppendLine (Quote (assemblyItem.ItemSpec));

			HashSet<string> assemblyNames = new HashSet<string> (StringComparer.OrdinalIgnoreCase);
			foreach (var assembly in AssemblyPaths) {
				var assemblyPath = assembly.ItemSpec;
				var assemblyName = Path.GetFileNameWithoutExtension (assemblyPath);

				// If there are multiple paths with the same assembly name, only use the first one.
				if (!assemblyNames.Add (assemblyName))
					continue;

				args.Append ("-reference ").AppendLine (Quote (assemblyPath));

				string trimMode = assembly.GetMetadata ("TrimMode");
				if (!String.IsNullOrEmpty (trimMode)) {
					args.Append ("-p ");
					args.Append (trimMode);
					args.Append (" ").AppendLine (Quote (assemblyName));
				}

				// Add per-assembly optimization arguments
				foreach (var optimization in _optimizationNames) {
					string optimizationValue = assembly.GetMetadata (optimization);
					if (String.IsNullOrEmpty (optimizationValue))
						continue;

					if (!Boolean.TryParse (optimizationValue, out bool enabled))
						throw new ArgumentException ($"optimization metadata {optimization} must be True or False");

					SetOpt (args, optimization, assemblyName, enabled);
				}
			}

			if (ReferenceAssemblyPaths != null) {
				foreach (var assembly in ReferenceAssemblyPaths) {
					var assemblyPath = assembly.ItemSpec;
					var assemblyName = Path.GetFileNameWithoutExtension (assemblyPath);

					// Don't process references for which we already have
					// implementation assemblies.
					if (assemblyNames.Contains (assemblyName))
						continue;

					args.Append ("-reference ").AppendLine (Quote (assemblyPath));

					// Treat reference assemblies as "skip". Ideally we
					// would not even look at the IL, but only use them to
					// resolve surface area.
					args.Append ("-p skip ").AppendLine (Quote (assemblyName));
				}
			}

			if (OutputDirectory != null)
				args.Append ("-out ").AppendLine (Quote (OutputDirectory.ItemSpec));

			if (NoWarn != null)
				args.Append ("--nowarn ").AppendLine (Quote (NoWarn));

			if (Warn != null)
				args.Append ("--warn ").AppendLine (Quote (Warn));

			if (_treatWarningsAsErrors is bool treatWarningsAsErrors && treatWarningsAsErrors)
				args.Append ("--warnaserror ");
			else
				args.Append ("--warnaserror- ");

			if (WarningsAsErrors != null)
				args.Append ("--warnaserror ").AppendLine (Quote (WarningsAsErrors));

			if (WarningsNotAsErrors != null)
				args.Append ("--warnaserror- ").AppendLine (Quote (WarningsNotAsErrors));

			// Add global optimization arguments
			if (_beforeFieldInit is bool beforeFieldInit)
				SetOpt (args, "beforefieldinit", beforeFieldInit);

			if (_overrideRemoval is bool overrideRemoval)
				SetOpt (args, "overrideremoval", overrideRemoval);

			if (_unreachableBodies is bool unreachableBodies)
				SetOpt (args, "unreachablebodies", unreachableBodies);

			if (_unusedInterfaces is bool unusedInterfaces)
				SetOpt (args, "unusedinterfaces", unusedInterfaces);

			if (_iPConstProp is bool iPConstProp)
				SetOpt (args, "ipconstprop", iPConstProp);

			if (_sealer is bool sealer)
				SetOpt (args, "sealer", sealer);

			if (CustomData != null) {
				foreach (var customData in CustomData) {
					var key = customData.ItemSpec;
					var value = customData.GetMetadata ("Value");
					if (String.IsNullOrEmpty (value))
						throw new ArgumentException ("custom data requires \"Value\" metadata");
					args.Append ("--custom-data ").Append (" ").Append (key).Append ("=").AppendLine (Quote (value));
				}
			}

			if (FeatureSettings != null) {
				foreach (var featureSetting in FeatureSettings) {
					var feature = featureSetting.ItemSpec;
					var featureValue = featureSetting.GetMetadata ("Value");
					if (String.IsNullOrEmpty (featureValue))
						throw new ArgumentException ("feature settings require \"Value\" metadata");
					args.Append ("--feature ").Append (feature).Append (" ").AppendLine (featureValue);
				}
			}

			if (_removeSymbols == false)
				args.AppendLine ("-b");

			if (TrimMode != null)
				args.Append ("-c ").Append (TrimMode).Append (" -u ").AppendLine (TrimMode);

			if (CustomSteps != null) {
				foreach (var customStep in CustomSteps) {
					args.Append ("--custom-step ");
					var stepPath = customStep.ItemSpec;
					var stepType = customStep.GetMetadata ("Type");
					if (stepType == null)
						throw new ArgumentException ("custom step requires \"Type\" metadata");
					var customStepString = $"{stepType},{stepPath}";

					// handle optional before/aftersteps
					var beforeStep = customStep.GetMetadata ("BeforeStep");
					var afterStep = customStep.GetMetadata ("AfterStep");
					if (!String.IsNullOrEmpty (beforeStep) && !String.IsNullOrEmpty (afterStep))
						throw new ArgumentException ("custom step may not have both \"BeforeStep\" and \"AfterStep\" metadata");
					if (!String.IsNullOrEmpty (beforeStep))
						customStepString = $"-{beforeStep}:{customStepString}";
					if (!String.IsNullOrEmpty (afterStep))
						customStepString = $"+{afterStep}:{customStepString}";

					args.AppendLine (Quote (customStepString));
				}
			}

			if (ExtraArgs != null)
				args.AppendLine (ExtraArgs);

			if (DumpDependencies)
				args.AppendLine ("--dump-dependencies");

			return args.ToString ();
		}
	}
}
