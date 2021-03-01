﻿using System.Diagnostics;
using System.Security;
using System.Security.Permissions;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.CoreLink
{
#if NETCOREAPP
	[IgnoreTestCase ("Not important for .NET Core build")]
#endif
	[SetupLinkerTrimMode ("link")]
	[SetupLinkerArgument ("--strip-security", "true")]
	[SetupLinkerArgument ("--used-attrs-only", "true")]
	[Reference ("System.dll")]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (SecurityPermissionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (PermissionSetAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (ReflectionPermissionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (RegistryPermissionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (StrongNameIdentityPermissionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (CodeAccessSecurityAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (EnvironmentPermissionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (FileIOPermissionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (HostProtectionAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (SecurityCriticalAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (SecuritySafeCriticalAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (SuppressUnmanagedCodeSecurityAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (SecurityRulesAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (AllowPartiallyTrustedCallersAttribute))]
	[RemovedTypeInAssembly (PlatformAssemblies.CoreLib, typeof (UnverifiableCodeAttribute))]
	// Fails with `Runtime critical type System.Reflection.CustomAttributeData not found` which is a known short coming
	[SkipPeVerify (SkipPeVerifyForToolchian.Pedump)]
	[SkipPeVerify ("System.dll")]
	// System.Core.dll is referenced by System.dll in the .NET FW class libraries. Our GetType reflection marking code
	// detects a GetType("SHA256CryptoServiceProvider") in System.dll, which then causes a type in System.Core.dll to be marked.
	// PeVerify fails on the original GAC copy of System.Core.dll so it's expected that it will also fail on the stripped version we output
	[SkipPeVerify ("System.Core.dll")]
	public class NoSecurityPlusOnlyKeepUsedRemovesAllSecurityAttributesFromCoreLibraries
	{
		public static void Main ()
		{
			// Use something that has security attributes to make this test more meaningful
			var process = new Process ();
		}
	}
}