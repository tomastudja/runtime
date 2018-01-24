﻿using System;
using Mono.Linker.Tests.Extensions;

namespace Mono.Linker.Tests.TestCasesRunner {
	public class SetupCompileInfo {
		public string OutputName;
		public NPath[] SourceFiles;
		public string[] Defines;
		public string[] References;
		public string AdditionalArguments;
		public bool AddAsReference;
	}
}
