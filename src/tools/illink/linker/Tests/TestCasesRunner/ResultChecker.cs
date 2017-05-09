﻿﻿using System;
using System.Collections.Generic;
using System.Linq;
using Mono.Cecil;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Extensions;
using NUnit.Framework;

namespace Mono.Linker.Tests.TestCasesRunner {
	public class ResultChecker {

		public virtual void Check (LinkedTestCaseResult linkResult)
		{
			Assert.IsTrue (linkResult.OutputAssemblyPath.FileExists (), $"The linked output assembly was not found.  Expected at {linkResult.OutputAssemblyPath}");

			using (var original = ReadAssembly (linkResult.ExpectationsAssemblyPath)) {
				PerformOutputAssemblyChecks (original.Definition, linkResult.OutputAssemblyPath.Parent);

				using (var linked = ReadAssembly (linkResult.OutputAssemblyPath)) {
					var checker = new AssemblyChecker (original.Definition, linked.Definition);
					checker.Verify (); 
				}

				VerifyLinkingOfOtherAssemblies (original.Definition, linkResult.OutputAssemblyPath.Parent);
			}
		}

		static AssemblyContainer ReadAssembly (NPath assemblyPath)
		{
			var readerParams = new ReaderParameters ();
			var resolver = new AssemblyResolver ();
			readerParams.AssemblyResolver = resolver;
			resolver.AddSearchDirectory (assemblyPath.Parent.ToString ());
			return new AssemblyContainer (AssemblyDefinition.ReadAssembly (assemblyPath.ToString (), readerParams), resolver);
		}

		void PerformOutputAssemblyChecks (AssemblyDefinition original, NPath outputDirectory)
		{
			var assembliesToCheck = original.MainModule.Types.SelectMany (t => t.CustomAttributes).Where (attr => ExpectationsProvider.IsAssemblyAssertion(attr));

			foreach (var assemblyAttr in assembliesToCheck) {
				var name = (string) assemblyAttr.ConstructorArguments.First ().Value;
				var expectedPath = outputDirectory.Combine (name);
				Assert.IsTrue (expectedPath.FileExists (), $"Expected the assembly {name} to exist in {outputDirectory}, but it did not");
			}
		}

		void VerifyLinkingOfOtherAssemblies (AssemblyDefinition original, NPath outputDirectory)
		{
			var checks = BuildOtherAssemblyCheckTable (original);

			foreach (var assemblyName in checks.Keys) {
				using (var linkedAssembly = ReadAssembly (outputDirectory.Combine (assemblyName))) {
					foreach (var checkAttrInAssembly in checks [assemblyName]) {
						var expectedTypeName = checkAttrInAssembly.ConstructorArguments [1].Value.ToString ();
						var linkedType = linkedAssembly.Definition.MainModule.GetType (expectedTypeName);

						if (checkAttrInAssembly.AttributeType.Name == nameof (RemovedTypeInAssemblyAttribute)) {
							if (linkedType != null)
								Assert.Fail ($"Type `{expectedTypeName}' should have been removed");
						} else if (checkAttrInAssembly.AttributeType.Name == nameof (KeptTypeInAssemblyAttribute)) {
							if (linkedType == null)
								Assert.Fail ($"Type `{expectedTypeName}' should have been kept");
						} else if (checkAttrInAssembly.AttributeType.Name == nameof (RemovedMemberInAssemblyAttribute)) {
							if (linkedType == null)
								continue;

							VerifyRemovedMemberInAssembly (checkAttrInAssembly, linkedType);
						} else if (checkAttrInAssembly.AttributeType.Name == nameof (KeptMemberInAssemblyAttribute)) {
							if (linkedType == null)
								Assert.Fail ($"Type `{expectedTypeName}' should have been kept");

							VerifyKeptMemberInAssembly (checkAttrInAssembly, linkedType);
						} else {
							throw new NotImplementedException ($"Type {expectedTypeName}, has an unknown other assembly attribute of type {checkAttrInAssembly.AttributeType}");
						}
					}
				}
			}
		}

