using System;
using System.Diagnostics.CodeAnalysis;
using System.Linq.Expressions;
using Mono.Linker.Tests.Cases.Expectations.Assertions;
using Mono.Linker.Tests.Cases.Expectations.Helpers;
using Mono.Linker.Tests.Cases.Expectations.Metadata;

namespace Mono.Linker.Tests.Cases.Warnings.WarningSuppression
{
#if !NETCOREAPP
	[Reference ("System.Core.dll")]
#endif
	[SkipKeptItemsValidation]
	[LogDoesNotContain ("TriggerUnrecognizedPattern()")]
	public class SuppressWarningsInMembersAndTypes
	{
		public static void Main ()
		{
			var suppressWarningsInType = new SuppressWarningsInType ();
			suppressWarningsInType.Warning1 ();
			suppressWarningsInType.Warning2 ();
			var nestedType = new SuppressWarningsInType.NestedType ();
			nestedType.Warning3 ();

			var suppressWarningsInMembers = new SuppressWarningsInMembers ();
			suppressWarningsInMembers.Method ();
			suppressWarningsInMembers.SuppressionHasNullParameters ();
			int propertyThatTriggersWarning = suppressWarningsInMembers.Property;

			NestedType.Warning ();

			SuppressOnTypeMarkedEntirely.Test ();
		}

		public static Type TriggerUnrecognizedPattern ()
		{
			return typeof (SuppressWarningsInMembersAndTypes);
		}

		[UnconditionalSuppressMessage ("Test", "IL2072:Suppress UnrecognizedReflectionPattern warnings on a nested type")]
		public class NestedType
		{
			public static void Warning ()
			{
				Expression.Call (TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
			}
		}
	}

	[UnconditionalSuppressMessage ("Test", "IL2072:UnrecognizedReflectionPattern")]
	public class SuppressWarningsInType
	{
		public void Warning1 ()
		{
			Expression.Call (SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
		}

		public void Warning2 ()
		{
			Expression.Call (SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
		}

		public class NestedType
		{
			public void Warning3 ()
			{
				void Warning4 ()
				{
					Expression.Call (SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
				}

				SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern ();
				Warning4 ();
			}
		}
	}

	public class SuppressWarningsInMembers
	{
		[UnconditionalSuppressMessage ("Test", "IL2072:UnrecognizedReflectionPattern")]
		public void Method ()
		{
			Expression.Call (SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
		}

		[UnconditionalSuppressMessage ("Test", "IL2072:Suppression with scope value equal to null",
			Scope = null,
			Target = null,
			MessageId = null)]
		public void SuppressionHasNullParameters ()
		{
			Expression.Call (SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
		}

		public int Property {
			[UnconditionalSuppressMessage ("Test", "IL2072:UnrecognizedReflectionPattern")]
			get {
				Expression.Call (SuppressWarningsInMembersAndTypes.TriggerUnrecognizedPattern (), "", Type.EmptyTypes);
				return 0;
			}
		}
	}

	class SuppressOnTypeMarkedEntirely
	{
		[LogDoesNotContain (nameof (TypeWithSuppression) + " has more than one unconditional suppression")]
		[UnconditionalSuppressMessage ("Test", "IL2026")]
		class TypeWithSuppression
		{
			[ExpectedNoWarnings]
			public TypeWithSuppression ()
			{
				MethodWithRUC ();

				// Triggering the suppression check a second time
				// still shouldn't warn about duplicate suppressions.
				MethodWithRUC ();
			}

			int _field;
		}

		public static void Test ()
		{
			typeof (TypeWithSuppression).RequiresAll ();
		}

		[RequiresUnreferencedCode ("")]
		static void MethodWithRUC () { }
	}
}