		static void VerifyRemovedMemberInAssembly (CustomAttribute inAssemblyAttribute, TypeDefinition linkedType)
		{
			var originalType = ((TypeReference) inAssemblyAttribute.ConstructorArguments [1].Value).Resolve ();
			foreach (var memberNameAttr in (CustomAttributeArgument[]) inAssemblyAttribute.ConstructorArguments [2].Value) {
				string memberName = (string) memberNameAttr.Value;

				// We will find the matching type from the original assembly first that way we can confirm
				// that the name defined in the attribute corresponds to a member that actually existed
				var originalFieldMember = originalType.Fields.FirstOrDefault (m => m.Name == memberName);
				if (originalFieldMember != null) {
					var linkedField = linkedType.Fields.FirstOrDefault (m => m.Name == memberName);
					if (linkedField != null)
						Assert.Fail ($"Field `{memberName}` on Type `{originalType}` should have been removed");

					continue;
				}

				var originalPropertyMember = originalType.Properties.FirstOrDefault (m => m.Name == memberName);
				if (originalPropertyMember != null) {
					var linkedProperty = linkedType.Properties.FirstOrDefault (m => m.Name == memberName);
					if (linkedProperty != null)
						Assert.Fail ($"Property `{memberName}` on Type `{originalType}` should have been removed");

					continue;
				}

				var originalMethodMember = originalType.Methods.FirstOrDefault (m => m.GetSignature () == memberName);
				if (originalMethodMember != null) {
					var linkedMethod = linkedType.Methods.FirstOrDefault (m => m.GetSignature () == memberName);
					if (linkedMethod != null)
						Assert.Fail ($"Method `{memberName}` on Type `{originalType}` should have been removed");

					continue;
				}

				Assert.Fail ($"Invalid test assertion.  No member named `{memberName}` exists on the original type `{originalType}`");
			}
		}

		static void VerifyKeptMemberInAssembly (CustomAttribute inAssemblyAttribute, TypeDefinition linkedType)
		{
			var originalType = ((TypeReference) inAssemblyAttribute.ConstructorArguments [1].Value).Resolve ();
			foreach (var memberNameAttr in (CustomAttributeArgument[]) inAssemblyAttribute.ConstructorArguments [2].Value) {
				string memberName = (string) memberNameAttr.Value;

				// We will find the matching type from the original assembly first that way we can confirm
				// that the name defined in the attribute corresponds to a member that actually existed
				var originalFieldMember = originalType.Fields.FirstOrDefault (m => m.Name == memberName);
				if (originalFieldMember != null) {
					var linkedField = linkedType.Fields.FirstOrDefault (m => m.Name == memberName);
					if (linkedField == null)
						Assert.Fail ($"Field `{memberName}` on Type `{originalType}` should have been kept");

					continue;
				}

				var originalPropertyMember = originalType.Properties.FirstOrDefault (m => m.Name == memberName);
				if (originalPropertyMember != null) {
					var linkedProperty = linkedType.Properties.FirstOrDefault (m => m.Name == memberName);
					if (linkedProperty == null)
						Assert.Fail ($"Property `{memberName}` on Type `{originalType}` should have been kept");

					continue;
				}

				var originalMethodMember = originalType.Methods.FirstOrDefault (m => m.GetSignature () == memberName);
				if (originalMethodMember != null) {
					var linkedMethod = linkedType.Methods.FirstOrDefault (m => m.GetSignature () == memberName);
					if (linkedMethod == null)
						Assert.Fail ($"Method `{memberName}` on Type `{originalType}` should have been kept");

					continue;
				}

				Assert.Fail ($"Invalid test assertion.  No member named `{memberName}` exists on the original type `{originalType}`");
			}
		}

		static Dictionary<string, List<CustomAttribute>> BuildOtherAssemblyCheckTable (AssemblyDefinition original)
		{
			var checks = new Dictionary<string, List<CustomAttribute>> ();

			foreach (var typeWithRemoveInAssembly in original.AllDefinedTypes ()) {
				foreach (var attr in typeWithRemoveInAssembly.CustomAttributes.Where (IsTypeInOtherAssemblyAssertion)) {
					var assemblyName = (string) attr.ConstructorArguments [0].Value;
					List<CustomAttribute> checksForAssembly;
					if (!checks.TryGetValue (assemblyName, out checksForAssembly))
						checks [assemblyName] = checksForAssembly = new List<CustomAttribute> ();

					checksForAssembly.Add (attr);
				}
			}

			return checks;
		}

		static bool IsTypeInOtherAssemblyAssertion (CustomAttribute attr)
		{
			return attr.AttributeType.Name == nameof (RemovedTypeInAssemblyAttribute)
				|| attr.AttributeType.Name == nameof (KeptTypeInAssemblyAttribute)
				|| attr.AttributeType.Name == nameof (RemovedMemberInAssemblyAttribute)
				|| attr.AttributeType.Name == nameof (KeptMemberInAssemblyAttribute);
		}

		struct AssemblyContainer : IDisposable
		{
			public readonly AssemblyResolver Resolver;
			public readonly AssemblyDefinition Definition;

			public AssemblyContainer (AssemblyDefinition definition, AssemblyResolver resolver)
			{
				Definition = definition;
				Resolver = resolver;
			}

			public void Dispose ()
			{
				Resolver?.Dispose ();
				Definition?.Dispose ();
			}
		}
	}
}